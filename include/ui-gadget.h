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

/**
 * @file	ui-gadget.h
 * @version	0.1
 * @brief	This file contains the public API of the UI gadget library
 */

/**
 * @addtogroup APPLICATION_FRAMEWORK
 * @{
 *
 * @defgroup	UI_Gadget UI gadget library
 * @version	0.1
 * @brief	A library to develop/use a UI gadget
 */

/**
 * @addtogroup UI_Gadget
 * @{
 *
 * @defgroup	UI_Gadget_For_User User API Reference Guide
 * @brief	A module to use a UI gadget. Caller uses this module and APIs.
 *
 * @section Header To Use Them:
 * @code
 * #include <ui-gadget.h>
 * @endcode
 */

/**
 * @addtogroup UI_Gadget_For_User
 * @{
 */

#include <X11/Xlib.h>
#include <app.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * struct ui_gadget is an opaque type representing a UI gadget
 * @see ug_create(), ug_destroy()
 * @see ug_get_layout(), ug_get_parent_layout(), ug_get_mode()
 */
typedef struct ui_gadget_s *ui_gadget_h;

/**
 * UI gadget mode
 * @see ug_create()
 * @see ug_get_mode()
 */
enum ug_mode {
	UG_MODE_FULLVIEW, /**< Fullview mode */
	UG_MODE_FRAMEVIEW, /**< Frameview mode */
	UG_MODE_INVALID, /**< Invalid mode */
	UG_MODE_MAX
};

/**
 * UI gadget event
 * @see ug_send_event()
 */
enum ug_event {
	UG_EVENT_NONE = 0x00,		/**< No event */
	UG_EVENT_LOW_MEMORY,		/**< Low memory event */
	UG_EVENT_LOW_BATTERY,		/**< Low battery event */
	UG_EVENT_LANG_CHANGE,		/**< Language change event */
	UG_EVENT_ROTATE_PORTRAIT,	/**< Rotate event: Portrait */
	UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN,	/**< Rotate event: Portrait upsidedown */
	UG_EVENT_ROTATE_LANDSCAPE,	/**< Rotate event: Landscape */
	UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN,
			/**< Rotate event: Landscape upsidedown */
	UG_EVENT_REGION_CHANGE,		/**< Region change event */
	UG_EVENT_MAX
};

/**
 * UI gadget key event
 * @see ug_send_key_event()
 */
enum ug_key_event {
	UG_KEY_EVENT_NONE = 0x00,	/**< No event */
	UG_KEY_EVENT_END,		/**< End key event */
	UG_KEY_EVENT_MAX
};

/**
 * UI gadget option
 *
 * @see ug_init()
 */
enum ug_option {
	UG_OPT_INDICATOR_ENABLE = 0x00,
			/**< Indicator option:
			Enable with both portrait and landscape window */
	UG_OPT_INDICATOR_PORTRAIT_ONLY = 0x01,
			/**< Indicator option: Enable with portrait window */
	UG_OPT_INDICATOR_LANDSCAPE_ONLY = 0x02,
			/**< Indicator option: Enable with landscape window */
	UG_OPT_INDICATOR_DISABLE = 0x03,
			/**< Indicator option:
			Disable with both portrait and landscape view window */
	UG_OPT_INDICATOR_MANUAL = 0x04,
			/**< Indicator option:
			Indicator will be handled manually */
	UG_OPT_OVERLAP_ENABLE = 0x08,
			/**< Overlap option: Enable indicator overlap  */		
	UG_OPT_MAX
};

#define GET_OPT_INDICATOR_VAL(opt) opt % UG_OPT_OVERLAP_ENABLE
#define GET_OPT_OVERLAP_VAL(opt) opt & UG_OPT_OVERLAP_ENABLE

#define UG_SERVICE_DATA_RESULT "__UG_SEND_REUSLT__"

/**
 * UI gadget callback type
 * @see ug_create()
 */
struct ug_cbs {
	/** layout callback */
	void (*layout_cb) (ui_gadget_h ug, enum ug_mode mode,
				void *priv);
	/** result callback */
	void (*result_cb) (ui_gadget_h ug, service_h result, void *priv);
	/** destroy callback */
	void (*destroy_cb) (ui_gadget_h ug, void *priv);
	/** end callback */
	void (*end_cb) (ui_gadget_h ug, void *priv);
	/** private data */
	void *priv;
	void *reserved[3];
};

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

