/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dsp_config.h"
#include "rpmsg_lite.h"
#include "srtm_utils.h"
#include "srtm_config.h"

#include "xaf-utils-test.h"

#include "fsl_gpio.h"
// TYM DSP add >>
#include "FlowEngine.h"
// TYM DSP add <<
#define AUDIO_BUFFER_FILL_THRESHOLD (16 * 1024)
int test_num = 0;
/*******************************************************************************
 * Utility Functions
 ******************************************************************************/

// TYM DSP add >>
#define BUF_SIZE 100*1024
uint8_t MEMPOOL_BUF[BUF_SIZE];

typedef enum {
	HAL_GPT_STATUS_ERROR_RESTART_ERROR	 = -7,	/**< The SW GPT is restart the same timer that has not expired. */
	HAL_GPT_STATUS_ERROR_START_TOO_LONG	 = -6,	 /**< The user start time_out? is too long. */
	HAL_GPT_STATUS_ERROR_PORT_USE_FULL	 = -5,/**< The SW GPT users full. */
	HAL_GPT_STATUS_ERROR_PORT_USED	 = -4,	/**< The timer has beed used. */
	HAL_GPT_STATUS_ERROR	 = -3,	/**< GPT function error occurred. */
	HAL_GPT_STATUS_ERROR_PORT	 = -2,	/**< A wrong GPT port is set. */
	HAL_GPT_STATUS_INVALID_PARAMETER = -1,	/**< An invalid parameter. */
	HAL_GPT_STATUS_OK	= 0	/**< No error occurred during the function call.*/
} hal_gpt_status_t;

typedef enum {

	HAL_GPT_CLOCK_SOURCE_32K = 0,	/**< Set the GPT clock source to 32kHz, 1 tick = 1/32768 seconds. */

	HAL_GPT_CLOCK_SOURCE_1M = 1	/**< Set the GPT clock source to 1MHz, 1 tick = 1 microsecond.*/

} hal_gpt_clock_source_t;

hal_gpt_status_t hal_gpt_get_free_run_count(hal_gpt_clock_source_t clock_source, uint32_t *count)
{
	return 0;
}

//extern void TEST_ALL_AOS(void);
FlowEngine*	engine;

void Flow_path_initialize()
{
		printf("[TYM] Flow_path_initialize\r\n");
	    FlowEngine_Mempool_Create(MEMPOOL_BUF, BUF_SIZE);
	    engine = FlowEngine_create("engine", NULL, 2, 2);
	    FlowEngine_prepare(engine, 48000, 48);

	    FlowEngine_query_cmd(engine, "setRoot/lockAudio/1/");
	    FlowEngine_query_cmd(engine, "clearLayout/");
	    FlowEngine_query_cmd(engine, "init/5/");
	    FlowEngine_query_cmd(engine, "obj/OUT/OUT_1/3/-1/-1/-1/0/");
	    FlowEngine_query_cmd(engine, "obj/IN/IN_1/0/-1/-1/-1/0/");
	    FlowEngine_query_cmd(engine, "obj/IN/IN_2/1/-1/-1/-1/0/");
	    FlowEngine_query_cmd(engine, "obj/OUT/OUT_2/4/-1/-1/-1/0/");
	    FlowEngine_query_cmd(engine, "obj/GAIN/GAIN_1/2/2/2/1/0/");
	    FlowEngine_query_cmd(engine, "setCoord/GAIN_1/gain/-40.0/0/0/");
	    FlowEngine_query_cmd(engine, "reprepare/");
	    FlowEngine_query_cmd(engine, "buildProcStep/");
	    FlowEngine_query_cmd(engine, "wire/IN_1@0/GAIN_1@0/");
	    FlowEngine_query_cmd(engine, "wire/IN_2@0/GAIN_1@1/");
	    FlowEngine_query_cmd(engine, "wire/GAIN_1@0/OUT_1@0/");
	    FlowEngine_query_cmd(engine, "wire/GAIN_1@1/OUT_2@0/");
	    FlowEngine_query_cmd(engine, "setRoot/lockAudio/0/");

}

// TYM DSP add <<


/* Wrap stdlib malloc() for audio framework allocator */
void *DSP_Malloc(int32_t size, int32_t id)
{
    /* If malloc can return un-aligned data, must return 4-byte aligned pointer for XAF. */
    return malloc(size);
}

/* Wrap stdlib free() for audio framework allocator */
void DSP_Free(void *ptr, int32_t id)
{
    free(ptr);
}

