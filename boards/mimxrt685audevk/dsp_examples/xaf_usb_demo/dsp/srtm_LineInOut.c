/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <xtensa/xos.h>
#include "fsl_i2s.h"
#include "xaf-utils-test.h"
#include "xa_error_standards.h"
#include "audio/xa-renderer-api.h"
#include "audio/xa-pcm-gain-api.h"
#include "audio/xa-capturer-api.h"

#if XA_CLIENT_PROXY
#include "client_proxy_api.h"
#endif

#include "dsp_config.h"
#include "srtm_utils.h"
#include "srtm_config_audio.h"

#include "FlowEngine.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//#define AUDIO_FRMWK_BUF_SIZE (64 * 1024)
//#define AUDIO_COMP_BUF_SIZE  (256 * 1024)

#define AUDIO_FRMWK_BUF_SIZE (32 * 1024)
#define AUDIO_COMP_BUF_SIZE  (140 * 1024)

#define PCM_GAIN_FRAME_SIZE (1024)	// TYM DSP chg from 1920 record setiing to 1024 TDM 
#define RENDERER_FRAME_SIZE (1024)

enum
{
    XA_COMP = -1,
    XA_CAPTURER_0,
    XA_GAIN_0,
    XA_RENDERER_0,
    NUM_COMP_IN_GRAPH,
};

const int comp_get_order_lineinout[] = {XA_CAPTURER_0,
							XA_GAIN_0,
							XA_RENDERER_0};

#define MAX_CHANNELS  8
#define MAX_WIDTH     32
#define FRAME_SIZE_MS	10
/*******************************************************************************
 * Component Setup/ Config
 ******************************************************************************/
unsigned char LineInOut_process_stack[STACK_SIZE];
unsigned char LineInOut_cleanup_stack[STACK_SIZE];
int srtm_lineInOut_cleanup(void *arg, int wake_value);

static void *comp_get_pointer(dsp_handle_t *dsp, int cid, bool addr)
{
    switch (cid)
    {
        case XA_GAIN_0:
            return (addr ? &dsp->comp_gain : dsp->comp_gain);
        case XA_RENDERER_0:
            return (addr ? &dsp->comp_renderer : dsp->comp_renderer);
        case XA_CAPTURER_0:
            return (addr ? &dsp->comp_capturer : dsp->comp_capturer);
        default:
            DSP_PRINTF("[DSP_LINE_INOUT] Check component type in comp_get_order table\n\r");
            return NULL;
    }
}

static void comp_clean(dsp_handle_t *dsp)
{
    dsp->audio_device      = NULL;
    dsp->comp_gain         = NULL;
    dsp->comp_renderer     = NULL;
    dsp->comp_capturer 	   = NULL;
    dsp->comp              = NULL;
}

static int LineInOut_close(dsp_handle_t *dsp, bool success)
{
    int i, cid;
    XAF_ERR_CODE ret;

    for (i = 0; i < NUM_COMP_IN_GRAPH; i++)
    {
        cid = comp_get_order_lineinout[i];
        if (comp_get_pointer(dsp, cid, false) == NULL)
            continue;
        ret = xaf_comp_delete(comp_get_pointer(dsp, cid, false));
        if (ret != XAF_NO_ERR)
            DSP_PRINTF("[DSP_LINE_INOUT] xaf_comp_delete[%d] failure: %d\r\n", i, ret);
    }

    ret = xaf_adev_close(dsp->audio_device, XAF_ADEV_NORMAL_CLOSE);
    if (ret != XAF_NO_ERR)
        DSP_PRINTF("[DSP_LINE_INOUT] xaf_adev_close failure: %d\r\n", ret);
    else
        DSP_PRINTF("[DSP_LINE_INOUT] Audio device closed\r\n\r\n");

    // TYM DSP mark : temp no feedback to application (cm33)
//    /* Send message to the application */
//    if (success)
//        DSP_SendUsbEnd(dsp, USB_SpeakerDevice);
//    else
//        DSP_SendUsbError(dsp, USB_SpeakerDevice);

    return (success ? 0 : -1);
}

