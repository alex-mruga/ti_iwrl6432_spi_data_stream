#ifndef RANGEPROC_DPC_H
#define RANGEPROC_DPC_H

#include <stdint.h>
#include <kernel/dpl/DebugP.h>
#include "kernel/dpl/SemaphoreP.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <datapath/dpu/rangeproc/v0/rangeprochwa.h>
#include "drivers/hwa.h"

#include "dpu_res.h"
#include "defines.h"

#define DPC_OBJDET_QFORMAT_RANGE_FFT 17

extern SemaphoreP_Object dpcCfgDoneSemHandle;

/**
 *  @b Description
 *  @n
 *    This header file defines the interface for range processing
 *    DPU initialization and related functions.
 */

/**
 * @brief Handle for Range Processing DPU
 */
extern DPU_RangeProcHWA_Handle rangeProcHWADpuHandle;

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


void dpcTask();

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
static void frameStartISR(void *arg);

int32_t registerChirpInterrupt(void);
void chirpStartISR(void *arg);

uint32_t Cycleprofiler_getTimeStamp(void);


int32_t registerChirpAvailableInterrupts(void);
static void ChirpAvailISR(void *arg);


#endif /* RANGEPROC_DPC_H */
