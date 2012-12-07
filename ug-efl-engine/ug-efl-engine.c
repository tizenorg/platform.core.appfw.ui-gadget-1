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

static Evas_Object *navi = NULL;
static Evas_Object *conform = NULL;
struct cb_data {
	ui_gadget_h ug;
	void (*hide_end_cb)(ui_gadget_h ug);
};
static void __hide_finished(void *data, Evas_Object *obj, void *event_info);

static void _on_hideonly_cb(void *data, Evas_Object *obj)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	Elm_Object_Item *navi_top;

	if (!ug)
		return;

	_DBG("\t[%s][%d] obj=%p ug=%p state=%d\n", __FUNCTION__, __LINE__, obj,
		ug, ug->layout_state);

	evas_object_intercept_hide_callback_del(ug->layout, _on_hideonly_cb);

	if (ug->layout_state == UG_LAYOUT_NOEFFECT) {
		;
	}

	if (ug->layout_state == UG_LAYOUT_SHOW) {
		ug->layout_state = UG_LAYOUT_HIDEEFFECT;
	}

	navi_top = elm_naviframe_top_item_get(navi);
	if (navi_top == ug->effect_layout) {
		elm_naviframe_item_pop(navi);
	} else {
		elm_object_item_del(ug->effect_layout);
		ug->effect_layout = NULL;
	}
}

static void _del_effect_layout(ui_gadget_h ug)
{
	GSList *child, *trail;

	if (!ug)
		return;

	_DBG("\t[%s][%d] ug=%p state=%d\n", __FUNCTION__, __LINE__, ug,
		ug->layout_state);

	evas_object_intercept_hide_callback_del(ug->layout, _on_hideonly_cb);

	if (ug->children) {
		child = ug->children;
		while (child) {
			trail = g_slist_next(child);
			_del_effect_layout(child->data);
			child = trail;
		}
	}

	/* effect_layout of frameview is null */
	/* remove navi item */
	if (ug->effect_layout) {
		_DBG("\t[%s][%d] remove navi tiem: ug=%p\n", __FUNCTION__,
			__LINE__,ug);
		if (ug->layout_state == UG_LAYOUT_HIDEEFFECT) {
			_DBG("\t del cb, ug=%p\n", ug);
			evas_object_smart_callback_del(navi, "transition,finished",
							__hide_finished);
		}
		elm_object_item_del(ug->effect_layout);
		ug->effect_layout = NULL;
	}

	if (navi) {
		Elm_Object_Item *t = elm_naviframe_top_item_get(navi);
		Elm_Object_Item *b = elm_naviframe_bottom_item_get(navi);
		if (t == b) {
			_DBG("\t[%s][%d]remove navi\n", __FUNCTION__, __LINE__);
			evas_object_del(navi);
			navi = NULL;
		}
	}
	evas_object_hide(ug->layout);
}

static void __hide_finished(void *data, Evas_Object *obj, void *event_info)
{
	struct cb_data *cb_d = (struct cb_data *)data;

	if (!cb_d)
		return;

	evas_object_smart_callback_del(obj, "transition,finished",
					__hide_finished);

	ui_gadget_h ug = cb_d->ug;
	_DBG("\t[%s][%d] obj=%p ug=%p state=%d\n", __FUNCTION__, __LINE__, obj,
		ug, ug->layout_state);

	ug->effect_layout = NULL;
	_del_effect_layout(ug);
	cb_d->hide_end_cb(ug);
	free(cb_d);
}

static int __find_child(ui_gadget_h p, ui_gadget_h ug)
{
	GSList *child = NULL;

	if (!p || !ug)
		return 0;
	child = p->children;

	while (child) {
		if (child->data == ug)
			return 1;
		if (__find_child(child->data, ug))
			return 1;
		child = g_slist_next(child);
	}

	return 0;
}

