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
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <Ecore.h>

#include "ug.h"
#include "ug-manager.h"
#include "ug-engine.h"
#include "ug-dbg.h"

#define Idle_Cb Ecore_Cb

#define ugman_idler_add(func, data)  \
	ecore_job_add((Ecore_Cb) func, (void *)data);

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
		_WRN("ug_relation_del failed: no parent");
		return -1;
	}

	if(p->children) {
		p->children = g_slist_remove(p->children, ug);
	}

	if (ug->children) {
		g_slist_free(ug->children);
		ug->children = NULL;
	}

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

static int __ug_x_get_window_property(Display *dpy, Window win, Atom atom,
					  Atom type, unsigned int *val,
					  unsigned int len)
{
	unsigned char *prop_ret;
	Atom type_ret;
	unsigned long bytes_after;
	unsigned long  num_ret;
	int format_ret;
	unsigned int i;
	int num;

	prop_ret = NULL;
	if (XGetWindowProperty(dpy, win, atom, 0, 0x7fffffff, False,
			       type, &type_ret, &format_ret, &num_ret,
			       &bytes_after, &prop_ret) != Success)
		return -1;

	if (type_ret != type || format_ret != 32)
		num = -1;
	else if (num_ret == 0 || !prop_ret)
		num = 0;
	else {
		if (num_ret < len)
			len = num_ret;
		for (i = 0; i < len; i++) {
			val[i] = ((unsigned long *)prop_ret)[i];
		}
		num = len;
	}

	if (prop_ret)
		XFree(prop_ret);

	return num;
}

static enum ug_event __ug_x_rotation_get(Display *dpy, Window win)
{
	Window active_win;
	Window root_win;
	int rotation = -1;
	int ret = -1;
	enum ug_event func_ret;

	Atom atom_active_win;
	Atom atom_win_rotate_angle;

	root_win = XDefaultRootWindow(dpy);

	atom_active_win = XInternAtom(dpy, "_NET_ACTIVE_WINDOW", False);
	ret = __ug_x_get_window_property(dpy, root_win, atom_active_win,
					     XA_WINDOW,
					     (unsigned int *)&active_win, 1);
	if (ret < 0) {
		func_ret = UG_EVENT_ROTATE_PORTRAIT;
		goto func_out;
	}

	atom_win_rotate_angle =
		XInternAtom(dpy, "_E_ILLUME_ROTATE_ROOT_ANGLE", False);
	ret = __ug_x_get_window_property(dpy, root_win,
					  atom_win_rotate_angle, XA_CARDINAL,
					  (unsigned int *)&rotation, 1);

	_DBG("x_rotation_get / ret(%d),degree(%d)", ret, rotation);

