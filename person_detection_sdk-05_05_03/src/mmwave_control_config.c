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


/* Copy of file with the same name from the motion and presence detection demo project with adjustments */
/**************************************************************************
 *************************** Include Files ********************************
 **************************************************************************/
#define DebugP_LOG_ENABLED 1
#define RDIF_LANE_RATE_UPDATE 1

/* Standard Include Files. */
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include <drivers/hw_include/cslr_adcbuf.h>
#include <drivers/hw_include/xwrL64xx/cslr_soc_baseaddress.h>

/* mmWave SDK Include Files: */
#include <control/mmwave/mmwave.h>
#include <kernel/dpl/DebugP.h>
#include <utils/testlogger/logger.h>

#include "defines.h"
#include "common/sys_defs.h"


/**************************************************************************
 *************************** Local Definitions ****************************
 **************************************************************************/

/*!
 * @brief  Sensor Perchirp LUT, total 64 bytes used, 4 values per params
 */
typedef struct
{
    uint32_t StartFreqHighRes[4]; /* LUT address 0 */
    uint32_t StartFreqLowRes[4]; /* LUT address 16 */
    int16_t ChirpSlope[4]; /* LUT address 32 */
    uint16_t ChirpIdleTime[4]; /* LUT address 40 */
    uint16_t ChirpAdcStartTime[4]; /* LUT address 48 */
    int16_t ChirpTxStartTime[4]; /* LUT address 56 */
    uint8_t ChirpTxEn[4]; /* LUT address 64 */
    uint8_t ChirpBpmEn[4]; /* LUT address 68 */
} T_SensPerChirpLut;

// "chirpComnCfg" in SDK Documentation
T_RL_API_SENS_CHIRP_PROF_COMN_CFG profileComCfg;
T_RL_API_SENS_CHIRP_PROF_TIME_CFG profileTimeCfg;
T_RL_API_FECSS_RF_PWR_CFG_CMD channelCfg;
T_RL_API_SENS_FRAME_CFG frameCfg;


static void Mmwave_populateDefaultProfileCfg (T_RL_API_SENS_CHIRP_PROF_COMN_CFG* ptrProfileCfg, T_RL_API_SENS_CHIRP_PROF_TIME_CFG* ptrProfileTimeCfg);
static void Mmwave_populateDefaultChirpCfg (T_RL_API_SENS_PER_CHIRP_CFG* ptrChirpCfg, T_RL_API_SENS_PER_CHIRP_CTRL* ptrChirpCtrl);
void MMWave_populateChannelCfg();
void Mmwave_populateDefaultCalibrationCfg (MMWave_CalibrationCfg* ptrCalibrationCfg);
void Mmwave_populateDefaultStartCfg (MMWave_StrtCfg* ptrStartCfg);

/**************************************************************************
 ************************* Extern Declarations ****************************
 **************************************************************************/
extern MMWave_Handle gCtrlHandle;

T_SensPerChirpLut* sensPerChirpLuTable = (T_SensPerChirpLut*)(0x21880000U);

/**************************************************************************
 ************************* Common Test Functions **************************
 **************************************************************************/

/**
 *  @b Description
 *  @n
 *      Utility function which populates the profile configuration with
 *      well defined defaults.
 *
 *  @param[out]  ptrProfileCfg
 *      Pointer to the populated profile configuration
 *
 *  @retval
 *      Not applicable
 */
