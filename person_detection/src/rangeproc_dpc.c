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
    /* number of TX antennas (on the IWRL6432BOOST 2)*/
    params->numTxAntennas = NUM_TX_ANTENNAS;
    /* number of RX antennas, product of TX- and RX-antennas (on the IWRL6432BOOST 2*3=6) */
    params->numVirtualAntennas = NUM_VIRT_ANTENNAS;
    /* size of range FFT: equal to number of ADC samples*/
    params->rangeFftSize = NUM_ADC_SAMPLES;
    /* size of real part of range FFT: half of the range FFT size, since the ADC samples are real valued*/
    params->numRangeBins = NUM_ADC_SAMPLES/2;
    /* number of chirps per frame (= number of chirps per burst, if Nburst = 1) */
    params->numChirpsPerFrame = NUM_CHIRPS_PER_FRAME;
    /* number of doppler chirps per frame (derived from rangeproc init example) */
    params->numDopplerChirpsPerFrame = params->numChirpsPerFrame/params->numTxAntennas;
    params->numDopplerChirpsPerProc = params->numDopplerChirpsPerFrame;
    /* enable BPM mode (instead of TDM) */
    params->isBpmEnabled = TRUE;

    /* windowing */
    params->windowSize = sizeof(uint32_t) * ((NUM_ADC_SAMPLES +1 ) / 2); // symmetric window (Blackman), for real samples (therefore /2)
    /* dataSize defines the size of buffer that holds ADC data of every frame */
    /* ADCBufData.dataSize omitted due to forum post: https://e2e.ti.com/support/sensors-group/sensors/f/sensors-forum/1324580/awrl6432boost-adc-buffer-data-size-in-motion-and-presence-detection-demo */
    //params->ADCBufData.dataSize = NUM_ADC_SAMPLES * NUM_RX_ANTENNAS * sizeof(uint16_t) * 2; // times 2, because of ping and pong C:\ti\mmwave-sdk\docs\MotionPresenceDetectionDemo_documentation.pdf 
    params->ADCBufData.dataProperty.numAdcSamples = NUM_ADC_SAMPLES;
    params->ADCBufData.dataProperty.numRxAntennas = NUM_RX_ANTENNAS;
    
    /* FFT optimizing params (derived from rangeproc DPU example) */
    params->rangeFFTtuning.fftOutputDivShift = 2;
    params->rangeFFTtuning.numLastButterflyStagesToScale = 0; /* no scaling needed as ADC is 16-bit and we have 8 bits to grow */  

    /* Set Motion Mode (Minor/Major) */
    params->enableMajorMotion = 1;
    params->enableMinorMotion = 0;
    params->numMinorMotionChirpsPerFrame = 0; // obsolete, not using minor motion
}
