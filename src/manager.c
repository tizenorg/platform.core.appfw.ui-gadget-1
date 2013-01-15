/*
 *  UI Gadget
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jayoun Lee <airjany@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <glib.h>
#include <utilX.h>

#include <Elementary.h>
#include <Ecore.h>

#include "ug.h"
#include "ug-manager.h"
#include "ug-engine.h"
#include "ug-dbg.h"

struct ug_manager {
	ui_gadget_h root;
	ui_gadget_h fv_top;
	GSList *fv_list;

	void *win;
	Window win_id;
	Display *disp;
	void *conform;

	enum ug_option base_opt;
	enum ug_event last_rotate_evt;

	int walking;

	int is_initted:1;
	int is_landscape:1;
	int destroy_all:1;

	struct ug_engine *engine;
};

static struct ug_manager ug_man;

static inline void job_start(void);
static inline void job_end(void);

static int ug_relation_add(ui_gadget_h p, ui_gadget_h c)
{
	c->parent = p;
	/* prepend element to avoid the inefficiency,
		which is to traverse the entire list to find the end*/
	p->children = g_slist_prepend(p->children, c);

	return 0;
}

static int ug_relation_del(ui_gadget_h ug)
{
	ui_gadget_h p;

	p = ug->parent;
	if (!p) {
		_ERR("ug_relation_del failed: no parent");
		return -1;
	}
	p->children = g_slist_remove(p->children, ug);
	if (ug->children)
		g_slist_free(ug->children);
	ug->parent = NULL;

	return 0;
}

static int ug_fvlist_add(ui_gadget_h c)
{
	ug_man.fv_list = g_slist_prepend(ug_man.fv_list, c);
	ug_man.fv_top = c;

	return 0;
}

static int ug_fvlist_del(ui_gadget_h c)
{
	ui_gadget_h t;

	ug_man.fv_list = g_slist_remove(ug_man.fv_list, c);

	/* update fullview top ug*/
	t = g_slist_nth_data(ug_man.fv_list, 0);
	ug_man.fv_top = t;

	return 0;
}

static void ugman_tree_dump(ui_gadget_h ug)
{
	static int i;
	int lv;
	const char *name;
	GSList *child;
	ui_gadget_h c;

	if (!ug)
		return;

	name = ug->name;
	if (ug == ug_man.root) {
		i = 0;
		_DBG("============== TREE_DUMP =============");
		_DBG("ROOT: Manager");
		name = "Manager";
	}

	child = ug->children;
	if (!child)
		return;

	i++;
	lv = i;

	while (child) {
		c = child->data;
		_DBG("[%d] %s [%c] (%p) (PARENT:  %s)",
		     lv,
		     c && c->name ? c->name : "NO CHILD INFO FIXIT!!!",
		     c && c->mode == UG_MODE_FULLVIEW ? 'F' : 'f', c, name);
		ugman_tree_dump(c);
		child = g_slist_next(child);
	}
}

static int ugman_ug_find(ui_gadget_h p, ui_gadget_h ug)
{
	GSList *child = NULL;

	if (!p || !ug)
		return 0;
	child = p->children;

	while (child) {
		if (child->data == ug)
			return 1;
		if (ugman_ug_find(child->data, ug))
			return 1;
		child = g_slist_next(child);
	}

	return 0;
}

static int ugman_ug_start(void *data)
{
	ui_gadget_h ug = data;
	struct ug_module_ops *ops = NULL;

	if (!ug || ug->state != UG_STATE_CREATED
	    || ug->state == UG_STATE_RUNNING)
		return 0;

	_DBG("ug=%p", ug);

	ug->state = UG_STATE_RUNNING;

	if (ug->module)
		ops = &ug->module->ops;

	if (ops && ops->start)
		ops->start(ug, ug->service, ops->priv);

	return 0;
}

