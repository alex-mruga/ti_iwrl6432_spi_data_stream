/*
 * Copyright (C) 2022-23 Texas Instruments Incorporated
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

/**************************************************************************
 *************************** Include Files ********************************
 **************************************************************************/

/* Standard Include Files. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "kernel/dpl/DebugP.h"
#include "kernel/dpl/SystemP.h"
#include "control/mmwave/mmwave.h"
#include "ti_board_open_close.h"

#include <board/flash.h>
#include <mmwavelink/mmwavelink.h>
#include <mmwavelink/include/rl_device.h>
#include <mmwavelink/include/rl_sensor.h>
#include <kernel/dpl/CacheP.h>

#include "factory_cal.h"
#include "mmwave_basic.h"
#include "mmwave_control_config.h"
#include "defines.h"

// external
extern T_RL_API_SENS_CHIRP_PROF_COMN_CFG profileComCfg;
extern T_RL_API_SENS_CHIRP_PROF_TIME_CFG profileTimeCfg;
extern T_RL_API_FECSS_RF_PWR_CFG_CMD channelCfg;
extern T_RL_API_SENS_FRAME_CFG frameCfg;

extern MMWave_Handle gCtrlHandle;
extern T_RL_API_FECSS_RUNTIME_TX_CLPC_CAL_CMD fecTxclpcCalCmd;

/**
 * @brief Magic word for factory calibration data validation.
 *
 * This value is stored in flash alongside calibration data and checked upon 
 * restoration to verify data integrity. It acts as a simple checksum to ensure 
 * the calibration data is valid.
 */
#define MMWDEMO_CALIB_STORE_MAGIC (0x7CB28DF9U)

Mmw_calibData calibData __attribute__((aligned(8))) = {0};

/**
 * @brief Restores factory calibration data from flash.
 *
 * This function reads the calibration data stored in flash memory and restores it.
 * It requires that the system has been previously calibrated.
 *
 * Derived from `mmwDemo_factoryCal` and `MmwDemo_calibRestore` in `factory_cal.c` 
 * from the demo project.
 *
 * @return SystemP_SUCCESS on success, -1 on failure.
 */
