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

#ifndef __UI_GADGET_EFL_H__
#define __UI_GADGET_EFL_H__

#include <ui-gadget-common.h>
#include <Elementary.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Easy-to-use macro of ug_init_efl() for EFL
 * @see ug_init_efl()
 */
#define UG_INIT_EFL(win, opt) \
	ug_init_efl(win, opt)

/**
 * \par Description:
 * This function initializes default window and indicator state.
 *
 * @param[in] win A pointer to window evas object.
 * @param[in] opt Default indicator state to restore application's indicator state
 * @return 0 on success, -1 on error
 *
 * \pre None
 * \post None
 * \see UG_INIT_EFL()
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * Evas_Object *win;
 * ...
 * // create window
 * ...
 * ug_init_efl(win, UG_OPT_INDICATOR_ENABLE);
 * // for convenience you can use following macro: ELM_INIT_EFL(win, UG_OPT_INDICATOR_ENABLE);
 * ...
 * \endcode
 */
int ug_init_efl(Evas_Object *win, enum ug_option opt);

#ifdef __cplusplus
}
#endif

#endif				/* __UI_GADGET_EFL_H__ */