static void Mmwave_populateDefaultProfileCfg (T_RL_API_SENS_CHIRP_PROF_COMN_CFG* ptrProfileCfg, T_RL_API_SENS_CHIRP_PROF_TIME_CFG* ptrProfileTimeCfg)
{
    float rfBandwidth;
    float rampDownTime;
    float scale = 65536./(3*100*100);

    /* Populate the *default* profile configuration: */
    profileComCfg.c_DigOutputSampRate           = CHIRPCOMNCFG_DIG_OUTPUT_SAMP_RATE;   // 23; // M_RL_SENS_DIG_OUT_SAMP_RATE_MAX_12P5M
    profileComCfg.c_DigOutputBitsSel            = CHIRPCOMNCFG_DIG_OUTPUT_BITS_SEL;    // 0; // M_RL_SENS_DIG_OUT_12BITS_4LSB_ROUND
    profileComCfg.c_DfeFirSel                   = CHIRPCOMNCFG_DFE_FIR_SEL;            // 0; // M_RL_SENS_DFE_FIR_LONG_FILT
    profileComCfg.h_NumOfAdcSamples             = CHIRPCOMNCFG_NUM_OF_ADC_SAMPLES;     // 128; // 256U; /* 2.56us */
    profileComCfg.c_ChirpTxMimoPatSel           = CHIRPCOMNCFG_CHIRP_TX_MIMO_PAT_SEL;  // 4; // 0; // M_RL_SENS_TX_MIMO_PATRN_DIS
    profileComCfg.c_MiscSettings                = CHIRPCOMNCFG_MISC_SETTINGS;          // 0U; /* HPF FINIT, CRD ena, PA blank dis */
    profileComCfg.c_HpfFastInitDuration         = CHIRPCOMNCFG_HPF_FAST_INIT_DURATION; // 15U; /* 1.5us */
    profileComCfg.h_ChirpRampEndTime            = CHIRPCOMNCFG_CHIRP_RAMP_END_TIME * 10.0;    // 361; // 600; // 250U; /* 25us low res */
    profileComCfg.c_ChirpRxHpfSel               = CHIRPCOMNCFG_CHIRP_RX_HPF_SEL;       // 1; // M_RL_SENS_RX_HPF_SEL_350KHZ

    /* Populate the *default* timing configuration: */
    profileTimeCfg.h_ChirpIdleTime              = CHIRPTIMINGCFG_CHIRP_IDLE_TIME * 10.0;       // 80; // 400; // 65U; /* 6.5us low res */
    profileTimeCfg.h_ChirpAdcStartTime          = CHIRPTIMINGCFG_CHIRP_ADC_START_TIME;  // 300; // 30770;
    profileTimeCfg.xh_ChirpTxStartTime          = CHIRPTIMINGCFG_CHIRP_TX_START_TIME;   // 0; // -10; /* -0.2us */
    profileTimeCfg.xh_ChirpRfFreqSlope          = CHIRPTIMINGCFG_CHIRP_RF_FREQ_SLOPE;   // 419; // 699; // 3495; /* 100MHz/us , 77G - 2621 */
    /* Front End Firmware expects Start freq (MHz) as 1 LSB = (3 x APLL_FREQ / 2^16) * 2^6 resolution  */
    profileTimeCfg.w_ChirpRfFreqStart           = (CHIRPTIMINGCFG_CHIRP_RF_FREQ_START * 1000.0 * 256.0)/(300);
    profileTimeCfg.h_ChirpTxEnSel               = CHIRPTIMINGCFG_CHIRP_TX_EN_SEL;       // 0x3U; /* 2 TX enable in chirp */
    profileTimeCfg.h_ChirpTxBpmEnSel            = CHIRPTIMINGCFG_CHIRP_TX_BPM_EN_SEL;   // 0x3U; // 0; // 0x2U; /* TX1 BPM enable in chirp */

    rfBandwidth = (profileComCfg.h_ChirpRampEndTime*0.1) * CHIRPTIMINGCFG_CHIRP_RF_FREQ_SLOPE; //In MHz/usec
    rampDownTime = MIN((profileTimeCfg.h_ChirpIdleTime*0.1-1.0), 6.0); //In usec
    profileComCfg.h_CrdNSlopeMag = (uint16_t) fabs((scale * rfBandwidth / rampDownTime + 0.5));

    // magic calculations, derived from demo project
    profileTimeCfg.xh_ChirpRfFreqSlope  = (profileTimeCfg.xh_ChirpRfFreqSlope * 1048576.0)/(3* 100 * 100);


    /* Initialize the profile configuration: */
    memset ((void*)ptrProfileCfg, 0, sizeof(T_RL_API_SENS_CHIRP_PROF_COMN_CFG));

    /* Populate the *default* profile configuration: */
    ptrProfileCfg->c_DigOutputSampRate = profileComCfg.c_DigOutputSampRate; //23; //8; //M_RL_SENS_DIG_OUT_SAMP_RATE_MAX_12P5M;
    ptrProfileCfg->c_DigOutputBitsSel = profileComCfg.c_DigOutputBitsSel; //0; //M_RL_SENS_DIG_OUT_12BITS_4LSB_ROUND;
    ptrProfileCfg->c_DfeFirSel = profileComCfg.c_DfeFirSel; //0; //M_RL_SENS_DFE_FIR_LONG_FILT;
    ptrProfileCfg->c_VcoMultiChipMode = 0; //M_RL_SENS_VCO_MULT_CHIP_SINGLE;
    ptrProfileCfg->h_NumOfAdcSamples = profileComCfg.h_NumOfAdcSamples; //128; //256U; /* 2.56us */
    ptrProfileCfg->c_ChirpTxMimoPatSel = profileComCfg.c_ChirpTxMimoPatSel; //4; //0; //M_RL_SENS_TX_MIMO_PATRN_DIS;

    ptrProfileCfg->c_MiscSettings = profileComCfg.c_MiscSettings; //0U; /* HPF FINIT, CRD ena, PA blank dis */
    ptrProfileCfg->c_HpfFastInitDuration = profileComCfg.c_HpfFastInitDuration; //15U; /* 1.5us */
    ptrProfileCfg->h_CrdNSlopeMag = profileComCfg.h_CrdNSlopeMag; //0; //0x800U; /* default slope */

    //ptrProfileCfg->h_ChirpRampEndTime = 200U; /* 4us high res */
    ptrProfileCfg->h_ChirpRampEndTime = profileComCfg.h_ChirpRampEndTime; //361; //600; //250U; /* 25us low res */
    ptrProfileCfg->c_ChirpRxHpfSel = profileComCfg.c_ChirpRxHpfSel; //1; //M_RL_SENS_RX_HPF_SEL_350KHZ;

    /*ptrProfileCfg->c_ChirpRxGainSel = (36U | \
                    (M_RL_SENS_RF_GAIN_TARG_1 << M_RL_SENS_RF_GAIN_OFFSET));
    ptrProfileCfg->c_ChirpTxBackOffSel[0] = 0U;
    ptrProfileCfg->c_ChirpTxBackOffSel[1] = 0U;*/

    /* Initialize the profile time configuration: */
    memset ((void*)ptrProfileTimeCfg, 0, sizeof(T_RL_API_SENS_CHIRP_PROF_TIME_CFG));

    //ptrProfileTimeCfg->h_ChirpIdleTime = 325U; /* 6.5us high res */
    ptrProfileTimeCfg->h_ChirpIdleTime = profileTimeCfg.h_ChirpIdleTime; //80; //400; //65U; /* 6.5us low res */
    ptrProfileTimeCfg->h_ChirpAdcStartTime = profileTimeCfg.h_ChirpAdcStartTime; //300; //30770;
    /*((UINT16)25U | \
            ((UINT16)5U << M_RL_SENS_CHIRP_ADC_SKIP_SAMP_OFFSET)); 0.5us Fract + 0.4us skip */
    ptrProfileTimeCfg->xh_ChirpTxStartTime = profileTimeCfg.xh_ChirpTxStartTime; //0; //-10; /* -0.2us */
    ptrProfileTimeCfg->xh_ChirpRfFreqSlope = profileTimeCfg.xh_ChirpRfFreqSlope; //419; //699; //3495; /* 100MHz/us , 77G - 2621 */
    //ptrProfileTimeCfg->w_ChirpRfFreqStart  = M_RL_SENS_CHIRP_RFFREQ_HR_57G; /* 57GHz / 76GHz High */
    ptrProfileTimeCfg->w_ChirpRfFreqStart  = profileTimeCfg.w_ChirpRfFreqStart; //51200; //50347; //M_RL_SENS_CHIRP_RFFREQ_LR_57G; /* 57GHz / 76GHz low */
    ptrProfileTimeCfg->h_ChirpTxEnSel = profileTimeCfg.h_ChirpTxEnSel;//0x3U; /* 2 TX enable in chirp */
    ptrProfileTimeCfg->h_ChirpTxBpmEnSel = profileTimeCfg.h_ChirpTxBpmEnSel; //0x3U; //0; //0x2U; /* TX1 BPM enable in chirp */
}