/**
 * \par Description:
 * This function creates a UI gadget
 *
 * \par Purpose:
 * This function is used for creating a UI gadget instance. In addition, following callbacks could be registered with the function: layout callback, result callback, and destroy callback. (see struct ug_cbs)
 *
 * \par Typical use case:
 * Anyone who want to create UI gadget could use the function.
 *
 * \par Method of function operation:
 * First, the UI gadget with given name is dynamically loaded(dlopen). Next, state operations of loaded UI gadget are invoked according to its lifecycle. There are three callbacks which could be registered with the function: layout callback, result callback, and destroy callback. If the state is changed to "Create", the layout callback is invoked for layout arrangement. If ug_send_result() is invoked in the loaded UI gadget , the result callback is invoked. And, if ug_destroy_me() is invoked in the loaded UI gadget , the destroy callback is invoked.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init()
 *
 * @param[in] parent parent's UI gadget. If the UI gadget uses the function, the parent has to be the UI gadget. Otherwise, if an application uses the function, the parent has to be NULL
 * @param[in] name name of UI gadget
 * @param[in] mode mode of UI gadget (UG_MODE_FULLVIEW | UG_MODE_FRAMEVIEW)
 * @param[in] service argument for the UI gadget  (see \ref service_PG "Tizen managed api reference guide")
 * @param[in] cbs callback functions (layout callback, result callback, destroy callback, see struct ug_cbs) and private data.
 * @return The pointer of UI gadget, NULL on error
 *
 * \pre ug_init()
 * \post None
 * \see struct ug_cbs, enum ug_mode
 * \remarks If you passed "service", you MUST release it using service_destroy() after ug_create()
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * service_h service;
 * ui_gadget_h ug;
 * struct ug_cbs cbs = {0, };
 *
 * // set callbacks: layout callback, result callback, destroy callback
 * cbs.layout_cb = _layout_cb;
 * cbs.result_cb = _result_cb;
 * cbs.destroy_cb = _destroy_cb;
 * cbs.priv = user_data;
 *
 * // create arguments
 * service_create(&service);
 * service_add_extra_data(service, "Content", "Hello");
 *
 * // create "helloUG-efl" UI gadget instance
 * ug = ug_create(NULL, "helloUG-efl", UG_MODE_FULLVIEW, service, &cbs);
 *
 * // release arguments
 * service_destroy(b);
 * ...
 * \endcode
 */
ui_gadget_h ug_create(ui_gadget_h parent, const char *name,
					enum ug_mode mode, service_h service,
					struct ug_cbs *cbs);

/**
 * \par Description:
 * This function pauses all UI gadgets
 *
 * \par Purpose:
 * This function is used for pausing UI gadgets with "Running" state. Eventually, state of the UI gadgets would be "Stopped."
 *
 * \par Typical use case:
 * Application developers who want to pause loaded UI gadgets could use the function.
 *
 * \par Method of function operation:
 * "Pause" state operations of UI gadgets with "Running" state in the UI gadget tree are invoked by post-order traversal.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init()
 *
 * @return 0 on success, -1 on error
 *
 * \pre ug_init()
 * \post None
 * \see ug_resume()
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * // pause all UI gadget instances
 * ug_pause();
 * ...
 * \endcode
 */
int ug_pause(void);

/**
 * \par Description:
 * This function resumes all UI gadgets
 *
 * \par Purpose:
 * This function is used for resuming UI gadgets with "Stopped" state. Eventually, state of all UI gadgets would be "Running."
 *
 * \par Typical use case:
 * Application developers who want to resume loaded UI gadgets could use the function.
 *
 * \par Method of function operation:
 * "Resume" state operations of UI gadgets with "Stopped" state in the UI gadget tree are invoked by post-order traversal.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init()
 *
 * @return 0 on success, -1 on error
 *
 * \pre ug_init()
 * \post None
 * \see ug_pause()
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * // resume all UI gadget instances
 * ug_resume();
 * ...
 * \endcode
 */
int ug_resume(void);

/**
 * \par Description:
 * This function destroys the given UI gadget instance
 *
 * \par Purpose:
 * This function is used for destroying given UI gadget instance and its children. Eventually, state of the instance would be "Destroyed."
 *
 * \par Typical use case:
 * Anyone who want to destroy specific UI gadget could use the function.
 *
 * \par Method of function operation:
 * "Destroy" state operations of the given UI gadget instance and its children are invoked.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init() and creation UI gadget with ug_create()
 *
 * @param[in] ug The UI gadget
 * @return 0 on success, -1 on error
 *
 * \pre ug_init(), ug_create()
 * \post None
 * \see ug_destroy_all()
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * // destroy UI gadget instance
 * ug_destroy(ug);
 * ...
 * \endcode
 */
int ug_destroy(ui_gadget_h ug);