static int ugman_ug_pause(void *data)
{
	ui_gadget_h ug = data;
	struct ug_module_ops *ops = NULL;
	GSList *child = NULL;

	job_start();

	if (!ug || ug->state != UG_STATE_RUNNING)
		goto end;

	ug->state = UG_STATE_STOPPED;

	if (ug->children) {
		child = ug->children;
		while (child) {
			ugman_ug_pause(child->data);
			child = g_slist_next(child);
		}
	}

	if (ug->module)
		ops = &ug->module->ops;

	if (ops && ops->pause)
		ops->pause(ug, ug->service, ops->priv);

 end:
	job_end();
	return 0;
}

static int ugman_ug_resume(void *data)
{
	ui_gadget_h ug = data;
	struct ug_module_ops *ops = NULL;
	GSList *child = NULL;

	job_start();

	if (!ug)
		goto end;

	switch (ug->state) {
	case UG_STATE_CREATED:
		ugman_ug_start(ug);
		goto end;
	case UG_STATE_STOPPED:
		break;
	default:
		goto end;
	}

	ug->state = UG_STATE_RUNNING;

	if (ug->children) {
		child = ug->children;
		while (child) {
			ugman_ug_resume(child->data);
			child = g_slist_next(child);
		}
	}

	if (ug->module)
		ops = &ug->module->ops;

	if (ops && ops->resume)
		ops->resume(ug, ug->service, ops->priv);

 end:
	job_end();
	return 0;
}

static int ugman_indicator_update(enum ug_option opt, enum ug_event event)
{
	int enable;
	int cur_state;

	if (!ug_man.win) {
		_ERR("indicator update failed: no window");
		return -1;
	}

	if(GET_OPT_OVERLAP_VAL(opt)) {
		_DBG("update overlap indicator / opt(%d)", opt);
		elm_object_signal_emit(ug_man.conform, "elm,state,indicator,overlap", "");
	} else {
		_DBG("update no overlap indicator / opt(%d)", opt);
		elm_object_signal_emit(ug_man.conform, "elm,state,indicator,nooverlap", "");
	}

	switch (GET_OPT_INDICATOR_VAL(opt)) {
	case UG_OPT_INDICATOR_ENABLE:
		if (event == UG_EVENT_NONE)
			enable = 1;
		else {
			cur_state = utilx_get_indicator_state(ug_man.disp, ug_man.win_id);
			enable = cur_state ? 1 : 0;
		}
		break;
	case UG_OPT_INDICATOR_PORTRAIT_ONLY:
		enable = ug_man.is_landscape ? 0 : 1;
		break;
	case UG_OPT_INDICATOR_LANDSCAPE_ONLY:
		enable = ug_man.is_landscape ? 1 : 0;
		break;
	case UG_OPT_INDICATOR_DISABLE:
		enable = 0;
		break;
	case UG_OPT_INDICATOR_MANUAL:
		return 0;
	default:
		_ERR("update failed: Invalid opt(%d)", opt);
		return -1;
	}

	utilx_enable_indicator(ug_man.disp, ug_man.win_id, enable);

	return 0;
}

static int ugman_ug_getopt(ui_gadget_h ug)
{
	if (!ug)
		return -1;
	/* Indicator Option */
	if (ug->mode == UG_MODE_FULLVIEW)
		ugman_indicator_update(ug->opt, UG_EVENT_NONE);

	return 0;
}

static int ugman_ug_event(ui_gadget_h ug, enum ug_event event)
{
	struct ug_module_ops *ops = NULL;
	GSList *child = NULL;

	if (!ug)
		return 0;

	if (ug->children) {
		child = ug->children;
		while (child) {
			ugman_ug_event(child->data, event);
			child = g_slist_next(child);
		}
	}

	if (ug->module)
		ops = &ug->module->ops;

	if (ops && ops->event)
		ops->event(ug, event, ug->service, ops->priv);

	return 0;
}

