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
static void on_show_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void (*show_end_cb)(void* data) = NULL;
static void (*hide_end_cb)(void* data) = NULL;
static void __show_finished(void *data, Evas_Object *obj, void *event_info);

static Evas_Object *_get_win_conformant(Evas_Object *win)
{
	Evas_Object *con = NULL;

	if (!win) {
		_WRN("\t Invalid param error");
		return NULL;
	}

	con = evas_object_data_get(win, "\377 elm,conformant");
	if (con)
		_DBG("\t success to get conformant");
	else
		_WRN("\t fail to get conformant");

	return con;
}

static void _layout_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;

	_WRN("ug(%p) layout is deleted by abnormal path", ug);

	evas_object_event_callback_del(ug->layout, EVAS_CALLBACK_DEL,
		(Evas_Object_Event_Cb)_layout_del_cb);

	ug->layout_state = UG_LAYOUT_DESTROY;
	ug->layout = NULL;
}

static void __del_effect_end(ui_gadget_h ug)
{
	if (navi) {
		Elm_Object_Item *t = elm_naviframe_top_item_get(navi);
		Elm_Object_Item *b = elm_naviframe_bottom_item_get(navi);
		if (t == b) {
			_DBG("\t unset navi");
			elm_object_part_content_unset(conform, "elm.swallow.ug");
			evas_object_hide(navi);
		}
	}
	if (ug->layout) {
		evas_object_hide(ug->layout);
		evas_object_event_callback_del(ug->layout, EVAS_CALLBACK_DEL,
			(Evas_Object_Event_Cb)_layout_del_cb);
	}

	ug->layout_state = UG_LAYOUT_DESTROY;

	hide_end_cb(ug);
}

static void __del_finished(void *data, Evas_Object *obj, void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;

	_DBG("\t obj=%p ug=%p", obj, ug);

	evas_object_smart_callback_del(obj, "transition,finished",
					__del_finished);

	if(ug->layout_state == UG_LAYOUT_HIDEEFFECT)
		__del_effect_end(ug);
	else
		_ERR("wrong ug(%p) state(%d)", ug, ug->layout_state);
}

static void __del_effect_top_layout(ui_gadget_h ug)
{
	_DBG("\t cb transition add ug=%p", ug);
	evas_object_smart_callback_add(navi, "transition,finished",
				__del_finished, ug);
	elm_naviframe_item_pop(navi);
	ug->effect_layout = NULL;
	ug->layout_state = UG_LAYOUT_HIDEEFFECT;
}

static void __del_effect_layout(ui_gadget_h ug, ui_gadget_h t_ug)
{
	if (!ug)
		return;

	_DBG("\t ug=%p state=%d , t_ug=%p", ug, ug->layout_state, t_ug);

	_DBG("\t remove navi item: ug=%p state=%d", ug, ug->layout_state);
	elm_object_item_del(ug->effect_layout);
	ug->effect_layout = NULL;

	__del_effect_end(ug);
}

static void __hide_end(ui_gadget_h ug)
{
	if (navi) {
		Elm_Object_Item *t = elm_naviframe_top_item_get(navi);
		Elm_Object_Item *b = elm_naviframe_bottom_item_get(navi);
		if (t == b) {
			_DBG("\t unset navi");
			elm_object_part_content_unset(conform, "elm.swallow.ug");
			evas_object_hide(navi);
		}
	}

	if (ug->layout) {
		evas_object_hide(ug->layout);
	}
}

static void __hide_effect_end(ui_gadget_h ug)
{
	__hide_end(ug);

	ug->layout_state = UG_LAYOUT_HIDE;
}

static void __hide_finished(void *data, Evas_Object *obj, void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;

	_DBG("\t obj=%p ug=%p", obj, ug);

	evas_object_smart_callback_del(obj, "transition,finished",
					__hide_finished);

	if(ug->layout_state == UG_LAYOUT_HIDEEFFECT)
		__hide_effect_end(ug);
	else
		_ERR("wrong ug(%p) state(%d)", ug, ug->layout_state);
}

