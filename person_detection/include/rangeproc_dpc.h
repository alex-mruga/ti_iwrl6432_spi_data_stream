#ifndef RANGEPROC_DPC_H
#define RANGEPROC_DPC_H

#include <stdint.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"
#include <datapath/dpu/rangeproc/v0/rangeprochwa.h>
#include "drivers/hwa.h"

/**
 *  @b Description
 *  @n
 *    This header file defines the interface for range processing
 *    DPU initialization and related functions.
 */

/**
 * @brief Handle for HWA
 */
extern HWA_Handle hwaHandle;

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

#endif /* RANGEPROC_DPC_H */
