/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "main_cm33.h"
#include "fsl_debug_console.h"


#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "dsp_support.h"
#include "dsp_ipc.h"
#include "cmd.h"

#include "composite.h"

#include "dsp_config.h"
#include "fsl_power.h"
#include "fsl_pca9420.h"

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#include "fsl_ctimer.h"
#endif
#include "usb_device_hid.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_CODEC_VOLUME 100U
#define APP_TASK_STACK_SIZE (6 * 1024)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void USB_DeviceClockInit(void);
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
void CTIMER_SOF_TOGGLE_HANDLER_PLL(uint32_t i);
#else
extern void USB_DeviceCalculateFeedback(void);
#endif
extern usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
extern void USB_DeviceApplicationInit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_device_composite_struct_t g_composite;

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
ctimer_callback_t *cb_func_pll[] = {(ctimer_callback_t *)CTIMER_SOF_TOGGLE_HANDLER_PLL};
static ctimer_config_t ctimerInfoPll;
#endif

app_handle_t app;
extern int8_t FlowCmdStep;
extern usb_device_composite_struct_t *g_UsbDeviceComposite;

/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_DeviceClockInit(void)
{
    uint8_t usbClockDiv = 1;
    uint32_t usbClockFreq;
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    /* attach AUDIO PLL clock to SCTimer input7. */
    CLOCK_AttachClk(kAUDIO_PLL_to_CTIMER0);
    g_composite.audioUnified.curAudioPllFrac = CLKCTL1->AUDIOPLL0NUM;
#endif

    /* enable USB IP clock */
    CLOCK_SetClkDiv(kCLOCK_DivPfc1Clk, 5);
    CLOCK_AttachClk(kXTALIN_CLK_to_USB_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivUsbHsFclk, usbClockDiv);
    CLOCK_EnableUsbhsDeviceClock();
    RESET_PeripheralReset(kUSBHS_PHY_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSBHS_DEVICE_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSBHS_HOST_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSBHS_SRAM_RST_SHIFT_RSTn);
    /*Make sure USDHC ram buffer has power up*/
    POWER_DisablePD(kPDRUNCFG_APD_USBHS_SRAM);
    POWER_DisablePD(kPDRUNCFG_PPD_USBHS_SRAM);
    POWER_ApplyPD();

    /* save usb ip clock freq*/
    usbClockFreq = g_xtalFreq / usbClockDiv;
    /* enable USB PHY PLL clock, the phy bus clock (480MHz) source is same with USB IP */
    CLOCK_EnableUsbHs0PhyPllClock(kXTALIN_CLK_to_USB_CLK, usbClockFreq);

#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL_SYS_CLK_HZ, &phyConfig);

    /* the following code should run after phy initialization and should wait some microseconds to make sure utmi clock
     * valid */
    /* enable usb1 host clock */
    CLOCK_EnableClock(kCLOCK_UsbhsHost);
    /*  Wait until host_needclk de-asserts */
    while (SYSCTL0->USBCLKSTAT & SYSCTL0_USBCLKSTAT_HOST_NEED_CLKST_MASK)
    {
        __ASM("nop");
    }
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    USBHSH->PORTMODE |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_UsbhsHost);
}

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
void USB_AudioPllChange(void)
{
    CLKCTL1->AUDIOPLL0NUM = g_composite.audioUnified.curAudioPllFrac;
}

void CTIMER_CaptureInit(void)
{
    INPUTMUX->CT32BIT_CAP_SEL[0][0] = 0x12U; /* 0xFU for USB1.*/
    CTIMER_GetDefaultConfig(&ctimerInfoPll);

    /* Initialize CTimer module */
    CTIMER_Init(CTIMER0, &ctimerInfoPll);

    CTIMER_SetupCapture(CTIMER0, kCTIMER_Capture_0, kCTIMER_Capture_RiseEdge, true);

    CTIMER_RegisterCallBack(CTIMER0, (ctimer_callback_t *)&cb_func_pll[0], kCTIMER_SingleCallback);

    /* Start the L counter */
    CTIMER_StartTimer(CTIMER0);
}

