/*
 * Copyright (C) 2020 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include <fwupdplugin.h>

struct FuPluginData {
	GFile *file;
	GFileMonitor *monitor;
};

static void
fu_plugin_linux_tainted_init(FuPlugin *plugin)
{
	fu_plugin_alloc_data(plugin, sizeof(FuPluginData));
}

static void
fu_plugin_linux_tainted_destroy(FuPlugin *plugin)
{
	FuPluginData *data = fu_plugin_get_data(plugin);
	if (data->file != NULL)
		g_object_unref(data->file);
	if (data->monitor != NULL) {
		g_file_monitor_cancel(data->monitor);
		g_object_unref(data->monitor);
	}
}

static void
fu_plugin_linux_tainted_changed_cb(GFileMonitor *monitor,
				   GFile *file,
				   GFile *other_file,
				   GFileMonitorEvent event_type,
				   gpointer user_data)
{
	FuPlugin *plugin = FU_PLUGIN(user_data);
	FuContext *ctx = fu_plugin_get_context(plugin);
	fu_context_security_changed(ctx);
}

static gboolean
fu_plugin_linux_tainted_startup(FuPlugin *plugin, GError **error)
{
	FuPluginData *data = fu_plugin_get_data(plugin);
	g_autofree gchar *fn = NULL;
	g_autofree gchar *procfs = NULL;

	procfs = fu_common_get_path(FU_PATH_KIND_PROCFS);
	fn = g_build_filename(procfs, "sys", "kernel", "tainted", NULL);
	data->file = g_file_new_for_path(fn);
	data->monitor = g_file_monitor(data->file, G_FILE_MONITOR_NONE, NULL, error);
	if (data->monitor == NULL)
		return FALSE;
	g_signal_connect(data->monitor,
			 "changed",
			 G_CALLBACK(fu_plugin_linux_tainted_changed_cb),
			 plugin);
	return TRUE;
}

static void
fu_plugin_linux_tainted_add_security_attrs(FuPlugin *plugin, FuSecurityAttrs *attrs)
{
	FuPluginData *data = fu_plugin_get_data(plugin);
	gsize bufsz = 0;
	g_autofree gchar *buf = NULL;
	g_autoptr(FwupdSecurityAttr) attr = NULL;
	g_autoptr(GError) error_local = NULL;

	/* create attr */
	attr = fwupd_security_attr_new(FWUPD_SECURITY_ATTR_ID_KERNEL_TAINTED);
	fwupd_security_attr_set_plugin(attr, fu_plugin_get_name(plugin));
	fwupd_security_attr_add_flag(attr, FWUPD_SECURITY_ATTR_FLAG_RUNTIME_ISSUE);
	fu_security_attrs_append(attrs, attr);

	/* load file */
	if (!g_file_load_contents(data->file, NULL, &buf, &bufsz, NULL, &error_local)) {
		g_autofree gchar *fn = g_file_get_path(data->file);
		g_warning("could not open %s: %s", fn, error_local->message);
		fwupd_security_attr_set_result(attr, FWUPD_SECURITY_ATTR_RESULT_NOT_VALID);
		return;
	}
	if (g_strcmp0(buf, "0\n") != 0) {
		fwupd_security_attr_set_result(attr, FWUPD_SECURITY_ATTR_RESULT_TAINTED);
		return;
	}

	/* success */
	fwupd_security_attr_add_flag(attr, FWUPD_SECURITY_ATTR_FLAG_SUCCESS);
	fwupd_security_attr_set_result(attr, FWUPD_SECURITY_ATTR_RESULT_NOT_TAINTED);
}

void
fu_plugin_init_vfuncs(FuPluginVfuncs *vfuncs)
{
	vfuncs->build_hash = FU_BUILD_HASH;
	vfuncs->init = fu_plugin_linux_tainted_init;
	vfuncs->destroy = fu_plugin_linux_tainted_destroy;
	vfuncs->startup = fu_plugin_linux_tainted_startup;
	vfuncs->add_security_attrs = fu_plugin_linux_tainted_add_security_attrs;
}