static int capturer_setup(void *p_capturer, xaf_format_t *format, bool i2s)
{
    int param[22];
    int num_params = 3;

    param[0] = XA_CAPTURER_CONFIG_PARAM_PCM_WIDTH;
    param[1] = format->pcm_width;
    param[2] = XA_CAPTURER_CONFIG_PARAM_CHANNELS;
    param[3] = format->channels;
    param[4] = XA_CAPTURER_CONFIG_PARAM_SAMPLE_RATE;
    param[5] = format->sample_rate;


    if (i2s)
    {
        num_params = 11;

        param[6] = XA_CAPTURER_CONFIG_PARAM_FRAME_SIZE;		// TYM DSP add
		param[7] = RENDERER_FRAME_SIZE;						// TYM DSP add
//        param[7] = (format->pcm_width >> 3) * format->channels * (format->sample_rate * FRAME_SIZE_MS / 1000);						// TYM DSP add
//        param[7] = 192;						// TYM DSP add
        param[8]  = XA_CAPTURER_CONFIG_PARAM_I2S_MASTER_SLAVE;
        param[9]  = 0;
        param[10] = XA_CAPTURER_CONFIG_PARAM_I2S_MODE;
        param[11] = (int)kI2S_ModeDspWsShort;	// kI2S_ModeI2sClassic
        param[12] = XA_CAPTURER_CONFIG_PARAM_I2S_SCK_POLARITY;
        param[13] = 0;
        param[14] = XA_CAPTURER_CONFIG_PARAM_I2S_WS_POLARITY;
        param[15] = 0;
        param[16] = XA_CAPTURER_CONFIG_PARAM_AUDIO_BUFFER_1;
        param[17] = (int)DSP_AUDIO_BUFFER_2_PING;
        param[18] = XA_CAPTURER_CONFIG_PARAM_AUDIO_BUFFER_2;
        param[19] = (int)DSP_AUDIO_BUFFER_2_PONG;
        param[20] = XA_CAPTURER_CONFIG_PARAM_I2S_INTERFACE;
        param[21] = AUDIO_I2S_CAPTURER_DEVICE;
    }

    return xaf_comp_set_config(p_capturer, num_params, &param[0]);
}

static int pcm_gain_setup(void *p_comp, xaf_format_t *format, bool i2s)
{
    int param[10];

    param[0] = XA_PCM_GAIN_CONFIG_PARAM_CHANNELS;
    param[1] = format->channels;
    param[2] = XA_PCM_GAIN_CONFIG_PARAM_SAMPLE_RATE;
    param[3] = format->sample_rate;
    param[4] = XA_PCM_GAIN_CONFIG_PARAM_PCM_WIDTH;
    param[5] = format->pcm_width;
    param[6] = XA_PCM_GAIN_CONFIG_PARAM_FRAME_SIZE;
//    param[7] = (format->pcm_width >> 3) * format->channels * (format->sample_rate * FRAME_SIZE_MS / 1000);	// TYM DSP mark
    param[7] = PCM_GAIN_FRAME_SIZE;		// TYM DSP add tofu
    param[8] = XA_PCM_GAIN_CONFIG_PARAM_GAIN_FACTOR;
    param[9] = 10;

    return xaf_comp_set_config(p_comp, 5, &param[0]);
}

static XAF_ERR_CODE renderer_setup(void *p_renderer, xaf_format_t *format, bool i2s)
{
    int param[26];
    int num_param;
    int numChannels = format->channels;
    num_param = 7;

    param[0]  = XA_RENDERER_CONFIG_PARAM_PCM_WIDTH;
    param[1]  = format->pcm_width;
    param[2]  = XA_RENDERER_CONFIG_PARAM_CHANNELS;
    param[3]  = format->channels;
    param[4]  = XA_RENDERER_CONFIG_PARAM_SAMPLE_RATE;
    param[5]  = format->sample_rate;
    param[6]  = XA_RENDERER_CONFIG_PARAM_FRAME_SIZE;
//    param[7]  = (format->pcm_width / 8) * numChannels * (format->sample_rate * FRAME_SIZE_MS / 1000);
    param[7]  = RENDERER_FRAME_SIZE;
    param[8]  = XA_RENDERER_CONFIG_PARAM_AUDIO_BUFFER_1;
    param[9]  = (int)DSP_AUDIO_BUFFER_1_PING;
    param[10] = XA_RENDERER_CONFIG_PARAM_AUDIO_BUFFER_2;
    param[11] = (int)DSP_AUDIO_BUFFER_1_PONG;
    param[12] = XA_RENDERER_CONFIG_PARAM_I2S_INTERFACE;
    param[13] = AUDIO_I2S_RENDERER_DEVICE;
    if (numChannels > 2)
    {
        num_param = 10;

        param[14] = XA_RENDERER_CONFIG_PARAM_I2S_MODE;
        param[15] = kI2S_ModeDspWsShort;
        param[16] = XA_RENDERER_CONFIG_PARAM_I2S_WS_POLARITY;
        param[17] = true;
        param[18] = XA_RENDERER_CONFIG_PARAM_I2S_POSITION;
        param[19] = 1U;

        param[20] = XA_RENDERER_CONFIG_PARAM_CODEC_PCM_WIDTH;
        param[21] = MAX_WIDTH;
        param[22] = XA_RENDERER_CONFIG_PARAM_CODEC_CHANNELS;
        param[23] = MAX_CHANNELS;
        param[24] = XA_RENDERER_CONFIG_PARAM_CODEC_FRAME_SIZE;
//        param[25] = (MAX_WIDTH / 8) * MAX_CHANNELS * (format->sample_rate * FRAME_SIZE_MS / 1000);
        param[25] = RENDERER_FRAME_SIZE;
    }

    return xaf_comp_set_config(p_renderer, num_param, &param[0]);
}