/**
 * \par Description:
 * This function destroys all UI gadgets of an application
 *
 * \par Purpose:
 * This function is used for destroying all UI gadgets. Eventually, state of all UI gadgets would be "Destroyed."
 *
 * \par Typical use case:
 * Application developers who want to destroy loaded UI gadgets could use the function.
 *
 * \par Method of function operation:
 * "Destroy" state operations of all UI gadgets in the UI gadget tree are invoked by post-order traversal.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init()
 *
 * @return 0 on success, -1 on error
 *
 * \pre ug_init()
 * \post None
 * \see ug_destroy()
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * // destroy all UI gadget instances
 * ug_destroy_all();
 * ...
 * \endcode
 */
int ug_destroy_all(void);

/**
 * \par Description:
 * This function gets base layout of the given UI gadget instance
 *
 * \par Purpose:
 * This function is used for getting base layout pointer of given UI gadget instance.
 *
 * \par Typical use case:
 * Anyone who want to get base layout of UI gadget could use the function.
 *
 * \par Method of function operation:
 * This function returns base layout pointer which is created in "Create" operation of the given UI gadget instance.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init() and creation UI gadget with ug_create()
 *
 * @param[in] ug The UI gadget
 * @return The pointer of base layout, NULL on error. The result value is void pointer for supporting both GTK (GtkWidget *) and EFL (Evas_Object *)
 *
 * \pre ug_init(), ug_create()
 * \post None
 * \see ug_get_parent_layout()
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * Evas_Object *ly;
 * // get a base layout
 * ly = (Evas_Object *)ug_get_layout(ug);
 * ...
 * \endcode
 */
void *ug_get_layout(ui_gadget_h ug);

/**
 * \par Description:
 * This function gets base layout of parent of the given UI gadget instance
 *
 * \par Purpose:
 * This function is used for getting base layout pointer of parent of the given UI gadget instance.
 *
 * \par Typical use case:
 * Anyone who want to get base layout of UI gadget's parent could use the function.
 *
 * \par Method of function operation:
 * This function returns base layout pointer which is created in "Create" operation of parent of the given UI gadget instance.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init() and creation UI gadget with ug_create()
 *
 * @param[in] ug The UI gadget
 * @return The pointer of base layout, NULL on error. The result value is void pointer for supporting both GTK (GtkWidget *) and EFL (Evas_Object *)
 *
 * \pre ug_init(), ug_create()
 * \post None
 * \see ug_get_layout()
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * Evas_Object *ly;
 * // get a base layout of parent of the given UI gadget instance
 * ly = (Evas_Object *)ug_get_parent_layout(ug);
 * ...
 * \endcode
 */
void *ug_get_parent_layout(ui_gadget_h ug);

/**
 * \par Description:
 * This function gets default window
 *
 * \par Purpose:
 * This function is used for getting default window which is registered with ug_init()
 *
 * \par Typical use case:
 * Anyone who want to get default window could use the function.
 *
 * \par Method of function operation:
 * This function returns default window pointer which is registered with ug_init()
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init()
 *
 * @return The pointer of default window, NULL on error. The result value is void pointer for supporting both GTK (GtkWidget *) and EFL (Evas_Object *)
 *
 * \pre ug_init()
 * \post None
 * \see None
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * Evas_Object *win;
 * // get default window
 * win = (Evas_Object *)ug_get_window();
 * ...
 * \endcode
 */
void *ug_get_window(void);

/**
 * \par Description:
 * This function gets ug conformant
 *
 * \par Purpose:
 * This function is used for getting ug conformant
 *
 * \par Typical use case:
 * Anyone who want to get ug conformant could use the function.
 *
 * \par Method of function operation:
 * This function returns ug conformant pointer
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init()
 *
 * @return The pointer of default window, NULL on error. The result value is void pointer for supporting both GTK (GtkWidget *) and EFL (Evas_Object *)
 *
 * \pre ug_init()
 * \post None
 * \see None
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * Evas_Object *conform;
 * // get default window
 * conform = (Evas_Object *)ug_get_conformant();
 * ...
 * \endcode
 */
void *ug_get_conformant(void);

/**
 * \par Description:
 * This function gets mode of the given UI gadget instance
 *
 * \par Purpose:
 * This function is used for getting mode of the given UI gadget instance. Mode could be UG_MODE_FULLVIEW or UG_MODE_FRAMEVIEW.
 *
 * \par Typical use case:
 * Anyone who want to get mode of UI gadget could use the function.
 *
 * \par Method of function operation:
 * This function returns mode which is registered with ug_create()
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init() and creation UI gadget with ug_create()
 *
 * @param[in] ug The UI gadget
 * @return UI gadget mode of the given UI gadget instance (UG_MODE_FULLVIEW | UG_MODE_FRAMEVIEW)
 *
 * \pre ug_init(), ug_create()
 * \post None
 * \see enum ug_mode
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * enum ug_mode mode;
 * // get mode (UG_MODE_FULLVIEW | UG_MODE_FRAMEVIEW)
 * mode = ug_get_mode(ug);
 * ...
 * \endcode
 */
