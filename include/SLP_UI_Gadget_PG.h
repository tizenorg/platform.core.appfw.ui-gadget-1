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

/**
 *
 * @ingroup SLP_PG
 * @defgroup SLP_PG_UI_GADGET UI gadget library
 * @{

<h1 class="pg">Introduction</h1>
<h2 class="pg">Purpose of this document</h2>
The purpose of this document is to describe how to develop/use UI gadget. This document gives programming guidelines to UI gadget devlopers and users.
<h2 class="pg">Scope</h2>
The scope of this document is limited to UI gadget component interface and API usage.

<h1 class="pg">UI gadget architecture</h1>
<h2 class="pg">UI gadget</h2>
An UI gadget is a visual component providing views (or features) of other applications, e.g., contact, image viewer. Because UI gadgets are supposed to deliver most commonly used features that the platform can natively support, developers can avoid unnecessary massive code writing using UI gadgets. Hence an UI gadget eventually includes logics to handle simple request. UI gadget is able to be managed by UI gadget library.
\image html SLP_UI_Gadget_PG_image00.png "Picture 1. UI gadget architecture diagram"

<h2 class="pg">UI gadget features</h2>
UI Gadget Library has the following features:
- It provides component interfaces for UI Gadget
- It manages UI gadget instances in an application according to their lifecycle

<h2 class="pg">Lifecycle</h2>
Essentially, an UI gadget has following five states (See Picture 2)
- The initial state is \b Ready
- If an UI gadget has been created, it is \b Created
- If an UI gadget has been started, it is \b Running
- If the application that is using an UI gadget is put into background, it is \b Stopped
- If an UI gadget has been destroyed, it is \b Destroyed

An UI gadget has five callback methods that you can implement to perform operations when the UI gadget moves between states
- When an UI gadget is created, \b create() is invoked
- When an UI gadget is started, \b start() is invoked
- When the application that is using an UI gadget is put into background, \b pause() is invoked
- When the application that is using an UI gadget is brought to the foreground, \b resume() is invoked.
- When an UI gadget is destroyed, \b destroy() is invoked

In addition, an UI gadget has callback methods for system events and message:
- When an system event is generated, event() is invoked
- When an UI gadget receives message from the caller, message() is invoked

\image html SLP_UI_Gadget_PG_image01.png "Picture 2. UI gadget state diagram"

<h2 class="pg">Management</h2>
UI gadgets in an application are managed as a TREE structure (See Picture 3.) The features for the tree are:
- Root of the tree is the UI gadget manager
- UI gadget caller is parent of callees
- Parents arrange the layout of their children

Every application which is using UI gadgets has one UI gadget manager as a root of the tree. And the UI gadget manager propagates system events and task management events by post-order traversal. Available system events are <i>low memory, low battery, language changed, region changed and window rotate event</i>. And task management events are <i>pause and resume</i>.

\image html SLP_UI_Gadget_PG_image02.png "Picture 3. UI gadget management policy"

<h1 class="pg">Getting started</h1>
<h2 class="pg">Overview</h2>
For using UI gadget, you need to know the working procedure between caller and UI gadget module.<br><br>
As an UI gadget module developer, the operations of UI gadget module are very important. These operations are called by UI gadget library when the caller requests something to UI gadget module and UI gadget module performs a proper job mainly related with lifecycle. “Section How to make UI gadget” describes detail of each operation.<br><br>
As an UI gadget caller, you need to know the APIs for using UI gadget module and callback functions for dealing with UI gadget module. The caller can create UI gadget module using ug_create() API and have right to arrange layout of UI gadget module and destroy UI gadget module. These are performed by caller when caller requests something to UI gadget library and related callback functions are called by UI gadget library. “Section How to use UI gadget” describes detail of each APIs and callback functions.
<h2 class="pg">How to make UI gadget</h2>
In this section, we are going to write your first UI gadget called "helloUG-efl". Before we get started, make sure you have read the overview, especially, lifecycle section. We will mainly deal with the operations of lifecycle.

\note <b>Sample codes</b> are included in the UI gadget source package. The samples for UI gadget developers are located in "samples/helloUG-efl/", and the samples for UI gadget users are in "samples/ugcaller/."

<br>
<h3 class="pg">UI gadget template</h3>
To create an UI gadget, start by generating boilerplate code using UI gadget template as follow:
@verbatim
# ug-gen.sh helloUG-efl helloUG-efl EFL
@endverbatim

\note <b>How to install UI gadget template:</b>
@verbatim
# unrpm ui-gadget-template-xxx.rpm
# or
# rpm -Uvh ui-gadget-templat-xxx.rpm
@endverbatim

\note <b>How to use UI gadget template:</b>
@verbatim
# ug-gen.sh [destination] [name] [UI library]
@endverbatim
- destination: destination directory
- name: UI gadget name
- UI library: UI library to use. Only EFL is available for now

After you generate code, you get following files:
- <i>helloUG-efl.c</i>	(Source)
- <i>helloUG-efl.h</i>	(Private header)
- <i>CMakeList.txt</i>	(Build script)
- <i>po/*</i>		(I18N files)

<i>helloUG-efl.c</i> contains base code, and the most important parts are <i>UG_MODULE_INIT</i> and <i>UG_MODULE_EXIT</i> which are symbols to export for dynamic linking. <i>UG_MODULE_INIT</i> is invoked when the UI gadget is loading, and it sets operations, private data, and the option. <i>UG_MODULE_EXIT</i> is invoked when the UI gadget is unloading, and it clears private data.<br><br>
Even if you don't understand generated code right now, don't worry about it. What you have to do is just implementation of operations according to their role (see next section.)
@code
// in helloUG-efl.c
UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	struct ug_data *ugd;		// User defined private data

	if (!ops)
		return -1;

	ugd = calloc(1, sizeof(struct ug_data));
	if (!ugd)
		return -1;

	// create operation
	ops->create = on_create;
	// start operation
	ops->start = on_start;
	// pause operation
	ops->pause = on_pause;
	// resume operation
	ops->resume = on_resume;
	// destroy operation
	ops->destroy = on_destroy;
	// message operation
	ops-> message = on_message;
	// event operation
	ops->event = on_event;
	// private data
	ops->priv = ugd;
	// option
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	struct ug_data *ugd;

	if (!ops)
		return;

	ugd = ops->priv;
	if (ugd)
		free(ugd);		// clear private data
}

@endcode

\note <b>struct ug_module_ops</b> is a data structure describing operations, private data, and the option of UI gadget:
@code
struct ug_module_ops {
	void *(*create)(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv);
	void (*start)(ui_gadget_h ug, service_h service, void *priv);
	void (*pause)(ui_gadget_h ug, service_h service, void *priv);
	void (*resume)(ui_gadget_h ug, service_h service, void *priv);
	void (*destroy)(ui_gadget_h ug, service_h service, void *priv);
	void (*message)(ui_gadget_h ug, service_h *msg, service_h service, void *priv);
	void (*event)(ui_gadget_h ug, enum ug_event event, service_h service, void *priv);
	void *reserved[5];
	void *priv;
	enum ug_option opt;
};
@endcode

\note <b>enum ug_option</b> is UI gadget options, available options are:
@code
// Enable indicator
UG_OPT_INDICATOR_ENABLE
// Enable indicator with portrait window
UG_OPT_INDICATOR_PORTRAIT_ONLY
// Enable indicator with landscape window
UG_OPT_INDICATOR_LANDSCAPE_ONLY
// Disable indicator
UG_OPT_INDICATOR_DISABLE
// current indicator status will be held
UG_OPT_INDICATOR_MANUAL
@endcode

\note <b>struct ug_data</b> is a user defined private data structure describing base layout, own UI gadget handler, and whatever you need:
@code
struct ug_data {
	Evas_Object *base;
	ui_gadget_h ug;

	// PUT WHATEVER YOU NEED
}
@endcode

<br>
<h3 class="pg">Operations</h3>
There are five state operations, a message operation, and an event operation: <i>create, start, pause, resume, destroy, message, and event.</i>
<br><br>
When "helloUG-efl" is created, the create operation is invoked (See Picture 2-1).<br><br>

\image html SLP_UI_Gadget_PG_image2-1.png "Picture 2-1. Create operation of UI gadget module"

The implementation of create operation is <b>on_create()</b>. Basically, in the operation, we have to make a base layout and return it. Hence, we made base layout using <i>"window layout winset."</i> In case of fullview, we let indicator area be shown, otherwise, we don't (see <i>create_fullview()</i> and <i>create_frameview()</i>.) In addition, in the base layout, we put a box including a label and two buttons (see <i>create_content()</i>.) The label is labeled "Hello UI Gadget." And the first button, labeled "Send result", is for sending result to the "helloUG-efl" caller. The other button, labeled "Back", is for sending destroy request to the caller. For more information about two buttons, please see <i>Send results and request to destroy section</i>.

\note <b>Arguments:</b> All operations receive servive type data which is named <i>service</i> (see \ref service_PG "Tizen Managed APi Reference Guide > Application Framework -> Application") And the argument <i>service</i> is automatically released by UI gadget manager after the UI gadget is destroyed.

@code
// in helloUG-efl.c
static void *on_create(ui_gadget_h ug, enum ug_mode mode, service_h service, void *priv)
{
	Evas_Object *parent;
	Evas_Object *content;
	struct ug_data *ugd;

	if (!ug || !priv)
		return NULL;

	ugd = priv;
	ugd->ug = ug;

	parent = ug_get_parent_layout(ug);
	if (!parent)
		return NULL;

	if (mode == UG_MODE_FULLVIEW)
		ugd->base = create_fullview(parent, ugd);
	else
		ugd->base = create_frameview(parent, ugd);

	if (ugd->base) {
		content = create_content(parent, ugd);
		elm_layout_content_set(ugd->base, "elm.swallow.content", content);
	}
	return ugd->base;
}

static Evas_Object *create_fullview(Evas_Object *parent, struct ug_data *ugd)
{
	Evas_Object *base;

	base = elm_layout_add(parent);
	if (!base)
		return NULL;
	elm_layout_theme_set(base, "layout", "application", "default");
	// In case of fullview, show indicator area
	edje_object_signal_emit(_EDJ(base), "elm,state,show,indicator", "elm");
	edje_object_signal_emit(_EDJ(base), "elm,state,show,content", "elm");

	return base;
}

static Evas_Object *create_frameview(Evas_Object *parent, struct ug_data *ugd)
{
	Evas_Object *base;

	base = elm_layout_add(parent);
	if (!base)
		return NULL;

	elm_layout_theme_set(base, "layout", "application", "default");
	// In case of frameview, do not show indicator area
	edje_object_signal_emit(_EDJ(base), "elm,state,show,content", "elm");

	return base;
}

static Evas_Object *create_content(Evas_Object *parent, struct ug_data *ugd)
{
	Evas_Object *bx, *eo;

	// add box
	bx = elm_box_add(parent);

	// add label and pack it in the box
	eo = elm_label_add(parent);
	elm_object_text_set(eo, _("Hello UI Gadget"));
	evas_object_size_hint_align_set(eo, 0.5, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(eo);
	elm_box_pack_end(bx, eo);

	// add buttons and pack it in the box
	eo = elm_button_add(parent);
	elm_object_text_set(eo, _("Send result"));
	evas_object_size_hint_align_set(eo, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(eo, "clicked", result_cb, ugd);
	elm_object_style_set(eo, "bottom_btn");

	evas_object_show(eo);
	elm_box_pack_end(bx, eo);

	eo = elm_button_add(parent);
	elm_object_text_set(eo, _("Back"));
	evas_object_size_hint_align_set(eo, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(eo, "clicked", back_cb, ugd);
	elm_object_style_set(eo, "bottom_btn");

	evas_object_show(eo);
	elm_box_pack_end(bx, eo);
	return bx;
}

@endcode

When "helloUG-efl" starts, the start operation is invoked (See Picture 2-2).<br><br>

\image html SLP_UI_Gadget_PG_image2-2.png "Picture 2-2. Start operation of UI gadget module"

The implementation of start operation is <b>on_start()</b>. Usually every job would be completed before this operation, so just describe additional job you need to do in this operation.

@code
// in helloUG-efl.c
static void on_start(ui_gadget_h ug, service_h service, void *priv)
{

}
@endcode

When "helloUG-efl" is destroyed, the destroy operation is invoked(See Picture 2-3). Ui gadget cannot destroy itself, so it moust be destroyed by caller using ug_destroy().<br><br>

\image html SLP_UI_Gadget_PG_image2-3.png "Picture 2-3. Destroy operation of UI gadget module"

The implementation of destroy operation is <b>on_destroy()</b>. We usually release the resources that have been used. In the following method, we delete base layout:

@code
// in helloUG-efl.c
static void on_destroy(ui_gadget_h ug, service_h service, void *priv)
{
	struct ug_data *ugd;

	if (!ug || !priv)
		return;

	ugd = priv;

	evas_object_del(ugd->base);
	ugd->base = NULL;
}
@endcode

When the application using "helloUG-efl" is put into background, the pause operation is invoked (See Picture 2-4). But UI gadget could get state only from caller, so pause operation (e.g., on_pause) is called by caller using ug_pause().<br><br>

\image html SLP_UI_Gadget_PG_image2-4.png "Picture 2-4. Pause operation of UI gadget module"

When the application is brought to the foreground, the resume operation is invoked (See Picture 2-5). Resume operation (e.g.,on_resume) is called by caller using ug_resume().<br><br>

\image html SLP_UI_Gadget_PG_image2-5.png "Picture 2-5. Resume operation of UI gadget module"

Besides, when an UI gadget receives message from its caller suing ug_send_message(), the message operation(e.g., on_message) is invoked (See Picture 2-6).<br><br>

\image html SLP_UI_Gadget_PG_image2-6.png "Picture 2-6. Send message to UI gadget module"

And when a system event is generated and UI gadget receives event from caller using ug_send_event(), the event operation(e.g., on_event) is invoked (See Picture 2-7).<br><br>

\image html SLP_UI_Gadget_PG_image2-7.png "Picture 2-7. Send system event to UI gadget module"

The implementation of pause, resume, message, and event operations are on_pause(), on_resume(), on_message(), and on_event(). In on_pause() and on_resume(), you can describe actions performed when a state is changed to pause or resume. For example, music player UI gadget can stop playing music or restart playing music in these operations. In on_message(), you can get service type data from caller and deal with it. In on_event(), you can describe a proper job related to the passed system event.

@code
// in helloUG-efl.c
static void on_pause(ui_gadget_h ug, service_h service, void *priv)
{
       // Do what you need to do when paused.
}
static void on_resume(ui_gadget_h ug, service_h service, void *priv)
{
       // Do what you need to do when paused.
}

static void on_message(ui_gadget_h ug, service msg, service_h service, void *priv)
{
       // Do what you need to do when paused.
}

static void on_event(ui_gadget_h ug, enum ug_event event, service_h service, void *priv)
{
	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		break;
	case UG_EVENT_LOW_BATTERY:
		break;
	case UG_EVENT_LANG_CHANGE:
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
		break;
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
		break;
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		break;
	case UG_EVENT_REGION_CHANGE:
		break;
	default:
		break;
	}
}
@endcode

\warning Message data of message operation is service type data, named <i>msg.</i> <b>Because the message data is released after message operation is finished,</b> if you want to keep using it, please use <b>service_clone()()</b> which duplicates given service data (see \ref service_PG "Tizen Managed API Reference Guide")

<br>
<h3 class="pg">Send results and destroy request</h3>
Usually, an UI gadget needs to send results or destroy request to the UI gadget caller.<br><br>
To send result, use <b>ug_send_result()</b>, then UG library calls result callback function registered by caller (See Picture 2-8)(See <i>API reference guide</i>).<br><br>

\image html SLP_UI_Gadget_PG_image2-8.png "Picture 2-8. Send result"

And to send the destroy request, use <b>ug_destroy_me().</b>(), then UG library calls destroy callback function registered by caller (See Picture 2-9) (See <i>API reference quide</i>).<br><br>

\image html SLP_UI_Gadget_PG_image2-9.png "Picture 2-9. UI gadget destroy request"

We use service library for composing result data. The service provides us a few APIs to make a list of dictionary data that consists of key and value. (ex. {"name"  "John Doe"}) To get more information of service, please see \ref service_PG "Tizen Managed API Reference Guide".

\warning After send your result data, you have to release it using <b>service_destroy()</b> API.

In our "helloUG-efl", we made two buttons for sending results and destroy request as below:
@code
// in helloUG-efl.c

//Include to use service APIs
#include <app.h>

static void result_cb(void *data, Evas_Object *obj, void *event_info)
{
	service_h result;
	struct ug_data *ugd;
	int ret;

	if (!data)
		return;

	ugd = data;

	ret = service_create(&result);

	service_add_extra_data(result, "name", "hello-UG");
	service_add_extra_data(result, "description", "sample UI gadget");

	ug_send_result(ugd->ug, result);

	// release service
	service_destroy(result);
}

static void back_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct ug_data *ugd;
	int i;

	if (!data)
		return;

	ugd = data;

	//Send destroy request
	ug_destroy_me(ugd->ug);
}
@endcode

\note <b>To use service</b>
- Install capi-appfw-application-dev package (add in your RPM spec file)
- Modify CMakeFile.txt to use capi package as follow:
@code
	…
		pkg_check_modules(pkgs REQUIRED elementary ui-gadget-1 capi-appfw-application)
	…
@endcode

<br>
<h3 class="pg">Internationalization</h3>
Basically, we use <b><i>dgettext</i></b> for translating a text string into the user's native language because each UI gadget uses different textdomain.
@code
// in helloUG-efl.h
#define PKGNAME			"ug-helloUG-efl"
#define _(s)			dgettext(PKGNAME, s)
#define dgettext_noop(s)	(s)
#define N_(s)			dgettext_noop(s)
@endcode

The PKGNAME is textdomain of "helloUG-efl", and _() is a dgettext wrapper, and N_() is dummy macro. In addition, _() and N_() are additional keywords for marking translatable string for xgettext. Especially, N_() is a dummy keyword for special case as follow:
@code
static const char *message[] = {
	N_("translatable string"),
};
@endcode

For more information, please see <a href="http://www.gnu.org/software/gettext/manual/gettext.html">GNU gettext utilities</a>.

\note <b>xgettext</b> extracts gettext strings from given input files. The canonical keyword for marking translatable strings is 'gettext'. For convenience, many packages use '_' as a keyword instead of 'gettext', and write '_("translatable string")' instead of 'gettext("translatable string")'.

<br>
<h3 class="pg">Rotation and indicator</h3>
When the UI gadget is created as fullview, we have to consider whether the indicator is shown or not. For instance, "Image viewer" shows the indicator on the portrait mode but not on the landscape mode. Hence, we provided option field named <i>opt</i> of <i>struct ug_module_ops</i> in UG_MODULE_INIT.
Available options are as following:
- UG_OPT_INDICATOR_ENABLE (default)
- UG_OPT_INDICATOR_POTRAIT_ONLY
- UG_OPT_LANDSCAPE_ONLY
- UG_OPT_INDICATOR_DISABLE
- UG_OPT_INDICATOR_MANUAL

And we used UG_OPT_INDICATOR_POTRAIT_ONLY in "helloUG-efl"

<br>
<h3 class="pg">Build and test</h3>
Before you build, you have to make sure whether translatable strings exist or not. IF translatable strings EXIST, please follow these steps before you build:
@verbatim
# cd po
# ./update-po.sh
# cd ..
@endverbatim
IF NOT, please remove the following line in your CMakeList.txt
@verbatim
ADD_SUBDIRECTORY(po)
@endverbatim

To build "helloUG-efl", follow these steps:
@verbatim
# mkdir build
# cd build
# cmake -DCMAKE_INSTALL_PREFIX=/usr ..
# make
# make install
or
make home project for your UI gadget in obs.
you can find RPM spec file in packaging directory by template.
@endverbatim

\note <b>Naming rule:</b> The output library name is <b>"libug-helloUG-efl.so"</b>, and we use <b>"helloUG-efl"</b> as UI gadget name except prefix <b>"libug-"</b> and postfix ".so" In other word, when you make an UI gadget, the name of library MUST be <b>"libug-XXXXXX.so"</b>
\note <b>Installation directory:</b> UI gadgets MUST be installed in "/usr/lib/ug/" which is preload Ug and "/opt/lib/ug/" is downloaded UG

Finally, we made our first UI gadget, "helloUG-efl." Let's test it using <i>ug-launcher</i> which is simple UI gadget launcher.
Fullview test
@verbatim
# ug-launcher -n helloUG-efl
@endverbatim
Frameview test
@verbatim
# ELM_THEME=beat ug-launcher -n helloUG-efl -f
@endverbatim

\note <b>How to use UG launcher</b>
@verbatim
# ug-launcher [-F] [-f] -n <UG_NAME> [-d <Argument>]
@endverbatim
- -d: argument, key, value pair.
- -F: Fullview mode (default)
- -f: frameview mode
\note <b> Example: </b>
@verbatim
# ug-launcher -F -n helloUG-efl -d "name,John doe" -d "age,30"
@endverbatim

<br>
<h2 class="pg">How to use UI gadget</h2>
Now, we are going to use "helloUG-efl" of previous section.

<br>
<h3 class="pg">Initialize</h3>

If you are UI gadget developer who is trying to use UI gadgets, please skip this section. This section is for application developers who use UI gadgets.
You have to initialize UI gadget manager before you use UI gadgets. To initialize, use <b>ug_init()</b> with arguments: <i>disp, xid, win and opt</i>. <i>disp</i> is default display, and <i>xid</i> is X window id of win. <i>win</i> is window evas object for UI gadgets, and it is usually main window. <i>opt</i> is rotation and indicator option for your application (see <i>Rotation and indicator section</i>.)

The <i>disp</i> and <i>xid</i> are used for indicator management. If you don't know how to get display and X window ID, just use following macro: <b>UG_INIT_EFL(win, opt);</b>

\note <b>Prototype of ug_init() (See API Reference guide):</b>
@code
int ug_init (Display *disp, Window xid, void *win, enum ug_option opt);
@endcode
\note <b>Macros for convenience (see 3 API reference quide):</b>
@code
UG_INIT_EFL(win, opt);
@endcode

<br>
<h3 class="pg">Create UI gadget instance</h3>

To create UI gadget instance, you have to invoke <b>ug_create()</b> which has five arguments: <i>parent, name, mode, service, and cbs.</i>

First, the <i>parent</i> is provided for specifying parent UI gadget, and it helps UI gadget manager to manage UI gadget tree (see <i>Management section.</i>) For instance, if the UI gadget 'A' uses other UI gadgets,  the parent has to be the 'A.' Otherwise, if an application uses UI gadgets, the <i>parent</i> has to be NULL.

Second, the <i>name</i> is the UI gadget's name (ex. "helloUG-efl")

Third, the <i>mode</i> could be UG_MODE_FULLVIEW to show the UI gadget as fullview, or UG_MODE_FRAMEVIEW to show it as frameview.

Fourth, the <i>service</i> is arguments for the UI gadget which is service type (see \ref service_PG "Tizen Managed API Reference Guide")

\warning After create UI gadget, you have to release the argument using <b>service_destroy()</b> API.

Fifth, the <i>cbs</i> is data describing layout callback, result callback, destroy callback, and private data. In detail, layout callback is used for layout arrangement, and it invoked after the UI gadget is created, and result callback is invoked to receive result from the UI gadget. And destroy callback is invoked to deal with destroy request from the UI gadget.

\warning Result data of the result callback is service type data, named <i>result</i>. <b>Because the result data is released after result callback is finished</b>, if you want to keep using it, please use <b>service_clone()</b> which duplicates given service data (see \ref service_PG "Tizen Managed API Reference Guide")

Using ug_create(), you can create UI gadget. After UI Gadget create operation is completed, layout callback function is called with base layout for layout arrangement (See Picture 2-11).<br><br>

\image html SLP_UI_Gadget_PG_image2-11.png "Picture 2-11. Create UI gadget"

The registered callback functions like result_cb or destroy_cb by caller is called when the UI gadget request sending result or destroy operation. <br><br>
When UI gadget requests to UI gadget library for sending result to caller using ug_send_result(), UI gadget library calls result callback function which is registered by caller(See Picture 2-12).<br><br>

\image html SLP_UI_Gadget_PG_image2-12.png "Picture 2-12. Send result"

And using ug_destroy_me(), UI gadget sends the destroy request to caller (See Picture 2-13). UI gadget cannot destroy itself, so it should request to caller.<br><br>

\image html SLP_UI_Gadget_PG_image2-13.png "Picture 2-13. Destroy request"

\note <b>Prototype of ug_create() (See API reference guide):</b>
@code
ui_gadget_h ug_create (ui_gadget_h parent,
			const char *name,
			enum ug_mode mode,
			service_h service,
			struct ug_cbs *cbs);

\note <b>struct ug_cbs</b> is describing some callbacks and private data:
@code
struct ug_cbs {
	void (*layout_cb)(ui_gadget_h ug, enum ug_mode mode, void *priv);
	void (*result_cb)(ui_gadget_h ug, service result, void *priv);
	void (*destroy_cb)(ui_gadget_h ug, void *priv);
	void *priv;
};
@endcode

Here are some examples:

@code
// FULLVIEW example
struct my_data {
	ui_gadget_h ug;
};

static void layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	Evas_Object *base, *win;

	if (!ug || !priv)
		return;

	base = ug_get_layout(ug);
	if (!base)
		return;

	switch (mode) {
	case UG_MODE_FULLVIEW:
		win = ug_get_window();
		if (!win)
			return;
		evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		ug_disable_effect(ug);
		evas_object_show(base);
		break;
	default:
		break;
	}
}

static void result_cb(ui_gadget_h ug, service result, void *priv)
{
	struct my_data *mydata;
	const char *val;

	if (!ug || !priv)
		return;

	mydata = priv;
	if (result) {
		service_get_extra_data(result, "name", val);
		if (val)
			fprintf(stderr, "The name of UI gadget that sends result is %s\n", val);
	}
	ug_destroy(ug);
	mydata->ug = NULL;

}

static void destroy_cb(ui_gadget_h ug, void *priv)
{
	struct my_data *mydata;

	if (!ug || !priv)
		return;

	mydata = priv;

	ug_destroy(ug);
	mydata->ug = NULL;
}

ui_gadget_h create_ug(struct my_data *data)
{
	ui_gadget_h ug;
	struct ug_cbs cbs = {0, };

	cbs.layout_cb = layout_cb;
	cbs.result_cb = result_cb;
	cbs.destroy_cb = destroy_cb;
	cbs.priv = (void *)data;

	ug = ug_create(NULL, "helloUG-efl", UG_MODE_FULLVIEW, NULL, &cbs);

	return ug;
}
@endcode

@code
// FRAMEVIEW example
struct my_data {
	ui_gadget_h ug;
	Evas_Object *main_layout;
};

static void layout_cb(ui_gadget_h ug, enum ug_mode mode, void *priv)
{
	Evas_Object *base, *win;
	struct my_data *mydata;

	if (!ug || !priv)
		return;

	mydata = priv;

	base = ug_get_layout(ug);
	if (!base)
		return;

	switch (mode) {
		case UG_MODE_FRAMEVIEW:
			elm_layout_content_set(mydata->main_layout, "content", base);
			break;
		default:
			break;
	}
}

static void result_cb(ui_gadget_h ug, service result, void *priv)
{
	struct my_data *mydata;
	const char *val;

	if (!ug || !priv)
		return;

	mydata = priv;

	if (result) {
		service_get_extra_data(result, "name", val);
		if (val)
			fprintf(stderr, "The name of UI gadget that sends result is %s\n", val);
	}

	ug_destroy(ug);
	mydata->ug = NULL;

}

static void destroy_cb(ui_gadget_h ug, void *priv)
{
	struct my_data *mydata;

	if (!ug || !priv)
		return;

	mydata = priv;

	ug_destroy(ug);
	mydata->ug = NULL;
}

ui_gadget_h create_ug(struct my_data *data)
{
	ui_gadget_h ug;
	struct ug_cbs cbs = {0, };

	cbs.layout_cb = layout_cb;
	cbs.result_cb = result_cb;
	cbs.destroy_cb = destroy_cb;
	cbs.priv = (void *)data;

	ug = ug_create(NULL, "helloUG-efl", UG_MODE_FRAMEVIEW, NULL, &cbs);

	return ug;
}
@endcode

<br>
<h2 class="pg">Send message</h2>

We provide API for sending message: <b>ug_send_message()</b>. When you send a message, you have to use service type data (see \ref service_PG "Tizen Managed API Reference Guide"). (See Picture 2-14)

\note <b>Prototype of ug_send_message() (See API reference guide):</b>
@code
int ug_send_message (ui_gadget_h ug, service msg);
@endcode

\image html SLP_UI_Gadget_PG_image2-14.png "Picture 2-14. Send message"

\warning After send your message, you have to release it using <b>service_destroy()</b> API.
@code
//example
	service_h msg;

	ret = service_create(&msg);

	service_add_extra_data(msg, "name", "hello-UG");
	service_add_extra_data(msg, "description", "sample UI gadget");

	//Send message
	ug_send_message(ug, msg);

	//release service
	service_destroy(msg);
@endcode

<br>
<h2 class="pg">Event propagation</h2>

If you are UI gadget developer who is trying to use UI gadgets, please skip this section. This section is for application developers who use UI gadgets.

We provide some APIs for event propagation: <b>ug_pause(), ug_resume(), and ug_send_event()</b>. <b>ug_pause()</b> and <b>ug_resume()</b> are used for task-managing. If the application is put into background, invoke <b>ug_pause()</b>, otherwise, if the application is brought to the foreground, invoke <b>ug_resume()</b>. <b>ug_send_event()</b> is used for system event: <i>low memory, low battery, language change, region change, rotate portrait, rotate portrait upside-down, rotate landscape, and rotate landscape upside-down.</i>

\note <b>Prototype of ug_pause(), ug_resume(), and ug_send_event() (See API reference guide):</b>
@code
int ug_pause (void);
int ug_resume (void);
int ug_send_event (enum ug_event event);
@endcode

\image html SLP_UI_Gadget_PG_image2-15.png "Picture 2-15. Pause event propagation"
\image html SLP_UI_Gadget_PG_image2-16.png "Picture 2-16. Resume event propagation"
\image html SLP_UI_Gadget_PG_image2-17.png "Picture 2-17. System event propagation"

<br>
<h2 class="pg">Destroy all UI gadgets</h2>

If you are UI gadget developer who is trying to use UI gadgets, please skip this section. This section is for application developers who use UI gadgets.

When you terminate your application, destroy all UI gadgets using <b>ug_destroy_all()</b>.

\note <b>Prototype of ug_destroy_all(See API reference guide): </b>
@code
int ug_destroy_all (void);
@endcode

<br>
<h2 class="pg">Disable UI gadget effect</h2>

We provide API for disabling transition effect: <i>ug_disable_effect</i>. If you want to disable showing/hiding transition effect when create and destroy a UI Gadget, <i>ug_disable_effect</i> should be called before show UI Gadget layout.

\note <b>Prototype of ug_disable_effect(See API reference guide): </b>
@code
int ug_disable_effect (void);
@endcode

Here are some examples:

@code
To disable transition effect :

	static void layout_cb(struct ui_gadget *ug, enum ug_mode mode, void *priv)
	{
		Evas_Object *base, *win;
		if (!ug || !priv)
			return;
		base = ug_get_layout(ug);
		if (!base)
			return;

		win = ug_get_window();
		switch (mode) {
		case UG_MODE_FULLVIEW:
			evas_object_size_hint_weight_set(base, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			// disable effect
			ug_disable_effect(ug);
			evas_object_show(base);
			break;
		default:
			break;
        }
@endcode

 * @}
 */