/**
 *  @b Description
 *  @n
 *      Utility function which populates the chirp configuration with
 *      well defined defaults.
 *
 *  @param[out]  ptrChirpCfg
 *      Pointer to the populated chirp configuration
 *
 *  @retval
 *      Not applicable
 */
static void Mmwave_populateDefaultChirpCfg (T_RL_API_SENS_PER_CHIRP_CFG* ptrChirpCfg, T_RL_API_SENS_PER_CHIRP_CTRL* ptrChirpCtrl)
{
    /* Initialize the chirp configuration: */
    memset ((void*)ptrChirpCfg, 0, sizeof(T_RL_API_SENS_PER_CHIRP_CFG));

    /* Populate the chirp configuration: */
    ptrChirpCfg->h_ParamArrayLen[M_RL_SENS_PER_CHIRP_FREQ_START] = 4;
    ptrChirpCfg->h_ParamArrayLen[M_RL_SENS_PER_CHIRP_FREQ_SLOPE] = 4;
    ptrChirpCfg->h_ParamArrayLen[M_RL_SENS_PER_CHIRP_IDLE_TIME] = 4;
    ptrChirpCfg->h_ParamArrayLen[M_RL_SENS_PER_CHIRP_ADC_START_TIME] = 4;
    ptrChirpCfg->h_ParamArrayLen[M_RL_SENS_PER_CHIRP_TX_START_TIME] = 4;
    ptrChirpCfg->h_ParamArrayLen[M_RL_SENS_PER_CHIRP_TX_ENABLE] = 4;
    ptrChirpCfg->h_ParamArrayLen[M_RL_SENS_PER_CHIRP_BPM_ENABLE] = 4;

    /* repeat count is not applicable for acc chirps, so new chirp param is picked after 2*2 chirps */
    ptrChirpCfg->h_ParamRptCount[M_RL_SENS_PER_CHIRP_FREQ_START] = 1;
    ptrChirpCfg->h_ParamRptCount[M_RL_SENS_PER_CHIRP_FREQ_SLOPE] = 1;
    ptrChirpCfg->h_ParamRptCount[M_RL_SENS_PER_CHIRP_IDLE_TIME] = 1;
    ptrChirpCfg->h_ParamRptCount[M_RL_SENS_PER_CHIRP_ADC_START_TIME] = 1;
    ptrChirpCfg->h_ParamRptCount[M_RL_SENS_PER_CHIRP_TX_START_TIME] = 1;
    ptrChirpCfg->h_ParamRptCount[M_RL_SENS_PER_CHIRP_TX_ENABLE] = 1;
    ptrChirpCfg->h_ParamRptCount[M_RL_SENS_PER_CHIRP_BPM_ENABLE] = 1;

    /* Initialize the chirp control configuration: */
    memset ((void*)ptrChirpCtrl, 0, sizeof(T_RL_API_SENS_PER_CHIRP_CTRL));

    /* Sensor per chirp control api */
    ptrChirpCtrl->h_ParamArrayStartAdd[M_RL_SENS_PER_CHIRP_FREQ_START] = \
        ((UINT32)(&sensPerChirpLuTable->StartFreqLowRes[0]) & M_RL_SENS_PER_CHIRP_LUT_ADD_MASK);
    ptrChirpCtrl->h_ParamArrayStartAdd[M_RL_SENS_PER_CHIRP_FREQ_SLOPE] = \
        ((UINT32)(&sensPerChirpLuTable->ChirpSlope[0]) & M_RL_SENS_PER_CHIRP_LUT_ADD_MASK);
    ptrChirpCtrl->h_ParamArrayStartAdd[M_RL_SENS_PER_CHIRP_IDLE_TIME] = \
        ((UINT32)(&sensPerChirpLuTable->ChirpIdleTime[0]) & M_RL_SENS_PER_CHIRP_LUT_ADD_MASK);
    ptrChirpCtrl->h_ParamArrayStartAdd[M_RL_SENS_PER_CHIRP_ADC_START_TIME] = \
        ((UINT32)(&sensPerChirpLuTable->ChirpAdcStartTime[0]) & M_RL_SENS_PER_CHIRP_LUT_ADD_MASK);
    ptrChirpCtrl->h_ParamArrayStartAdd[M_RL_SENS_PER_CHIRP_TX_START_TIME] = \
        ((UINT32)(&sensPerChirpLuTable->ChirpTxStartTime[0]) & M_RL_SENS_PER_CHIRP_LUT_ADD_MASK);
    ptrChirpCtrl->h_ParamArrayStartAdd[M_RL_SENS_PER_CHIRP_TX_ENABLE] = \
        ((UINT32)(&sensPerChirpLuTable->ChirpTxEn[0]) & M_RL_SENS_PER_CHIRP_LUT_ADD_MASK);
    ptrChirpCtrl->h_ParamArrayStartAdd[M_RL_SENS_PER_CHIRP_BPM_ENABLE] = \
        ((UINT32)(&sensPerChirpLuTable->ChirpBpmEn[0]) & M_RL_SENS_PER_CHIRP_LUT_ADD_MASK);

    ptrChirpCtrl->h_PerChirpParamCtrl = M_RL_SENS_PER_CHIRP_CTRL_MAX;
}

