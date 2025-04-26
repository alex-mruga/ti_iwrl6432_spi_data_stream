/**
 * @file rangeproc_dpc.c
 * @brief Implementation of the Range Processing DPC (Data Processing Chain)
 *
 * This file contains the core functions for range processing in the radar signal chain.
 * It manages FFT processing, data handling, and hardware acceleration interfaces.
 *
 * @details
 * This file is a modified version of the original from the motion and presence 
 * detection demo project. It has been adjusted to fit the current application 
 * needs while maintaining compatibility with TI's mmWave SDK.
 *
 * @copyright Copyright (C) 2022-24 Texas Instruments Incorporated
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

#include <kernel/dpl/DebugP.h>
#include <utils/mathutils/mathutils.h>
#include "drivers/edma/v0/edma.h"
#include "kernel/dpl/SemaphoreP.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include "drivers/prcm/v0/prcm.h"
#include "drivers/hwa.h"

#include "system.h"
#include "defines.h"
#include "dpu_res.h"
#include "mmwave_basic.h"
#include "mem_pool.h"
#include "spi_transmit.h"
#include "rangeproc_dpc.h"


/*! @brief for debugging: hardware interrupt objects for registering chirp available ISR */
HwiP_Object gHwiChirpAvailableHwiObject;

/*! @brief for debugging: hardware interrupt objects for registering frame start ISR */
HwiP_Object gHwiFrameStartHwiObject;

/*! @brief for debugging: Chirp counter for when chirp ISR is registered */
uint32_t gChirpCount;

/*! @brief for debugging: Frame counter for when chirp ISR is registered */
uint32_t gFrameCount;

/*! @brief for debugging: Pointer to radar cube data for easier debugging access */
cmplx16ImRe_t * gRadarCubeDebugPtr = NULL;

/*! @brief for debugging: Pointer to radar ADC data for easier debugging access */
void *gAdcDataDebugPtr = NULL;


/*! @brief Rangeproc Callback EDMA Interrupt object (Ping and Poing, hence 2 objects) */
Edma_IntrObject intrObj_Rangeproc[2];


void spiTask() {
    spi_transmit_loop();
}

void dpcTask() {
    int32_t retVal = -1;
    DPU_RangeProcHWA_OutParams outParams;

    gChirpCount = 0;
    gFrameCount = 0;
    
    DPC_ObjDet_MemPoolReset(&gSysContext.L3RamObj);
    DPC_ObjDet_MemPoolReset(&gSysContext.CoreLocalRamObj);

    /* configure DPUs: */
    RangeProc_config();
    // TODO: configure rest of DPUs if required

    SemaphoreP_post(&dpcCfgDoneSemHandle);
    
    // for debugging: register Frame Start ISR
    if (registerFrameStartInterrupt() != 0) {
        DebugP_log("Error: Failed to register frame start interrupts\n");
        DebugP_assert(0);
    }

    // for debugging: register Chirp available ISR
    if (registerChirpInterrupt() != 0) {
        DebugP_log("Error: Failed to register chirp interrupt\n");
        DebugP_assert(0);
    }

    // for debugging: register Chirp available ISR
    if (registerChirpAvailableInterrupts() != 0) {
        DebugP_log("Error: Failed to register chirp interrupt\n");
        DebugP_assert(0);
    }

    // give initial trigger for the first frame 
    retVal = DPU_RangeProcHWA_control(gSysContext.rangeProcHWADpuHandle, DPU_RangeProcHWA_Cmd_triggerProc, NULL, 0);
    if (retVal < 0) {
        /* Not Expected */
        DebugP_log("RangeProc DPU control error %d\n", retVal);
        DebugP_assert(0);
    }

    // endless loop for continuous chirping and processing of data
    while(true) {
        
        memset((void *)&outParams, 0, sizeof(DPU_RangeProcHWA_OutParams));

        retVal = DPU_RangeProcHWA_process(gSysContext.rangeProcHWADpuHandle, &outParams);
        if (retVal < 0) {
            /* Not Expected */
            DebugP_log("RangeProc DPU process error %d\n", retVal);
            DebugP_assert(0);
        }
        // trigger SPI transmission
        SemaphoreP_post(&spi_tx_start_sem);

        // wait for SPI transmission to complete
        SemaphoreP_pend(&spi_tx_done_sem, SystemP_WAIT_FOREVER);

        /* give initial trigger for the next frame */
        retVal = DPU_RangeProcHWA_control(gSysContext.rangeProcHWADpuHandle,
                    DPU_RangeProcHWA_Cmd_triggerProc, NULL, 0);
        if (retVal < 0) {
            DebugP_log("Error: DPU_RangeProcHWA_control failed with error code %d", retVal);
            DebugP_assert(0);
        }
    }
}

