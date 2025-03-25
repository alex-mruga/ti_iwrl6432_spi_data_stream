#ifndef FACTORY_CAL_H
#define FACTORY_CAL_H

/**
 * @file factory_cal.h
 * @brief Factory calibration realted functions.
 *
 * This header defines data structure and function to hold calibration data. 
 *
 *
 * @note This function in this file is adapted from the Motion and Presence Detection Demo:
 *       ${MMWAVE_SDK_INSTALL_PATH}\examples\mmw_demo\motion_and_presence_detection\source\calibration\factory_cal.c
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



/**
 * @brief Magic word for factory calibration data validation.
 *
 * This value is stored in flash alongside calibration data and checked upon 
 * restoration to verify data integrity. It acts as a simple checksum to ensure 
 * the calibration data is valid.
 */
#define MMWDEMO_CALIB_STORE_MAGIC (0x7CB28DF9U)



/*!
 * @brief
 * Structure holds calibration save configuration used during sensor open.
 *
 * @details
 *  The structure holds calibration save configuration.
 */
typedef struct Mmw_calibData_t
{
    /*! @brief      Magic word for calibration data */
    uint32_t 	    magic;

    /*! @brief      RX TX Calibration data */
    T_RL_API_FECSS_RXTX_CAL_DATA  calibData;
} Mmw_calibData;

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
int32_t restoreFactoryCal(void);

#endif //FACTORY_CAL_H