static void on_destroy(ui_gadget_h ug, ui_gadget_h t_ug,
		       void (*hide_end_cb) (ui_gadget_h ug))
{
	struct cb_data *cb_d;
	Elm_Object_Item *navi_top;

	if (!ug)
		return;
	_DBG("\t[%s][%d] ug=%p tug=%p state=%d\n", __FUNCTION__, __LINE__, ug,
		t_ug, ug->layout_state);

	evas_object_intercept_hide_callback_del(ug->layout,
						_on_hideonly_cb);

	if (ug != t_ug) {
		_del_effect_layout(ug);
		hide_end_cb(ug);
		return;
	}

	if (ug->layout_state == UG_LAYOUT_SHOW) {
		struct cb_data *cb_d;
		cb_d = (struct cb_data *)calloc(1, sizeof(struct cb_data));
		cb_d->ug = ug;
		cb_d->hide_end_cb = hide_end_cb;

		_DBG("\t[%s][%d]cb add ug=%p\n", __FUNCTION__, __LINE__, ug);

		evas_object_smart_callback_add(navi, "transition,finished",
					__hide_finished, cb_d);
		elm_naviframe_item_pop(navi);
		ug->layout_state = UG_LAYOUT_HIDEEFFECT;
	} else if (ug->layout_state == UG_LAYOUT_HIDE
		   || ug->layout_state == UG_LAYOUT_NOEFFECT) {
		_del_effect_layout(ug);
		hide_end_cb(ug);
	} else if (ug->layout_state == UG_LAYOUT_HIDEEFFECT
		   || ug->layout_state == UG_LAYOUT_SHOWEFFECT) {
		ug->layout_state = UG_LAYOUT_DESTROY;
	} else {
		_ERR("[UG Effect Plug-in] : layout state error!!");
	}
}

static void __show_finished(void *data, Evas_Object *obj, void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h) data;
	if (!ug)
		return;
	_DBG("\t[%s][%d] obj=%p ug=%p state=%d\n", __FUNCTION__, __LINE__, obj,
		ug, ug->layout_state);

	evas_object_smart_callback_del(obj, "transition,finished",
					__show_finished);

	if (ug->layout_state == UG_LAYOUT_NOEFFECT)
		return;

	if (ug->layout_state == UG_LAYOUT_DESTROY)
		;
	else
		ug->layout_state = UG_LAYOUT_SHOW;
}

static void on_show_cb(void *data, Evas *e, Evas_Object *obj,
		       void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;
	_DBG("\t[%s][%d] obj=%p ug=%p state=%d\n", __FUNCTION__, __LINE__, obj,
		ug, ug->layout_state);

	evas_object_intercept_hide_callback_add(ug->layout,
						_on_hideonly_cb, ug);

	elm_object_part_content_set(conform, "elm.swallow.ug", navi);

	if (ug->layout_state == UG_LAYOUT_HIDE
	    || ug->layout_state == UG_LAYOUT_INIT) {
		_DBG("\t UG_LAYOUT_Init obj=%p\n", obj);
		ug->layout_state = UG_LAYOUT_SHOWEFFECT;
		evas_object_smart_callback_add(navi, "transition,finished",
						__show_finished, ug);
		ug->effect_layout = elm_naviframe_item_push(navi, NULL, NULL, NULL,
						    ug->layout, NULL);
	} else if (ug->layout_state == UG_LAYOUT_NOEFFECT) {
		_DBG("\t UG_LAYOUT_NOEFFECT obj=%p\n", obj);
		Elm_Object_Item *navi_top = elm_naviframe_top_item_get(navi);
		ug->effect_layout = elm_naviframe_item_insert_after(navi,
				navi_top, NULL, NULL, NULL, ug->layout, NULL);
	} else {
		_ERR("\t [%s][%d] layout state error!! state=%d\n",
			__FUNCTION__, __LINE__, ug->layout_state);
	}
}

static void *on_create(void *win, ui_gadget_h ug)
{
	const Eina_List *l;
	Evas_Object *subobj;
	Evas_Object *navi_bg;
	Evas_Object *con = NULL;
	static const char *ug_effect_edj_name = "/usr/share/edje/ug_effect.edj";

	_DBG("[%s][%d] START==========>\n", __FUNCTION__, __LINE__);

	con = evas_object_data_get(win, "\377 elm,conformant");
	if (con) {
		conform = con;
		_DBG("\t There is conformant\n");
	}
	else
		_DBG("\t There is NO conformant\n");

	if (!navi) {
		navi = elm_naviframe_add(conform);
		elm_object_style_set(navi, "uglib");
		elm_naviframe_content_preserve_on_pop_set(navi, EINA_TRUE);
		_DBG("\t new navi first navi=%p\n", navi);
		elm_naviframe_prev_btn_auto_pushed_set(navi, EINA_FALSE);

		navi_bg = evas_object_rectangle_add(evas_object_evas_get(navi));
		evas_object_size_hint_fill_set(navi_bg, EVAS_HINT_FILL,
						EVAS_HINT_FILL);
		evas_object_color_set(navi_bg, 0, 0, 0, 0);
		elm_naviframe_item_push(navi, NULL, NULL, NULL, navi_bg, NULL);
	}

	evas_object_hide(ug->layout);
	evas_object_event_callback_add(ug->layout, EVAS_CALLBACK_SHOW,
				       on_show_cb, ug);

	ug->layout_state = UG_LAYOUT_INIT;

	return NULL;
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
