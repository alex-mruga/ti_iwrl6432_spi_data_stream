/**
 * @file main.c
 * @brief Main Program for Radar Signal Processing.
 *
 * This file contains the main entry point and initialization logic for the radar
 * signal processing application. It sets up the hardware, initializes the radar
 * sensor, configures the Data Processing Units (DPUs), and starts the FreeRTOS
 * scheduler to manage tasks for radar processing and UART communication.
 *
 * The application performs the following key steps:
 * 1. Initializes the hardware and drivers.
 * 2. Configures the radar sensor and performs factory calibration.
 * 3. Initializes the DPUs for range processing.
 * 4. Creates FreeRTOS tasks for radar processing (DPC) and UART communication.
 * 5. Starts the radar sensor and enters the FreeRTOS scheduler.
 *
 * This implementation is adapted from the Motion and Presence Detection Demo
 * provided in the TI mmWave SDK.
 *
 * @note This module relies on the TI mmWave SDK, FreeRTOS, and various hardware
 *       drivers for radar sensor control, DPU configuration, and UART communication.
 *
 * @copyright (C) 2022-24 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <kernel/dpl/DebugP.h>
#include "board/flash.h"
#include "drivers/uart/v0/uart_sci.h"
#include "kernel/dpl/SystemP.h"
#include "ti_drivers_config.h"
#include "ti_board_config.h"
#include "ti_drivers_open_close.h"
#include "ti_drivers_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include <mmwavelink/mmwavelink.h>
#include <mmwavelink/include/rl_device.h>

#include <kernel/dpl/SemaphoreP.h>
#include <datapath/dpu/rangeproc/v0/rangeprochwa.h>
#include "rangeproc_dpc.h"
#include "mmwave_basic.h"
#include "mmwave_control_config.h"
#include "factory_cal.h"

// --- FRERTOS
//#define MAIN_TASK_PRI  (configMAX_PRIORITIES-1)
#define MAIN_TASK_PRI  1

#define MAIN_TASK_SIZE (16384U/sizeof(configSTACK_DEPTH_TYPE))
#define DPC_TASK_STACK_SIZE 8192
#define UART_TASK_STACK_SIZE 2048

#define DPC_TASK_PRI 5
#define UART_TASK_PRI 10

StaticTask_t gMainTaskObj;
TaskHandle_t gMainTask;
StackType_t gMainTaskStack[MAIN_TASK_SIZE] __attribute__((aligned(32)));
// ---
StaticTask_t gDpcTaskObj;
TaskHandle_t gDpcTask;
StackType_t  gDpcTaskStack[DPC_TASK_STACK_SIZE] __attribute__((aligned(32)));
// ---
StaticTask_t gUartTaskObj;
TaskHandle_t gUartTask;
StackType_t  gUartTaskStack[UART_TASK_STACK_SIZE] __attribute__((aligned(32)));

T_RL_API_FECSS_RUNTIME_TX_CLPC_CAL_CMD fecTxclpcCalCmd;

// Semaphores
SemaphoreP_Object pend_main_sem;
SemaphoreP_Object dpcCfgDoneSemHandle;

SemaphoreP_Object uart_tx_start_sem;
SemaphoreP_Object uart_tx_done_sem;



//external
extern T_RL_API_FECSS_RF_PWR_CFG_CMD channelCfg;

void rangeproc_main(void *args);

void freertos_main(void *args)
{
    /*** INIT ***/
    /* Peripheral Driver Initialization */
    Drivers_open();
    Board_driversOpen();

    // init uart
    //UART_init();

    /* Create binary semaphore to pend Main task and wait for dpu config */
    SemaphoreP_constructBinary(&pend_main_sem, 0);
    SemaphoreP_constructBinary(&dpcCfgDoneSemHandle, 0);

    SemaphoreP_constructBinary(&uart_tx_start_sem, 0);
    SemaphoreP_constructBinary(&uart_tx_done_sem, 0);
    
    // Mmwave_HwaConfig_custom();
    /* The following function call and comment is copied from the motion and presence detection demo (motion_detect.c motion_detect()) */
    /*HWASS_SHRD_RAM, TPCCA and TPCCB memory have to be init before use. */
    /*APPSS SHRAM0 and APPSS SHRAM1 memory have to be init before use. However, for awrL varients these are initialized by RBL */
    /*FECSS SHRAM (96KB) has to be initialized before use as RBL does not perform initialization.*/
    SOC_memoryInit(SOC_RCM_MEMINIT_HWA_SHRAM_INIT|SOC_RCM_MEMINIT_TPCCA_INIT|SOC_RCM_MEMINIT_TPCCB_INIT|SOC_RCM_MEMINIT_FECSS_SHRAM_INIT|SOC_RCM_MEMINIT_APPSS_SHRAM0_INIT|SOC_RCM_MEMINIT_APPSS_SHRAM1_INIT);
    DebugP_log("starting init \n");

    if(gFlashHandle[0] == NULL) {
        DebugP_log("Flash initialization failed!");
    }

    // initialize memory segments from memory pools
    mempool_init();

    // initialize default antenna geometry
    

    if(mmwave_initSensor() == SystemP_FAILURE){
        exit(1);
    }

    
    if(hwa_open_handler() == SystemP_FAILURE){
        exit(1);
    }

    // init all required DPUs
    rangeProc_dpuInit();

    DebugP_log("init passed");
    /* FECSS RF Power ON*/

    MMWave_populateChannelCfg();

    int32_t retVal;
    retVal = rl_fecssRfPwrOnOff(M_DFP_DEVICE_INDEX_0, &channelCfg);
    if(retVal != M_DFP_RET_CODE_OK)
    {
        DebugP_log("Error: FECSS RF Power ON/OFF failed\r\n");
        retVal = SystemP_FAILURE;
        exit(1);
    }

    /* Check if the device is RF-Trimmed */
    /* Checking one Trim is enough */
    if(SOC_rcmReadSynthTrimValid() != 1U) { // 1 is valid
        DebugP_log("Error: Device is not RF-Trimmed!\r\n");
        exit(1);
    }

    /*** CONFIG ***/
    // TODO: factory calibration (mmwDemo_factoryCal()) 
    if(mmwave_openSensor() == SystemP_FAILURE){
        exit(1);
    }
    if(mmwave_configSensor() == SystemP_FAILURE){
        exit(1);
    }

    // /* Perform factory Calibrations. */
    retVal = restoreFactoryCal();
    if(retVal != SystemP_SUCCESS)
    {
        DebugP_log("Error: mmWave factory calibration failed\r\n");
        retVal = SystemP_FAILURE;
    }

    gDpcTask = xTaskCreateStatic(dpcTask, /* Pointer to the function that implements the task. */
                                 "dpc_task",      /* Text name for the task.  This is to facilitate debugging only. */
                                 DPC_TASK_STACK_SIZE,   /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                 NULL,                  /* We are not using the task parameter. */
                                 DPC_TASK_PRI,          /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                 gDpcTaskStack,      /* pointer to stack base */
                                 &gDpcTaskObj);         /* pointer to statically allocated task object memory */
    configASSERT(gDpcTask != NULL);

    SemaphoreP_pend(&dpcCfgDoneSemHandle, SystemP_WAIT_FOREVER);

    gUartTask = xTaskCreateStatic(uartTask, /* Pointer to the function that implements the task. */
                                 "uart_task",      /* Text name for the task.  This is to facilitate debugging only. */
                                 UART_TASK_STACK_SIZE,   /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                 NULL,                  /* We are not using the task parameter. */
                                 UART_TASK_PRI,          /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                 gUartTaskStack,      /* pointer to stack base */
                                 &gUartTaskObj);         /* pointer to statically allocated task object memory */
    configASSERT(gUartTask != NULL);

    if(mmwave_startSensor() == SystemP_FAILURE){
        exit(1);
    }
    
        /* Never return for this task. */
    SemaphoreP_pend(&pend_main_sem, SystemP_WAIT_FOREVER);

    Board_driversClose();
    Drivers_close();

    vTaskDelete(NULL);
}


int main()
{
    /* init SOC specific modules */
    System_init();
    Board_init();

    /* This task is created at highest priority, it should create more tasks and then delete itself */
    gMainTask = xTaskCreateStatic( freertos_main,   /* Pointer to the function that implements the task. */
                                  "freertos_main", /* Text name for the task.  This is to facilitate debugging only. */
                                  MAIN_TASK_SIZE,  /* Stack depth in units of StackType_t typically uint32_t on 32b CPUs */
                                  NULL,            /* We are not using the task parameter. */
                                  MAIN_TASK_PRI,   /* task priority, 0 is lowest priority, configMAX_PRIORITIES-1 is highest */
                                  gMainTaskStack,  /* pointer to stack base */
                                  &gMainTaskObj ); /* pointer to statically allocated task object memory */
    configASSERT(gMainTask != NULL);

    /* Start the scheduler to start the tasks executing. */
    vTaskStartScheduler();

    /* The following line should never be reached because vTaskStartScheduler()
    will only return if there was not enough FreeRTOS heap memory available to
    create the Idle and (if configured) Timer tasks.  Heap management, and
    techniques for trapping heap exhaustion, are described in the book text. */
    DebugP_assertNoLog(0);

    return 0;
}


