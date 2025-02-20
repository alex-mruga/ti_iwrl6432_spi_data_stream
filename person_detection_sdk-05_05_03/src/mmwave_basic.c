/*
 * Copyright (C) 2022-24 Texas Instruments Incorporated
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the name of Texas Instruments Incorporated nor the names of
 *   its contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
 
#include "drivers/hwa.h"
#include "kernel/dpl/SystemP.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include "kernel/dpl/DebugP.h"
#include "defines.h"
#include "mem_pool.h"

#include "mmwave_basic.h"

/**************************************************************************
 ************************** Extern Definitions ****************************
 **************************************************************************/
extern void Mmwave_populateDefaultOpenCfg (MMWave_OpenCfg* ptrOpenCfg);
extern void Mmwave_populateDefaultChirpControlCfg (MMWave_CtrlCfg* ptrCtrlCfg);
extern void Mmwave_populateDefaultCalibrationCfg (MMWave_CalibrationCfg* ptrCalibrationCfg);
extern void Mmwave_populateDefaultStartCfg (MMWave_StrtCfg* ptrStartCfg);
/**************************************************************************/


/*! L3 RAM buffer for object detection DPC */
#define L3_MEM_SIZE (0x40000 + 160*1024)
uint8_t gMmwL3[L3_MEM_SIZE]  __attribute((section(".l3")));

/*! Local RAM buffer for object detection DPC */
#define MMWDEMO_OBJDET_CORE_LOCAL_MEM_SIZE ((8U+6U+4U+2U+8U) * 1024U)
uint8_t gMmwCoreLocMem[MMWDEMO_OBJDET_CORE_LOCAL_MEM_SIZE];

void mempool_init(void){
    /* Shared memory pool for rangeproc DPU (window)*/
    L3RamObj.cfg.addr = (void *)&gMmwL3[0];
    L3RamObj.cfg.size = sizeof(gMmwL3);

        /* Local memory pool */
    CoreLocalRamObj.cfg.addr = (void *)&gMmwCoreLocMem[0];
    CoreLocalRamObj.cfg.size = sizeof(gMmwCoreLocMem);
}

int32_t hwa_open_handler() {
    // Status handle for HWA_open
    int32_t status = SystemP_SUCCESS;

    hwaHandle = HWA_open(0, NULL, &status);
    if (hwaHandle == NULL)
    {
        DebugP_log("Error: Unable to open the HWA Instance err:%d\n", status);
        DebugP_assert(0);
        status = SystemP_FAILURE;
    }
    DebugP_log("Successfully opened HWA");
    return status;
}

int32_t mmwave_initSensor()
{
    int32_t             errCode;
    int32_t             retVal = SystemP_SUCCESS;
    MMWave_InitCfg      initCfg;
    MMWave_ErrorLevel   errorLevel;
    int16_t             mmWaveErrorCode;
    int16_t             subsysErrorCode;

    /* Initialize the mmWave control init configuration */
    memset ((void*)&initCfg, 0, sizeof(MMWave_InitCfg));

    initCfg.iswarmstart = false;

    /* Initialize and setup the mmWave Control module */
    gCtrlHandle = MMWave_init(&initCfg, &errCode);
    if (gCtrlHandle == NULL)
    {
        /* Error: Unable to initialize the mmWave control module */
        MMWave_decodeError(errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);

        /* Error: Unable to initialize the mmWave control module */
        DebugP_log("Error: mmWave Control Initialization failed [Error code %d] [errorLevel %d] [mmWaveErrorCode %d] [subsysErrorCode %d]\n", errCode, errorLevel, mmWaveErrorCode, subsysErrorCode);
        retVal = SystemP_FAILURE;
        
    }
    return retVal;
}

int32_t mmwave_openSensor(void)
{
    int32_t             errCode;
    MMWave_ErrorLevel   errorLevel;
    int16_t             mmWaveErrorCode;
    int16_t             subsysErrorCode;
    int32_t             retVal = SystemP_SUCCESS;
    

    Mmwave_populateDefaultOpenCfg(&mmwOpenCfg);
    /**********************************************************
     **********************************************************/

    /* Open mmWave module, this is only done once */

    /* Open the mmWave module: */
    if (MMWave_open (gCtrlHandle, &mmwOpenCfg, &errCode) < 0)
    {
        /* Error: decode and Report the error */
        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);
        DebugP_log ("Error: mmWave Open failed [Error code: %d Subsystem: %d]\n",
                        mmWaveErrorCode, subsysErrorCode);
        retVal = SystemP_FAILURE;
    }

    return retVal;
}

