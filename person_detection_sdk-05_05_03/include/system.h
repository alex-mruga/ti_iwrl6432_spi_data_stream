#ifndef SYSTEM_H
#define SYSTEM_H

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

#include <common/syscommon.h>
#include <control/mmwave/mmwave.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <datapath/dpu/rangeproc/v0/rangeprochwa.h>
#include <drivers/hwa.h>
#include "kernel/dpl/SemaphoreP.h"


/*!
 * @brief DMA channel defines (as opposed to more dynamic DPC_ObjDet_HwaDmaTrigSrcChanPoolAlloc() in MPD demo project)
 */
#define DMA_TRIG_SRC_CHAN_0 0
#define DMA_TRIG_SRC_CHAN_1 1


/*!
 * @brief Memory Configuration used during init API
 */
typedef struct DPC_ObjectDetection_MemCfg_t
{
    /*! @brief   Start address of memory provided by the application
     *           from which DPC will allocate.
     */
    void *addr;

    /*! @brief   Size limit of memory allowed to be consumed by the DPC */
    uint32_t size;
} DPC_ObjectDetection_MemCfg;

/*!
 * @brief Memory pool object to manage memory based on @ref DPC_ObjectDetection_MemCfg_t.
 */
typedef struct MemPoolObj_t
{
    /*! @brief Memory configuration */
    DPC_ObjectDetection_MemCfg cfg;

    /*! @brief   Pool running adress.*/
    uintptr_t currAddr;

    /*! @brief   Pool max address. This pool allows setting address to desired
     *           (e.g for rewinding purposes), so having a running maximum
     *           helps in finding max pool usage
     */
    uintptr_t maxCurrAddr;
} MemPoolObj;


/*! @brief Global struct which holds all handles and configs. */
typedef struct{

    /*! @brief This is the mmWave control handle which is used to configure the BSS. */
    MMWave_Handle gCtrlHandle;

    /*! @brief This is the hwa handle which is used access and configure the HWA. */
    HWA_Handle hwaHandle;

    /*! @brief  Configuration to open DFP */
    MMWave_OpenCfg mmwOpenCfg;

    /*! @brief  Configuration for mmwave control */
    MMWave_CtrlCfg mmwCtrlCfg;

    /*! @brief  Configuration for mmwave start (equal to gMmwMssMCB.sensorStart from mmwave demo project) */
    MMWave_StrtCfg sensorStartCfg;

    /*! @brief L3 ram memory pool object */
    MemPoolObj    L3RamObj;

    /*! @brief Core Local ram memory pool object */
    MemPoolObj    CoreLocalRamObj;

    /*! @brief Handle for Range Processing DPU */
    DPU_RangeProcHWA_Handle rangeProcHWADpuHandle;

    /*! @brief Config for Rangeproc DPU */
    DPU_RangeProcHWA_Config rangeProcDpuCfg;

    T_RL_API_SENS_CHIRP_PROF_COMN_CFG profileComCfg;
    T_RL_API_SENS_CHIRP_PROF_TIME_CFG profileTimeCfg;
    T_RL_API_FECSS_RF_PWR_CFG_CMD channelCfg;
    T_RL_API_SENS_FRAME_CFG frameCfg;
    T_RL_API_FECSS_RUNTIME_TX_CLPC_CAL_CMD fecTxclpcCalCmd;
} SystemContext_t;

extern SystemContext_t gSysContext;

#endif //SYSTEM_H