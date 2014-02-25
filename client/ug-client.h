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

#ifndef __UG_CLIENT_H__
#define __UG_CLIENT_H__

#include <Elementary.h>
#include <ui-gadget.h>
#include <bundle.h>

#if !defined(PACKAGE)
#define PACKAGE "ug-client"
#endif

#if !defined(LOCALEDIR)
#define LOCALEDIR "/usr/share/locale"
#endif

#if !defined(EDJDIR)
#define EDJDIR "/usr/share/edje/ug-client"
#endif

#define EDJ_FILE EDJDIR "/" PACKAGE ".edj"
#define GRP_MAIN "main"

struct appdata {
	Evas_Object *win;
	int rotate;
	Evas_Object *ly_main;
	Evas_Object *conform;

	ui_gadget_h ug;
	char *name;
	int is_frameview;
	int is_transient;

	bundle *data;
	service_h request;
};

#endif				/* __UG_CLIENT_H__ */