/**
 *  @b Description
 *  @n
 *      The function is used to populate the default open configuration.
 *
 *  @param[out]  ptrOpenCfg
 *      Pointer to the open configuration
 *
 *  @retval
 *      Not applicable
 */
 // Derived from hwa_fft1d example project - Deactivate Factory Calibration
void Mmwave_populateDefaultOpenCfg (MMWave_OpenCfg* ptrOpenCfg)
{
    ptrOpenCfg->useRunTimeCalib = false;
    ptrOpenCfg->useCustomCalibration = false;
    ptrOpenCfg->customCalibrationEnableMask = 0U;
    ptrOpenCfg->fecRDIFCtrlCmd.c_RdifEnable = M_RL_FECSS_RDIF_DIS;
    ptrOpenCfg->fecRDIFCtrlCmd.h_RdifSampleCount = NUM_ADC_SAMPLES; //profileComCfg.h_NumOfAdcSamples;
}

static void Mmwave_EnChannelSetOffset(
    CSL_app_hwa_adcbuf_ctrlRegs *ptrAdcBufCtrlRegs,
    uint8_t channel,
    uint16_t offset
)
{
    switch(channel)
    {
        case 0U:
            /* Enable the channel */
            CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG1,
                    APP_HWA_ADCBUF_CTRL_ADCBUFCFG1_ADCBUFCFG1_RX0EN,
                    1);

            /* Setup the offset */
            CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG2,
                    APP_HWA_ADCBUF_CTRL_ADCBUFCFG2_ADCBUFCFG2_ADCBUFADDRX0,
                    (offset >> 4));
            break;
        case 1U:
            /* Enable the channel */
            CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG1,
                    APP_HWA_ADCBUF_CTRL_ADCBUFCFG1_ADCBUFCFG1_RX1EN,
                    1);

            /* Setup the offset */
            CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG2,
                    APP_HWA_ADCBUF_CTRL_ADCBUFCFG2_ADCBUFCFG2_ADCBUFADDRX1,
                    (offset >> 4));
            break;
        case 2U:
            /* Enable the channel */
            CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG1,
                    APP_HWA_ADCBUF_CTRL_ADCBUFCFG1_ADCBUFCFG1_RX2EN,
                    1);

            /* Setup the offset */
            CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG3,
                    APP_HWA_ADCBUF_CTRL_ADCBUFCFG3_ADCBUFCFG3_ADCBUFADDRX2,
                    (offset >> 4));
            break;

        default:
            /* Not  supported channels, code should not end up here */
            DebugP_assert(0);
            break;
    }
}