static int ugman_ug_destroy(void *data)
{
	ui_gadget_h ug = data;
	struct ug_module_ops *ops = NULL;
	GSList *child, *trail;

	job_start();

	if (!ug)
		goto end;

	_DBG("ugman_ug_destroy start ug(%p)", ug);

	switch (ug->state) {
	case UG_STATE_CREATED:
	case UG_STATE_RUNNING:
	case UG_STATE_STOPPED:
	case UG_STATE_DESTROYING:
		break;
	default:
		goto end;
	}

	ug->state = UG_STATE_DESTROYED;

	if (ug->module)
		ops = &ug->module->ops;

	if (ug->children) {
		child = ug->children;
		while (child) {
			trail = g_slist_next(child);
			ugman_ug_destroy(child->data);
			child = trail;
		}
	}

	if (ops && ops->destroy) {
		_DBG("ug module destory cb call");
		ops->destroy(ug, ug->service, ops->priv);
	}

	ug_relation_del(ug);

	if (ug->mode == UG_MODE_FULLVIEW) {
		if (ug_man.fv_top == ug) {
			ug_fvlist_del(ug);
			ugman_ug_getopt(ug_man.fv_top);
		} else {
			ug_fvlist_del(ug);
		}
	}

	ug_free(ug);

	if (ug_man.root == ug)
		ug_man.root = NULL;

	ugman_tree_dump(ug_man.root);
 end:
	job_end();

	return 0;
}

static void ug_hide_end_cb(ui_gadget_h ug)
{
	ecore_idler_add(ugman_ug_destroy, ug);
}

static int ugman_ug_create(void *data)
{
	ui_gadget_h ug = data;
	struct ug_module_ops *ops = NULL;
	struct ug_cbs *cbs;
	struct ug_engine_ops *eng_ops = NULL;

	if (!ug || ug->state != UG_STATE_READY)
		return -1;

	ug->state = UG_STATE_CREATED;

	if (ug->module)
		ops = &ug->module->ops;

	if (ug_man.engine)
		eng_ops = &ug_man.engine->ops;

	if (ops && ops->create) {
		ug->layout = ops->create(ug, ug->mode, ug->service, ops->priv);
		if (!ug->layout) {
			ug_relation_del(ug);
			return -1;
		}
		if (ug->mode == UG_MODE_FULLVIEW) {
			if (eng_ops && eng_ops->create)
				ug_man.conform = eng_ops->create(ug_man.win, ug, ugman_ug_start);
		}
		cbs = &ug->cbs;

		if (cbs && cbs->layout_cb)
			cbs->layout_cb(ug, ug->mode, cbs->priv);

		ugman_ug_getopt(ug);
	}

	ugman_ug_event(ug, ug_man.last_rotate_evt);
	ugman_tree_dump(ug_man.root);

	return 0;
}

int ugman_ug_add(ui_gadget_h parent, ui_gadget_h ug)
{
	if (!ug_man.is_initted) {
		_ERR("ugman_ug_add failed: manager is not initted");
		return -1;
	}

	if (!ug_man.root) {
		if (parent) {
			_ERR("ugman_ug_add failed: parent has to be NULL w/o root");
			errno = EINVAL;
			return -1;
		}

		ug_man.root = ug_root_create();
		if (!ug_man.root)
			return -1;
		ug_man.root->opt = ug_man.base_opt;
		ug_man.root->layout = ug_man.win;
		ug_fvlist_add(ug_man.root);
	}

	if (!parent)
		parent = ug_man.root;

	if (ug_relation_add(parent, ug))
		return -1;

	if (ugman_ug_create(ug) == -1)
		return -1;

	if (ug->mode == UG_MODE_FULLVIEW)
		ug_fvlist_add(ug);

	return 0;
}