/* Explicitly start the capturer component.
 * This will initiate the I2S DMA input. */
static int capturer_start_operation(void *p_comp)
{
    int param[2];

    param[0] = XA_CAPTURER_CONFIG_PARAM_STATE;
    param[1] = XA_CAPTURER_STATE_START;

    return xaf_comp_set_config(p_comp, 1, &param[0]);
}

/* Explicitly start the renderer component.
 * This will begin the I2S DMA output with zeros until valid data comes in. */
static int renderer_start_operation(void *p_comp)
{
    int param[2];

    param[0] = XA_RENDERER_CONFIG_PARAM_STATE;
    param[1] = XA_RENDERER_STATE_START;

    return xaf_comp_set_config(p_comp, 1, &param[0]);
}


// TYM DSP add >>
#define BUF_SIZE 1024 * 1024
AT_QUICKACCESS_SECTION_CODE (uint8_t MEMPOOL_BUF[BUF_SIZE]);
//uint8_t* MEMPOOL_BUF;
static uint8_t FlowExecOnce = 0;
FlowEngine*	engine = NULL;

/*******************************************************************************
 * Commands processing
 ******************************************************************************/
#if 1
int srtm_line_inout_init(dsp_handle_t *dsp, unsigned int *pCmdParams, bool i2s)
{
    int channels, sampling_rate, width, cid, cid_prev, i;
    xaf_format_t comp_format[NUM_COMP_IN_GRAPH];
    int (*comp_setup[NUM_COMP_IN_GRAPH])(void *p_comp, xaf_format_t *, bool);
    void *usbSpeaker_inbuf[1];
    xaf_comp_type comp_type[NUM_COMP_IN_GRAPH];
    xf_id_t comp_id[NUM_COMP_IN_GRAPH];
    int comp_ninbuf[NUM_COMP_IN_GRAPH];
    int comp_noutbuf[NUM_COMP_IN_GRAPH];
    XAF_ERR_CODE ret;
    xaf_comp_status comp_status;
    xaf_adev_config_t device_config;
    xaf_comp_config_t comp_config[NUM_COMP_IN_GRAPH];
    int comp_info[4];
    uint32_t read_length;
    int renderer_state, param[4];	// TYM DSP add

    /* Parameters reading */
    channels      = pCmdParams[0];
    sampling_rate = pCmdParams[1];
    width         = pCmdParams[2];

    comp_clean(dsp);

    /* Component data preparation */
    for (i = 0; i < NUM_COMP_IN_GRAPH; i++)
    {
        cid = comp_get_order_lineinout[i];
        memset(&comp_format[cid], 0, sizeof(xaf_format_t));
        comp_setup[cid]   = NULL;
        comp_type[cid]    = -1;
        comp_id[cid]      = NULL;
        comp_ninbuf[cid]  = 0;
        comp_noutbuf[cid] = 0;

        switch (cid)
        {
            case XA_CAPTURER_0:
                comp_format[cid].sample_rate = sampling_rate;
                comp_format[cid].channels    = channels;
                comp_format[cid].pcm_width   = width;
                comp_setup[cid]              = capturer_setup;
                comp_type[cid]               = XAF_CAPTURER;
                if (i2s)
                {
                	DSP_PRINTF("[TYM][LINE] i2s\r\n");
                    comp_id[cid] = "capturer_i2s";
                }
                else
                {
                	DSP_PRINTF("[TYM][LINE] dmic\r\n");
                    comp_id[cid] = "capturer_dmic";
                }
                break;
            case XA_GAIN_0:
                comp_format[cid].sample_rate = sampling_rate;
                comp_format[cid].channels    = channels;
                comp_format[cid].pcm_width   = width;
                comp_setup[cid]              = pcm_gain_setup;
                comp_type[cid]               = XAF_POST_PROC;
                comp_id[cid]                 = "post-proc/pcm_gain";
                comp_ninbuf[cid]             = 1;
                break;
            case XA_RENDERER_0:
                comp_format[cid].sample_rate = sampling_rate;
                comp_format[cid].channels    = channels;
                comp_format[cid].pcm_width   = width;
                comp_setup[cid]              = renderer_setup;
                comp_type[cid]               = XAF_RENDERER;
                comp_id[cid]                 = "renderer";
                break;

            default:
                DSP_PRINTF("[DSP_LINE_INOUT] Check component type in comp_get_order table\n\r");
                return -1;
        }
    }
    if (FlowExecOnce == 0)
	{
//    	MEMPOOL_BUF = malloc(BUF_SIZE);
		FlowEngine_Mempool_Create(MEMPOOL_BUF, BUF_SIZE);
		engine = FlowEngine_create("engine", NULL, 8, 8);
		FlowEngine_prepare(engine, 48000, 32);

		FlowEngine_query_cmd(engine, "setRoot/lockAudio/1/");
		FlowEngine_query_cmd(engine, "clearLayout/");
		FlowEngine_query_cmd(engine, "init/17/");
		FlowEngine_query_cmd(engine, "obj/IN/IN_1/0/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/IN/IN_2/1/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/IN/IN_3/2/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/IN/IN_4/3/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/IN/IN_5/4/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/IN/IN_6/5/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/IN/IN_7/6/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/IN/IN_8/7/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/OUT/OUT_1/9/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/OUT/OUT_2/10/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/OUT/OUT_3/11/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/OUT/OUT_4/12/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/OUT/OUT_5/13/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/OUT/OUT_6/14/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/OUT/OUT_7/15/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/OUT/OUT_8/16/-1/-1/-1/0/");
		FlowEngine_query_cmd(engine, "obj/GAIN/GAIN_1/8/8/8/1/0/");
		FlowEngine_query_cmd(engine, "setCoord/GAIN_1/gain/0.0/0/0/");
		FlowEngine_query_cmd(engine, "reprepare/");
		FlowEngine_query_cmd(engine, "buildProcStep/");
		FlowEngine_query_cmd(engine, "wire/IN_1@0/GAIN_1@0/");
		FlowEngine_query_cmd(engine, "wire/IN_2@0/GAIN_1@1/");
		FlowEngine_query_cmd(engine, "wire/IN_3@0/GAIN_1@2/");
		FlowEngine_query_cmd(engine, "wire/IN_4@0/GAIN_1@3/");
		FlowEngine_query_cmd(engine, "wire/IN_5@0/GAIN_1@4/");
		FlowEngine_query_cmd(engine, "wire/IN_6@0/GAIN_1@5/");
		FlowEngine_query_cmd(engine, "wire/IN_7@0/GAIN_1@6/");
		FlowEngine_query_cmd(engine, "wire/IN_8@0/GAIN_1@7/");
		FlowEngine_query_cmd(engine, "wire/GAIN_1@0/OUT_1@0/");
		FlowEngine_query_cmd(engine, "wire/GAIN_1@1/OUT_2@0/");
		FlowEngine_query_cmd(engine, "wire/GAIN_1@2/OUT_3@0/");
		FlowEngine_query_cmd(engine, "wire/GAIN_1@3/OUT_4@0/");
		FlowEngine_query_cmd(engine, "wire/GAIN_1@4/OUT_5@0/");
		FlowEngine_query_cmd(engine, "wire/GAIN_1@5/OUT_6@0/");
		FlowEngine_query_cmd(engine, "wire/GAIN_1@6/OUT_7@0/");
		FlowEngine_query_cmd(engine, "wire/GAIN_1@7/OUT_8@0/");
		FlowEngine_query_cmd(engine, "setRoot/lockAudio/0/");
		FlowExecOnce++;
	}

    /* Initialize XAF */
    xaf_adev_config_default_init(&device_config);

    device_config.pmem_malloc                 = DSP_Malloc;
    device_config.pmem_free                   = DSP_Free;
    device_config.audio_component_buffer_size = AUDIO_COMP_BUF_SIZE;
    device_config.audio_framework_buffer_size = AUDIO_FRMWK_BUF_SIZE;

    ret = xaf_adev_open(&dsp->audio_device, &device_config);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("[DSP_LINE_INOUT] xaf_adev_open failure: %d\r\n", ret);
        return -1;
    }

    DSP_PRINTF("[DSP_LINE_INOUT] Audio Device Ready\n\r");

    /* Create and setup all components */
    for (i = 0; i < NUM_COMP_IN_GRAPH; i++)
    {
        cid = comp_get_order_lineinout[i];
        DSP_PRINTF("[TYM][DSP_LINE_INOUT] i: %d, cid: %d\r\n",i,cid);
        xaf_comp_config_default_init(&comp_config[cid]);

        comp_config[cid].comp_id            = comp_id[cid];
        comp_config[cid].num_input_buffers  = comp_ninbuf[cid];
        comp_config[cid].num_output_buffers = comp_noutbuf[cid];
        comp_config[cid].comp_type          = comp_type[cid];
        comp_config[cid].pp_inbuf = comp_ninbuf[cid] > 0 ? (pVOID(*)[XAF_MAX_INBUFS]) & usbSpeaker_inbuf[0] : NULL;
        DSP_PRINTF("[TYM][DSP_LINE_INOUT] id[%d]:%s, in_num: %d, out_num: %d,  type: %d\r\n", i, comp_config[cid].comp_id, comp_config[cid].num_input_buffers, comp_config[cid].num_output_buffers, comp_config[cid].comp_type);
        /* Create component */
        ret = xaf_comp_create(dsp->audio_device, comp_get_pointer(dsp, cid, true), &comp_config[cid]);
        if (ret != XAF_NO_ERR)
        {
            DSP_PRINTF("[DSP_LINE_INOUT] xaf_comp_create[%d] failure: %d\r\n", i, ret);
            goto error_cleanup;
        }

        /* Setup component */
        ret = comp_setup[cid](comp_get_pointer(dsp, cid, false), &comp_format[cid], i2s);
        DSP_PRINTF("[TYM][DSP_LINE_INOUT] ret:%d, id[%d]:%s, sRate: %d, ch: %d,  width: %d\r\n",ret, i, comp_config[cid].comp_id, comp_format[cid].sample_rate, comp_format[cid].channels, comp_format[cid].pcm_width);

        /* Start component */
        ret = xaf_comp_process(dsp->audio_device, comp_get_pointer(dsp, cid, false), NULL, 0, XAF_START_FLAG);
        if (ret != XAF_NO_ERR)
        {
            DSP_PRINTF("[DSP_LINE_INOUT] xaf_comp_process XAF_START_FLAG %s failure: %d\r\n", comp_id[cid], ret);
            goto error_cleanup;
        }

        /* Check status of the component */
        ret = xaf_comp_get_status(dsp->audio_device, comp_get_pointer(dsp, cid, false), &comp_status, &comp_info[0]);
        if (ret != XAF_NO_ERR)
        {
            DSP_PRINTF("[DSP_LINE_INOUT] xaf_comp_get_status %s failure: %d\r\n", comp_id[cid], ret);
            goto error_cleanup;
        }

        if (i == 1)
        {
            if (comp_status != XAF_INIT_DONE)
            {
                DSP_PRINTF("[DSP_LINE_INOUT] xaf_comp_get_status failed to init\n\r");
                goto error_cleanup;
            }
            dsp->comp = comp_get_pointer(dsp, cid, false);
        }

        DSP_PRINTF("[DSP_LINE_INOUT] %s component started\r\n", comp_id[cid]);

        /* Start renderer DMA output.  Will output I2S zeros until valid data is available. */
        if (cid == XA_RENDERER_0){
            renderer_start_operation(comp_get_pointer(dsp, cid, false));
        }
        if (cid == XA_CAPTURER_0){
            capturer_start_operation(comp_get_pointer(dsp, cid, false));
        }

        /* Connect all components */
        if (i > 0)
        {
            cid_prev = comp_get_order_lineinout[i - 1];
            if (i==1)
            	ret      = xaf_connect(comp_get_pointer(dsp, cid_prev, false), 0, comp_get_pointer(dsp, cid, false), 0, 4);
            else
            	ret      = xaf_connect(comp_get_pointer(dsp, cid_prev, false), 1, comp_get_pointer(dsp, cid, false), 0, 4);
            if (ret != XAF_NO_ERR)
            {
                DSP_PRINTF("[DSP_LINE_INOUT] xaf_connect failure: %d\r\n", ret);
                goto error_cleanup;
            }

            DSP_PRINTF("[DSP_LINE_INOUT] Connected %s -> %s\n\r", comp_id[cid_prev], comp_id[cid]);
        }
    }

