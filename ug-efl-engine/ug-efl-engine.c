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



#include <glib.h>
#include <Elementary.h>
#include <ui-gadget-engine.h>

#include "ug.h"
#include "ug-efl-engine.h"
#include "ug-dbg.h"

#ifndef UG_ENGINE_API
#define UG_ENGINE_API __attribute__ ((visibility("default")))
#endif

struct cb_data {
	ui_gadget_h ug;
	void(*hide_end_cb)(ui_gadget_h ug);
};


static void _on_hideonly_cb(void *data, Evas_Object *obj)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;

	if (ug->layout_state == UG_LAYOUT_SHOW) {
		ug->layout_state = UG_LAYOUT_HIDEEFFECT;
		edje_object_signal_emit(elm_layout_edje_get(ug->effect_layout),
					"elm,state,hideonly", "");
	}
}

static void _signal_hideonly_finished(void *data, Evas_Object *obj,
				      const char *emission, const char *source)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;

	evas_object_intercept_hide_callback_del(ug->layout, _on_hideonly_cb);

	evas_object_hide(ug->layout);
	elm_object_part_content_unset(ug->effect_layout, "elm.swallow.content");

	if (ug->layout_state == UG_LAYOUT_NOEFFECT)
		return;

	evas_object_hide(ug->effect_layout);

	if (ug->layout_state == UG_LAYOUT_DESTROY)
		edje_object_signal_emit(elm_layout_edje_get(ug->effect_layout),
					"elm,state,hidealready", "");
	else
		ug->layout_state = UG_LAYOUT_HIDE;
}

static void _del_effect_layout(ui_gadget_h ug)
{
	if (!ug || !ug->effect_layout)
		return;

	evas_object_intercept_hide_callback_del(ug->layout, _on_hideonly_cb);

	evas_object_hide(ug->layout);
	elm_object_part_content_unset(ug->effect_layout, "elm.swallow.content");
	evas_object_hide(ug->effect_layout);
	evas_object_del(ug->effect_layout);
	ug->effect_layout = NULL;
}

static void _signal_hide_finished(void *data, Evas_Object *obj,
				  const char *emission, const char *source)
{
	struct cb_data *cb_d = (struct cb_data*)data;

	if (!cb_d)
		return;

	ui_gadget_h ug = cb_d->ug;

	_del_effect_layout(ug);
	cb_d->hide_end_cb(ug);
	free(cb_d);
}

static void _signal_hidealready_finished(void *data, Evas_Object *obj,
				const char *emission, const char *source)
{
	struct cb_data *cb_d = (struct cb_data*)data;

	if (!cb_d)
		return;

	ui_gadget_h ug = cb_d->ug;

	_del_effect_layout(ug);
	cb_d->hide_end_cb(ug);
	free(cb_d);
}

static void _do_destroy(ui_gadget_h ug, ui_gadget_h fv_top)
{
	GSList *child;
	GSList *trail;
	static int depth = 0;

	if (ug->children) {
		child = ug->children;
		while (child) {
			trail = g_slist_next(child);
			depth++;
			_do_destroy(child->data, fv_top);
			depth--;
			child = trail;
		}
	}

	_DBG("[UG Effect Plug-in] : start destroy. ug(%p), fv_top(%p),"
					" depth(%d), layout_state(%d)\n",
					 ug, fv_top, depth, ug->layout_state);

	/* fv_top is null while destroying frameview ug */
	if (fv_top == NULL) {
		_del_effect_layout(ug);
		return;
	}
	/* only show transition effect of top view UG */
	if (ug != fv_top) {
		if (depth) {
			_del_effect_layout(ug);
			return;
		}
	}

	if (ug->layout_state == UG_LAYOUT_SHOW) {
		evas_object_intercept_hide_callback_del(ug->layout,
							_on_hideonly_cb);
		edje_object_signal_emit(elm_layout_edje_get(ug->effect_layout),
					"elm,state,hide", "");
	} else if (ug->layout_state == UG_LAYOUT_HIDE
		   || ug->layout_state == UG_LAYOUT_NOEFFECT) {
		edje_object_signal_emit(elm_layout_edje_get(ug->effect_layout),
					"elm,state,hidealready", "");
	} else if (ug->layout_state == UG_LAYOUT_HIDEEFFECT
		   || ug->layout_state == UG_LAYOUT_SHOWEFFECT) {
		ug->layout_state = UG_LAYOUT_DESTROY;
	} else {
		_ERR("[UG Effect Plug-in] : layout state error!!");
	}
}