static void __on_hideonly_cb(void *data, Evas_Object *obj)
{
	ui_gadget_h ug = (ui_gadget_h)data;

	if (!ug)
		return;

	_DBG("\t obj=%p ug=%p layout_state=%d state=%d", obj, ug, ug->layout_state, ug->state);

	evas_object_intercept_hide_callback_del(ug->layout, __on_hideonly_cb);
	evas_object_event_callback_add(ug->layout, EVAS_CALLBACK_SHOW, on_show_cb, ug);

	if (ug->layout_state == UG_LAYOUT_SHOW) {
		ug->layout_state = UG_LAYOUT_HIDE;
	} else if (ug->layout_state == UG_LAYOUT_NOEFFECT) {
		;
	} else {
		_ERR("wrong ug(%p) state(%d)", ug, ug->layout_state);
		return;
	}

	if ((elm_naviframe_top_item_get(navi) == ug->effect_layout) 
		&& (ug->layout_state != UG_LAYOUT_NOEFFECT)) {
		_DBG("\t cb transition add ug=%p", ug);
		evas_object_smart_callback_add(navi, "transition,finished",
				__hide_finished, ug);
		elm_naviframe_item_pop(navi);
		ug->layout_state = UG_LAYOUT_HIDEEFFECT;
	} else {
		elm_object_item_del(ug->effect_layout);
		__hide_end(ug);
	}

	ug->effect_layout = NULL;
}

static void on_destroy(ui_gadget_h ug, ui_gadget_h t_ug,
		       void (*hide_cb)(void* data))
{
	if (!ug)
		return;
	_DBG("\t ug=%p tug=%p layout_state=%d", ug, t_ug, ug->layout_state);

	evas_object_intercept_hide_callback_del(ug->layout,
						__on_hideonly_cb);

	if(hide_cb == NULL) {
		/* ug_destroy_all case */
		evas_object_event_callback_del(ug->layout, EVAS_CALLBACK_DEL,
			(Evas_Object_Event_Cb)_layout_del_cb);
		return;
	}

	if(!hide_end_cb)
		hide_end_cb = hide_cb;

	if (ug != t_ug) {
		_DBG("requested ug(%p) is not top ug(%p)", ug, t_ug);
		__del_effect_layout(ug, t_ug);
		return;
	}

	if(ug->layout_state == UG_LAYOUT_SHOW) {
		__del_effect_top_layout(ug);
	} else if (ug->layout_state == UG_LAYOUT_SHOWEFFECT) {
		evas_object_smart_callback_del(navi, "transition,finished",
					__show_finished);
		__del_effect_top_layout(ug);
	} else if (ug->layout_state == UG_LAYOUT_HIDE
		|| ug->layout_state == UG_LAYOUT_NOEFFECT) {
		__del_effect_layout(ug, t_ug);
	} else if (ug->layout_state == UG_LAYOUT_HIDEEFFECT) {
		;
	} else {
		_WRN("[UG Effect Plug-in] : layout state(%p) error!!", ug->layout_state);
		__del_effect_end(ug);
	}
}

static void __show_finished(void *data, Evas_Object *obj, void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;

	_DBG("\tobj=%p ug=%p", obj, ug);

	evas_object_smart_callback_del(obj, "transition,finished",
					__show_finished);

	if (ug->layout_state == UG_LAYOUT_DESTROY) {
		_DBG("ug(%p) already destroyed", ug);
	} else if (ug->layout_state == UG_LAYOUT_SHOWEFFECT) {
		ug->layout_state = UG_LAYOUT_SHOW;
		if((show_end_cb)&&(ug->state == UG_STATE_CREATED))
			show_end_cb(ug);
	} else {
		_ERR("wrong state(%d)", ug->layout_state);
	}

	return;
}

