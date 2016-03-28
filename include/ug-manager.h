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

#ifndef __UG_MANAGER_H__
#define __UG_MANAGER_H__

#include <Evas.h>
#include "ug.h"

int ugman_ug_add(ui_gadget_h parent, ui_gadget_h ug);
ui_gadget_h ugman_ug_load(ui_gadget_h parent,
				const char *name,
				enum ug_mode mode,
				app_control_h app_control,
				struct ug_cbs *cbs);
int ugman_ug_del(ui_gadget_h ug);
int ugman_ug_del_all(void);

#ifndef UG_WAYLAND
int ugman_init(Display *disp, Window xid, void *win, enum ug_option opt);
#else
int ugman_init(void *win, enum ug_option opt);
#endif

int ugman_init_efl(Evas_Object *win, enum ug_option opt);

int ugman_resume(void);
int ugman_resume_ug(ui_gadget_h ug);
int ugman_pause(void);
int ugman_pause_ug(ui_gadget_h ug);
int ugman_send_event(enum ug_event event);
int ugman_send_key_event(enum ug_key_event event);
int ugman_send_message(ui_gadget_h ug, app_control_h msg);

void *ugman_get_window(void);
void *ugman_get_conformant(void);

int ugman_ug_exist(ui_gadget_h ug);

#ifdef ENABLE_UG_CREATE_CB
int ugman_create_cb(void (*create_cb)(char *, char *, char *, void *), void *user_data);
#endif

#endif				/* __UG_MANAGER_H__ */