ui_gadget_h ugman_ug_load(ui_gadget_h parent,
				const char *name,
				enum ug_mode mode,
				service_h service, struct ug_cbs *cbs)
{
	int r;
	ui_gadget_h ug;

	ug = calloc(1, sizeof(struct ui_gadget_s));
	if (!ug) {
		_ERR("ug_create() failed: Memory allocation failed");
		return NULL;
	}

	ug->module = ug_module_load(name);
	if (!ug->module) {
		_ERR("ug_create() failed: Module loading failed");
		goto load_fail;
	}

	ug->name = strdup(name);

	ug->mode = mode;
	service_clone(&ug->service, service);
	ug->opt = ug->module->ops.opt;
	ug->state = UG_STATE_READY;
	ug->children = NULL;

	if (cbs)
		memcpy(&ug->cbs, cbs, sizeof(struct ug_cbs));

	r = ugman_ug_add(parent, ug);
	if (r) {
		_ERR("ug_create() failed: Tree update failed");
		goto load_fail;
	}

	return ug;

 load_fail:
	ug_free(ug);
	return NULL;
}

int ugman_ug_destroying(ui_gadget_h ug)
{
	struct ug_module_ops *ops = NULL;
	GSList *child, *trail;

	ug->destroy_me = 1;
	ug->state = UG_STATE_DESTROYING;

	if (ug->module)
		ops = &ug->module->ops;

	if (ug->children) {
		child = ug->children;
		while (child) {
			trail = g_slist_next(child);
			ugman_ug_destroying(child->data);
			child = trail;
		}
	}

	if (ops && ops->destroying)
		ops->destroying(ug, ug->service, ops->priv);

	return 0;
}

int ugman_ug_del(ui_gadget_h ug)
{
	struct ug_engine_ops *eng_ops = NULL;

	if (!ug || !ugman_ug_exist(ug) || ug->state == UG_STATE_DESTROYED) {
		_ERR("ugman_ug_del failed: Invalid ug");
		errno = EINVAL;
		return -1;
	}

	_DBG("ugman_ug_del start ug(%p)", ug);

	if (ug->destroy_me) {
		_ERR("ugman_ug_del failed: ug is alreay on destroying");
		return -1;
	}

	if (!ug_man.is_initted) {
		_ERR("ugman_ug_del failed: manager is not initted");
		return -1;
	}

	if (!ug_man.root) {
		_ERR("ugman_ug_del failed: no root");
		return -1;
	}

	ugman_ug_destroying(ug);

	/* pre call for indicator update time issue */
	if (ug_man.fv_top == ug) {
		ui_gadget_h t;
		t = g_slist_nth_data(ug_man.fv_list, 1);
		ugman_ug_getopt(t);
	}

	if (ug_man.engine)
		eng_ops = &ug_man.engine->ops;

	if (eng_ops && eng_ops->destroy)
		if (ug->mode == UG_MODE_FULLVIEW)
			eng_ops->destroy(ug, ug_man.fv_top, ug_hide_end_cb);
		else {
			eng_ops->destroy(ug, NULL, ug_hide_end_cb);
		}
	else
		ecore_idler_add(ugman_ug_destroy, ug);

	return 0;
}

int ugman_ug_del_all(void)
{
	/*  Terminate */
	if (!ug_man.is_initted) {
		_ERR("ugman_ug_del_all failed: manager is not initted");
		return -1;
	}

	if (!ug_man.root) {
		_ERR("ugman_ug_del_all failed: no root");
		return -1;
	}

	if (ug_man.walking > 0)
		ug_man.destroy_all = 1;
	else
		ugman_ug_destroy(ug_man.root);

	return 0;
}

int ugman_init(Display *disp, Window xid, void *win, enum ug_option opt)
{
	ug_man.is_initted = 1;
	ug_man.win = win;
	ug_man.disp = disp;
	ug_man.win_id = xid;
	ug_man.base_opt = opt;
	ug_man.last_rotate_evt = UG_EVENT_ROTATE_PORTRAIT;
	ug_man.engine = ug_engine_load();

	return 0;
}

