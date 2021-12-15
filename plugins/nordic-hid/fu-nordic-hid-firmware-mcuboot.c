/*
 * Copyright (C) 2021 Denis Pynkin <denis.pynkin@collabora.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#include "config.h"

#include "fu-nordic-hid-firmware-mcuboot.h"

typedef struct __attribute__((packed)) {
	guint8 major;
	guint8 minor;
	guint16 revision;
	guint32 build_num;
} FuNordicHidFirmwareMcubootVersion;

typedef struct __attribute__((packed)) {
	guint32 magic;
	guint32 load_addr;
	guint16 hdr_size;
	guint16 protect_tlv_size;
	guint32 img_size;
	guint32 flags;
	FuNordicHidFirmwareMcubootVersion ver;
	guint32 _padding;
} FuNordicHidFirmwareMcubootHeader;

struct _FuNordicHidFirmwareMcuboot {
	FuNordicHidFirmwareCommonClass parent_instance;
};

G_DEFINE_TYPE(FuNordicHidFirmwareMcuboot,
	      fu_nordic_hid_firmware_mcuboot,
	      FU_TYPE_NORDIC_HID_FIRMWARE_COMMON)

static GBytes *
fu_nordic_hid_firmware_mcuboot_write(FuFirmware *firmware, GError **error)
{
	/* TODO: write the test blob */
	return NULL;
}

static gboolean
fu_nordic_hid_firmware_mcuboot_parse(FuFirmware *firmware,
				     GBytes *fw,
				     guint64 addr_start,
				     guint64 addr_end,
				     FwupdInstallFlags flags,
				     GError **error)
{
	const guint8 *buf;
	gsize bufsz = 0;

	if (!FU_FIRMWARE_CLASS(fu_nordic_hid_firmware_mcuboot_parent_class)
		 ->parse(firmware, fw, addr_start, addr_end, flags, error))
		return FALSE;

	buf = g_bytes_get_data(fw, &bufsz);
	if (buf == NULL) {
		g_set_error_literal(error,
				    FWUPD_ERROR,
				    FWUPD_ERROR_INVALID_FILE,
				    "unable to get the image binary");
		return FALSE;
	}

	/* TODO: simple validation of the image */

	return TRUE;
}

static void
fu_nordic_hid_firmware_mcuboot_init(FuNordicHidFirmwareMcuboot *self)
{
}

static void
fu_nordic_hid_firmware_mcuboot_class_init(FuNordicHidFirmwareMcubootClass *klass)
{
	FuFirmwareClass *klass_firmware = FU_FIRMWARE_CLASS(klass);
	klass_firmware->parse = fu_nordic_hid_firmware_mcuboot_parse;
	klass_firmware->write = fu_nordic_hid_firmware_mcuboot_write;
}

FuFirmware *
fu_nordic_hid_firmware_mcuboot_new(void)
{
	g_warning("start");
	return FU_FIRMWARE(g_object_new(FU_TYPE_NORDIC_HID_FIRMWARE_MCUBOOT, NULL));
}