static void Mmwave_ADCBufConfig
(
    uint16_t rxChannelEn,
    uint32_t chanDataSize
)
{
    CSL_app_hwa_adcbuf_ctrlRegs *ptrAdcBufCtrlRegs = (CSL_app_hwa_adcbuf_ctrlRegs *)CSL_APP_HWA_ADCBUF_CTRL_U_BASE;
    uint8_t channel = 0;
    uint16_t offset = 0;

    /* Disable all ADCBuf channels */
    CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG1, APP_HWA_ADCBUF_CTRL_ADCBUFCFG1_ADCBUFCFG1_RX0EN, 0);
    CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG1, APP_HWA_ADCBUF_CTRL_ADCBUFCFG1_ADCBUFCFG1_RX1EN, 0);
    CSL_FINS(ptrAdcBufCtrlRegs->ADCBUFCFG1, APP_HWA_ADCBUF_CTRL_ADCBUFCFG1_ADCBUFCFG1_RX2EN, 0);

    /* Enable Rx Channels */
    for (channel = 0; channel < SYS_COMMON_NUM_RX_CHANNEL; channel++)
    {
        if(rxChannelEn & (0x1U << channel))
        {
            /* Enable Channel and configure offset. */
            Mmwave_EnChannelSetOffset(ptrAdcBufCtrlRegs, channel, offset);

            /* Calculate offset for the next channel */
            offset  += chanDataSize;
        }
    }

    return;
}

