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

#ifndef __UI_GADGET_H__
#define __UI_GADGET_H__

#include <X11/Xlib.h>
#include "ui-gadget-common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Easy-to-use macro of ug_init() for EFL
 * @see ug_init()
 */
#define UG_INIT_EFL(win, opt) \
	ug_init((Display *)ecore_x_display_get(), elm_win_xwindow_get(win), \
		win, opt)

/**
 * Easy-to-use macro of ug_init() for GTK
 * @see ug_init()
 */
#define UG_INIT_GTK(win, opt) \
	win ?  ug_init(gdk_display_get_default(), win, \
	GDK_WINDOW_XWINDOW(GTK_WIDGET(win)->window), win, opt) : -1

/**
 * \par Description:
 * This function initializes default window, display, xwindow id, and indicator state.
 *
 * \par Purpose:
 * First of all, to use UI gadgets in an application, default window to draw the UI gadgets has to be registered. Besides, to change indicator state for the full-view UI gadget, display and xwindow id have to be registered, and to restore application's indicator state, default indicator option has to be registered. This function is used for registering them.
 *
 * \par Typical use case:
 * Application developers who want to use UI gadget MUST register display, xwindow id, default window, and option with the function at first.
 *
 * \par Method of function operation:
 * Register display, xwindow id, default window, and option.
 *
 * \par Context of function:
 * None
 *
 * \note If you are unfamiliar with display and xwindow id, please use following macros: UG_INIT_EFL, UG_INIT_GTK. The macros kindly generate proper functions to get display and xwindow id.
 *
 * @param[in] disp Default display
 * @param[in] xid Default xwindow id of default window
 * @param[in] win Default window object, it is void pointer for supporting both GTK (GtkWidget *) and EFL (Evas_Object *)
 * @param[in] opt Default indicator state to restore application's indicator state
 * @return 0 on success, -1 on error
 *
 * \pre None
 * \post None
 * \see UG_INIT_EFL(), UG_INIT_GTK()
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
 * ug_init((Display *)ecore_x_display_get(), elm_win_xwindow_get(win), win, UG_OPT_INDICATOR_ENABLE);
 * // for convenience you can use following macro: ELM_INIT_EFL(win, UG_OPT_INDICATOR_ENABLE);
 * ...
 * \endcode
 */
int ug_init(Display *disp, Window xid, void *win, enum ug_option opt);

#ifdef __cplusplus
}
#endif

#endif				/* __UI_GADGET_H__ */