int32_t restoreFactoryCal(void)
{
    uint16_t         calRfFreq = 0U;
    MMWave_calibCfg  factoryCalCfg = {0U};
    int32_t          retVal = SystemP_SUCCESS;
    int32_t          errCode;
    MMWave_ErrorLevel   errorLevel;
    int16_t          mmWaveErrorCode;
    int16_t          subsysErrorCode;

    /* Enable sensor boot time calibration: */
    factoryCalCfg.isFactoryCalEnabled = true;

    /*
    * @brief  FECSS RFS Boot calibration control:
    * | bits [0] | RESERVED
    * | bits [1] | VCO calibration ON/OFF control
    * | bits [2] | PD calibration ON/OFF control
    * | bits [3] | LODIST calibration ON/OFF control
    * | bits [4] | RESERVED 
    * | bits [5] | RX IFA calibration ON/OFF control
    * | bits [6] | RX Gain calibration ON/OFF control
    * | bits [7] | TX power calibration ON/OFF control
    */
    /* As part of Factory Calibration, enable all calibrations except RX IFA calibration */
    factoryCalCfg.fecRFFactoryCalCmd.h_CalCtrlBitMask = 0xCEU;
    factoryCalCfg.fecRFFactoryCalCmd.c_MiscCalCtrl = 0x0U;

    factoryCalCfg.fecRFFactoryCalCmd.c_CalRxGainSel = CLI_FACCALCFG_RX_GAIN;
    factoryCalCfg.fecRFFactoryCalCmd.c_CalTxBackOffSel[0] = CLI_FACCALCFG_TX_BACKOFF_SEL;
    factoryCalCfg.fecRFFactoryCalCmd.c_CalTxBackOffSel[1] = CLI_FACCALCFG_TX_BACKOFF_SEL;

    /* Calculate Calibration Rf Frequency. Use Center frequency of the bandwidth(being used in demo) for calibration */
    calRfFreq = (profileTimeCfg.w_ChirpRfFreqStart) + \
                ((((CHIRPTIMINGCFG_CHIRP_RF_FREQ_SLOPE * 256.0)/300) * (profileComCfg.h_ChirpRampEndTime * 0.1)) / 2);
    factoryCalCfg.fecRFFactoryCalCmd.xh_CalRfSlope = 0x4Du; /* 2.2Mhz per uSec*/


    factoryCalCfg.fecRFFactoryCalCmd.h_CalRfFreq = calRfFreq;
    if(channelCfg.h_TxChCtrlBitMask == 0x3)
    {
        factoryCalCfg.fecRFFactoryCalCmd.c_TxPwrCalTxEnaMask[0] = 0x3;
        factoryCalCfg.fecRFFactoryCalCmd.c_TxPwrCalTxEnaMask[1] = 0x1;
    }
    else 
    {
        if(channelCfg.h_TxChCtrlBitMask == 0x1)
        {
            factoryCalCfg.fecRFFactoryCalCmd.c_TxPwrCalTxEnaMask[0] = 0x1;
            factoryCalCfg.fecRFFactoryCalCmd.c_TxPwrCalTxEnaMask[1] = 0x1;
        }
        if(channelCfg.h_TxChCtrlBitMask == 0x2)
        {
            factoryCalCfg.fecRFFactoryCalCmd.c_TxPwrCalTxEnaMask[0] = 0x2;
            factoryCalCfg.fecRFFactoryCalCmd.c_TxPwrCalTxEnaMask[1] = 0x2;
        }
    }

    /* Check if the device is RF-Trimmed */
    
    factoryCalCfg.ptrAteCalibration = NULL;
    factoryCalCfg.isATECalibEfused  = true;

    /* Read flash memory */
    retVal = Flash_read(gFlashHandle[0], CLI_FACCALCFG_FLASH_OFFSET, (uint8_t *) &calibData, sizeof(Mmw_calibData));
    CacheP_wb((uint8_t *) &calibData, sizeof(Mmw_calibData), CacheP_TYPE_ALL);

    if(retVal == SystemP_FAILURE)
    {
        DebugP_log("Could not read from Flash to restore Calibration data!");
        return -1;
    }

    /* Validate Calib data Magic number */
    if(calibData.magic != MMWDEMO_CALIB_STORE_MAGIC)
    {
        /* Header validation failed */
        DebugP_log("Error: MmwDemo Factory calibration data header validation failed.\r\n");
        return -1;
    }

    /* Populate calibration data pointer. */
    factoryCalCfg.ptrFactoryCalibData = &calibData.calibData;

    /* Disable factory calibration. */
    factoryCalCfg.isFactoryCalEnabled = false;

    retVal = MMWave_factoryCalibConfig(gCtrlHandle, &factoryCalCfg, &errCode);
    if (retVal != SystemP_SUCCESS)
    {

        /* Error: Unable to perform boot calibration */
        MMWave_decodeError (errCode, &errorLevel, &mmWaveErrorCode, &subsysErrorCode);

        /* Error: Unable to initialize the mmWave control module */
        DebugP_log("Error: mmWave Control Initialization failed [Error code %d] [errorLevel %d] [mmWaveErrorCode %d] [subsysErrorCode %d]\n", errCode, errorLevel, mmWaveErrorCode, subsysErrorCode);
        if (mmWaveErrorCode == MMWAVE_ERFSBOOTCAL)
        {
            DebugP_log("Error: Factory Calibration failure\n");
            return -1;
        }
        else
        {
            DebugP_log("Error: Invalid Factory calibration arguments\n");
            return -1;
        }
    }

    /* Configuring command for Run time CLPC calibration (Required if CLPC calib is enabled) */
    fecTxclpcCalCmd.c_CalMode = 0x0u; /* No Override */
    fecTxclpcCalCmd.c_CalTxBackOffSel[0] = factoryCalCfg.fecRFFactoryCalCmd.c_CalTxBackOffSel[0];
    fecTxclpcCalCmd.c_CalTxBackOffSel[1] = factoryCalCfg.fecRFFactoryCalCmd.c_CalTxBackOffSel[1];
    fecTxclpcCalCmd.h_CalRfFreq = factoryCalCfg.fecRFFactoryCalCmd.h_CalRfFreq;
    fecTxclpcCalCmd.xh_CalRfSlope = factoryCalCfg.fecRFFactoryCalCmd.xh_CalRfSlope;
    fecTxclpcCalCmd.c_TxPwrCalTxEnaMask[0] = factoryCalCfg.fecRFFactoryCalCmd.c_TxPwrCalTxEnaMask[0];
    fecTxclpcCalCmd.c_TxPwrCalTxEnaMask[1] = factoryCalCfg.fecRFFactoryCalCmd.c_TxPwrCalTxEnaMask[1];

    return retVal;
}