//    read_length = DSP_AudioReadRing(dsp, usbSpeaker_inbuf[0], PCM_GAIN_FRAME_SIZE);
//
//    if (read_length > 0)
//        ret = xaf_comp_process(dsp->audio_device, dsp->comp, usbSpeaker_inbuf[0], read_length, XAF_INPUT_READY_FLAG);
//    else
//    {
//        DSP_PRINTF("[DSP_LINE_INOUT] Initial data not available\r\n");
//        goto error_cleanup;
//    }
//    if (ret != XAF_NO_ERR)
//    {
//        DSP_PRINTF("[DSP_LINE_INOUT] xaf_comp_process XAF_INPUT_READY_FLAG failure: %d\r\n", ret);
//        goto error_cleanup;
//    }

#if 0
    /* Delete previous cleanup thread if valid */
    if (xos_thread_get_state(&dsp->cleanup_thread) != XOS_THREAD_STATE_INVALID)
    {
        xos_thread_delete(&dsp->cleanup_thread);
        memset((void *)&dsp->cleanup_thread, 0, sizeof(XosThread));
    }

    /* Initialize playback state */
    dsp->line_inout = true;
    xos_event_clear(&dsp->pipeline_event, XOS_EVENT_BITS_ALL);

    /* Initialize buffer read/write functions */
    dsp->audio_read  = DSP_AudioReadRing;
    dsp->audio_write = DSP_AudioWriteRing;

    /* Threads */
    /* Start processing thread */
    xos_thread_create(&dsp->process_thread, NULL, DSP_ProcessThread, (void *)dsp, "DSP_ProcessThread",
    		LineInOut_process_stack, STACK_SIZE, 7, 0, 0);

    /* Start cleanup/exit thread */
    xos_thread_create(&dsp->cleanup_thread, NULL, srtm_lineInOut_cleanup, (void *)dsp, "srtm_lineInOut_cleanup",
    		LineInOut_cleanup_stack, STACK_SIZE, 7, 0, 0);
