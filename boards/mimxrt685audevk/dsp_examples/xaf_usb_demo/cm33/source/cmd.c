/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "cmd.h"
#include "dsp_ipc.h"

#include <string.h>
#include <stdint.h>
#include "fsl_debug_console.h"
#include "fsl_shell.h"

#include "composite.h"
// TYM FW add >>
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "task.h"
#include "fsl_usart.h"
#include "usb_device_hid.h"
// TYM FW add <<
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/

/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*${prototype:start}*/
static void initMessage(srtm_message *msg);

static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellUsbSpeaker(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellUsbMic(shell_handle_t shellHandle, int32_t argc, char **argv);
// TYM DSP add >>
static shell_status_t shellFlowDSP(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellLineInOut(shell_handle_t shellHandle, int32_t argc, char **argv);
// TYM DSP add <<
#if XA_CLIENT_PROXY
static shell_status_t shellEAPeffect(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
// TYM DSP add >>
static shell_status_t shellFlowDSP(shell_handle_t shellHandle, int32_t argc, char **argv);
extern app_handle_t app;
extern void USB_DeviceAudioRecorderStatusReset(void);
extern void USB_DeviceAudioSpeakerStatusReset(void);
// TYM DSP add <<

/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_device_composite_struct_t g_composite;
extern uint8_t *audioPlayDataBuff;
extern uint8_t *audioPlayDMATempBuff;
extern uint8_t *audioRecDataBuff;
extern uint8_t *audioRecDMATempBuff;

/*${variable:start}*/
SHELL_COMMAND_DEFINE(version, "\r\n\"version\": Query DSP for component versions\r\n", shellEcho, 0);
SHELL_COMMAND_DEFINE(usb_speaker,
                     "\r\n\"usb_speaker\": Perform usb speaker device and playback on DSP\r\n"
                     "  USAGE: usb_speaker [start|stop]\r\n"
                     "    start          Start usb speaker device and playback on DSP\r\n"
                     "    stop           Stop usb speaker device and playback on DSP\r\n",
                     shellUsbSpeaker,
                     1);

#if XA_CLIENT_PROXY
SHELL_COMMAND_DEFINE(eap,
                     "\r\n\"eap\": Set EAP parameters\r\n"
                     "  USAGE: eap [1|2|3|4|5|6|7|+|-|l|r]\r\n"
                     "  OPTIONS:\r\n"
                     "    1:	All effect Off \r\n"
                     "    2:	Voice enhancer \r\n"
                     "    3:	Music enhancer \r\n"
                     "    4:	Auto volume leveler \r\n"
                     "    5:	Loudness maximiser  \r\n"
                     "    6:	3D Concert sound  \r\n"
                     "    7:	Custom\r\n"
                     "    8:	Tone Generator\r\n"
                     "    9:	Crossover 2 way speaker\r\n"
                     "   10:	Crossover for subwoofer\r\n"
                     "    +:	Volume up\r\n"
                     "    -:	Volume down\r\n"
                     "    l:	Balance left\r\n"
                     "    r:	Balance right\r\n",
                     shellEAPeffect,
                     1);
#endif

SHELL_COMMAND_DEFINE(usb_mic,
                     "\r\n\"usb_mic\": Record DMIC audio and playback on usb microphone audio device\r\n"
                     "  USAGE: usb_mic [start|stop]\r\n"
                     "    start          Start record and playback on usb microphone audio device\r\n"
                     "    stop           Stop record and playback on usb microphone audio device\r\n",
                     shellUsbMic,
                     1);
// TYM DSP add >>
SHELL_COMMAND_DEFINE(flow, "\r\n\"flow\": Send command to control FlowDSP param\r\n", shellFlowDSP, 2);
SHELL_COMMAND_DEFINE(line,
					 "\r\n\"line\": Perform line in & out and playback on DSP\r\n"
					 "  USAGE: line [start]\r\n"
					 "	  start			Start line in & out and playback on DSP\r\n",
					 shellLineInOut,
					 1);
// TYM DSP add <<
static bool usb_playing   = false;
static bool usb_recording = false;

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;
static handleShellMessageCallback_t *g_handleShellMessageCallback;
static void *g_handleShellMessageCallbackData;
// TYM FW add >>
extern int8_t FlowCmdStep;
extern usb_device_composite_struct_t *g_UsbDeviceComposite;
// TYM FW add <<
/*${variable:end}*/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*${function:start}*/
static void initMessage(srtm_message *msg)
{
    /* Common field for command */
    /* For single command, command list not used at the moment */
    msg->head.type = SRTM_MessageTypeRequest;

    msg->head.majorVersion = SRTM_VERSION_MAJOR;
    msg->head.minorVersion = SRTM_VERSION_MINOR;
}

static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_GENERAL;
    msg.head.command  = SRTM_Command_ECHO;

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}

static shell_status_t shellUsbSpeaker(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;

    if (strcmp(argv[1], "start") == 0)
    {
        if (usb_recording)
        {
            PRINTF("Stop the USB microphone first, then start the USB speaker again\r\n");
            return kStatus_SHELL_Error;
        }
        if (!usb_playing)
        {
            USB_DeviceAudioSpeakerStatusReset();
            USB_DeviceRun(g_composite.deviceHandle);
            usb_playing = true;

            /* Small delay for USB starting */
            TickType_t timeTMP = xTaskGetTickCount();
            while (xTaskGetTickCount() < (timeTMP + 500))
            {
                ;
            }

            /* Clear buffers */
            memset(audioPlayDataBuff, 0, AUDIO_SHARED_BUFFER_1_SIZE);

            msg.head.command = SRTM_Command_UsbSpeakerStart;

            /* Param 0 input buffer address */
            /* Param 1 input buffer size */
            /* Param 2 number of channels */
            /* Param 3 sampling_rate */
            /* Param 4 pcm_width */
            msg.param[0] = (uint32_t)(&audioPlayDataBuff[0]);
#if 0
            msg.param[1] = 2 * ((0U != g_composite.audioUnified.audioPlayTransferSize) ?
                                    g_composite.audioUnified.audioPlayTransferSize :
                                    HS_ISO_OUT_ENDP_PACKET_SIZE);
            msg.param[2] = AUDIO_IN_FORMAT_CHANNELS;
            msg.param[3] = AUDIO_IN_SAMPLING_RATE;
            msg.param[4] = AUDIO_IN_FORMAT_BITS;
#else
            // TYM DSP add tofu>>
            msg.param[1] = 1024;
            msg.param[2] = 8;
            msg.param[3] = 48000;
            msg.param[4] = 32;
            // TYM DSP add tofu<<
#endif

            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        }
        else
        {
            PRINTF("USB is already playing \r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        if (usb_playing)
        {
            msg.head.command = SRTM_Command_UsbSpeakerStop;
            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
            return kStatus_SHELL_Success;
        }
        else
        {
            PRINTF("USB is not playing \r\n");
            return kStatus_SHELL_Error;
        }
    }
    else
    {
        PRINTF("Unknown parameter\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t shellUsbMic(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;

    if (strcmp(argv[1], "start") == 0)
    {
        if (usb_playing)
        {
            PRINTF("Stop the USB speaker first, then start the USB microphone again\r\n");
            return kStatus_SHELL_Error;
        }
        if (!usb_recording)
        {
            USB_DeviceAudioRecorderStatusReset();
            USB_DeviceRun(g_composite.deviceHandle);
            usb_recording = true;

            /* Small delay for USB starting */
            TickType_t timeTMP = xTaskGetTickCount();
            while (xTaskGetTickCount() < (timeTMP + 500))
            {
                ;
            }

            /* Clear buffers */
            memset(audioRecDataBuff, 0, AUDIO_SHARED_BUFFER_2_SIZE);

            msg.head.command = SRTM_Command_UsbMicStart;

            /* Param 0 output buffer address */
            /* Param 1 output buffer size */
            /* Param 2 number of channels */
            /* Param 3 sampling_rate */
            /* Param 4 pcm_width */
            msg.param[0] = (uint32_t)(&audioRecDataBuff[0]);
            msg.param[1] = AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE;
            msg.param[2] = AUDIO_IN_FORMAT_CHANNELS;
            msg.param[3] = AUDIO_IN_SAMPLING_RATE;
            msg.param[4] = AUDIO_IN_FORMAT_BITS;

            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        }
        else
        {
            PRINTF("USB is already recording\r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        if (usb_recording)
        {
            msg.head.command = SRTM_Command_UsbMicStop;
            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
            return kStatus_SHELL_Success;
        }
        else
        {
            PRINTF("USB is not recording\r\n");
            return kStatus_SHELL_Error;
        }
    }
    else
    {
        PRINTF("Unknown parameter\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

#if XA_CLIENT_PROXY
static shell_status_t shellEAPeffect(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    int effectNum    = atoi(argv[1]);
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_FilterCfg;
    /* Param 0 Number of EAP config*/

    if (effectNum > 0 && effectNum < 11)
    {
        msg.param[0] = effectNum;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else if (strcmp(argv[1], "+") == 0)
    {
        msg.param[0] = 11;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else if (strcmp(argv[1], "-") == 0)
    {
        msg.param[0] = 12;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else if (strcmp(argv[1], "l") == 0)
    {
        msg.param[0] = 13;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else if (strcmp(argv[1], "r") == 0)
    {
        msg.param[0] = 14;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else
    {
        PRINTF("Effect parameter is out of range! Please see help. \r\n");
        return kStatus_SHELL_Error;
    }
}
#endif
// TYM FW add >>
static shell_status_t shellFlowDSP(shell_handle_t shellHandle, int32_t argc, char **argv)
{
#if 0
    srtm_message msg = {0};

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_FLOWCMD;
    msg.head.command = SRTM_Command_FlowDSPSetParam;
    if (usb_playing)
    {
		PRINTF("[CM33] Send flow command. \r\n");
		msg.flow_msg = (char *) argv[1];
		msg.param[0] = atoi(argv[2]);

		g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    }
    else
    {
    	PRINTF("[CM33] USB audio is not ready. \r\n");
    }
    return kStatus_SHELL_Success;
#endif
}

static shell_status_t shellLineInOut(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command = SRTM_Command_LINEINOUT;

    /* Param 0 Number of Channels*/
    /* Param 1 Sampling Rate*/
    /* Param 2 PCM bit Width*/

    msg.param[0] = 8;		// TYM DSP set for TDM
    msg.param[1] = 48000;	// TYM DSP chg from 16k to 48k
//    msg.param[1] = 16000;	// DMIC in 16k
    msg.param[2] = 32;		// TYM DSP set for TDM
    PRINTF("[TYM][CM33] Send shellLineInOut command %d, %d, %d. \r\n",msg.param[0], msg.param[1], msg.param[2]);

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);

    return kStatus_SHELL_Success;
}
// TYM DSP add <<
void send_FlowPathInit_Cmd(char flowPathInitChar)
{
    srtm_message msg = {0};

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command = SRTM_Command_LINEINOUT;

    /* Param 0 Number of Channels*/
    /* Param 1 Sampling Rate*/
    /* Param 2 PCM bit Width*/

    msg.param[0] = 8;       // TYM DSP set for TDM
    msg.param[1] = 48000;   // TYM DSP chg from 16k to 48k
//    msg.param[1] = 16000; // DMIC in 16k
    msg.param[2] = 32;      // TYM DSP set for TDM
    //PRINTF("[TYM][CM33] Send shellLineInOut command %d, %d, %d. \r\n",msg.param[0], msg.param[1], msg.param[2]);
// TODO Check use USB_HID work or not
//    dsp_ipc_send_sync(&msg);
    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
}
void send_FlowStudio_Cmd(char* flowCmdCharPtr, uint32_t cmdLength)
{
    srtm_message msg = {0};
    msg.head.type = SRTM_MessageTypeRequest;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;
    msg.head.category = SRTM_MessageCategory_FLOWCMD;
    msg.head.command = SRTM_Command_FlowDSPSetParam;
    memcpy(msg.flow_msg, flowCmdCharPtr, cmdLength);
    msg.param[0] = cmdLength;

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
}
// TYM FW add <<

void shellCmd(handleShellMessageCallback_t *handleShellMessageCallback, void *arg)
{
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, ">> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(version));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(usb_speaker));
#if XA_CLIENT_PROXY
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(eap));
#endif
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(usb_mic));
    // TYM DSP add >>
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(flow));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(line));
    // TYM DSP add <<
    g_handleShellMessageCallback     = handleShellMessageCallback;
    g_handleShellMessageCallbackData = arg;

#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    SHELL_Task(s_shellHandle);
#endif
}

static void handleDSPMessageInner(app_handle_t *app, srtm_message *msg, bool *notify_shell)
{
    uint32_t audioSpeakerPreReadDataCount = 0U;
    uint32_t preAudioSendCount            = 0U;
    *notify_shell                         = true;

    char string_buff[SRTM_CMD_PARAMS_MAX];

    if (msg->head.type == SRTM_MessageTypeResponse)
    {
        // Need to comment for Flow studio using
        //PRINTF("[APP_DSP_IPC_Task] response from DSP, cmd: %d, error: %d\r\n", msg->head.command, msg->error);
    }

    /* Processing returned data*/
    switch (msg->head.category)
    {
        case SRTM_MessageCategory_GENERAL:
            switch (msg->head.command)
            {
                /* echo returns version info of key components*/
                case SRTM_Command_ECHO:
                    PRINTF("Component versions from DSP:\r\n");
                    PRINTF("Audio Framework version %d.%d \r\n", msg->param[0] >> 16, msg->param[0] & 0xFF);
                    PRINTF("Audio Framework API version %d.%d\r\n", msg->param[1] >> 16, msg->param[1] & 0xFF);
                    PRINTF("NatureDSP Lib version %d.%d\r\n", msg->param[2] >> 16, msg->param[2] & 0xFF);
                    PRINTF("NatureDSP API version %d.%d\r\n", msg->param[3] >> 16, msg->param[3] & 0xFF);
                    break;

                case SRTM_Command_SYST:
                    break;
                default:
                    PRINTF("Incoming unknown message command %d from category %d \r\n", msg->head.command,
                           msg->head.category);
            }
            break;

        case SRTM_MessageCategory_AUDIO:
            if (usb_playing && (msg->head.command != SRTM_Command_FilterCfg) && (msg->head.command != SRTM_Command_FlowDSPSetParam) &&
                (msg->head.command < SRTM_Command_UsbSpeakerStart || msg->head.command > SRTM_Command_UsbSpeakerError))
            {
                PRINTF("This command cannot be processed at this time because USB is being played!\r\n");
                break;
            }
            else if (!usb_playing && msg->head.command == SRTM_Command_FilterCfg)
            {
                PRINTF("Please play usb first, then apply an EAP preset.\r\n");
                break;
            }
            switch (msg->head.command)
            {
                case SRTM_Print_String:
                    for (int i = 0; i < SRTM_CMD_PARAMS_MAX; i++)
                    {
                        string_buff[i] = (char)msg->param[i];
                    }
                    PRINTF("%s", string_buff);
                    break;

                case SRTM_Command_UsbSpeakerStart:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP USB playback start failed! return error = %d\r\n", msg->error);
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioSpeakerStatusReset();
                        usb_playing = false;
                    }
                    else
                    {
                        PRINTF("DSP USB playback start\r\n");
                    }

                    /* Release shell to be able to set different EAP presets*/
                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbSpeakerData:
                    msg->head.type = SRTM_MessageTypeResponse;

                    if ((USB_AudioSpeakerBufferSpaceUsed() < (g_composite.audioUnified.audioPlayTransferSize)) &&
                        (g_composite.audioUnified.startPlayFlag == 1U))
                    {
                        g_composite.audioUnified.startPlayFlag          = 0;
                        g_composite.audioUnified.speakerDetachOrNoInput = 1;
                    }
                    if (0U != g_composite.audioUnified.startPlayFlag)
                    {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
                        USB_DeviceCalculateFeedback();
#endif

                        msg->param[0]     = (uint32_t)(&audioPlayDataBuff[g_composite.audioUnified.tdReadNumberPlay]);
                        msg->param[1]     = g_composite.audioUnified.audioPlayTransferSize;
                        preAudioSendCount = g_composite.audioUnified.audioSendCount[0];
                        g_composite.audioUnified.audioSendCount[0] += g_composite.audioUnified.audioPlayTransferSize;
                        if (preAudioSendCount > g_composite.audioUnified.audioSendCount[0])
                        {
                            g_composite.audioUnified.audioSendCount[1] += 1U;
                        }
                        g_composite.audioUnified.audioSendTimes++;
                        g_composite.audioUnified.tdReadNumberPlay += g_composite.audioUnified.audioPlayTransferSize;
                        if (g_composite.audioUnified.tdReadNumberPlay >= g_composite.audioUnified.audioPlayBufferSize)
                        {
                            g_composite.audioUnified.tdReadNumberPlay = 0;
                        }
                        audioSpeakerPreReadDataCount = g_composite.audioUnified.audioSpeakerReadDataCount[0];
                        g_composite.audioUnified.audioSpeakerReadDataCount[0] +=
                            g_composite.audioUnified.audioPlayTransferSize;
                        if (audioSpeakerPreReadDataCount > g_composite.audioUnified.audioSpeakerReadDataCount[0])
                        {
                            g_composite.audioUnified.audioSpeakerReadDataCount[1] += 1U;
                        }
                    }
                    else
                    {
                        msg->param[0] = (uint32_t)(&audioPlayDMATempBuff[0]);
                        msg->param[1] = (0U != g_composite.audioUnified.audioPlayTransferSize) ?
                                            g_composite.audioUnified.audioPlayTransferSize :
                                            HS_ISO_OUT_ENDP_PACKET_SIZE;
                    }

                    /* Send response message to DSP with new data */
                    dsp_ipc_send_sync(msg);

                    /* Don't release shell until receive notification of usb end */
                    *notify_shell = false;
                    break;

                case SRTM_Command_UsbSpeakerEnd:
                    PRINTF("DSP USB playback complete\r\n");

                    USB_DeviceStop(g_composite.deviceHandle);
                    USB_DeviceAudioSpeakerStatusReset();
                    usb_playing = false;

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbSpeakerStop:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP USB stop failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("DSP USB stopped\r\n");
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioSpeakerStatusReset();
                        usb_playing = false;
                    }

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbSpeakerError:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP requested USB stop due to error failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("DSP USB stopped, unsupported format!!!.\r\n");
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioSpeakerStatusReset();
                        usb_playing = false;
                    }

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbMicStart:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP USB record start failed! return error = %d\r\n", msg->error);
                        USB_DeviceStop(g_composite.deviceHandle);
                        usb_recording = false;
                    }
                    else
                    {
                        PRINTF("DSP USB record start\r\n");
                    }

                    /* Release shell to be able to send different command */
                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbMicData:
                    g_composite.audioUnified.tdWriteNumberRec = msg->param[0];
                    *notify_shell                             = true;
                    break;

                case SRTM_Command_UsbMicEnd:
                    PRINTF("DSP USB recording complete\r\n");

                    USB_DeviceStop(g_composite.deviceHandle);
                    USB_DeviceAudioRecorderStatusReset();
                    usb_recording = false;

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbMicStop:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP USB stop failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("DSP USB stopped\r\n");
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioRecorderStatusReset();
                        usb_recording = false;
                    }

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbMicError:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP requested USB stop due to error failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("DSP USB stopped\r\n");
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioRecorderStatusReset();
                        usb_recording = false;
                    }

                    *notify_shell = true;
                    break;

#if XA_CLIENT_PROXY
                case SRTM_Command_FilterCfg:
                {
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP Filter cfg failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("DSP Filter cfg success!\r\n");
                    }
                    *notify_shell = true;
                    break;
                }
#endif
                // TYM DSP add >>
                case SRTM_Command_FlowDSPSetParam:
                {
                	if (msg->error != SRTM_Status_Success)
                	{
                		PRINTF("DSP FlowDSP set parameter failed! return error = %d\r\n", msg->error);
                	}
                	else
                	{
                        if( FlowCmdStep == FlowCmd_Step0_PreFlowPathInit)
                        {
                            if( msg->flow_msg[0] == 'S' && msg->flow_msg[1] == 'u' && msg->flow_msg[2] == 'c')
                            {
                                //PRINTF( msg->flow_msg, "%s");
                                FlowCmdStep = FlowCmd_Step1_Header1;
                                GPIO_PinWrite(BOARD_INITPINS_LED0853_GPIO, BOARD_INITPINS_LED0853_PORT, BOARD_INITPINS_LED0853_PIN, 1);
                                // Use the handle to delete the task.
                                if( app->audio_path_init_task_handle != NULL )
                                {
                                    vTaskDelete( app->audio_path_init_task_handle );
                                }
                            }
                            else
                            {
                                // Encounter some error
                                PRINTF( "Error on Flow Path initialize\r\n");
                            }
                        }
                        else
                        {
                            USART_Type *const s_UsartAdapterBase[] = USART_BASE_PTRS;
                            uint8_t dest[64];
                            memset(dest, 0, sizeof(dest));
                            dest[0] = 1;
                            dest[1] = 0;
                            dest[2] = 0;
                            dest[3] = msg->param[0];

                            memcpy( &dest[4], msg->flow_msg, msg->param[0] );
                            // TODO  do both UART writing and USB_DeviceHidSend
                            USART_WriteBlocking(s_UsartAdapterBase[0], dest, 4 + msg->param[0]);
                            USB_DeviceHidSend(g_UsbDeviceComposite->hidKeyboard.hidHandle, USB_HID_KEYBOARD_ENDPOINT,
                                                    dest, USB_HID_KEYBOARD_REPORT_LENGTH);
                        }
                	}
                	*notify_shell = true;
                	break;
                }
                // TYM DSP add <<
                default:
                    // TODO TYM FW add need define
                    //PRINTF("Incoming unknown message category %d \r\n", msg->head.category);
                    break;
            }
            break;
    }
}

void handleDSPMessage(app_handle_t *app, srtm_message *msg)
{
    bool notify_shell = false;

    handleDSPMessageInner(app, msg, &notify_shell);

    if (notify_shell)
    {
        /* Signal to shell that response has been processed. */
        if (app->shell_task_handle != NULL)
        {
            xTaskNotifyGive(app->shell_task_handle);
        }
    }
}
/*${function:end}*/
