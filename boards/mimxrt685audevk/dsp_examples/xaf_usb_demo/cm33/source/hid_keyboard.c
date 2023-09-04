/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_hid.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"
#include "hid_keyboard.h"
#include "fsl_debug_console.h"

#include "srtm_config.h"
#include "dsp_ipc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
usb_status_t USB_DeviceHidKeyboardAction(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

//USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_KeyboardBuffer[USB_HID_KEYBOARD_REPORT_LENGTH * 2];
static uint8_t s_KeyboardBufferOut[USB_HID_KEYBOARD_REPORT_LENGTH * 2];
usb_device_composite_struct_t *g_UsbDeviceComposite;
static usb_device_hid_keyboard_struct_t s_UsbDeviceHidKeyboard;
extern volatile bool g_ButtonPress;
extern volatile bool g_CodecSpeakerMuteUnmute;

/*******************************************************************************
 * Code
 ******************************************************************************/
#if USB_HID_DEBUG_MSG
    usb_status_t USB_DeviceHidKeyboardAction(void)
    {
        if (g_ButtonPress)
        {
            s_UsbDeviceHidKeyboard.buffer[0] = 0x04U;
            g_ButtonPress                    = false;
            return USB_DeviceHidSend(g_UsbDeviceComposite->hidKeyboard.hidHandle, USB_HID_KEYBOARD_ENDPOINT,
                                     s_UsbDeviceHidKeyboard.buffer, USB_HID_KEYBOARD_REPORT_LENGTH);
        }
        else if (g_CodecSpeakerMuteUnmute)
        {
            s_UsbDeviceHidKeyboard.buffer[0] = 0x00U;
            g_CodecSpeakerMuteUnmute         = false;
            return USB_DeviceHidSend(g_UsbDeviceComposite->hidKeyboard.hidHandle, USB_HID_KEYBOARD_ENDPOINT,
                                     s_UsbDeviceHidKeyboard.buffer, USB_HID_KEYBOARD_REPORT_LENGTH);
        }
        else
        {
            return kStatus_USB_Success;
        }
    }
#endif

usb_status_t USB_DeviceHidKeyboardCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
#if USB_HID_DEBUG_MSG
    PRINTF("USB_DeviceHidKeyboardCallback event:%d\r\n");
#endif
    usb_status_t return_result;

    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse:
            if (g_UsbDeviceComposite->attach)
            {
#if  USB_HID_DEBUG_MSG
                PRINTF("kUSB_DeviceHidEventSendResponse\r\n");
#endif
            }
            break;
        case kUSB_DeviceHidEventRecvResponse:
            if (g_UsbDeviceComposite->attach)
            {
                error = USB_DeviceHidRecv(handle, USB_HID_KEYBOARD_OUT_ENDPOINT,
                                         (uint8_t *)&s_UsbDeviceHidKeyboard.bufferOut[0],
                                         USB_HID_KEYBOARD_REPORT_LENGTH);
                int size = s_UsbDeviceHidKeyboard.bufferOut[1];
#if  USB_HID_DEBUG_MSG
                PRINTF("s_UsbDeviceHidKeyboard.bufferOut:");
                for(int i = 0; i < size + 3; i++)
                {
                    PRINTF("%02x",s_UsbDeviceHidKeyboard.bufferOut[i] );
                }
                PRINTF("\r\n");
#endif
                // Send flow studio command to hifi4
                srtm_message msg = {0};
                uint32_t cmdLength = 1;

                msg.head.type = SRTM_MessageTypeRequest;
                msg.head.majorVersion = SRTM_VERSION_MAJOR;
                msg.head.minorVersion = SRTM_VERSION_MINOR;
                msg.head.category = SRTM_MessageCategory_FLOWCMD;
                msg.head.command = SRTM_Command_FlowDSPSetParam;
                memcpy(msg.flow_msg, &s_UsbDeviceHidKeyboard.bufferOut[3], size);
                msg.param[0] = size;

                dsp_ipc_send_sync(&msg);
            }
            break;
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            error = kStatus_USB_Success;
            break;
        default:
            break;
    }

    return error;
}

usb_status_t USB_DeviceHidKeyboardSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        USB_DeviceHidRecv(g_UsbDeviceComposite->hidKeyboard.hidHandle, USB_HID_KEYBOARD_OUT_ENDPOINT,
                                 s_UsbDeviceHidKeyboard.bufferOut, USB_HID_KEYBOARD_REPORT_LENGTH);
        // Keep USB_DeviceHidKeyboardAction but not worked
        //return USB_DeviceHidKeyboardAction(); /* run the cursor movement code */
        return kStatus_USB_Success;
    }
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidKeyboardSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    if (USB_HID_KEYBOARD_INTERFACE_INDEX == interface)
    {
        USB_DeviceHidRecv(g_UsbDeviceComposite->hidKeyboard.hidHandle, USB_HID_KEYBOARD_OUT_ENDPOINT,
                                 s_UsbDeviceHidKeyboard.bufferOut, USB_HID_KEYBOARD_REPORT_LENGTH);
        // Keep USB_DeviceHidKeyboardAction but not worked
        //return USB_DeviceHidKeyboardAction(); /* run the cursor movement code */
        return kStatus_USB_Success;
    }
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidKeyboardInit(usb_device_composite_struct_t *deviceComposite)
{
    g_UsbDeviceComposite          = deviceComposite;
    s_UsbDeviceHidKeyboard.buffer = s_KeyboardBuffer;
    s_UsbDeviceHidKeyboard.bufferOut = s_KeyboardBufferOut;
    return kStatus_USB_Success;
}