#else
    DSP_PRINTF("[TYM][DSP] Start capturer->pcm/gain->renderer\r\n");
//    while (1)
//    {
//        /* Sleep 100 msec */
//        xos_thread_sleep_msec(100);
//
//        /* Check renderer for errors */
//        cid            = XA_RENDERER_0;
//        param[0]       = XA_RENDERER_CONFIG_PARAM_STATE;
//        param[2]       = XA_RENDERER_CONFIG_PARAM_BYTES_PRODUCED;
//        ret            = xaf_comp_get_config(comp_get_pointer(dsp, cid, false), 2, &param[0]);
//        renderer_state = param[1];
//
//        if (ret != XAF_NO_ERR)
//        {
//            DSP_PRINTF("renderer get_config error:%x\n", ret);
//            return -1;
//        }
//    }

    char flowCmdResult[64] = "Success, Flow_path_initialize done";
    FlowDSP_SendCmdBack(dsp, flowCmdResult);
#endif

    return 0;

error_cleanup:
    return LineInOut_close(dsp, false);
}
#else
int srtm_line_inout_init(dsp_handle_t *dsp, unsigned int *pCmdParams, bool i2s)
{
    void *p_adev = NULL;
    void *p_comp[NUM_COMP_IN_GRAPH];
    xf_id_t comp_id[NUM_COMP_IN_GRAPH];
    int i, k, cid;
    int (*comp_setup[NUM_COMP_IN_GRAPH])(void *p_comp, xaf_format_t *, bool);
    xaf_comp_type comp_type[NUM_COMP_IN_GRAPH];
    xaf_format_t comp_format[NUM_COMP_IN_GRAPH];
    xaf_comp_status comp_status;
    int comp_ninbuf[NUM_COMP_IN_GRAPH];
    int comp_noutbuf[NUM_COMP_IN_GRAPH];
    int param[4], renderer_state;
    int channels, sampling_rate, width;
    int info[4];
    XAF_ERR_CODE ret;
    xaf_adev_config_t device_config;
    xaf_comp_config_t comp_config[NUM_COMP_IN_GRAPH];

    channels      = pCmdParams[0];
    sampling_rate = pCmdParams[1];
    width         = pCmdParams[2];

    for (i = 0; i < NUM_COMP_IN_GRAPH; i++)
    {
        cid         = comp_get_order_lineinout[i];
        p_comp[cid] = NULL;
        memset(&comp_format[cid], 0, sizeof(xaf_format_t));
        comp_setup[cid]   = NULL;
        comp_type[cid]    = -1;
        comp_id[cid]      = NULL;
        comp_ninbuf[cid]  = 0;
        comp_noutbuf[cid] = 0;

        switch (cid)
        {
            case XA_CAPTURER_0:
                comp_format[cid].sample_rate = sampling_rate;
                comp_format[cid].channels    = channels;
                comp_format[cid].pcm_width   = width;
                comp_setup[cid]              = capturer_setup;
                comp_type[cid]               = XAF_CAPTURER;
                if (i2s)
                {
                    comp_id[cid] = "capturer_i2s";
                }
                else
                {
                    comp_id[cid] = "capturer_dmic";
                }
                break;

            case XA_GAIN_0:
                comp_format[cid].sample_rate = sampling_rate;
                comp_format[cid].channels    = channels;
                comp_format[cid].pcm_width   = width;
                comp_setup[cid]              = pcm_gain_setup;
                comp_type[cid]               = XAF_POST_PROC;
                comp_id[cid]                 = "post-proc/pcm_gain";
                break;
            case XA_RENDERER_0:
                comp_format[cid].sample_rate = sampling_rate;
                comp_format[cid].channels    = channels;
                comp_format[cid].pcm_width   = width;
                comp_setup[cid]              = renderer_setup;
                comp_type[cid]               = XAF_RENDERER;
                comp_id[cid]                 = "renderer";
                break;

            default:
                DSP_PRINTF("Check component type in comp_get_order_lineinout table\n\r");
                return -1;
        }
    }

    xaf_adev_config_default_init(&device_config);

    device_config.pmem_malloc                 = DSP_Malloc;
    device_config.pmem_free                   = DSP_Free;
    device_config.audio_component_buffer_size = AUDIO_COMP_BUF_SIZE;
    device_config.audio_framework_buffer_size = AUDIO_FRMWK_BUF_SIZE;

    ret = xaf_adev_open(&p_adev, &device_config);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_adev_open failure: %d\r\n", ret);
        return -1;
    }

    DSP_PRINTF("\nAudio Device Ready\n\r");

    /* Create and setup all components */
    for (k = 0; k < NUM_COMP_IN_GRAPH; k++)
    {
        cid = comp_get_order_lineinout[k];
		DSP_PRINTF("[TYM][DSP_LINE_INOUT] k: %d, cid: %d\r\n",k,cid);
        xaf_comp_config_default_init(&comp_config[cid]);

        comp_config[cid].comp_id            = comp_id[cid];
        comp_config[cid].num_input_buffers  = comp_ninbuf[cid];
        comp_config[cid].num_output_buffers = comp_noutbuf[cid];
        comp_config[cid].comp_type          = comp_type[cid];
        DSP_PRINTF("[TYM] cid: %d\n\r",k);
        ret = xaf_comp_create(p_adev, &p_comp[cid], &comp_config[cid]);
        if (ret != XAF_NO_ERR)
        {
            DSP_PRINTF("xaf_comp_create[%d] failure: %d\r\n", k, ret);
            return -1;
        }
        DSP_PRINTF("[TYM] comp_setup --\n\r");
        ret = comp_setup[cid](p_comp[cid], &comp_format[cid], i2s);
        if (ret != XAF_NO_ERR)
        {
            DSP_PRINTF("comp_setup failure: %d\r\n", ret);
            return -1;
        }
    }

    /* Start capturer */
    ret = xaf_comp_process(p_adev, p_comp[XA_CAPTURER_0], NULL, 0, XAF_START_FLAG);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_comp_process XAF_START_FLAG CAPTURER_0 failure: %d\r\n", ret);
        return -1;
    }

    ret = xaf_comp_get_status(p_adev, p_comp[XA_CAPTURER_0], &comp_status, &info[0]);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_comp_get_status CAPTURER_0 failure: %d\r\n", ret);
        return -1;
    }

    /* Start renderer component */
    ret = xaf_comp_process(p_adev, p_comp[XA_RENDERER_0], NULL, 0, XAF_START_FLAG);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_comp_process XAF_START_FLAG RENDERER_0 failure: %d\r\n", ret);
        return -1;
    }

    ret = xaf_comp_get_status(p_adev, p_comp[XA_RENDERER_0], &comp_status, &info[0]);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_comp_get_status RENDERER_0 failure: %d\r\n", ret);
        return -1;
    }

    /* Connect all the non-input components and then only START each of the dest components */
    ret = xaf_connect(p_comp[XA_CAPTURER_0], 0, p_comp[XA_GAIN_0], 0, 4);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_connect CAPTURER_0 -> GAIN_0 failure: %d\r\n", ret);
        return -1;
    }
    ret = capturer_start_operation(p_comp[XA_CAPTURER_0]);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("Capturer start operation failure.\r\n");
        return -1;
    }

    DSP_PRINTF("connected CAPTURER -> GAIN_0\n\r");

    /* Start PCM gain */
    ret = xaf_comp_process(p_adev, p_comp[XA_GAIN_0], NULL, 0, XAF_START_FLAG);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_comp_process XAF_START_FLAG GAIN_0 failure: %d\r\n", ret);
        return -1;
    }

    ret = xaf_comp_get_status(p_adev, p_comp[XA_GAIN_0], &comp_status, &info[0]);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_comp_get_status GAIN_0 failure: %d\r\n", ret);
        return -1;
    }

    /* Start renderer DMA output.  Will output I2S zeros until valid data is
     * available. */
    renderer_start_operation(p_comp[XA_RENDERER_0]);

    ret = xaf_connect(p_comp[XA_GAIN_0], 1, p_comp[XA_RENDERER_0], 0, 4);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_connect GAIN_0 -> RENDERER_0 failure: %d\r\n", ret);
        return -1;
    }

    DSP_PRINTF("connected XA_GAIN_0 -> XA_RENDERER_0\n\r");


    // TYM DSP add >>
