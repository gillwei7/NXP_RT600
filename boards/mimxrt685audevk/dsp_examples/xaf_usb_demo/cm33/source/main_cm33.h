/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MAIN_CM33_H__
#define __MAIN_CM33_H__

#include "FreeRTOS.h"
#include "task.h"

#include "rpmsg_lite.h"
#include "rpmsg_ns.h"
#include "rpmsg_queue.h"

#define CUSTOM_SHELL_TASK 1
/*******************************************************************************
 * Definitions
 ******************************************************************************/
typedef struct _app_handle
{
    TaskHandle_t shell_task_handle;
    TaskHandle_t ipc_task_handle;
    TaskHandle_t trigger_task_handle;
} app_handle_t;

enum
{
    FlowCmd_Step0_PreFlowPathInit = 0,
    FlowCmd_Step1_Header1,
	FlowCmd_Step2_Header2,
	FlowCmd_Step3_Length1,
	FlowCmd_Step4_Length2,
	FlowCmd_Step5_Cmd
};

#endif /* __MAIN_CM33_H__ */
