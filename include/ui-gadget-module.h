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

#ifndef __UI_GADGET_MODULE_H__
#define __UI_GADGET_MODULE_H__

/**
 * @defgroup	UI_Gadget_For_Developer Developer API Reference Guide
 * @ingroup	UI_Gadget
 * @brief	A module to develop a UI gadget. Callees (UI gadgets) uses this modules and APIs. (callee -> caller)
 *
 * @section Header To Use Them:
 * @code
 * #include <ui-gadget-module.h>
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
 * UI gadget module operation type
 * @see @ref lifecycle_sec
 */
struct ug_module_ops {
	/** create operation */
	void *(*create) (ui_gadget_h ug, enum ug_mode mode, service_h service,
					void *priv);
	/** start operation */
	void (*start) (ui_gadget_h ug, service_h service, void *priv);
	/** pause operation */
	void (*pause) (ui_gadget_h ug, service_h service, void *priv);
	/** resume operation */
	void (*resume) (ui_gadget_h ug, service_h service, void *priv);
	/** destroy operation */
	void (*destroy) (ui_gadget_h ug, service_h service, void *priv);
	/** message operation */
	void (*message) (ui_gadget_h ug, service_h msg, service_h service, void *priv);
	/** event operation */
	void (*event) (ui_gadget_h ug, enum ug_event event, service_h service,
				void *priv);
	/** key event operation */
	void (*key_event) (ui_gadget_h ug, enum ug_key_event event,
					service_h service, void *priv);
	/** destroying operation */
	void (*destroying) (ui_gadget_h ug, service_h service, void *priv);
	/** reserved operations */
	void *reserved[3];

	/** private data */
	void *priv;

	/** option */
	enum ug_option opt;
};

/**
 * \par Description:
 * This function makes a request that caller of the given UI gadget instance destroys the instance.
 * It just makes a request, but not destroys UI gadget
 *
 * \par Purpose:
 * This function is used for sending a request that caller of the given UI gadget instance destroys the instance.
 *
 * \par Typical use case:
 * UI gadget developer who want to send a request that caller of the given UI gadget instance destroys the instance could use the function.
 *
 * \par Method of function operation:
 * Destroy callback which is registered by caller with ug_create() is invoked.
 *
 * \par Context of function:
 * This function supposed to be called in the created UI gadget.
 *
 * @param[in] ug the UI gadget
 * @return 0 on success, -1 on error
 *
 * \pre None
 * \post None
 * \see None
 * \remarks The API just makes a request, but not destroys UI gadget
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget-module.h>
 * ...
 * // send a "destroy me" request
 * ug_destroy_me(ug);
 * ...
 * \endcode
 */
int ug_destroy_me(ui_gadget_h ug);

/**
 * \par Description:
 * This function sends result to caller of the given UI gadget instance.
 *
 * \par Purpose:
 * This function is used for sending result to caller of the given UI gadget instance. The result have to be composed with service handle.
 *
 * \par Typical use case:
 * UI gadget developer who want to send result to caller of the given UI gadget instance could use the function.
 *
 * \par Method of function operation:
 * Result callback which is registered by caller with ug_create() is invoked.
 *
 * \par Context of function:
 * This function supposed to be called in the created UI gadget.
 *
 * @param[in] ug the UI gadget
 * @param[in] result the result, which is service type (see \ref service_PG "Tizen managed api reference guide")
 * @return 0 on success, -1 on error
 *
 * \pre None
 * \post None
 * \see None
 * \remarks After send your message, you have to release it using service_destroy()
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget-module.h>
 * ...
 * // make a result with service
 * service_h result;
 * service_create(&result);
 * service_add_extra_data(result, "Content", "Hello");
 *
 * // send the result
 * ug_send_result(ug, result);
 *
 * // release the result
 * service_destroy(result);
 * ...
 * \endcode
 */
int ug_send_result(ui_gadget_h ug, service_h send);


/**
 * \par Description:
 * This function sends result to caller of the given UI gadget instance.
 *
 * \par Purpose:
 * This function is used for sending result to caller of the given UI gadget instance. The result have to be composed with service handle.
 *
 * \par Typical use case:
 * UI gadget developer who want to send result to caller of the given UI gadget instance could use the function.
 *
 * \par Method of function operation:
 * Result callback which is registered by caller with ug_create() is invoked.
 *
 * \par Context of function:
 * This function supposed to be called in the created UI gadget.
 *
 * @param[in] ug the UI gadget
 * @param[in] the service handle in which the results of the callee (see \ref service_PG "Tizen managed api reference guide")
 * @param[in] The result code of the launch request. (This is valid in case that ug is launched by appcontrol)
 * @return 0 on success, -1 on error
 *
 * \pre None
 * \post None
 * \see None
 * \remarks After send your message, you have to release it using service_destroy()
 *
 * \par Sample code:
 * \code
 * #include <ui-gadget-module.h>
 * ...
 * // make a result with service
 * service_h result;
 * service_create(&result);
 * service_add_extra_data(result, "Content", "Hello");
 *
 * // send the result
 * ug_send_result_full(ug, result, SERVICE_RESULT_SUCCEEDED);
 *
 * // release the result
 * service_destroy(result);
 * ...
 * \endcode
 */
int ug_send_result_full(ui_gadget_h ug, service_h send, service_result_e result);

#ifdef __cplusplus
}
#endif
/**
 * @}
 */
#endif				/* __UI_GADGET_MODULE_H__ */
