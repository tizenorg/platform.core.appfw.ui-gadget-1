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

#ifndef __UG_MODULE_H__
#define __UG_MODULE_H__

#include "ui-gadget-module.h"

struct ug_module {
	void *handle;
	char *module_name;
	struct ug_module_ops ops;
	char *addr;
};

struct ug_module *ug_module_load(const char *name);
int ug_module_unload(struct ug_module *module);
int ug_exist(const char* name);

#endif				/* __UG_MODULE_H__ */