static void on_destroy(ui_gadget_h ug, ui_gadget_h fv_top)
{
	if (!ug)
		return;
	_do_destroy(ug, fv_top);
}

static void _signal_show_finished(void *data, Evas_Object *obj,
				  const char *emission, const char *source)
{
	ui_gadget_h ug = (ui_gadget_h )data;
	if (!ug)
		return;

	if (ug->layout_state == UG_LAYOUT_NOEFFECT)
		return;

	if (ug->layout_state == UG_LAYOUT_DESTROY)
		edje_object_signal_emit(elm_layout_edje_get(ug->effect_layout),
					"elm,state,hide", "");
	else
		ug->layout_state = UG_LAYOUT_SHOW;
}

static void on_show_cb(void *data, Evas *e, Evas_Object *obj,
		       void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h )data;
	if (!ug)
		return;

	if (ug->layout_state == UG_LAYOUT_NOEFFECT) {
		evas_object_hide(ug->effect_layout);
		evas_object_show(ug->layout);
		return;
	}

	if (ug->layout_state == UG_LAYOUT_HIDE
	    || ug->layout_state == UG_LAYOUT_INIT) {
		ug->layout_state = UG_LAYOUT_SHOWEFFECT;
		evas_object_show(ug->effect_layout);
		elm_object_part_content_set(ug->effect_layout, "elm.swallow.content",
				       ug->layout);
		evas_object_intercept_hide_callback_add(ug->layout,
							_on_hideonly_cb, ug);
		edje_object_signal_emit(elm_layout_edje_get(ug->effect_layout),
					"elm,state,show", "");
	}
}

static void *on_create(void *win, ui_gadget_h ug,
		       void (*hide_end_cb) (ui_gadget_h ug))
{
	static const char *ug_effect_edj_name = "/usr/share/edje/ug_effect.edj";
	struct cb_data *cb_d;

	Evas_Object *ly = elm_layout_add((Evas_Object *) win);

	if (!ly)
		return NULL;

	evas_object_size_hint_weight_set(ly, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add((Evas_Object *) win, ly);
	elm_layout_file_set(ly, ug_effect_edj_name, "ug_effect");
	evas_object_show(ly);

	evas_object_hide(ug->layout);

	cb_d = calloc(1, sizeof(struct cb_data));
	cb_d->ug = ug;
	cb_d->hide_end_cb = hide_end_cb;

	edje_object_signal_callback_add(elm_layout_edje_get(ly),
					"elm,action,hide,finished", "",
					_signal_hide_finished, cb_d);
	edje_object_signal_callback_add(elm_layout_edje_get(ly),
					"elm,action,hidealready,finished", "",
					_signal_hidealready_finished, cb_d);
	edje_object_signal_callback_add(elm_layout_edje_get(ly),
					"elm,action,hideonly,finished", "",
					_signal_hideonly_finished, ug);
	edje_object_signal_callback_add(elm_layout_edje_get(ly),
					"elm,action,show,finished", "",
					_signal_show_finished, ug);

	evas_object_event_callback_add(ug->layout, EVAS_CALLBACK_SHOW,
				       on_show_cb, ug);

	ug->layout_state = UG_LAYOUT_INIT;

	return ly;
}

UG_ENGINE_API int UG_ENGINE_INIT(struct ug_engine_ops *ops)
{
	if (!ops)
		return -1;

	ops->create = on_create;
	ops->destroy = on_destroy;

	return 0;
}

UG_ENGINE_API void UG_ENGINE_EXIT(struct ug_engine_ops *ops)
{
}
