/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_HID_KEYBOARD_H__
#define __USB_DEVICE_HID_KEYBOARD_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _usb_device_hid_keyboard_struct
{
    usb_device_handle deviceHandle;
    class_handle_t hidHandle;
    uint8_t *buffer;
    uint8_t *bufferOut;
    uint8_t idleRate;
} usb_device_hid_keyboard_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#endif /* __USB_DEVICE_HID_KEYBOARD_H__ */
