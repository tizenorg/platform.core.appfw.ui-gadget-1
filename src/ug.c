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

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "ug.h"
#include "ug-module.h"
#include "ug-manager.h"
#include "ug-dbg.h"

#ifndef UG_API
#define UG_API __attribute__ ((visibility("default")))
#endif

ui_gadget_h ug_root_create(void)
{
	ui_gadget_h ug;

	ug = calloc(1, sizeof(struct ui_gadget_s));
	if (!ug) {
		_ERR("ug root create failed: Memory allocation failed");
		return NULL;
	}

	ug->mode = UG_MODE_FULLVIEW;
	ug->state = UG_STATE_RUNNING;
	ug->children = NULL;

	return ug;
}

int ug_free(ui_gadget_h ug)
{
	if (!ug) {
		_ERR("ug free failed: Invalid ug");
		errno = EINVAL;
		return -1;
	}

	if (ug->module) {
		ug_module_unload(ug->module);
	}
	if (ug->name) {
		free((void *)ug->name);
		ug->name = NULL;
	}
	if (ug->service) {
		service_destroy(ug->service);
		ug->service = NULL;
	}
	free(ug);
	ug = NULL;
	return 0;
}

UG_API ui_gadget_h ug_create(ui_gadget_h parent,
				   const char *name,
				   enum ug_mode mode,
				   service_h service, struct ug_cbs *cbs)
{
	if (!name) {
		_ERR("ug_create() failed: Invalid name");
		errno = EINVAL;
		return NULL;
	}

	if (mode < UG_MODE_FULLVIEW || mode >= UG_MODE_INVALID) {
		_ERR("ug_create() failed: Invalid mode");
		errno = EINVAL;
		return NULL;
	}

	return ugman_ug_load(parent, name, mode, service, cbs);
}

UG_API int ug_init(Display *disp, Window xid, void *win, enum ug_option opt)
{
	if (!win || !xid || !disp) {
		_ERR("ug_init() failed: Invalid arguments");
		return -1;
	}

	if (opt < UG_OPT_INDICATOR_ENABLE || opt >= UG_OPT_MAX) {
		_ERR("ug_init() failed: Invalid option");
		return -1;
	}

	return ugman_init(disp, xid, win, opt);
}

UG_API int ug_pause(void)
{
	return ugman_pause();
}

UG_API int ug_resume(void)
{
	return ugman_resume();
}

UG_API int ug_destroy(ui_gadget_h ug)
{
	return ugman_ug_del(ug);
}

UG_API int ug_destroy_all(void)
{
	return ugman_ug_del_all();
}

UG_API int ug_destroy_me(ui_gadget_h ug)
{
	if (!ug || !ugman_ug_exist(ug)) {
		_ERR("ug_destroy_me() failed: Invalid ug");
		errno = EINVAL;
		return -1;
	}

	if (ug->state == UG_STATE_DESTROYING) {
		_ERR("ug_destory_me() failed:ug(%p) is already on destroying", ug);
		return -1;
	}

	if (!ug->cbs.destroy_cb) {
		_ERR("ug_destroy_me() failed: destroy callback does not "
			"exist");
		return -1;
	}

	ug->cbs.destroy_cb(ug, ug->cbs.priv);
	return 0;
}

UG_API void *ug_get_layout(ui_gadget_h ug)
{
	if (!ug || !ugman_ug_exist(ug)) {
		_ERR("ug_get_layout() failed: Invalid ug");
		errno = EINVAL;
		return NULL;
	}
	return ug->layout;
}

UG_API void *ug_get_parent_layout(ui_gadget_h ug)
{
	ui_gadget_h parent;
	if (!ug || !ugman_ug_exist(ug)) {
		_ERR("ug_get_parent_layout() failed: Invalid ug");
		errno = EINVAL;
		return NULL;
	}

	parent = ug->parent;

	if (parent)
		return parent->layout;
	return NULL;
}

UG_API enum ug_mode ug_get_mode(ui_gadget_h ug)
{
	if (!ug || !ugman_ug_exist(ug)) {
		_ERR("ug_get_mode() failed: Invalid ug");
		errno = EINVAL;
		return UG_MODE_INVALID;
	}

	return ug->mode;
}

UG_API void *ug_get_window(void)
{
	return ugman_get_window();
}

UG_API void *ug_get_conformant(void)
{
	return ugman_get_conformant();
}

UG_API int ug_send_event(enum ug_event event)
{
	if (event <= UG_EVENT_NONE || event >= UG_EVENT_MAX) {
		_ERR("ug_send_event() failed: Invalid event");
		return -1;
	}

	return ugman_send_event(event);
}

UG_API int ug_send_key_event(enum ug_key_event event)
{
	if (event <= UG_KEY_EVENT_NONE || event >= UG_KEY_EVENT_MAX) {
		_ERR("ug_send_key_event() failed: Invalid event");
		return -1;
	}

	return ugman_send_key_event(event);
}

UG_API int ug_send_result(ui_gadget_h ug, service_h send)
{
	service_h send_dup = NULL;

	if (!ug || !ugman_ug_exist(ug)) {
		_ERR("ug_send_result() failed: Invalid ug");
		errno = EINVAL;
		return -1;
	}

	if (!ug->cbs.result_cb) {
		_ERR("ug_send_result() failed: result callback does not exist");
		return -1;
	}

	if (send) {
		service_clone(&send_dup, send);
		if (!send_dup) {
			_ERR("ug_send_result() failed: service_destroy failed");
			return -1;
		}
	}

	ug->cbs.result_cb(ug, send_dup, ug->cbs.priv);

	if (send_dup)
		service_destroy(send_dup);

	return 0;
}

UG_API int ug_send_result_full(ui_gadget_h ug, service_h send, service_result_e result)
{
	service_h send_dup = NULL;
	char tmp_result[4] = {0,};

	if (!ug || !ugman_ug_exist(ug)) {
		_ERR("ug_send_result() failed: Invalid ug");
		errno = EINVAL;
		return -1;
	}

	if (!ug->cbs.result_cb) {
		_ERR("ug_send_result() failed: result callback does not exist");
		return -1;
	}

	if (send) {
		service_clone(&send_dup, send);
		if (!send_dup) {
			_ERR("ug_send_result() failed: service_destroy failed");
			return -1;
		}
	}

	snprintf(tmp_result, 4, "%d", result);

	service_add_extra_data(send_dup, UG_SERVICE_DATA_RESULT, (const char*)tmp_result);

	ug->cbs.result_cb(ug, send_dup, ug->cbs.priv);

	if (send_dup)
		service_destroy(send_dup);

	return 0;
}

UG_API int ug_send_message(ui_gadget_h ug, service_h msg)
{
	int r;

	service_h msg_dup = NULL;
	if (msg) {
		service_clone(&msg_dup, msg);
		if (!msg_dup) {
			_ERR("ug_send_message() failed: service_destroy failed");
			return -1;
		}
	}

	r = ugman_send_message(ug, msg_dup);

	if (msg_dup)
		service_destroy(msg_dup);

	return r;
}

UG_API int ug_disable_effect(ui_gadget_h ug)
{
	if (ug->layout_state != UG_LAYOUT_INIT) {
		_ERR("ug_disable_effect() failed: ug has already been shown");
		return -1;
	}
	ug->layout_state = UG_LAYOUT_NOEFFECT;

	return 0;
}

UG_API int ug_is_installed(const char *name)
{
	if(name == NULL){
		_ERR("name is null");
		return -1;
	}

	return ug_exist(name);
}

