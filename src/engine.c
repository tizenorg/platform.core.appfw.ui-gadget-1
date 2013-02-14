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

#include "ug-engine.h"
#include "ug-dbg.h"

#define UG_ENGINE_INIT_SYM "UG_ENGINE_INIT"
#define UG_ENGINE_EXIT_SYM "UG_ENGINE_EXIT"

enum ug_engine_type {
	UG_ENGINE_EFL = 0x00,
};

static int file_exist(const char *filename)
{
	FILE *file;

	file = fopen(filename, "r");
	if (file)
	{
		fclose(file);
		return 0;
	}
	return -1;
}

struct ug_engine *ug_engine_load()
{
	void *handle;
	struct ug_engine *engine;
	char engine_file[PATH_MAX];
	enum ug_engine_type type = UG_ENGINE_EFL;
	int (*engine_init)(struct ug_engine_ops *ops);

	engine = calloc(1, sizeof(struct ug_engine));

	if (!engine) {
		errno = ENOMEM;
		return NULL;
	}

	if (type == UG_ENGINE_EFL) { /* UG_ENGINE_EFL is default*/
		if (snprintf(engine_file, PATH_MAX, "/usr/lib/libui-gadget-1-efl-engine.so") < 0){
			goto engine_free;
		}
		else if (file_exist(engine_file) < 0) {
			goto engine_free;
		}
	}
	else
		goto engine_free;

	handle = dlopen(engine_file, RTLD_LAZY);
	if (!handle) {
		_ERR("dlopen failed: %s", dlerror());
		goto engine_free;
	}

	engine_init = dlsym(handle, UG_ENGINE_INIT_SYM);
	if (!engine_init) {
		_ERR("dlsym failed: %s", dlerror());
		goto engine_dlclose;
	}

	if (engine_init(&engine->ops))
		goto engine_dlclose;

	engine->handle = handle;
	return engine;

engine_dlclose:
	dlclose(handle);

engine_free:
	free(engine);
	return NULL;
}

int ug_engine_unload(struct ug_engine *engine)
{
	void (*engine_exit)(struct ug_engine_ops *ops);

	if (!engine) {
		errno = EINVAL;
		return -1;
	}

	if (engine->handle) {
		engine_exit = dlsym(engine->handle, UG_ENGINE_EXIT_SYM);
		if (engine_exit)
			engine_exit(&engine->ops);
		else
			_ERR("dlsym failed: %s", dlerror());

		dlclose(engine->handle);
	}

	free(engine);
	engine = NULL;
	return 0;
}