void rangeProc_dpuInit() {
    int32_t errorCode = 0;
    DPU_RangeProcHWA_InitParams initParams;
    initParams.hwaHandle = gSysContext.hwaHandle;

    gSysContext.rangeProcHWADpuHandle = DPU_RangeProcHWA_init(&initParams, &errorCode);
    if (gSysContext.rangeProcHWADpuHandle == NULL) {
        DebugP_log ("Debug: RangeProc DPU initialization returned error %d\n", errorCode);
        DebugP_assert (0);
        return;
    }
}

void RangeProc_config() {
    DPU_RangeProcHWA_HW_Resources *pHwConfig = &gSysContext.rangeProcDpuCfg.hwRes;
    DPU_RangeProcHWA_StaticConfig *params = &gSysContext.rangeProcDpuCfg.staticCfg;
    uint32_t bytesPerRxChan;

    memset((void *)&gSysContext.rangeProcDpuCfg, 0, sizeof(DPU_RangeProcHWA_Config));

    // disable low power mode
    params->lowPowerMode = 0;

    /*
      For values refer to "Sensor front-end parameters" in:
      https://software-dl.ti.com/ra-processors/esd/MMWAVE-L-SDK/05_04_00_01/exports/api_guide_xwrL64xx/MMWAVE_DEMO.html
     */
    /* number of TX antennas (on the IWRL6432BOOST 2)*/
    params->numTxAntennas = gSysContext.numTxAntennas;
    /* number of RX antennas, product of TX- and RX-antennas (on the IWRL6432BOOST 2*3=6) */
    params->numVirtualAntennas = gSysContext.numTxAntennas * gSysContext.numRxAntennas;
    /* size of real part of range FFT: half of the range FFT size, since the ADC samples are real valued*/
    params->numRangeBins = CLI_NUM_RBINS; //CLI_NUM_ADC_SAMPLES/2;
    /* number of chirps per frame (= number of chirps per burst, if Nburst = 1) */
    params->numChirpsPerFrame = CLI_NUM_BURSTS_PER_FRAME * CLI_NUM_CHIRPS_PER_BURST;
    /* number of doppler chirps per frame (derived from rangeproc init example): one doppler chirp each set of TX antennas */
    params->numDopplerChirpsPerFrame = params->numChirpsPerFrame / gSysContext.numTxAntennas;
    /* number of doppler chirps per processing evolution: only differs from numDopplerChirpsPerFrame with minor motion mode*/
    params->numDopplerChirpsPerProc = params->numDopplerChirpsPerFrame;
    /* BPM / TDM MIMO enable */
    if ((CLI_MIMO_SEL == 1) || (CLI_MIMO_SEL == 0)) {
        /* TDM-MIMO*/
        params->isBpmEnabled = FALSE;
    } else if (CLI_MIMO_SEL == 4) {
        /* BPM-MIMO*/
        params->isBpmEnabled = TRUE;
    } else {
        DebugP_log("Error: c_ChirpTxMimoPatSel must have value either 1 (TDM-MIMO) or 4 (BPM-MIMO)\n");
        exit(1);    
    }

    /* windowing */
    params->windowSize = sizeof(uint32_t) * ((CLI_NUM_ADC_SAMPLES +1 ) / 2); // symmetric window (Blackman), for real samples (therefore /2)
    params->window =  (int32_t *)DPC_ObjDet_MemPoolAlloc(&gSysContext.CoreLocalRamObj,
                                                        params->windowSize,
                                                        sizeof(uint32_t));

    if (params->window == NULL) {
        DebugP_log("Error allocating window memory");
        return;
    }

    /* adc buffer buffer, format fixed, interleave, size will change */
    params->ADCBufData.dataProperty.dataFmt = DPIF_DATAFORMAT_REAL16;
    params->ADCBufData.dataProperty.adcBits = 2U; // 12-bit only
    params->ADCBufData.dataProperty.numChirpsPerChirpEvent = 1U;
    params->ADCBufData.data = (void *)CSL_APP_HWA_ADCBUF_RD_U_BASE;
    params->ADCBufData.dataProperty.numRxAntennas = (uint8_t) gSysContext.numRxAntennas;

    /* dataSize defines the size of buffer that holds ADC data of every chirp */
    /* ADCBufData.dataSize omitted due to forum post: https://e2e.ti.com/support/sensors-group/sensors/f/sensors-forum/1324580/awrl6432boost-adc-buffer-data-size-in-motion-and-presence-detection-demo */
    params->ADCBufData.dataSize = CLI_NUM_ADC_SAMPLES * gSysContext.numRxAntennas * sizeof(uint16_t) * 2; // times 2, because of ping and pong C:\ti\mmwave-sdk\docs\MotionPresenceDetectionDemo_documentation.pdf 
    params->ADCBufData.dataProperty.numAdcSamples = CLI_NUM_ADC_SAMPLES;
    
    mathUtils_genWindow((uint32_t *)params->window,
                                (uint32_t) params->ADCBufData.dataProperty.numAdcSamples,
                                params->windowSize/sizeof(uint32_t),
                                MATHUTILS_WIN_BLACKMAN,
                                DPC_OBJDET_QFORMAT_RANGE_FFT);

    /* FFT optimizing params (derived from rangeproc DPU example) */
    params->rangeFFTtuning.fftOutputDivShift = 2;
    params->rangeFFTtuning.numLastButterflyStagesToScale = 0; /* no scaling needed as ADC is 16-bit and we have 8 bits to grow */  

    /* size of range FFT: equal to number of ADC samples*/
    params->rangeFftSize = CLI_NUM_ADC_SAMPLES;

    /* bytes per RX channel (each chirp is uint_16) */
    bytesPerRxChan = CLI_NUM_ADC_SAMPLES * sizeof(uint16_t);
    bytesPerRxChan = (bytesPerRxChan + 15) / 16 * 16; // ensure that value is multiple of 16 (for EDMA?)

    /* initialize RX channel offsets */
    uint32_t index;
    for (index = 0; index < SYS_COMMON_NUM_RX_CHANNEL; index++) {
        params->ADCBufData.dataProperty.rxChanOffset[index] = index * bytesPerRxChan;
    }

    /* Further hardware config params (copied from rangeproc config example) */
    params->ADCBufData.dataProperty.interleave = DPIF_RXCHAN_NON_INTERLEAVE_MODE;

    /* Set Motion Mode (Minor/Major) */
    params->enableMajorMotion = 1;
    params->enableMinorMotion = 0;
    params->numMinorMotionChirpsPerFrame = 0; // obsolete, not using minor motion

    /* Data Input EDMA */
    pHwConfig->edmaInCfg.dataIn.channel         = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_CH;
    pHwConfig->edmaInCfg.dataIn.channelShadow[0]   = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SHADOW_PING;
    pHwConfig->edmaInCfg.dataIn.channelShadow[1]   = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SHADOW_PONG;
    pHwConfig->edmaInCfg.dataIn.eventQueue      = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_EVENT_QUE;
    pHwConfig->edmaInCfg.dataInSignature.channel         = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SIG_CH;
    pHwConfig->edmaInCfg.dataInSignature.channelShadow   = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SIG_SHADOW;
    pHwConfig->edmaInCfg.dataInSignature.eventQueue      = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SIG_EVENT_QUE;

    // register EDMA interrupt object for rangeproc callback
    // this pointer holds 2 objects. 
    pHwConfig->intrObj = intrObj_Rangeproc;
 
    /* Data Output EDMA */
    pHwConfig->edmaOutCfg.path[0].evtDecim.channel = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PING_CH;
    pHwConfig->edmaOutCfg.path[0].evtDecim.channelShadow[0] = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PING_SHADOW_0;
    pHwConfig->edmaOutCfg.path[0].evtDecim.channelShadow[1] = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PING_SHADOW_1;
    pHwConfig->edmaOutCfg.path[0].evtDecim.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PING_EVENT_QUE;
    
    pHwConfig->edmaOutCfg.path[1].evtDecim.channel = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PONG_CH;
    pHwConfig->edmaOutCfg.path[1].evtDecim.channelShadow[0] = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PONG_SHADOW_0;
    pHwConfig->edmaOutCfg.path[1].evtDecim.channelShadow[1] = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PONG_SHADOW_1;
    pHwConfig->edmaOutCfg.path[1].evtDecim.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PONG_EVENT_QUE;
    
    pHwConfig->edmaOutCfg.path[0].dataOutMinor.channel = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PING_CH;
    pHwConfig->edmaOutCfg.path[0].dataOutMinor.channelShadow = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PING_SHADOW;
    pHwConfig->edmaOutCfg.path[0].dataOutMinor.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PING_EVENT_QUE;
    
    pHwConfig->edmaOutCfg.path[1].dataOutMinor.channel = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PONG_CH;
    pHwConfig->edmaOutCfg.path[1].dataOutMinor.channelShadow = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PONG_SHADOW;
    pHwConfig->edmaOutCfg.path[1].dataOutMinor.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PONG_EVENT_QUE;
 
    pHwConfig->edmaOutCfg.path[0].dataOutMajor.channel = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PING_CH;
    pHwConfig->edmaOutCfg.path[0].dataOutMajor.channelShadow = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PING_SHADOW;
    pHwConfig->edmaOutCfg.path[0].dataOutMajor.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PING_EVENT_QUE;
    
    pHwConfig->edmaOutCfg.path[1].dataOutMajor.channel = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PONG_CH;
    pHwConfig->edmaOutCfg.path[1].dataOutMajor.channelShadow = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PONG_SHADOW;
    pHwConfig->edmaOutCfg.path[1].dataOutMajor.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PONG_EVENT_QUE;
   
    /* radar cube config*/
    /* total size of radar cube in bytes (num range bins x num virtual antennas x sizeof x num doppler chirps) */
    pHwConfig->radarCube.dataSize = CLI_NUM_RBINS * params->numVirtualAntennas * sizeof(cmplx16ReIm_t) * params->numDopplerChirpsPerFrame;
    pHwConfig->radarCube.datafmt = DPIF_RADARCUBE_FORMAT_6;

        /* radar cube */
    gSysContext.rangeProcDpuCfg.hwRes.radarCube.data  = (cmplx16ImRe_t *) DPC_ObjDet_MemPoolAlloc(&gSysContext.L3RamObj,
                                                                                        pHwConfig->radarCube.dataSize,
                                                                                        sizeof(uint32_t));
    // bend global radar cube debug pointer to radar cube data 
    gRadarCubeDebugPtr = gSysContext.rangeProcDpuCfg.hwRes.radarCube.data;
    /* Further non EDMA related HWA configurations */
    pHwConfig->hwaCfg.paramSetStartIdx = 0;
    pHwConfig->hwaCfg.numParamSet = DPU_RANGEPROCHWA_NUM_HWA_PARAM_SETS;
    pHwConfig->hwaCfg.hwaWinRamOffset  = DPC_OBJDET_HWA_WINDOW_RAM_OFFSET; 
    pHwConfig->hwaCfg.hwaWinSym = 1;
    pHwConfig->hwaCfg.dataInputMode = DPU_RangeProcHWA_InputMode_ISOLATED;

    // unnecessarily complicated logic to increment channel numbers for DMA. Here now refactored to MACROs
    // pHwConfig->hwaCfg.dmaTrigSrcChan[0] = DPC_ObjDet_HwaDmaTrigSrcChanPoolAlloc(&gMmwMssMCB.HwaDmaChanPoolObj);
    // pHwConfig->hwaCfg.dmaTrigSrcChan[1] = DPC_ObjDet_HwaDmaTrigSrcChanPoolAlloc(&gMmwMssMCB.HwaDmaChanPoolObj);
    pHwConfig->hwaCfg.dmaTrigSrcChan[0] = (uint8_t) DMA_TRIG_SRC_CHAN_0;
    pHwConfig->hwaCfg.dmaTrigSrcChan[1] = (uint8_t) DMA_TRIG_SRC_CHAN_1;

    /* edma configuration */
    pHwConfig->edmaHandle  = gEdmaHandle[0];

    /* adc buffer buffer, format fixed, interleave, size will change */
    params->ADCBufData.dataProperty.dataFmt = DPIF_DATAFORMAT_REAL16;
    params->ADCBufData.dataProperty.adcBits = 2U; // 12-bit only
    params->ADCBufData.dataProperty.numChirpsPerChirpEvent = 1U;
    
    gAdcDataDebugPtr = params->ADCBufData.data;

    /* configure HWA with set parameters */
    int32_t retVal;
    retVal = DPU_RangeProcHWA_config(gSysContext.rangeProcHWADpuHandle, &gSysContext.rangeProcDpuCfg);
 
    if (retVal < 0) {
        DebugP_log("DEBUG: RANGE DPU config return error:%d \n", retVal);
        DebugP_assert(0);
    }
}