int ugman_resume(void)
{
	/* RESUME */
	if (!ug_man.is_initted) {
		_ERR("ugman_resume failed: manager is not initted");
		return -1;
	}

	if (!ug_man.root) {
		_ERR("ugman_resume failed: no root");
		return -1;
	}

	ecore_idler_add(ugman_ug_resume, ug_man.root);

	return 0;
}

int ugman_pause(void)
{
	/* PAUSE (Background) */
	if (!ug_man.is_initted) {
		_ERR("ugman_pause failed: manager is not initted");
		return -1;
	}

	if (!ug_man.root) {
		_ERR("ugman_pause failed: no root");
		return -1;
	}

	ecore_idler_add(ugman_ug_pause, ug_man.root);

	return 0;
}

static int ugman_send_event_pre(void *data)
{
	job_start();

	ugman_ug_event(ug_man.root, (enum ug_event)data);

	job_end();

	return 0;
}

int ugman_send_event(enum ug_event event)
{
	int is_rotation = 1;

	/* Propagate event */
	if (!ug_man.is_initted) {
		_ERR("ugman_send_event failed: manager is not initted");
		return -1;
	}

	/* In case of rotation, indicator state has to be updated */
	switch (event) {
	case UG_EVENT_ROTATE_PORTRAIT:
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		ug_man.last_rotate_evt = event;
		ug_man.is_landscape = 0;
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		ug_man.last_rotate_evt = event;
		ug_man.is_landscape = 1;
		break;
	default:
		is_rotation = 0;
	}

	if (!ug_man.root) {
		_ERR("ugman_send_event failed: no root");
		return -1;
	}

	ecore_idler_add(ugman_send_event_pre, (void *)event);

	if (is_rotation && ug_man.fv_top)
		ugman_indicator_update(ug_man.fv_top->opt, event);

	return 0;
}

static int ugman_send_key_event_to_ug(ui_gadget_h ug,
				      enum ug_key_event event)
{
	struct ug_module_ops *ops = NULL;

	if (!ug)
		return -1;

	if (ug->module) {
		ops = &ug->module->ops;
	} else {
		return -1;
	}

	if (ops && ops->key_event) {
		ops->key_event(ug, event, ug->service, ops->priv);
	} else {
		return -1;
	}

	return 0;
}

int ugman_send_key_event(enum ug_key_event event)
{
	if (!ug_man.is_initted) {
		_ERR("ugman_send_key_event failed: manager is not initted");
		return -1;
	}

	if (!ug_man.fv_top || !ugman_ug_exist(ug_man.fv_top)
	    || ug_man.fv_top->state == UG_STATE_DESTROYED) {
		_ERR("ugman_send_key_event failed: full view top UG is invalid");
		return -1;
	}

	return ugman_send_key_event_to_ug(ug_man.fv_top, event);
}

int ugman_send_message(ui_gadget_h ug, service_h msg)
{
	struct ug_module_ops *ops = NULL;
	if (!ug || !ugman_ug_exist(ug) || ug->state == UG_STATE_DESTROYED) {
		_ERR("ugman_send_message failed: Invalid ug");
		errno = EINVAL;
		return -1;
	}

	if (!msg) {
		_ERR("ugman_send_message failed: Invalid msg");
		errno = EINVAL;
		return -1;
	}

	if (ug->module)
		ops = &ug->module->ops;

	if (ops && ops->message)
		ops->message(ug, msg, ug->service, ops->priv);

	return 0;
}

void *ugman_get_window(void)
{
	return ug_man.win;
}

void *ugman_get_conformant(void)
{
	return ug_man.conform;
}


static inline void job_start(void)
{
	ug_man.walking++;
}

static inline void job_end(void)
{
	ug_man.walking--;

	if (!ug_man.walking && ug_man.destroy_all) {
		ug_man.destroy_all = 0;
		if (ug_man.root)
			ugman_ug_destroy(ug_man.root);
	}

	if (ug_man.walking < 0)
		ug_man.walking = 0;
}

int ugman_ug_exist(ui_gadget_h ug)
{
	return ugman_ug_find(ug_man.root, ug);
}
