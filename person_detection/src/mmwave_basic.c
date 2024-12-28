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
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include "kernel/dpl/DebugP.h"

#include "mmwave_basic.h"

/**************************************************************************
 ************************** Extern Definitions ****************************
 **************************************************************************/
extern void Mmwave_populateDefaultOpenCfg (MMWave_OpenCfg* ptrOpenCfg);
extern void Mmwave_populateDefaultChirpControlCfg (MMWave_CtrlCfg* ptrCtrlCfg);
/**************************************************************************/


HWA_Handle hwaHandle = NULL;

/*! @brief This is the mmWave control handle which is used to configure the BSS. */
MMWave_Handle ctrlHandle;
/*! @brief  Configuration to open DFP */
MMWave_OpenCfg mmwOpenCfg;

MMWave_CtrlCfg mmwCtrlCfg;

void board_init()
{
    /* Peripheral Driver Initialization */
    Drivers_open();
    Board_driversOpen();

    /* The following function call and comment is copied from the motion and presence detection demo (motion_detect.c motion_detect()) */
    /*HWASS_SHRD_RAM, TPCCA and TPCCB memory have to be init before use. */
    /*APPSS SHRAM0 and APPSS SHRAM1 memory have to be init before use. However, for awrL varients these are initialized by RBL */
    /*FECSS SHRAM (96KB) has to be initialized before use as RBL does not perform initialization.*/
    SOC_memoryInit(SOC_RCM_MEMINIT_HWA_SHRAM_INIT|SOC_RCM_MEMINIT_TPCCA_INIT|SOC_RCM_MEMINIT_TPCCB_INIT|SOC_RCM_MEMINIT_FECSS_SHRAM_INIT|SOC_RCM_MEMINIT_APPSS_SHRAM0_INIT|SOC_RCM_MEMINIT_APPSS_SHRAM1_INIT);

    DebugP_log("Completed Drivers_open(), Board_driversOpen() and SOC_memoryInit()");
}

void hwa_open_handler() {
    // Status handle for HWA_open
    int32_t status = SystemP_SUCCESS;

    hwaHandle = HWA_open(0, NULL, &status);
    if (hwaHandle == NULL)
    {
        DebugP_log("Error: Unable to open the HWA Instance err:%d\n", status);
        DebugP_assert(0);
    }

    DebugP_log("Successfully opened HWA");
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

    /* Initialize and setup the mmWave Control module */
    ctrlHandle = MMWave_init(&initCfg, &errCode);
    if (ctrlHandle == NULL)
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
    

    Mmwave_populateDefaultOpenCfg(&mmwOpenCfg);
    /**********************************************************
     **********************************************************/

    /* Open mmWave module, this is only done once */

    /* Open the mmWave module: */
    if (MMWave_open (ctrlHandle, &mmwOpenCfg, &errCode) < 0)
    {
        /* Error: decode and Report the error */
        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);
        DebugP_log ("Error: mmWave Open failed [Error code: %d Subsystem: %d]\n",
                        mmWaveErrorCode, subsysErrorCode);
        return -1;
    }

    return 0;
}

int32_t mmwave_configSensor(void)
{
    int32_t     errCode = 0;

    Mmwave_populateDefaultChirpControlCfg (&mmwCtrlCfg); /* regular frame config */

    /* Configure the mmWave module: */
    if (MMWave_config (ctrlHandle, &mmwCtrlCfg, &errCode) < 0)
    {
        MMWave_ErrorLevel   errorLevel;
        int16_t             mmWaveErrorCode;
        int16_t             subsysErrorCode;

        /* Error: Report the error */
        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);
        DebugP_log("Error: mmWave Config failed [Error code: %d Subsystem: %d]\n",
                        mmWaveErrorCode, subsysErrorCode);
        goto exit;
    }

exit:
    return errCode;
}

// static int32_t CLI_MMWaveFactoryCalConfig (int32_t argc, char* argv[])
// {
//     #if (CLI_REMOVAL == 0)
//     {
//         if (argc != 6)
//         {
//             CLI_write ("Error: Invalid usage of the CLI command\r\n");
//             return -1;
//         }

//         /* Populate configuration: */
//         gMmwMssMCB.factoryCalCfg.saveEnable = (uint32_t) atoi(argv[1]);
//         gMmwMssMCB.factoryCalCfg.restoreEnable = (uint32_t) atoi(argv[2]);
//         gMmwMssMCB.factoryCalCfg.rxGain = (uint32_t) atoi(argv[3]);
//         /* Front End Firmware expects in 0.5 dB resolution, hence multiplying by 2 */
//         gMmwMssMCB.factoryCalCfg.txBackoffSel = (uint32_t)(2 * atoi(argv[4]));
//         sscanf(argv[5], "0x%x", &gMmwMssMCB.factoryCalCfg.flashOffset);
//     }
//     #else
//     {
//         gMmwMssMCB.factoryCalCfg.saveEnable = CLI_FACCALCFG_SAVE_EN;
//         gMmwMssMCB.factoryCalCfg.restoreEnable = CLI_FACCALCFG_RES_EN;
//         gMmwMssMCB.factoryCalCfg.rxGain = CLI_FACCALCFG_RX_GAIN;
//         gMmwMssMCB.factoryCalCfg.txBackoffSel = CLI_FACCALCFG_TX_BACKOFF_SEL;
//         gMmwMssMCB.factoryCalCfg.flashOffset = CLI_FACCALCFG_FLASH_OFFSET;
//     }
//     #endif

//     /* Validate inputs */
//     /* <Save> and <re-store> shouldn't be enabled in CLI*/
//     if ((gMmwMssMCB.factoryCalCfg.saveEnable == 1) && (gMmwMssMCB.factoryCalCfg.restoreEnable == 1))
//     {
//         CLI_write ("Error: Save and Restore can be enabled only one at a time\r\n");
//         return -1;
//     }

//     /* Validate inputs */
//     /* RxGain should be between 30db to 40db */
//     if ( (gMmwMssMCB.factoryCalCfg.rxGain > 40U) || (gMmwMssMCB.factoryCalCfg.rxGain < 30U))
//     {
//         CLI_write ("Error: Valid RxGain should be between 30db to 40db\r\n");
//         return -1;
//     }

//     /* txBackoffSel should be between 0db to 26db */
//     if ((uint32_t) (gMmwMssMCB.factoryCalCfg.txBackoffSel/2) > 26U)
//     {
//         CLI_write ("Error: Valid txBackoffSel should be between 0db to 26db\r\n");
//         return -1;
//     }

//     /* This check if to avoid accedently courrupt OOB Demo image. */
//     if(gMmwMssMCB.factoryCalCfg.flashOffset < MMWDEMO_CALIB_FLASH_ADDR_1MB)
//     {
//         CLI_write ("Error: Valid flashOffset should be greater than 0x100000\r\n");
//         DebugP_assert(0);
//     }

//     return 0;
// }