/**
 *  @b Description
 *  @n
 *      This is to register Chirpt Interrupt
 */
int32_t registerChirpInterrupt(void) {
    int32_t           retVal = 0;
    int32_t           status = SystemP_SUCCESS;
    HwiP_Params       hwiPrms;

    /* Register interrupt */
    HwiP_Params_init(&hwiPrms);
    hwiPrms.intNum      = 16 + CSL_APPSS_INTR_MUXED_FECSS_CHIRPTIMER_CHIRP_START_AND_CHIRP_END;
    hwiPrms.callback    = chirpStartISR;
    /* Use this to change the priority */
    //hwiPrms.priority    = 0;
    hwiPrms.args        = NULL;
    status              = HwiP_construct(&gHwiChirpAvailableHwiObject, &hwiPrms);

    if (SystemP_SUCCESS != status) {
        retVal = SystemP_FAILURE;
    } else {
        HwiP_enableInt((uint32_t)CSL_APPSS_INTR_MUXED_FECSS_CHIRPTIMER_CHIRP_START_AND_CHIRP_END);
    }

    return retVal;
}

/**
*  @b Description
*  @n
*    Chirp Start ISR
*/
void chirpStartISR(void *arg) {
    HwiP_clearInt(CSL_APPSS_INTR_MUXED_FECSS_CHIRPTIMER_CHIRP_START_AND_CHIRP_END);
}