uint32_t DSP_AudioReadRing(dsp_handle_t *dsp, char *data, uint32_t size)
{
    uint32_t bytes_read;

    xos_mutex_lock(&dsp->audioMutex);
    bytes_read = ringbuf_read(dsp->audioBuffer, (uint8_t *)data, size);
    xos_mutex_unlock(&dsp->audioMutex);

    if (bytes_read != size)
    {
        /* UNDERRUN */
        // DSP_PRINTF("Read Ring UNDERRUN: %d (size: %d)\r\n", bytes_read, size);
    }

    return bytes_read;
}

uint32_t DSP_AudioWriteRing(dsp_handle_t *dsp, char *data, uint32_t size)
{
    uint32_t written;

    xos_mutex_lock(&dsp->audioMutex);
    written = ringbuf_write(dsp->audioBuffer, (uint8_t *)data, size);
    xos_mutex_unlock(&dsp->audioMutex);

    if (written != size)
    {
        /* OVERFLOW */
        // DSP_PRINTF("Write Ring OVERFLOW: %d (size: %d)\r\n", written, size);
    }

    return written;
}

uint32_t DSP_AudioSizeRing(dsp_handle_t *dsp)
{
    uint32_t size;

    xos_mutex_lock(&dsp->audioMutex);
    size = ringbuf_get_occupancy(dsp->audioBuffer);
    xos_mutex_unlock(&dsp->audioMutex);

    return size;
}

/* Read audio data from DSP buffer
 *
 * param dsp DSP handle with input buffer
 * param data Buffer for data
 * param size Size of data to read in bytes
 * return number of bytes read */
uint32_t DSP_AudioRead(dsp_handle_t *dsp, char *data, uint32_t size)
{
    uint32_t read_size = size;

    if (size + dsp->buffer_in.index > dsp->buffer_in.size)
    {
        read_size = dsp->buffer_in.size - dsp->buffer_in.index;
    }

    memcpy(data, &dsp->buffer_in.data[dsp->buffer_in.index], read_size);

    dsp->buffer_in.index += read_size;

    return read_size;
}

/* Consume audio data output from DSP processing
 *
 * param dsp DSP handle with output buffer
 * param data Buffer with data
 * param size Size of data to write in bytes
 * return number of bytes written */
uint32_t DSP_AudioWrite(dsp_handle_t *dsp, char *data, uint32_t size)
{
    uint32_t write_size = size;

    if (size + dsp->buffer_out.index > dsp->buffer_out.size)
    {
        write_size = dsp->buffer_out.size - dsp->buffer_out.index;
    }

    memcpy(&dsp->buffer_out.data[dsp->buffer_out.index], data, write_size);

    dsp->buffer_out.index += write_size;

    if (dsp->buffer_out.index >= dsp->buffer_out.size)
        dsp->buffer_out.index = 0;

    return write_size;
}

static void DSP_RequestData(dsp_handle_t *dsp)
{
    srtm_message msg = {0};

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_UsbSpeakerData;

    xos_mutex_lock(&dsp->rpmsgMutex);
    rpmsg_lite_send(dsp->rpmsg, dsp->ept, MCU_EPT_ADDR, (char *)&msg, sizeof(srtm_message), RL_DONT_BLOCK);
    xos_mutex_unlock(&dsp->rpmsgMutex);
}

static void DSP_SendBufferIndex(dsp_handle_t *dsp)
{
    srtm_message msg = {0};

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_UsbMicData;

    msg.param[0] = dsp->buffer_out.index;

    xos_mutex_lock(&dsp->rpmsgMutex);
    rpmsg_lite_send(dsp->rpmsg, dsp->ept, MCU_EPT_ADDR, (char *)&msg, sizeof(srtm_message), RL_DONT_BLOCK);
    xos_mutex_unlock(&dsp->rpmsgMutex);
}

void DSP_SendUsbEnd(dsp_handle_t *dsp, usb_device_type_t usb_dev)
{
    srtm_message msg = {0};

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.head.category = SRTM_MessageCategory_AUDIO;
    switch (usb_dev)
    {
        case USB_SpeakerDevice:
            msg.head.command = SRTM_Command_UsbSpeakerEnd;
            break;
        case USB_MicDevice:
            msg.head.command = SRTM_Command_UsbMicEnd;
            break;
        default:
            return;
    }

    xos_mutex_lock(&dsp->rpmsgMutex);
    rpmsg_lite_send(dsp->rpmsg, dsp->ept, MCU_EPT_ADDR, (char *)&msg, sizeof(srtm_message), RL_DONT_BLOCK);
    xos_mutex_unlock(&dsp->rpmsgMutex);
}

