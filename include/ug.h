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

#ifndef __UG_H__
#define __UG_H__

#include "ug-module.h"
#include "ui-gadget.h"

enum ug_state {
	UG_STATE_READY = 0x00,
	UG_STATE_CREATED,
	UG_STATE_RUNNING,
	UG_STATE_STOPPED,
	UG_STATE_DESTROYING,
	UG_STATE_PENDING_DESTROY,
	UG_STATE_DESTROYED,
	UG_STATE_INVALID,
	UG_STATE_MAX
};

enum ug_layout_state {
	UG_LAYOUT_INIT = 0x00,
	UG_LAYOUT_SHOW,
	UG_LAYOUT_SHOWEFFECT,
	UG_LAYOUT_HIDE,
	UG_LAYOUT_HIDEEFFECT,
	UG_LAYOUT_DESTROY,
	UG_LAYOUT_NOEFFECT,
	UG_LAYOUT_MAX
};

enum ug_ui_req {
	UG_UI_REQ_GET_CONFORMANT = 0x00,
	UG_UI_REQ_MAX
};

struct ui_gadget_s {
	const char *name;
	void *layout;
	enum ug_state state;
	enum ug_mode mode;
	enum ug_option opt;

	ui_gadget_h parent;
	void *children;

	struct ug_module *module;
	struct ug_cbs cbs;

	service_h service;

	int destroy_me:1;
	enum ug_layout_state layout_state;
	void *effect_layout;
};

ui_gadget_h ug_root_create(void);
int ug_free(ui_gadget_h ug);

#endif				/* __UG_H__ */
