/*
 * Copyright (C) 2021 Denis Pynkin <denis.pynkin@collabora.com>
 *
 * SPDX-License-Identifier: LGPL-2.1+
 */

#pragma once

#include <fwupdplugin.h>

#define FU_TYPE_NORDIC_HID_FIRMWARE_COMMON (fu_nordic_hid_firmware_common_get_type())
G_DECLARE_DERIVABLE_TYPE(FuNordicHidFirmwareCommon,
			 fu_nordic_hid_firmware_common,
			 FU,
			 NORDIC_HID_FIRMWARE_COMMON,
			 FuFirmware)

struct _FuNordicHidFirmwareCommonClass {
	FuFirmwareClass parent_class;
};