	if (ret == -1)
		func_ret = UG_EVENT_ROTATE_PORTRAIT;
	else {
		switch (rotation) {
			case 0:
				func_ret = UG_EVENT_ROTATE_PORTRAIT;
				break;
			case 90:
				func_ret = UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN;
				break;
			case 180:
				func_ret = UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN;
				break;
			case 270:
				func_ret = UG_EVENT_ROTATE_LANDSCAPE;
				break;
			default:
				func_ret = UG_EVENT_ROTATE_PORTRAIT;
				break;
		}
	}

func_out:
	return func_ret;
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
		_DBG("[%d] %s [%c] (mem : %s) (ug : %p) (PARENT:  %s)",
		     lv,
		     c && c->name ? c->name : "NO CHILD INFO FIXIT!!!",
		     c && c->mode == UG_MODE_FULLVIEW ? 'F' : 'f', 
			 c->module->addr, c, name);
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

static void ugman_ug_start(void *data)
{
	ui_gadget_h ug = data;
	struct ug_module_ops *ops = NULL;

	if (!ug) {
		_ERR("ug is null");
		return;
	} else if (ug->state != UG_STATE_CREATED) {
		_DBG("start cb will be not invoked because ug(%p) state(%d) is not created", ug, ug->state);
		return;
	}

	_DBG("ug=%p", ug);

	ug->state = UG_STATE_RUNNING;

	if (ug->module)
		ops = &ug->module->ops;

	if (ops && ops->start)
		ops->start(ug, ug->service, ops->priv);

	return;
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

	_DBG("ug(%p)->state : %d", ug, ug->state);

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

static int ugman_indicator_overlap_update(enum ug_option opt)
{
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

	return 0;
}

static int ugman_indicator_update(enum ug_option opt, enum ug_event event)
{
	int enable;
	int cur_state;

	cur_state = utilx_get_indicator_state(ug_man.disp, ug_man.win_id);

	_DBG("indicator update opt(%d) cur_state(%d)", opt, cur_state);

#ifndef ENABLE_UG_HANDLE_INDICATOR_HIDE
	enable = 1;
#else
	switch (GET_OPT_INDICATOR_VAL(opt)) {
		case UG_OPT_INDICATOR_ENABLE:
			if (event == UG_EVENT_NONE)
				enable = 1;
			else
				enable = cur_state ? 1 : 0;
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
#endif

	if(cur_state != enable) {
		_DBG("set indicator status as %d", enable);
		utilx_enable_indicator(ug_man.disp, ug_man.win_id, enable);
	}

	return 0;
}

static int ugman_ug_getopt(ui_gadget_h ug)
{
	if (!ug)
		return -1;

	/* Indicator Option */
	if (ug->mode == UG_MODE_FULLVIEW) {
		ugman_indicator_overlap_update(ug->opt);
		ugman_indicator_update(ug->opt, UG_EVENT_NONE);
	}

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

	_DBG("ug_event_cb : ug(%p) / event(%d)", ug, event);

	if (ops && ops->event)
		ops->event(ug, event, ug->service, ops->priv);

	return 0;
}

static int ugman_ug_destroy(void *data)
{
	ui_gadget_h ug = data;
	struct ug_module_ops *ops = NULL;
	struct ug_cbs *cbs;

	job_start();

	if (!ug)
		goto end;

	_DBG("ug(%p) state(%d)", ug, ug->state);

	switch (ug->state) {
		case UG_STATE_CREATED:
		case UG_STATE_RUNNING:
		case UG_STATE_STOPPED:
		case UG_STATE_DESTROYING:
		case UG_STATE_PENDING_DESTROY:
			break;
		default:
			_WRN("ug(%p) state is already destroyed", ug);
			goto end;
	}

	ug->state = UG_STATE_DESTROYED;

	if((ug != ug_man.root) && (ug->layout) &&
		(ug->mode == UG_MODE_FULLVIEW) &&
		(ug->layout_state != UG_LAYOUT_DESTROY)) {
		/* ug_destroy_all case */
		struct ug_engine_ops *eng_ops = NULL;

		if (ug_man.engine)
			eng_ops = &ug_man.engine->ops;

		if (eng_ops && eng_ops->destroy)
			eng_ops->destroy(ug, NULL, NULL);
	}

	if (ug->module)
		ops = &ug->module->ops;

	if (ops && ops->destroy) {
		_DBG("ug(%p) module destory cb call", ug);
		ops->destroy(ug, ug->service, ops->priv);
	}

	cbs = &ug->cbs;
	if (cbs && cbs->end_cb) {
		_DBG("ug(%p) end cb will be invoked", ug);
		cbs->end_cb(ug, cbs->priv);
	}

	if((ug->parent) && (ug->parent->state == UG_STATE_PENDING_DESTROY)) {
		if((ug->parent->children) && (g_slist_length(ug->parent->children) == 1)) {
			_WRN("pended parent ug(%p) destroy job is added to loop", ug->parent);
			ugman_idler_add((Idle_Cb)ugman_ug_destroy, ug->parent);
		} else {
			_WRN("pended parent ug(%p) will be destroyed after another children is destroyed", ug->parent);
		}
	}

	if (ug != ug_man.root)
		ug_relation_del(ug);

	if (ug->mode == UG_MODE_FULLVIEW) {
		if (ug_man.fv_top == ug) {
			ug_fvlist_del(ug);
			if(!ug_man.destroy_all)
				ugman_ug_getopt(ug_man.fv_top);
		} else {
			ug_fvlist_del(ug);
		}
	}

	_DBG("free ug(%p)", ug);
	ug_free(ug);

	if (ug_man.root == ug)
		ug_man.root = NULL;

	ugman_tree_dump(ug_man.root);
 end:
	job_end();

	return 0;
}

static void ug_hide_end_cb(void *data)
{
	ui_gadget_h ug = data;
	if (ug->children) {
		_WRN("child ug is still destroying. parent ug(%p) will be destroyed later", ug);
		ug->state = UG_STATE_PENDING_DESTROY;
	} else {
		ugman_idler_add((Idle_Cb)ugman_ug_destroy, (void *)ug);
	}
}

static int ugman_ug_create(void *data)
{
	ui_gadget_h ug = data;
	struct ug_module_ops *ops = NULL;
	struct ug_cbs *cbs;
	struct ug_engine_ops *eng_ops = NULL;

	if (!ug || ug->state != UG_STATE_READY) {
		_ERR("ug(%p) input param error");
		return -1;
	}

	ug->state = UG_STATE_CREATED;

	if (ug->module)
		ops = &ug->module->ops;

	if (ug_man.engine)
		eng_ops = &ug_man.engine->ops;

	if (ops && ops->create) {
		ug->layout = ops->create(ug, ug->mode, ug->service, ops->priv);
		if (!ug->layout) {
			ug_relation_del(ug);
			_ERR("ug(%p) layout is null", ug);
			return -1;
		}
		if (ug->mode == UG_MODE_FULLVIEW) {
			if (eng_ops && eng_ops->create) {
				ug_man.conform = eng_ops->create(ug_man.win, ug, ugman_ug_start);
				if(!ug_man.conform)
					return -1;
			}
		}
		cbs = &ug->cbs;

		if (cbs && cbs->layout_cb)
			cbs->layout_cb(ug, ug->mode, cbs->priv);

		_DBG("after caller layout cb call");
		ugman_indicator_update(ug->opt, UG_EVENT_NONE);
	}

	if(ug_man.last_rotate_evt == UG_EVENT_NONE) {
		ug_man.last_rotate_evt = __ug_x_rotation_get(ug_man.disp, ug_man.win_id);
	}
	ugman_ug_event(ug, ug_man.last_rotate_evt);

	if(ug->mode == UG_MODE_FRAMEVIEW)
		ugman_ug_start(ug);

	ugman_tree_dump(ug_man.root);

	return 0;
}

int ugman_ug_add(ui_gadget_h parent, ui_gadget_h ug)
{
	if (!ug_man.is_initted) {
		_ERR("failed: manager is not initted");
		return -1;
	}

	if (!ug_man.root) {
		if (parent) {
			_ERR("failed: parent has to be NULL w/o root");
			errno = EINVAL;
			return -1;
		}

		ug_man.root = ug_root_create();
		if (!ug_man.root) {
			_ERR("failed : ug root create fail");
			return -1;
		}
		ug_man.root->opt = ug_man.base_opt;
		ug_man.root->layout = ug_man.win;
		ug_fvlist_add(ug_man.root);
	}

	if (!parent) {
		parent = ug_man.root;
	} else {
		switch (parent->state) {
			case UG_STATE_DESTROYING:
			case UG_STATE_PENDING_DESTROY:
			case UG_STATE_DESTROYED:
				_WRN("parent(%p) state(%d) error", parent, parent->state);
				return -1;
			default:;
		}
	}

	if (ug_relation_add(parent, ug)) {
		_ERR("failed : ug_relation_add fail");
		return -1;
	}

	if (ugman_ug_create(ug) == -1) {
		_ERR("failed : ugman_ug_create fail");
		return -1;
	}
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

	_DBG("ugman_ug_destroying");

	ug->destroy_me = 1;
	ug->state = UG_STATE_DESTROYING;

	if (ug->module)
		ops = &ug->module->ops;

	if (ops && ops->destroying)
		ops->destroying(ug, ug->service, ops->priv);

	return 0;
}

int ugman_ug_del(ui_gadget_h ug)
{
	struct ug_engine_ops *eng_ops = NULL;

	if (!ug || !ugman_ug_exist(ug) || ug->state == UG_STATE_DESTROYED) {
		_ERR("ugman_ug_del failed: Invalid ug(%p)");
		errno = EINVAL;
		return -1;
	}

	_DBG("ugman_ug_del start ug(%p)", ug);

	if (ug->destroy_me) {
		_WRN("ugman_ug_del failed: ug is alreay on destroying");
		return -1;
	}

	if (!ug_man.is_initted) {
		_WRN("ugman_ug_del failed: manager is not initted");
		return -1;
	}

	if (!ug_man.root) {
		_ERR("ugman_ug_del failed: no root");
		return -1;
	}

	if (ug->children) {
		GSList *child, *trail;

		child = ug->children;
		_DBG("ugman_ug_del ug(%p) has child(%p)", ug, child->data);
		while (child) {
			trail = g_slist_next(child);
			ugman_ug_del(child->data);
			child = trail;
		}
	}

	ugman_ug_destroying(ug);

	/* pre call for indicator update time issue */
	bool is_update = false;
	ui_gadget_h t = NULL;
	if (ug_man.fv_top == ug) {
		is_update = true;
		t = g_slist_nth_data(ug_man.fv_list, 1);
	} else {
		if (ug->children) {
			GSList *child;
			child = g_slist_last(ug->children);
			if(ug_man.fv_top == (ui_gadget_h)child->data) {
				is_update = true;
				t = g_slist_nth_data(ug_man.fv_list,
					g_slist_index(ug_man.fv_list,(gconstpointer)ug)+1);
			}
		}
	}

	if((is_update)&&(t)) {
		ugman_ug_getopt(t);
	}

	if (ug_man.engine)
		eng_ops = &ug_man.engine->ops;

	if (ug->mode == UG_MODE_FULLVIEW) {
		if (eng_ops && eng_ops->destroy)
			eng_ops->destroy(ug, ug_man.fv_top, ug_hide_end_cb);
		else
			ugman_idler_add((Idle_Cb)ugman_ug_destroy, ug);
	} else {
		_DBG("ug(%p) mode is frameview", ug);
		ug_hide_end_cb(ug);
	}

	_DBG("ugman_ug_del(%p) end", ug);

	return 0;
}


int ugman_ug_del_child(ui_gadget_h ug)
{
	GSList *child, *trail;

	if (ug->children) {
		child = ug->children;
		_DBG("ug destroy all. ug(%p) has child(%p)", ug, child->data);
		while (child) {
			trail = g_slist_next(child);
			ugman_ug_del_child(child->data);
			child = trail;
		}
	}

	ugman_ug_destroy(ug);

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

	_DBG("ug_del_all. root(%p) walking(%d) ", ug_man.root, ug_man.walking);

	if (ug_man.walking > 0) {
		ug_man.destroy_all = 1;
	} else {
		ugman_ug_del_child(ug_man.root);
	}

	return 0;
}

int ugman_init(Display *disp, Window xid, void *win, enum ug_option opt)
{
	ug_man.win = win;
	ug_man.disp = disp;
	ug_man.win_id = xid;
	ug_man.base_opt = opt;
	ug_man.last_rotate_evt = UG_EVENT_NONE;

	if (!ug_man.is_initted) {
		ug_man.engine = ug_engine_load();
	}

	ug_man.is_initted = 1;

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
		_WRN("ugman_resume failed: no root");
		return -1;
	}

	_DBG("ugman_resume called");

	ugman_idler_add((Idle_Cb)ugman_ug_resume, ug_man.root);

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
		_WRN("ugman_pause failed: no root");
		return -1;
	}

	_DBG("ugman_pause called");

	ugman_idler_add((Idle_Cb)ugman_ug_pause, ug_man.root);

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

	if (!ug_man.root) {
		_WRN("ugman_send_event failed: no root");
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

	ugman_idler_add((Idle_Cb)ugman_send_event_pre, (void *)event);

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
	struct ug_engine_ops *eng_ops = NULL;
	void* ret = NULL;

	if(ug_man.conform) {
		_DBG("return cached conform(%p) info", ug_man.conform);
		return ug_man.conform;
	}

	if (ug_man.engine) {
		eng_ops = &ug_man.engine->ops;
	} else {
		_WRN("ui engine is not loaded");
		return NULL;
	}

	if (eng_ops && eng_ops->create) {
		ret = eng_ops->request(ug_man.win, NULL, UG_UI_REQ_GET_CONFORMANT);
		ug_man.conform = ret;
	} else {
		_WRN("ui engine is not loaded");
	}

	return ret;
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
		if (ug_man.root) {
			_DBG("ug_destroy_all pneding job exist. ug_destroy_all begin");
			ugman_ug_del_all();
		}
	}

	if (ug_man.walking < 0)
		ug_man.walking = 0;
}

int ugman_ug_exist(ui_gadget_h ug)
{
	return ugman_ug_find(ug_man.root, ug);
}