int32_t registerFrameStartInterrupt(void) {
    int32_t           retVal = 0;
    int32_t           status = SystemP_SUCCESS;
    HwiP_Params       hwiPrms;

    /* Register interrupt */
    HwiP_Params_init(&hwiPrms);
    hwiPrms.intNum      = 16 + CSL_APPSS_INTR_FECSS_FRAMETIMER_FRAME_START;
    hwiPrms.callback    = frameStartISR;
    /* Use this to change the priority */
    //hwiPrms.priority    = 0;
    status              = HwiP_construct(&gHwiFrameStartHwiObject, &hwiPrms);

    if (SystemP_SUCCESS != status) {
        retVal = SystemP_FAILURE;
    } else {
        HwiP_enableInt((uint32_t)CSL_APPSS_INTR_FECSS_FRAMETIMER_FRAME_START);
    }

    return retVal;
}

static void frameStartISR(void *arg) {
    /* Clear the interrupt */
    HwiP_clearInt(CSL_APPSS_INTR_FECSS_FRAMETIMER_FRAME_START);

    /* Record the frame start time for profiling or other processing */
    // demoStartTime = PRCMSlowClkCtrGet();
    gFrameCount++;
    /* Optionally, perform any other frame processing needed here */
    // For example, you might calculate the frame period or process the data further.
}

