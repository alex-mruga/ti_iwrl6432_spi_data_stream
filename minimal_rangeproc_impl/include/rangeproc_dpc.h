#ifndef RANGEPROC_DPC_H
#define RANGEPROC_DPC_H

/**
 * @file rangeproc_dpc.h
 * @brief Range Processing Data Path Unit (DPU) Interface.
 *
 * This file defines the interface for the Range Processing Data Path Unit (DPU) and
 * its associated functions. It includes declarations for DPU initialization, configuration,
 * and task management, as well as interrupt handling for frame start, chirp start, and
 * chirp available events. The module is designed to facilitate radar signal processing
 * using the Hardware Accelerator (HWA) and SPI communication for data transmission.
 *
 * The range processing DPU is responsible for processing radar data frames, including
 * FFT computation, object detection, and data transmission over SPI. This module
 * integrates with the TI mmWave SDK and is derived from the Motion and Presence Detection
 * Demo provided in the SDK.
 *
 * @note This module relies on the SemaphoreP API for synchronization and the HWA for
 *       hardware-accelerated signal processing.
 *
 * @copyright (C) 2022-24 Texas Instruments Incorporated
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



#define DPC_OBJDET_QFORMAT_RANGE_FFT 17

extern SemaphoreP_Object dpcCfgDoneSemHandle;
extern SemaphoreP_Object spi_tx_start_sem;
extern SemaphoreP_Object spi_tx_done_sem;

/**
 *  @b Description
 *  @n
 *    This header file defines the interface for range processing
 *    DPU initialization and related functions.
 */

/**
 * @brief Initializes the Range Processing DPU
 *
 * This function initializes the range processing DPU (Data Path Unit) and
 * sets up the required handles for HWA (Hardware Accelerator) and DPU.
 * It is derived from the dpc.c of the mmwavedemo project.
 *
 * @retval None
 */
void rangeProc_dpuInit(void);

/**
 * @brief Configures the Range Processing DPU
 *
 * This function configures the range processing DPU (Data Path Unit) and the HWA
 * It is derived from the dpc.c of the mmwavedemo project.
 *
 * @retval None
 */
void RangeProc_config();

/**
 * @brief Main function for Range Processing DPU
 *
 * This function opens the necessary drivers, initializes the HWA,
 * calls the range processing DPU initialization, and closes all drivers
 * after execution. It is the entry point for the range processing task.
 *
 * @param[in] args Arguments passed to the function (if any).
 *
 * @retval None
 */
void rangeproc_main(void *args);

/**
 * @brief Task function for Data Processing Chain (DPC).
 *
 * This function initializes and manages the data processing chain, including
 * configuring the DPUs, registering interrupts, and triggering SPI transmission.
 * It runs in an infinite loop, processing data frames and triggering the next frame.
 */
void dpcTask();

/**
 * @brief Task function for SPI transmission.
 *
 * This function continuously transmits data over SPI.
 */
void spiTask();

/**
 * @brief Registers an interrupt for the frame start event.
 *
 * This function configures and registers the interrupt handler for the frame start event,
 * which is triggered by the frame timer. It sets up the interrupt priority and callback function,
 * then enables the interrupt to trigger the corresponding handler.
 * Copied from ${MMWAVE_SDK_INSTALL_PATH}\examples\mmw_demo\motion_and_presence_detection\source\interrupts.c
 * 
 * @param None
 *
 * @retval SystemP_SUCCESS on successful registration, SystemP_FAILURE on error.
 */
int32_t registerFrameStartInterrupt(void);

/**
 * @brief Interrupt Service Routine for Frame Start.
 *
 * This ISR is called when a frame start event occurs. It clears the interrupt flag
 * and increments the frame count.
 *
 * @param arg Pointer to optional arguments (unused in this implementation).
 */
static void frameStartISR(void *arg);

/**
 * @brief Registers the Chirp Interrupt.
 *
 * This function registers the interrupt for the chirp start and end events.
 *
 * @return int32_t Returns SystemP_SUCCESS on success, SystemP_FAILURE on failure.
 */
int32_t registerChirpInterrupt(void);

/**
 * @brief ISR for Chirp Start event.
 *
 * This ISR is triggered when the chirp start event occurs, indicating that the RF time control is functioning correctly. It does not necessarily mean the RF is actively chirping.
 *
 * @param arg Unused optional argument.
 */
void chirpStartISR(void *arg);

uint32_t Cycleprofiler_getTimeStamp(void);

/**
 * @brief Registers the Chirp Available Interrupt.
 *
 * This function registers the interrupt for the chirp available event.
 *
 * @return int32_t Returns SystemP_SUCCESS on success, SystemP_FAILURE on failure.
 */
int32_t registerChirpAvailableInterrupts(void);
static void ChirpAvailISR(void *arg);


#endif /* RANGEPROC_DPC_H */