int32_t mmwave_configSensor(void)
{
    int32_t     errCode;
    int32_t     retVal = SystemP_SUCCESS;

    Mmwave_populateDefaultChirpControlCfg (&mmwCtrlCfg); /* regular frame config */

    /* Configure the mmWave module: */
    if (MMWave_config (gCtrlHandle, &mmwCtrlCfg, &errCode) < 0)
    {
        MMWave_ErrorLevel   errorLevel;
        int16_t             mmWaveErrorCode;
        int16_t             subsysErrorCode;

        /* Error: Report the error */
        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);
        DebugP_log("Error: mmWave Config failed [Error code: %d Subsystem: %d]\n",
                        mmWaveErrorCode, subsysErrorCode);

        retVal = SystemP_FAILURE;
    }

    return retVal;
}

int32_t mmwave_startSensor(void)
{
    MMWave_CalibrationCfg   calibrationCfg;
    int32_t                 retVal = SystemP_SUCCESS;
    int32_t                 errCode;

    /*****************************************************************************
     * RF :: now start the RF and the real time ticking
     *****************************************************************************/
    /* Initialize the calibration configuration: */
    memset ((void *)&calibrationCfg, 0, sizeof(MMWave_CalibrationCfg));
    /* Populate the calibration configuration: */
    Mmwave_populateDefaultCalibrationCfg(&calibrationCfg);

    /* Populate the start configuration: */
    Mmwave_populateDefaultStartCfg(&sensorStartCfg);

    DebugP_log("App: MMWave_start Issued\n");

    /* Start the mmWave module: The configuration has been applied successfully. */
    if (MMWave_start(gCtrlHandle, &calibrationCfg, &sensorStartCfg, &errCode) < 0)
    {
        MMWave_ErrorLevel   errorLevel;
        int16_t             mmWaveErrorCode;
        int16_t             subsysErrorCode;

        /* Error/Warning: Unable to start the mmWave module */
        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);
        DebugP_log("Error: mmWave Start failed [mmWave Error: %d Subsys: %d]\n", mmWaveErrorCode, subsysErrorCode);
        /* datapath has already been moved to start state; so either we initiate a cleanup of start sequence or
           assert here and re-start from the beginning. For now, choosing the latter path */
        retVal = SystemP_FAILURE;
    }

    return retVal;
}

int32_t mmwave_stop_close_deinit(void)
{
    int32_t                 errCode;
    int32_t                 retVal = SystemP_SUCCESS;

    if (MMWave_stop(gCtrlHandle,&errCode) < 0)
    {
        MMWave_ErrorLevel   errorLevel;
        int16_t             mmWaveErrorCode;
        int16_t             subsysErrorCode;

        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);
        DebugP_log("Error: mmWave Stop failed [Error code: %d Subsystem: %d]\n",
                        mmWaveErrorCode, subsysErrorCode);
        retVal = SystemP_FAILURE;
    }

    if (MMWave_close(gCtrlHandle,&errCode) < 0)
    {
        MMWave_ErrorLevel   errorLevel;
        int16_t             mmWaveErrorCode;
        int16_t             subsysErrorCode;

        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);
        DebugP_log("Error: mmWave Close failed [Error code: %d Subsystem: %d]\n",
                        mmWaveErrorCode, subsysErrorCode);
        retVal = SystemP_FAILURE;
    }

    if (MMWave_deinit(gCtrlHandle,&errCode) < 0)
    {
        MMWave_ErrorLevel   errorLevel;
        int16_t             mmWaveErrorCode;
        int16_t             subsysErrorCode;

        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);
        DebugP_log("Error: mmWave De-Init failed [Error code: %d Subsystem: %d]\n",
                        mmWaveErrorCode, subsysErrorCode);
        retVal = SystemP_FAILURE;
    }

    return retVal;
}