void DSP_SendUsbError(dsp_handle_t *dsp, usb_device_type_t usb_dev)
{
    srtm_message msg = {0};

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.head.category = SRTM_MessageCategory_AUDIO;
    switch (usb_dev)
    {
        case USB_SpeakerDevice:
            msg.head.command = SRTM_Command_UsbSpeakerError;
            break;
        case USB_MicDevice:
            msg.head.command = SRTM_Command_UsbMicError;
            break;
        default:
            return;
    }

    xos_mutex_lock(&dsp->rpmsgMutex);
    rpmsg_lite_send(dsp->rpmsg, dsp->ept, MCU_EPT_ADDR, (char *)&msg, sizeof(srtm_message), RL_DONT_BLOCK);
    xos_mutex_unlock(&dsp->rpmsgMutex);
}
// TYM DSP add >>
void FlowDSP_SendCmdBack(dsp_handle_t *dsp, char* dstMsg)
{
    srtm_message msg = {0};

    msg.head.type         = SRTM_MessageTypeRequest;
    msg.head.majorVersion = SRTM_VERSION_MAJOR;
    msg.head.minorVersion = SRTM_VERSION_MINOR;

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_FlowDSPSetParam;
    msg.flow_msg = dstMsg;
    xos_mutex_lock(&dsp->rpmsgMutex);
    rpmsg_lite_send(dsp->rpmsg, dsp->ept, MCU_EPT_ADDR, (char *)&msg, sizeof(srtm_message), RL_DONT_BLOCK);
    xos_mutex_unlock(&dsp->rpmsgMutex);
}

void FLOWDSP_SetParam(dsp_handle_t *dsp, char* paramStr,int32_t value)
{
	char charInput[64];

//	sprintf(valStr,"%d",value);
//	char dest[64] = "setCoord/";
//	char* postStr = "/0/0/";
//	strcat(dest,paramStr);
//	strcat(dest,valStr);
//	strcat(dest,postStr);
	DSP_PRINTF("[FLOWDSP_SetParam] paramStr: %s\r\nparamValue:%d\r\n", paramStr, value);
	memset( charInput, 0, sizeof(charInput) );
	memcpy( charInput, paramStr, value);
	char *flowCmdResult = FlowEngine_query_cmd(engine,charInput);
	FlowDSP_SendCmdBack(dsp, flowCmdResult);
}
// TYM DSP add <<

/* Thread for processing DSP pipeline
 *
 * This thread will take care of polling the audio framework for status, feeding
 * data when needed, and consuming output when needed.  It will end when the
 * input or output data is exhausted.
 */