static void on_show_cb(void *data, Evas *e, Evas_Object *obj,
		       void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;
	_DBG("\tobj=%p ug=%p layout=%p state=%d", obj, ug, ug->layout, ug->layout_state);

	evas_object_event_callback_del(ug->layout, EVAS_CALLBACK_SHOW, on_show_cb);

	evas_object_intercept_hide_callback_add(ug->layout,
						__on_hideonly_cb, ug);

	//if 'elm.swallow.ug' string is changed, msg team have to apply this changes.
	elm_object_part_content_set(conform, "elm.swallow.ug", navi);

	if (ug->layout_state == UG_LAYOUT_HIDEEFFECT
		|| ug->layout_state == UG_LAYOUT_HIDE
	    || ug->layout_state == UG_LAYOUT_INIT) {
		_DBG("\t UG_LAYOUT_Init(%d) obj=%p", ug->layout_state, obj);
		ug->layout_state = UG_LAYOUT_SHOWEFFECT;

		evas_object_smart_callback_add(navi, "transition,finished",
						__show_finished, ug);
		ug->effect_layout = elm_naviframe_item_push(navi, NULL, NULL, NULL,
						    ug->layout, NULL);
	} else if (ug->layout_state == UG_LAYOUT_NOEFFECT) {
		_DBG("\t UG_LAYOUT_NOEFFECT obj=%p", obj);
		Elm_Object_Item *navi_top = elm_naviframe_top_item_get(navi);
		ug->effect_layout = elm_naviframe_item_insert_after(navi,
				navi_top, NULL, NULL, NULL, ug->layout, NULL);
		//ug start cb
		if(show_end_cb)
			show_end_cb(ug);
	} else {
		_ERR("\tlayout state error!! state=%d\n", ug->layout_state);
	}

	_DBG("\ton_show_cb end ug=%p", ug);
}

static void *on_create(void *win, ui_gadget_h ug,
					void (*show_cb)(void* data))
{
	Evas_Object *navi_bg;
	Evas_Object *con = NULL;

	if (!ug)
		return NULL;
	_DBG("\t ug=%p state=%d", ug, ug->layout_state);

	con = evas_object_data_get(win, "\377 elm,conformant");
	if (con) {
		conform = con;
		_DBG("\t There is conformant");
	} else {
		_ERR("\t There is no conformant");
		return NULL;
	}

	if (!navi) {
		navi = elm_naviframe_add(conform);
		elm_object_focus_allow_set(navi, EINA_FALSE);
		elm_object_style_set(navi, "uglib");
		elm_naviframe_content_preserve_on_pop_set(navi, EINA_TRUE);
		_DBG("\t new navi first navi=%p", navi);
		elm_naviframe_prev_btn_auto_pushed_set(navi, EINA_FALSE);

		navi_bg = evas_object_rectangle_add(evas_object_evas_get(navi));
		evas_object_size_hint_fill_set(navi_bg, EVAS_HINT_FILL,
						EVAS_HINT_FILL);
		evas_object_color_set(navi_bg, 0, 0, 0, 0);
		elm_naviframe_item_push(navi, NULL, NULL, NULL, navi_bg, NULL);
	}

	if(!show_end_cb)
		show_end_cb = show_cb;

	evas_object_hide(ug->layout);
	evas_object_event_callback_add(ug->layout, EVAS_CALLBACK_SHOW, on_show_cb, ug);
	evas_object_event_callback_add(ug->layout, EVAS_CALLBACK_DEL,
		(Evas_Object_Event_Cb)_layout_del_cb, ug);

	ug->layout_state = UG_LAYOUT_INIT;

	return conform;
}

static void *on_request(void *data, ui_gadget_h ug, int req)
{
	void *ret;

	_DBG("on_request ug(%p) req(%d)", ug, req);

	switch(req)
	{
		case UG_UI_REQ_GET_CONFORMANT :
			ret = (void *)_get_win_conformant((Evas_Object *)data);
			break;
		default :
			_WRN("wrong req id(%d)", req);
			return NULL;
	}

	return ret;
}

UG_ENGINE_API int UG_ENGINE_INIT(struct ug_engine_ops *ops)
{
	if (!ops)
		return -1;

	ops->create = on_create;
	ops->destroy = on_destroy;
	ops->request = on_request;

	return 0;
}

UG_ENGINE_API void UG_ENGINE_EXIT(struct ug_engine_ops *ops)
{
}
