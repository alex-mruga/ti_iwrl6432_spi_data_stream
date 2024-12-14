#include <stdio.h>
#include <kernel/dpl/DebugP.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include <datapath/dpu/rangeproc/v0/rangeprochwa.h>
#include "drivers/hwa.h"

#include "rangeproc_dpc.h"

HWA_Handle hwaHandle;
DPU_RangeProcHWA_Handle rangeProcHWADpuHandle;
DPU_RangeProcHWA_Config rangeProcDpuCfg;

void rangeproc_main(void *args)
{
    // Status handle for HWA_open
    int32_t status = SystemP_SUCCESS;

    /* Open drivers to open the UART driver for console */
    Drivers_open();
    Board_driversOpen();

    hwaHandle = HWA_open(0, NULL, &status);
    if (hwaHandle == NULL)
    {
        DebugP_log("Error: Unable to open the HWA Instance err:%d\n", status);
        DebugP_assert(0);
    }

    rangeProc_dpuInit();
    DebugP_log("Hello World!\r\n");

    Board_driversClose();
    Drivers_close();
}

void rangeProc_dpuInit()
{
    int32_t errorCode = 0;
    DPU_RangeProcHWA_InitParams initParams;
    initParams.hwaHandle = hwaHandle;

    rangeProcHWADpuHandle = DPU_RangeProcHWA_init(&initParams, &errorCode);
    if (rangeProcHWADpuHandle == NULL)
    {
        DebugP_log ("Debug: RangeProc DPU initialization returned error %d\n", errorCode);
        DebugP_assert (0);
        return;
    }
}

void RangeProc_configParser()
{
    DPU_RangeProcHWA_HW_Resources *pHwConfig = &rangeProcDpuCfg.hwRes;
    DPU_RangeProcHWA_StaticConfig  * params;

    /*
      For values refer to "Sensor front-end parameters" in:
      https://software-dl.ti.com/ra-processors/esd/MMWAVE-L-SDK/05_04_00_01/exports/api_guide_xwrL64xx/MMWAVE_DEMO.html
     */
    params->numTxAntennas = NUM_TX_ANTENNAS;
    params->numVirtualAntennas = NUM_VIRT_ANTENNAS;
    params->numRangeBins = NUM_ADC_SAMPLES; // real only input /2 previouisly
    params->numChirpsPerFrame = testConfig->numChirpsPerFrame;
    params->numDopplerChirpsPerFrame = params->numChirpsPerFrame/params->numTxAntennas;
    params->numDopplerChirpsPerProc = params->numDopplerChirpsPerFrame;
    params->isBpmEnabled = TRUE;
    /* windowing */
    params->windowSize = sizeof(uint32_t) * ((testConfig->numAdcSamples +1 ) / 2); //symmetric window, for real samples
    params->ADCBufData.dataSize = testConfig->numAdcSamples * testConfig->numRxAntennas * 4 ;  
    params->ADCBufData.dataProperty.numAdcSamples = testConfig->numAdcSamples;
    params->ADCBufData.dataProperty.numRxAntennas = testConfig->numRxAntennas;
 
    params->rangeFFTtuning.fftOutputDivShift = 2;
    params->rangeFFTtuning.numLastButterflyStagesToScale = 0; /* no scaling needed as ADC is 16-bit and we have 8 bits to grow */  
    params->enableMajorMotion = 1;
    params->enableMinorMotion = 0;
    //params->numFramesPerMinorMotProc = 4;
    params->numMinorMotionChirpsPerFrame = 0;
 
    //params->rangeFftSize = HWAFFT_log2Approx(testConfig->numAdcSamples);
    params->rangeFftSize = testConfig->numAdcSamples;
}
