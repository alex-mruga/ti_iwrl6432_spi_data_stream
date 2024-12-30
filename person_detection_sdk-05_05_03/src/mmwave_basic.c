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

#include "mmwave_basic.h"

/**************************************************************************
 ************************** Extern Definitions ****************************
 **************************************************************************/
extern void Mmwave_populateDefaultOpenCfg (MMWave_OpenCfg* ptrOpenCfg);
extern void Mmwave_populateDefaultChirpControlCfg (MMWave_CtrlCfg* ptrCtrlCfg);
/**************************************************************************/


HWA_Handle hwaHandle = NULL;

/*! @brief This is the mmWave control handle which is used to configure the BSS. */
MMWave_Handle gCtrlHandle;

/*! @brief  Configuration to open DFP */
MMWave_OpenCfg mmwOpenCfg;

MMWave_CtrlCfg mmwCtrlCfg;

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
        return -1;
    }

    return 0;
}

int32_t mmwave_configSensor(void)
{
    int32_t     errCode = 0;

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










// DERIVED FROM hwa_fft1D.c



/**
 * Buffers (src and dest) are needed for mem-2-mem data transfers.
 * This define is for the MAXIMUM size and hence the maximum data
 * which could be transferred using the sample test cases below.
 */
#define hwa_src     CSL_APP_HWA_DMA0_RAM_BANK0_BASE
#define hwa_dst     CSL_APP_HWA_DMA0_RAM_BANK2_BASE

HWA_Handle        gHwaHandle;
SemaphoreP_Object gHwaDoneSem;


/*
 * ISR Callback
 */
static void HWAFFT_doneCallback(void *arg)
{
    SemaphoreP_Object *doneSem;
    
    if(arg != NULL)
    {
        doneSem = (SemaphoreP_Object *)arg;
        SemaphoreP_post(doneSem);
    }

    return;
}


void Mmwave_HwaConfig_custom (void)
{
    HWA_ParamConfig     paramCfg;
    HWA_CommonConfig    commonCfg;
    uint32_t            paramIdx;
    uint32_t            numSamples = 128U; //based on h_NumOfAdcSamples in common_full.c
    int32_t             status = SystemP_SUCCESS;
    
    /* Open HWA driver */
    hwaHandle = HWA_open(0, NULL, &status);
    if(hwaHandle == NULL)
    {
        DebugP_log("Error: Unable to open HWA instance. Error: %d\n", status);
        DebugP_assert(hwaHandle == NULL);
    }

    paramIdx = 0;
    memset(&paramCfg , 0, sizeof(HWA_ParamConfig));
    paramCfg.triggerMode = HWA_TRIG_MODE_RESERVED1;
    paramCfg.accelMode = HWA_ACCELMODE_FFT;
    paramCfg.source.srcAddr = HWADRV_ADDR_TRANSLATE_CPU_TO_HWA(hwa_src);
    paramCfg.source.srcSign = HWA_SAMPLES_SIGNED;
    paramCfg.source.srcAcnt = numSamples - 1U;
    paramCfg.source.srcAIdx = sizeof(int16_t);
    paramCfg.source.srcBcnt = 0U;
    paramCfg.source.srcBIdx = 0U;  /* dont care as bcnt is 0 */
    paramCfg.source.srcWidth = HWA_SAMPLES_WIDTH_16BIT;
    paramCfg.source.srcScale = 0x8;
    paramCfg.source.srcRealComplex = HWA_SAMPLES_FORMAT_REAL;
    paramCfg.source.srcConjugate = HWA_FEATURE_BIT_DISABLE;    /* no conjugate */
    paramCfg.source.bpmEnable = HWA_FEATURE_BIT_DISABLE;
    paramCfg.dest.dstAddr = HWADRV_ADDR_TRANSLATE_CPU_TO_HWA(hwa_dst);
    paramCfg.dest.dstSkipInit = 0U;
    paramCfg.dest.dstAcnt = numSamples - 1U;
    paramCfg.dest.dstAIdx = 2*sizeof(int16_t); /* x 2 for real and complex output */
    paramCfg.dest.dstBIdx = 0U;    /* dont care as bcnt is 0 */
    paramCfg.dest.dstRealComplex = HWA_SAMPLES_FORMAT_COMPLEX;
    paramCfg.dest.dstWidth = HWA_SAMPLES_WIDTH_16BIT;
    paramCfg.dest.dstSign = HWA_SAMPLES_SIGNED;
    paramCfg.dest.dstScale = 0U;
    paramCfg.dest.dstConjugate = HWA_FEATURE_BIT_DISABLE;      /* no conjugate */
    paramCfg.accelModeArgs.fftMode.fftEn = HWA_FEATURE_BIT_ENABLE;
    paramCfg.accelModeArgs.fftMode.fftSize = NUM_BURSTS_PER_FRAME;
    paramCfg.accelModeArgs.fftMode.butterflyScaling = 0x0;
    paramCfg.accelModeArgs.fftMode.windowEn = HWA_FEATURE_BIT_DISABLE;
    paramCfg.accelModeArgs.fftMode.windowStart = 0U;
    paramCfg.accelModeArgs.fftMode.winSymm = HWA_FFT_WINDOW_NONSYMMETRIC;
    paramCfg.complexMultiply.mode = HWA_COMPLEX_MULTIPLY_MODE_DISABLE;
    paramCfg.accelModeArgs.fftMode.magLogEn = HWA_FFT_MODE_MAGNITUDE_LOG2_DISABLED;
    paramCfg.accelModeArgs.fftMode.fftOutMode = HWA_FFT_MODE_OUTPUT_DEFAULT; /* FFT output */
    
    status += HWA_configParamSet(hwaHandle, paramIdx, &paramCfg, NULL);
    if(SystemP_SUCCESS != status)
    {
        DebugP_log("Error: HWA_configParamSet failed with error: %d!!\r\n", status);
    }

    if(SystemP_SUCCESS == status)
    {
        /* Init Common Params */
        memset(&commonCfg, 0, sizeof(HWA_CommonConfig));
        commonCfg.configMask = HWA_COMMONCONFIG_MASK_PARAMSTARTIDX |
                                HWA_COMMONCONFIG_MASK_PARAMSTOPIDX |
                                HWA_COMMONCONFIG_MASK_NUMLOOPS |
                                HWA_COMMONCONFIG_MASK_FFT1DENABLE |
                                HWA_COMMONCONFIG_MASK_TWIDDITHERENABLE |
                                HWA_COMMONCONFIG_MASK_LFSRSEED;
        
        commonCfg.paramStartIdx = 0U;
        commonCfg.paramStopIdx = 0U;
        commonCfg.numLoops = 1U;
        commonCfg.fftConfig.fft1DEnable = HWA_FEATURE_BIT_ENABLE;
        commonCfg.fftConfig.twidDitherEnable = HWA_FEATURE_BIT_DISABLE;
        commonCfg.fftConfig.lfsrSeed = 0x1234567;
        
        status = HWA_configCommon(hwaHandle, &commonCfg);
        if(status != SystemP_SUCCESS)
        {
            DebugP_log("Error: HWA_configCommon failed with error: %d\r\n", status);
        }
    }

    if(SystemP_SUCCESS == status)
    {
        /* Enable done interrupt */
        status = SemaphoreP_constructBinary(&gHwaDoneSem, 0);
        DebugP_assert(status == SystemP_SUCCESS);
        status = HWA_enableDoneInterrupt(hwaHandle, HWAFFT_doneCallback, &gHwaDoneSem);
        if(status != SystemP_SUCCESS)
        {
            DebugP_log("Error: HWA_enableDoneInterrupt failed with error: %d\r\n", status);
        }
    }
    
    DebugP_assert(status == SystemP_SUCCESS);

    /* Enable HWA */
    status += HWA_enable(hwaHandle, 1U);
    DebugP_assert(SystemP_SUCCESS == status);
}