//    get_render_config(p_comp[XA_RENDERER_0], &comp_format[XA_RENDERER_0]);
    // TYM DSP add <<

    while (1)
    {
        /* Sleep 100 msec */
        xos_thread_sleep_msec(100);

        /* Check renderer for errors */
        cid            = XA_RENDERER_0;
        param[0]       = XA_RENDERER_CONFIG_PARAM_STATE;
        param[2]       = XA_RENDERER_CONFIG_PARAM_BYTES_PRODUCED;
        ret            = xaf_comp_get_config(p_comp[cid], 2, &param[0]);
        renderer_state = param[1];

        if (ret != XAF_NO_ERR)
        {
            DSP_PRINTF("renderer get_config error:%x\n", ret);
            return -1;
        }
    }
    DSP_PRINTF("[TYM] out of while\n\r");

    for (k = 0; k < NUM_COMP_IN_GRAPH; k++)
    {
        cid = comp_get_order_lineinout[k];
        ret = xaf_comp_delete(p_comp[cid]);
        if (ret != XAF_NO_ERR)
        {
            DSP_PRINTF("xaf_comp_delete[%d] failure: %d\r\n", k, ret);
            return -1;
        }
    }

    xaf_adev_close(p_adev, XAF_ADEV_NORMAL_CLOSE);
    if (ret != XAF_NO_ERR)
    {
        DSP_PRINTF("xaf_adev_close failure: %d\r\n", ret);
        return -1;
    }

    DSP_PRINTF("Audio device closed\n\n\r\r");

    return 0;
}
#endif

int srtm_lineInOut_cleanup(void *arg, int wake_value)
{
    int32_t exitcode;
    dsp_handle_t *dsp = (dsp_handle_t *)arg;

    DSP_PRINTF("[DSP_CleanupThread] start\r\n");

    /* Wait for processing thread to complete before exiting. */
    xos_thread_join(&dsp->process_thread, &exitcode);
    xos_thread_delete(&dsp->process_thread);

    return LineInOut_close(dsp, true);
}
