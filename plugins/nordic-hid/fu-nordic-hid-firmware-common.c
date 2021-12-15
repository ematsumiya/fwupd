/*
 * Copyright (C) 2021 Denis Pynkin <denis.pynkin@collabora.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include <fwupdplugin.h>

#include "fu-nordic-hid-firmware-common.h"

typedef struct {
	guint32 crc32;
} FuNordicHidFirmwareCommonPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(FuNordicHidFirmwareCommon,
			   fu_nordic_hid_firmware_common,
			   FU_TYPE_FIRMWARE)
#define GET_PRIVATE(o) (fu_nordic_hid_firmware_common_get_instance_private(o))

static void
fu_nordic_hid_firmware_common_export(FuFirmware *firmware,
				     FuFirmwareExportFlags flags,
				     XbBuilderNode *bn)
{
	FuNordicHidFirmwareCommon *self = FU_NORDIC_HID_FIRMWARE_COMMON(firmware);
	FuNordicHidFirmwareCommonPrivate *priv = GET_PRIVATE(self);
	fu_xmlb_builder_insert_kx(bn, "crc32", priv->crc32);
}

static gchar *
fu_nordic_hid_firmware_common_get_checksum(FuFirmware *firmware,
					   GChecksumType csum_kind,
					   GError **error)
{
	FuNordicHidFirmwareCommon *self = FU_NORDIC_HID_FIRMWARE_COMMON(firmware);
	FuNordicHidFirmwareCommonPrivate *priv = GET_PRIVATE(self);
	if (!fu_firmware_has_flag(firmware, FU_FIRMWARE_FLAG_HAS_CHECKSUM)) {
		g_set_error_literal(error,
				    G_IO_ERROR,
				    G_IO_ERROR_NOT_SUPPORTED,
				    "unable to calculate the checksum of the update binary");
		return NULL;
	}
	return g_strdup_printf("%x", priv->crc32);
}

static guint32
fu_nordic_hid_firmware_common_crc32(const guint8 *buf, gsize bufsz)
{
	guint crc32 = 0x01;
	/* maybe skipped "^" step in fu_common_crc32_full()?
	 * according https://github.com/madler/zlib/blob/master/crc32.c#L225 */
	crc32 ^= 0xFFFFFFFFUL;
	return fu_common_crc32_full(buf, bufsz, crc32, 0xEDB88320);
}

static gboolean
fu_nordic_hid_firmware_common_parse(FuFirmware *firmware,
				    GBytes *fw,
				    guint64 addr_start,
				    guint64 addr_end,
				    FwupdInstallFlags flags,
				    GError **error)
{
	FuNordicHidFirmwareCommon *self = FU_NORDIC_HID_FIRMWARE_COMMON(firmware);
	FuNordicHidFirmwareCommonPrivate *priv = GET_PRIVATE(self);
	const guint8 *buf;
	gsize bufsz = 0;

	buf = g_bytes_get_data(fw, &bufsz);
	if (buf == NULL) {
		g_set_error_literal(error,
				    FWUPD_ERROR,
				    FWUPD_ERROR_INVALID_FILE,
				    "unable to get the image binary");
		return FALSE;
	}
	fu_firmware_add_flag(FU_FIRMWARE(self), FU_FIRMWARE_FLAG_HAS_CHECKSUM);
	priv->crc32 = fu_nordic_hid_firmware_common_crc32(buf, bufsz);

	/* do not strip the header */
	fu_firmware_set_bytes(firmware, fw);

	return TRUE;
}

static void
fu_nordic_hid_firmware_common_init(FuNordicHidFirmwareCommon *self)
{
}

static void
fu_nordic_hid_firmware_common_class_init(FuNordicHidFirmwareCommonClass *klass)
{
	FuFirmwareClass *klass_firmware = FU_FIRMWARE_CLASS(klass);

	klass_firmware->export = fu_nordic_hid_firmware_common_export;
	klass_firmware->get_checksum = fu_nordic_hid_firmware_common_get_checksum;
	klass_firmware->parse = fu_nordic_hid_firmware_common_parse;
}