void MMWave_populateChannelCfg()
{
    channelCfg.h_RxChCtrlBitMask  = RX_CH_CTRL_BITMASK;
    channelCfg.h_TxChCtrlBitMask  = TX_CH_CTRL_BITMASK;
    channelCfg.c_MiscCtrl         = CHANNEL_CFG_MISC_CTRL;

    // omitted "calculation" of rxAntOrder, since doppler nis not used in this project
}
    

/**
 *  @b Description
 *  @n
 *      The function is used to populate the default control configuration
 *      in chirp configuration mode
 *
 *  @param[out]  ptrCtrlCfg
 *      Pointer to the control configuration
 *
 *  @retval
 *      Not applicable
 */
void Mmwave_populateDefaultChirpControlCfg (MMWave_CtrlCfg* ptrCtrlCfg)
{
    T_RL_API_SENS_CHIRP_PROF_COMN_CFG       profileCfg;
    T_RL_API_SENS_CHIRP_PROF_TIME_CFG       profileTimeCfg;
    T_RL_API_SENS_PER_CHIRP_CFG             chirpCfg;
    T_RL_API_SENS_PER_CHIRP_CTRL            chirpCtrl;
    int32_t             errCode;
    MMWave_ChirpHandle  chirpHandle;

    MMWave_populateChannelCfg();
    
    Mmwave_ADCBufConfig(channelCfg.h_RxChCtrlBitMask, (profileComCfg.h_NumOfAdcSamples *2));

    /* Initialize the control configuration: */
    memset ((void*)ptrCtrlCfg, 0, sizeof(MMWave_CtrlCfg));

    /* Populate the frame configuration: */
    frameCfg.h_NumOfChirpsInBurst      = NUM_CHIRPS_PER_BURST;
    frameCfg.c_NumOfChirpsAccum        = NUM_CHIRPS_ACCUM;
    frameCfg.w_BurstPeriodicity        = W_BURST_PERIOD; 
    frameCfg.h_NumOfBurstsInFrame      = NUM_BURSTS_PER_FRAME;
    frameCfg.w_FramePeriodicity        = FRAME_PERIOD;
    frameCfg.h_NumOfFrames             = NUM_FRAMES;

    
    
    /* Populate the profile configuration: */
    Mmwave_populateDefaultProfileCfg (&profileCfg, &profileTimeCfg);

    /* Create the profile: */
    ptrCtrlCfg->frameCfg[0].profileHandle[0] = MMWave_addProfile (gCtrlHandle, &profileCfg, &profileTimeCfg, &errCode);
    if (ptrCtrlCfg->frameCfg[0].profileHandle[0] == NULL)
    {
        DebugP_logError ("Error: Unable to add the profile [Error code %d]\n", errCode);
        DebugP_log ("MMWave Add Profile Error");
        return;
    }
    DebugP_log ("MMWave Add Profile Success");

    /* Populate the default chirp configuration */
    Mmwave_populateDefaultChirpCfg (&chirpCfg, &chirpCtrl);

    /* Add the chirp to the profile: */
    chirpHandle = MMWave_addChirp (ptrCtrlCfg->frameCfg[0].profileHandle[0], &chirpCfg, &chirpCtrl, &errCode);
    if (chirpHandle == NULL)
    {
        DebugP_logError ("Error: Unable to add the chirp [Error code %d]\n", errCode);
        DebugP_log ("MMWave Add Chirp Error");
        return;
    }
    DebugP_log ("MMWave Add Chirp Success");

    /* Populate the frame configuration: */
    ptrCtrlCfg->frameCfg[0].frameCfg.h_NumOfChirpsInBurst = frameCfg.h_NumOfChirpsInBurst; //2; //10; //2U;
    ptrCtrlCfg->frameCfg[0].frameCfg.c_NumOfChirpsAccum = frameCfg.c_NumOfChirpsAccum; //0U;
   //ptrCtrlCfg->frameCfg[0].frameCfg.w_BurstPeriodicity = 2500U; /* 8 chirps = 40us + 10us idle */
    ptrCtrlCfg->frameCfg[0].frameCfg.w_BurstPeriodicity = frameCfg.w_BurstPeriodicity; //1698; //12000; //3480U; /* 4 chirps = 148us + 200us idle , (12 + 25) chirp*/
    ptrCtrlCfg->frameCfg[0].frameCfg.h_NumOfBurstsInFrame = frameCfg.h_NumOfBurstsInFrame; //64; //2; //32U;
    ptrCtrlCfg->frameCfg[0].frameCfg.w_FramePeriodicity = frameCfg.w_FramePeriodicity; //10000000; //120000; //29440U; /* 2 bursts = 696us + 40us idle, 40M XTAL */
    ptrCtrlCfg->frameCfg[0].frameCfg.h_NumOfFrames = frameCfg.h_NumOfFrames; //0; //10; //50U;
    ptrCtrlCfg->frameCfg[0].frameCfg.w_FrameEvent0TimeCfg = 0;

    /*\
        (ptrCtrlCfg->frameCfg[0].frameCfg.w_FramePeriodicity - 240U); frame period - 6us */
    ptrCtrlCfg->frameCfg[0].frameCfg.w_FrameEvent1TimeCfg = 0;

    /*\
        (ptrCtrlCfg->frameCfg[0].frameCfg.w_FramePeriodicity - 120U); frame period - 3us */

    ptrCtrlCfg->frameCfg[0].tempCfg.h_TempCtrlBitMask = 0x311;

    return;
}

