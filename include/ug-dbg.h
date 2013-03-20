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

#ifndef __UG_DBG_H__
#define __UG_DBG_H__

#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "UI_GADGET"

#define _ERR(fmt, arg...) LOGE("\x1b[31m" fmt "\x1b[0m", ##arg)
#define _DBG(fmt, arg...) LOGD("\x1b[32m" fmt "\x1b[0m", ##arg)
#define _INFO(fmt, arg...) LOGI("\x1b[33m" fmt "\x1b[0m", ##arg)
#define _WRN(fmt, arg...) LOGW("\x1b[34m" fmt "\x1b[0m", ##arg)

#endif				/* __UG_DBG_H__ */