//The function reads the FRAME_REF_TIMER that runs free at 40MHz
uint32_t Cycleprofiler_getTimeStamp(void) {
    uint32_t *frameRefTimer;
    frameRefTimer = (uint32_t *) 0x5B000020;
    return *frameRefTimer;
}

/**
 *  @b Description
 *  @n
 *      This is to register Chirp Available Interrupt
 */
int32_t registerChirpAvailableInterrupts(void) {
    int32_t           retVal = 0;
    int32_t           status = SystemP_SUCCESS;
    HwiP_Params       hwiPrms;

    /* Register interrupt */
    HwiP_Params_init(&hwiPrms);
    hwiPrms.intNum      = 16 + CSL_APPSS_INTR_MUXED_FECSS_CHIRP_AVAIL_IRQ_AND_ADC_VALID_START_AND_SYNC_IN; //CSL_MSS_INTR_RSS_ADC_CAPTURE_COMPLETE;
    hwiPrms.callback    = ChirpAvailISR;
    /* Use this to change the priority */
    //hwiPrms.priority    = 0;
    hwiPrms.args        = NULL;
    status              = HwiP_construct(&gHwiChirpAvailableHwiObject, &hwiPrms);

    if (SystemP_SUCCESS != status) {
        retVal = SystemP_FAILURE;
    } else {
        HwiP_enableInt((uint32_t)CSL_APPSS_INTR_MUXED_FECSS_CHIRP_AVAIL_IRQ_AND_ADC_VALID_START_AND_SYNC_IN);
    }

    return retVal;
}

/**
*  @b Description
*  @n
*    Chirp ISR
*/
static void ChirpAvailISR(void *arg) {
    HwiP_clearInt(CSL_APPSS_INTR_MUXED_FECSS_CHIRP_AVAIL_IRQ_AND_ADC_VALID_START_AND_SYNC_IN); // CSL_MSS_INTR_RSS_ADC_CAPTURE_COMPLETE
    gChirpCount++;
}

