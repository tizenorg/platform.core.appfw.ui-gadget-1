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

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/types.h>

#include <aul.h>
#include <pkgmgr-info.h>

#include "ug-module.h"
#include "ug-dbg.h"

#include <tzplatform_config.h>

#define UG_MODULE_INIT_SYM "UG_MODULE_INIT"
#define UG_MODULE_EXIT_SYM "UG_MODULE_EXIT"

#define MEM_ADDR_LEN 8
#define MEM_ADDR_TOT_LEN 17

static char *__ug_module_get_addr(const char *ug_name)
{
	FILE *file;
	char buf[PATH_MAX] = {0,};
	char mem[PATH_MAX] = {0,};

	char *token_param = NULL;
	char *saveptr = NULL;
	int cnt = 0;

	if (ug_name == NULL)
		goto func_out;

	snprintf(buf, sizeof(buf), "/proc/%d/maps", getpid());

	file = fopen(buf, "r");
	if (file == NULL) {
		_WRN("proc open fail(%d)", errno);
		goto func_out;
	}

	memset(buf, 0x00, PATH_MAX);

	while (fgets(buf, PATH_MAX, file) != NULL) {
		if (strstr(buf, ug_name)) {
			token_param = strtok_r(buf, " ", &saveptr);
			if ((token_param == NULL) || (strlen(token_param) > MEM_ADDR_TOT_LEN)) {
				_ERR("proc token param(%s) error", token_param);
				goto close_out;
			}

			if (cnt > 0) {
				memcpy((void *)(mem+MEM_ADDR_LEN+1),
					(const void *)(token_param+MEM_ADDR_LEN+1), MEM_ADDR_LEN);
			} else {
				memcpy((void *)mem, (const void *)token_param, strlen(token_param));
				cnt++;
			}
		} else {
			if (cnt > 0)
				goto close_out;
		}

		memset(buf, 0x00, PATH_MAX);
		saveptr = NULL;
	}

close_out:
	fclose(file);
	file = NULL;

func_out:
	if (strlen(mem) > 0)
		return strdup(mem);
	else
		return NULL;
}

static int __file_exist(const char *path)
{
	int ret;

	ret = access(path, R_OK);
	LOGD("ug_file(%s) check %s", path, ret ? "fail" : "ok");

	return ret;
}

static int __get_ug_info(const char *name, char **ug_file_path)
{
	char ug_file[PATH_MAX];
	char app_id[NAME_MAX];
	char *root_path;
	char *res_path = NULL;
	pkgmgrinfo_appinfo_h appinfo = NULL;

	/* get path using name(file name) */
	snprintf(ug_file, PATH_MAX, "%s/lib/libug-%s.so",
			tzplatform_getenv(TZ_SYS_RO_UG), name);
	if (!__file_exist(ug_file))
		goto out_func;
	snprintf(ug_file, PATH_MAX, "%s/lib/lib%s.so",
			tzplatform_getenv(TZ_SYS_RO_UG), name);
	if (!__file_exist(ug_file))
		goto out_func;

	/* get path using appid */
	if (aul_app_get_appid_bypid(getpid(), app_id, sizeof(app_id))) {
		LOGE("failed to get appid");
		return -1;
	}
	snprintf(ug_file, PATH_MAX, "%s/lib/libug-%s.so",
			tzplatform_getenv(TZ_SYS_RO_UG), app_id);
	if (!__file_exist(ug_file))
		goto out_func;
	snprintf(ug_file, PATH_MAX, "%s/lib/lib%s.so",
			tzplatform_getenv(TZ_SYS_RO_UG), app_id);
	if (!__file_exist(ug_file))
		goto out_func;

	/* get path using appid and root path */
	if (pkgmgrinfo_appinfo_get_usr_appinfo(app_id, getuid(), &appinfo)) {
		LOGE("failed to get app info");
		return -1;
	}
	if (pkgmgrinfo_appinfo_get_root_path(appinfo, &root_path)) {
		LOGE("failed to get app root path");
		pkgmgrinfo_appinfo_destroy_appinfo(appinfo);
		return -1;
	}
	snprintf(ug_file, PATH_MAX, "%s/lib/ug/libug-%s.so", root_path, app_id);
	if (!__file_exist(ug_file))
		goto out_func;
	snprintf(ug_file, PATH_MAX, "%s/lib/ug/lib%s.so", root_path, app_id);
	if (!__file_exist(ug_file))
		goto out_func;

	/* get path using appid and shared resource path */
	if (aul_get_app_shared_resource_path_by_appid(app_id, &res_path)) {
		LOGE("failed to get shared resource path");
		pkgmgrinfo_appinfo_destroy_appinfo(appinfo);
		return -1;
	}
	snprintf(ug_file, PATH_MAX, "%s/lib/ug/libug-%s.so", res_path, app_id);
	if (!__file_exist(ug_file))
		goto out_func;
	snprintf(ug_file, PATH_MAX, "%s/lib/ug/lib-%s.so", res_path, app_id);
	if (!__file_exist(ug_file))
		goto out_func;

out_func:
	if ((strlen(ug_file) > 0) && (ug_file_path))
		*ug_file_path = strdup(ug_file);

	free(res_path);
	if (appinfo)
		pkgmgrinfo_appinfo_destroy_appinfo(appinfo);

	return 0;
}

struct ug_module *ug_module_load(const char *name)
{
	void *handle;
	struct ug_module *module;
	int (*module_init) (struct ug_module_ops *ops);
	char *ug_file = NULL;

	if (__get_ug_info(name, &ug_file) < 0) {
		_ERR("error in getting ug file path");
		return NULL;
	}

	module = calloc(1, sizeof(struct ug_module));
	if (!module) {
		errno = ENOMEM;
		free(ug_file);
		return NULL;
	}

	handle = dlopen(ug_file, RTLD_LAZY);
	if (!handle) {
		_ERR("dlopen failed: %s", dlerror());
		goto module_free;
	}

	module_init = dlsym(handle, UG_MODULE_INIT_SYM);
	if (!module_init) {
		_ERR("dlsym failed: %s", dlerror());
		goto module_dlclose;
	}

	if (module_init(&module->ops))
		goto module_dlclose;

	module->handle = handle;
	module->module_name = strdup(name);

	module->addr = __ug_module_get_addr(name);

	free(ug_file);
	return module;

 module_dlclose:
	dlclose(handle);

 module_free:
	free(module);
	free(ug_file);
	return NULL;
}

int ug_module_unload(struct ug_module *module)
{
	void (*module_exit) (struct ug_module_ops *ops);

	if (!module) {
		errno = EINVAL;
		return -1;
	}

	if (module->handle) {
		module_exit = dlsym(module->handle, UG_MODULE_EXIT_SYM);
		if (module_exit)
			module_exit(&module->ops);
		else
			_ERR("dlsym failed: %s", dlerror());

		_DBG("dlclose(%s)", module->module_name);
		dlclose(module->handle);
		module->handle = NULL;
	}

	if (module->module_name)
		free(module->module_name);

	if (module->addr)
		free(module->addr);

	free(module);
	return 0;
}

int ug_exist(const char* name)
{
	int ret = 1;

	if (__get_ug_info(name, NULL) < 0) {
		_ERR("error in getting ug file path");
		ret = 0;
	}

	return ret;
}