void USB_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_composite.deviceHandle);
}

void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#endif

#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
}
#endif

void handleShellMessage(srtm_message *msg, void *arg)
{
    /* Send message to the DSP */
    dsp_ipc_send_sync(msg);

    /* Wait for response message to be processed before returning to shell. */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
}

static bool checkShellCmdState(void)
{
    char flowPathInitChar = '1';
    if(FlowCmdStep == FlowCmd_Step0_PreFlowPathInit)
    {
        send_FlowPathInit_Cmd(flowPathInitChar);
    }
    else
    {
        PRINTF("checkShellCmdState:%d\r\n",FlowCmdStep);
    }
}

void APP_Audio_Path_Init_Task(void *param)
{
    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };
    PRINTF("[APP_Audio_Path_Init_Task] start\r\n");
    GPIO_PortInit(BOARD_INITPINS_LED0853_GPIO, BOARD_INITPINS_LED0853_PORT);
    GPIO_PinInit(GPIO, BOARD_INITPINS_LED0853_PORT, BOARD_INITPINS_LED0853_PIN, &led_config);

    while (1)
    {
        vTaskDelay( pdMS_TO_TICKS( AUDIO_PATH_INIT_MS ) );

        /* Check shell read command state */
        checkShellCmdState();
    }
}

void APP_Shell_Task(void *param)
{
    PRINTF("[APP_Shell_Task] start\r\n");

    /* Handle shell commands.  Return when 'exit' command entered. */
    shellCmd(handleShellMessage, param);

    PRINTF("\r\n[APP_Shell_Task] audio demo end\r\n");
    while (1)
        ;
}

void APP_DSP_IPC_Task(void *param)
{
    srtm_message msg;
    app_handle_t *app = (app_handle_t *)param;

    PRINTF("[APP_DSP_IPC_Task] start\r\n");

    while (1)
    {
        /* Block for IPC message from DSP */
        dsp_ipc_recv_sync(&msg);
        /* Process message */
        handleDSPMessage(app, &msg);
    }
}

/*!
 * @brief Main function
 */

int main(void)
{
    int ret;

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* Clear MUA reset before run DSP core */
    RESET_PeripheralReset(kMU_RST_SHIFT_RSTn);

    /* I2C */
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM2);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 1);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    PRINTF("\r\n");
    PRINTF("******************************\r\n");
    PRINTF("DSP audio framework demo start\r\n");
    PRINTF("******************************\r\n");
    PRINTF("\r\n");

    /* Initialize RPMsg IPC interface between ARM and DSP cores. */
    BOARD_DSP_IPC_Init();

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

#if DSP_IMAGE_COPY_TO_RAM
    PRINTF("DSP image copied to DSP TCM\r\n");
#endif

    /* Initialize USB */
    USB_DeviceApplicationInit();


    /* Set IPC processing task priority = 3 */
    if (xTaskCreate(APP_DSP_IPC_Task, "DSP Msg Task", APP_TASK_STACK_SIZE, &app, tskIDLE_PRIORITY + 3,
                    &app.ipc_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create application task\r\n");
        while (1)
            ;
    }

    /* Set shell command task priority = 1 */
    if (xTaskCreate(APP_Shell_Task, "Shell Task", APP_TASK_STACK_SIZE, &app, tskIDLE_PRIORITY + 1,
                    &app.shell_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create application task\r\n");
        while (1)
            ;
    }

    /* Set Audio Path Init task priority = 2 */
    if (xTaskCreate(APP_Audio_Path_Init_Task, "Audio Path Init Task", APP_TASK_STACK_SIZE, &app, tskIDLE_PRIORITY + 2,
                    &app.audio_path_init_task_handle) != pdPASS)
    {
        PRINTF("\r\nFailed to create APP_Audio_Path_Init_Task\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();

    /* Shoud not reach this statement. */
    return 0;
}