enum ug_mode ug_get_mode(ui_gadget_h ug);

/**
 * \par Description:
 * This function propagates the given system event to all UI gadgets
 *
 * \par Purpose:
 * This function is used for propagating the given system event. Available system events are low memory, low battery, language changed, and window rotate event.
 *
 * \par Typical use case:
 * Application developers who want to propagate system event to all UI gadgets could use the function.
 *
 * \par Method of function operation:
 * Event operations of all UI gadgets in the UI gadget tree are invoked by post-order traversal.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init()
 *
 * @param[in] event UI gadget event. (see enum ug_event)
 * @return 0 on success, -1 on error
 *
 * \pre ug_init()
 * \post None
 * \see enum ug_event
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * // propagate low battery event to all UI gadget instances
 * ug_send_event(UG_EVENT_LOW_BATTERY);
 * ...
 * \endcode
 */
int ug_send_event(enum ug_event event);

/**
 * \par Description:
 * This function send key event to full view top UI gadget
 *
 * \par Purpose:
 * This function is used for sending key event to full view top UI gadget. Available key events are end event.
 *
 * \par Typical use case:
 * Application developers who want to send key event to full view top UI gadget could use the function.
 *
 * \par Method of function operation:
 * Key event operation of full view top UI gadget in the UI gadget tree are invoked.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init()
 *
 * @param[in] event UI gadget key event. (see enum ug_key_event)
 * @return 0 on success, -1 on error
 *
 * \pre ug_init()
 * \post None
 * \see enum ug_key_event
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * // send key event callback to full view top UI gadget instances
 * ug_send_key_event(UG_KEY_EVENT_END);
 * ...
 * \endcode
 */
int ug_send_key_event(enum ug_key_event event);

/**
 * \par Description:
 * This function sends message to the given UI gadget instance
 *
 * \par Purpose:
 * This function is used for sending message to created UI gadget. The message have to be composed with service handle.
 *
 * \par Typical use case:
 * Anyone who want to send message to created UI gadget.
 *
 * \par Method of function operation:
 * Message operation of given UI gadget instance is invoked.
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init() and creation UI gadget with ug_create()
 *
 * @param[in] ug The UI gadget
 * @param[in] msg message to send, which is service type (see \ref service_PG "Tizen managed api reference guide")
 * @return 0 on success, -1 on error
 *
 * \pre ug_init(), ug_create()
 * \post None
 * \see None
 * \remarks After send your message, you have to release it using service_destroy()
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * // make a message with service
 * service_h msg;
 * service_create(&msg)
 * service_add_extra_data(msg, "Content", "Hello");
 *
 * // send the message
 * ug_send_message(ug, msg);
 *
 * // release the message
 * service_destroy(msg);
 * ...
 * \endcode
 */
int ug_send_message(ui_gadget_h ug, service_h msg);

/**
 * \par Description:
 * This function disable transition effect of the given UI gadget instance
 *
 * \par Purpose:
 * This function is used for disabling transition effect of created UI gadget.
 *
 * \par Typical use case:
 * Anyone who want to disable transition effect of created UI gadget.
 *
 * \par Method of function operation:
 * No transition effect of given UI gadget is invoked
 *
 * \par Context of function:
 * This function supposed to be called after successful initialization with ug_init() and creation UI gadget with ug_create()
 *
 * @param[in] ug The UI gadget
 * @return 0 on success, -1 on error
 *
 * \pre ug_init(), ug_create()
 * \post None
 * \see None
 * \remarks Before show layout of given UI gadget, ug_disable_effect() should be called.
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * static void layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
 * {
 * ...
 * base = ug_get_layout(ug);
 * switch (mode) {
 * case UG_MODE_FULLVIEW:
 * // disable effect
 * ug_disable_effect(ug);
 * evas_object_show(base);
 * ...
 * \endcode
 */
int ug_disable_effect(ui_gadget_h ug);

/**
 * \par Description:
 * This function check whether given ug is installed or not
 *
 * \par Purpose:
 * This function is used for checking whether given ug is installed or not
 *
 * \par Typical use case:
 * Anyone who want to know whether given ug is installed or not
 *
 * \par Method of function operation:
 * This function returns value that ug is installed or not.
 *
 * \par Context of function:
 * N/A
 *
 * @param[in] ug The UI gadget
 * @return 1 - installed, 0 - not installed, -1 - error
 *
 * \pre None
 * \post None
 * \see None
 * \remarks None
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget.h>
 * ...
 * ret = ug_is_installed(ug);
 * ...
 */
int ug_is_installed(const char *name);

#ifdef __cplusplus
}
#endif
/**
 * @} @} @}
 */
#endif				/* __UI_GADGET_H__ */
