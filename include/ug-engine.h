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

#ifndef __UG_ENGINE_H__
#define __UG_ENGINE_H__

#include "ui-gadget-engine.h"

struct ug_engine {
	void *handle;
	struct ug_engine_ops ops;
};

struct ug_engine *ug_engine_load(void);
int ug_engine_unload(struct ug_engine *engine);


#endif /* __UG_ENGINE_H__ */
