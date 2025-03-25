#ifndef SYSTEM_H
#define SYSTEM_H


#include <common/syscommon.h>
#include <control/mmwave/mmwave.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/SystemP.h>
#include <datapath/dpu/rangeproc/v0/rangeprochwa.h>
#include <drivers/hwa.h>
#include "kernel/dpl/SemaphoreP.h"

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