/**
 *  @b Description
 *  @n
 *      The function is used to populate the default calibration
 *      configuration which is passed to start the mmWave module
 *
 *  @retval
 *      Not applicable
 */
void Mmwave_populateDefaultCalibrationCfg (MMWave_CalibrationCfg* ptrCalibrationCfg)
{
    /* Populate the calibration configuration: */
    ptrCalibrationCfg->chirpCalibrationCfg.enableCalibration    = false;
    ptrCalibrationCfg->chirpCalibrationCfg.enablePeriodicity    = false;
    ptrCalibrationCfg->chirpCalibrationCfg.periodicTimeInFrames = 10U;

    return;
}

/**
 *  @b Description
 *  @n
 *      The function is used to populate the default start
 *      configuration which is passed to start the mmWave module
 *
 *  @retval
 *      Not applicable
 */
void Mmwave_populateDefaultStartCfg (MMWave_StrtCfg* ptrStartCfg)
{
    /* Populate the start configuration: */
    ptrStartCfg->frameTrigMode      = SENSOR_START_FRAME_TRIG_MODE;
    ptrStartCfg->chirpStartSigLbEn  = SENSOR_START_CHIRP_START_SIG_LB_ENABLE;
    ptrStartCfg->frameLivMonEn      = SENSOR_START_FRAME_LIVE_MON_ENABLE;
    ptrStartCfg->frameTrigTimerVal  = SENSOR_START_FRAME_TRIG_TIMER_VAL;

    return;
}
