#ifndef MMWAVE_BASIC_H
#define MMWAVE_BASIC_H

/**
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
 *  @b Description
 *  @n
 *    This header file defines the interface for the basic mmwave initialization
 *      functionality on the basis of which the DPUs can be used.
 */

/**
 * @brief sets start address and size of shared mempool
*/
void mempool_init(void);

/**
 * @brief calls HWA_open() and thereby retrieves the HWA's handle (HWA_Handle)
*/
int32_t hwa_open_handler(void);

/**
 * @brief calls the MMWave_init() function and thereby retrieves the mmwave control handle (MMWave_Handle)
*/
int32_t mmwave_initSensor(void);

/**
 * @brief calls the MMWave_open() function, requires the configuration for open (MMWave_OpenCfg)
*/
int32_t mmwave_openSensor(void);

/**
 * @brief calls the MMWave_config() function, requires the configuration for control (MMWave_CtrlCfg)
*/
int32_t mmwave_configSensor(void);

/**
 * @brief calls the MMWave_start() function, requires the configuration for start (MMWave_StartCfg)
*/
int32_t mmwave_startSensor(void);

/**
 * @brief calls the MMWave_stop(), MMWave_close() and MMWave_deinit() function
*/
int32_t mmwave_stop_close_deinit(void);

void Mmwave_HwaConfig_custom (void);

#endif //MMWAVE_BASIC_H