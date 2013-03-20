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

#include <app_manager.h>

#include "ug-module.h"
#include "ug-dbg.h"

#define UG_MODULE_INIT_SYM "UG_MODULE_INIT"
#define UG_MODULE_EXIT_SYM "UG_MODULE_EXIT"

static int file_exist(const char *filename)
{
	FILE *file;
	if ((file = fopen(filename, "r"))) {
		fclose(file);
		return 1;
	}

	return 0;
}

struct ug_module *ug_module_load(const char *name)
{
	void *handle;
	struct ug_module *module;
	char ug_file[PATH_MAX];
	char *pkg_name = NULL;

	int (*module_init) (struct ug_module_ops *ops);

	module = calloc(1, sizeof(struct ug_module));
	if (!module) {
		errno = ENOMEM;
		return NULL;
	}

	app_manager_get_package(getpid(), &pkg_name);

	do {
		if (pkg_name) {
			snprintf(ug_file, PATH_MAX, "/usr/apps/%s/lib/libug-%s.so", pkg_name, name);
			if (file_exist(ug_file))
				break;
			snprintf(ug_file, PATH_MAX, "/opt/apps/%s/lib/libug-%s.so", pkg_name, name);
			if (file_exist(ug_file))
				break;
		}
		snprintf(ug_file, PATH_MAX, "/usr/ug/lib/libug-%s.so", name);
		if (file_exist(ug_file))
			break;
		snprintf(ug_file, PATH_MAX, "/opt/usr/ug/lib/libug-%s.so", name);
		if (file_exist(ug_file))
			break;
	} while (0);

	if(pkg_name) {
		free(pkg_name);
		pkg_name = NULL;
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
	return module;

 module_dlclose:
	dlclose(handle);

 module_free:
	free(module);
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

		dlclose(module->handle);
		module->handle = NULL;
	}

	if(module->module_name)
		free(module->module_name);

	free(module);
	return 0;
}
