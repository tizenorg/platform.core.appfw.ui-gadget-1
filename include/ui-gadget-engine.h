/*
 *  UI Gadget
 *
 * Copyright (c) 2000 - 2012 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jayoun Lee <airjany@samsung.com>, Jinwoo Nam <jwoo.nam@samsung.com>
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

#ifndef __UI_GADGET_ENGINE_H__
#define __UI_GADGET_ENGINE_H__

/**
 * @defgroup	UI_Gadget_For_Developer Developer API Reference Guide
 * @ingroup	UI_Gadget
 * @brief	A engine to develop a UI gadget. Callees (UI gadgets) uses this engines and APIs. (callee -> caller)
 *
 * @section Header To Use Them:
 * @code
 * #include <ui-gadget-engine.h>
 * @endcode
 */

/**
 * @addtogroup UI_Gadget_For_Developer
 * @{
 */

#include "ui-gadget.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * UI gadget engine operation type
 * @see @ref lifecycle_sec
 */
struct ug_engine_ops {
	/** create operation */
	void *(*create)(void *win, ui_gadget_h ug, void(*show_end_cb)(void *data));
	/** destroy operation */
	void (*destroy)(ui_gadget_h ug, ui_gadget_h fv_top, void(*hide_end_cb)(void *data));
	/** request operation */
	void *(*request)(void *data, ui_gadget_h ug, int req);
	/** reserved operations */
	void *reserved[3];
};

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* __UI_GADGET_ENGINE_H__ */
