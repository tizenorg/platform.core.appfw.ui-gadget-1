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

static void _layout_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_gadget_h ug = (ui_gadget_h)data;
	if (!ug)
		return;

	_WRN("ug(%p) layout is deleted by abnormal path", ug);

	evas_object_event_callback_del(ug->layout, EVAS_CALLBACK_DEL, _layout_del_cb);

	ug->layout_state = UG_LAYOUT_DESTROY;
	ug->layout = NULL;
}

static Eina_Bool __destroy_end_cb(void *data)
{
	GSList *child;
	ui_gadget_h ug = (ui_gadget_h)data;

	_DBG("\t __destroy_end_cb ug=%p", ug);

	if (ug->children) {
		child = ug->children;
		//_DBG("\t ug(%p) has children(%p)", ug, child);
		while (child) {
			if(!child->data) {
				_ERR("child->data is null");
				return ECORE_CALLBACK_CANCEL;
			}

			//_DBG("\t child(%p) layout_state(%d)", child, ((ui_gadget_h)child->data)->layout_state);

			if( ((ui_gadget_h)child->data)->layout_state == UG_LAYOUT_HIDEEFFECT) {
				//_DBG("\t wait hideeffect child(%p)", ug);
				return ECORE_CALLBACK_RENEW;
			}
			child = g_slist_next(child);
		}
	}

	hide_end_cb(ug);
	return ECORE_CALLBACK_CANCEL;
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
		evas_object_event_callback_del(ug->layout, EVAS_CALLBACK_DEL, _layout_del_cb);
	}

	ecore_idler_add((Ecore_Task_Cb)__destroy_end_cb, (void *)ug);

	ug->layout_state = UG_LAYOUT_DESTROY;
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
	GSList *child;

	if (!ug)
		return;

	_DBG("\t ug=%p state=%d , t_ug=%p", ug, ug->layout_state, t_ug);

	if (ug->children) {
		child = ug->children;
		_DBG("\t ug(%p) has children(%p)", ug, child);
		while (child) {
			__del_effect_layout(child->data, t_ug);
			child = g_slist_next(child);
		}
	}

	if((ug == t_ug)&&(ug->layout_state != UG_LAYOUT_NOEFFECT)){
		if (ug->layout_state != UG_LAYOUT_HIDEEFFECT) {
			__del_effect_top_layout(ug);
		} else {
			_ERR("\t top ug(%p) state is hideeffect.");
			return;
		}
	} else {
		_DBG("\t remove navi item: ug=%p state=%d", ug, ug->layout_state);
		elm_object_item_del(ug->effect_layout);
		ug->effect_layout = NULL;
	}

	__del_effect_end(ug);
}

static void __hide_effect_end(ui_gadget_h ug)
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

	if (elm_naviframe_top_item_get(navi) == ug->effect_layout) {
		_DBG("\t cb transition add ug=%p", ug);
		evas_object_smart_callback_add(navi, "transition,finished",
				__hide_finished, ug);
		elm_naviframe_item_pop(navi);
		ug->layout_state = UG_LAYOUT_HIDEEFFECT;
	} else {
		elm_object_item_del(ug->effect_layout);
		__hide_effect_end(ug);
	}

	ug->effect_layout = NULL;
}

static void on_destroy(ui_gadget_h ug, ui_gadget_h t_ug,
		       void (*hide_cb)(void* data))
{
	if (!ug)
		return;
	_DBG("\t ug=%p tug=%p state=%d", ug, t_ug, ug->layout_state);

	evas_object_intercept_hide_callback_del(ug->layout,
						__on_hideonly_cb);

	if(hide_cb == NULL) {
		/* ug_destroy_all case */
		evas_object_event_callback_del(ug->layout, EVAS_CALLBACK_DEL, _layout_del_cb);
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
	} else if (ug->layout_state == UG_LAYOUT_HIDE
		|| ug->layout_state == UG_LAYOUT_NOEFFECT
		|| ug->layout_state == UG_LAYOUT_SHOWEFFECT) {
		__del_effect_layout(ug, t_ug);
	} else if (ug->layout_state == UG_LAYOUT_HIDEEFFECT) {
		;
	} else {
		_ERR("[UG Effect Plug-in] : layout state error!!");
	}
}

static void __update_indicator_overlap(int opt)
{
	if (GET_OPT_OVERLAP_VAL(opt)) {
		_DBG("\t this is Overlap UG. Send overlap sig on_show_cb");
		elm_object_signal_emit(conform, "elm,state,indicator,overlap", "");
	}  else {
		_DBG("\t this is no overlap UG. Send no overlap sig on_show_cb");
		elm_object_signal_emit(conform, "elm,state,indicator,nooverlap", "");
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

		__update_indicator_overlap(ug->opt);

		evas_object_smart_callback_add(navi, "transition,finished",
						__show_finished, ug);
		ug->effect_layout = elm_naviframe_item_push(navi, NULL, NULL, NULL,
						    ug->layout, NULL);
	} else if (ug->layout_state == UG_LAYOUT_NOEFFECT) {
		_DBG("\t UG_LAYOUT_NOEFFECT obj=%p", obj);

		__update_indicator_overlap(ug->opt);

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
	}
	else
		_DBG("\t There is NO conformant");

	if (!navi) {
		navi = elm_naviframe_add(conform);
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
	evas_object_event_callback_add(ug->layout, EVAS_CALLBACK_DEL, _layout_del_cb, ug);

	ug->layout_state = UG_LAYOUT_INIT;

	return conform;
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