int DSP_ProcessThread(void *arg, int wake_value)
{
    dsp_handle_t *ctx = (dsp_handle_t *)arg;
    xaf_comp_status status;
    XAF_ERR_CODE ret;
    char *buffer;
    uint32_t buffer_len;
    uint32_t event_state;
    int info[4];
    int read_size, write_size;
    bool cmd_pending = true;

    DSP_PRINTF("[DSP_ProcessThread] start\r\n");
    Flow_path_initialize();
    ret = xaf_comp_process(NULL, ctx->comp, NULL, 0, XAF_EXEC_FLAG);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("[DSP_ProcessThread] xaf_comp_process XAF_EXEC_FLAG failure: %d\r\n", ret);
        return -1;
    }

    while (1)
    {
        /* Check for external events to the processing thread */
        xos_event_get(&ctx->pipeline_event, &event_state);
        if (event_state & DSP_EVENT_STOP)
        {
            xos_event_clear(&ctx->pipeline_event, DSP_EVENT_STOP);
            /* Send INPUT_OVER to decoder to gracefully shutdown pipeline */
            ret = xaf_comp_process(NULL, ctx->comp, NULL, 0, XAF_INPUT_OVER_FLAG);
            if (ret != XAF_NO_ERR)
            {
                DSP_PRINTF("[DSP_ProcessThread] xaf_comp_process XAF_INPUT_OVER_FLAG failure: %d\r\n", ret);
            }

            break;
        }

        if (cmd_pending)
        {
            /* Request data from cm33 */
            if (ctx->usb_playing)
                DSP_RequestData(ctx);

            ret = xaf_comp_get_status(NULL, ctx->comp, &status, &info[0]);
            if (ret != XAF_NO_ERR)
            {
                DSP_PRINTF("[DSP_ProcessThread] xaf_comp_get_status failure: %d\r\n", ret);
                if (ctx->usb_playing)
                {
                    ctx->usb_playing = false;
                    DSP_SendUsbError(ctx, USB_SpeakerDevice);
                }
                if (ctx->usb_recording)
                {
                    ctx->usb_recording = false;
                    DSP_SendUsbError(ctx, USB_MicDevice);
                }
                DSP_PRINTF("[DSP_ProcessThread] Error, exiting\r\n");
                return -1;
            }
//            DSP_PRINTF("[DSP_ProcessThread][TYM] Request data from cm33\n");
            cmd_pending = false;
        }

        if (status == XAF_EXEC_DONE)
        {
            DSP_PRINTF("[DSP_ProcessThread] Execution complete - exiting\r\n");
            break;
        }
        else if (status == XAF_NEED_INPUT)
        {
            /* Read input and feed data to pipeline for processing */
            buffer     = (char *)info[0];
            buffer_len = (uint32_t)info[1];

            read_size = ctx->audio_read(ctx, buffer, buffer_len);
//            int16_t temp = (int16_t)buffer[2 * i] + ((int16_t) buffer[2 * i + 1] << 8);
//            DSP_PRINTF("dst : %d\r\n", temp);
//            DSP_PRINTF("[TYM] read_size : %d, buffer_len: %d\r\n", read_size, buffer_len);

    	    FlowEngine_ProcessInt16(engine, buffer, buffer);
        	// TYM¡@DSP >>
//            test_num++;
//            int8_t tempB1 = 0;
//            int8_t tempB2 = 0;
//            if (test_num % 2000 == 0){
//            	for (int i=0; i<10; i++)
//				{
//            		tempB1 = buffer[2 * i];
//            		tempB2 = buffer[2 * i + 1];
//                    int16_t temp = (int16_t)buffer[2 * i] + ((int16_t) buffer[2 * i + 1]) << 8;
//                    DSP_PRINTF("[TYM]dst : %d\r\n", temp);
////					DSP_PRINTF("src audioBuffer[%d]: %d\r\n",i, ctx->audioBuffer->buf[i]);
////					DSP_PRINTF("dst buffer[%d]: %d\r\n",i, buffer[i]);
////
//				}
//            	test_num =0;
//            }
            // TYM DSP <<
            if (read_size > 0)
            {
                ret = xaf_comp_process(NULL, ctx->comp, (void *)buffer, read_size, XAF_INPUT_READY_FLAG);
                if (ret != XAF_NO_ERR)
                {
                    DSP_PRINTF("[DSP_ProcessThread] xaf_comp_process XAF_INPUT_READY_FLAG failure: %d\r\n", ret);
                    return -1;
                }

                cmd_pending = true;
            }
            else
            {
                // DSP_PRINTF("Read size is 0\r\n");
            }
        }
        else if (status == XAF_OUTPUT_READY)
        {
            /* Consume output from pipeline */
            buffer     = (char *)info[0];
            buffer_len = (uint32_t)info[1];
            DSP_PRINTF("[DSP_ProcessThread][TYM] XAF_OUTPUT_READY, onsume output from pipeline\n");

            if (buffer_len == 0)
            {
                /* No output available */
            }
            else
            {
                write_size = ctx->audio_write(ctx, buffer, buffer_len);

                if (ctx->usb_recording)
                    DSP_SendBufferIndex(ctx);

                ret = xaf_comp_process(NULL, ctx->comp, buffer, buffer_len, XAF_NEED_OUTPUT_FLAG);
                if (ret != XAF_NO_ERR)
                {
                    DSP_PRINTF("[DSP_ProcessThread] xaf_comp_process XAF_NEED_OUTPUT_FLAG failure: %d\r\n", ret);
                    return -1;
                }
                cmd_pending = true;
            }
        }
        else
        {
            /* Error or nonstandard response. */
            DSP_PRINTF("[DSP_ProcessThread] unexpected status: %d\r\n", status);
            cmd_pending = true;
        }
    }

    if (ctx->usb_playing)
        ctx->usb_playing = false;
    if (ctx->usb_recording)
        ctx->usb_recording = false;

    DSP_PRINTF("[DSP_ProcessThread] exiting\r\n");

    return 0;
}
