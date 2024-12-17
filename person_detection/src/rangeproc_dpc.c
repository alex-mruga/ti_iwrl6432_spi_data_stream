#include <stdio.h>
#include <kernel/dpl/DebugP.h>
#include "drivers/edma/v0/edma.h"
#include "kernel/dpl/SemaphoreP.h"
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include "drivers/prcm/v0/prcm.h"

#include <datapath/dpu/rangeproc/v0/rangeprochwa.h>
#include "drivers/hwa.h"

#include "rangeproc_dpc.h"

HWA_Handle hwaHandle;
DPU_RangeProcHWA_Handle rangeProcHWADpuHandle;
DPU_RangeProcHWA_Config rangeProcDpuCfg;

HwiP_Object gHwiChirpAvailableHwiObject;
HwiP_Object gHwiFrameStartHwiObject;

// Ping and Poing, hence 2 objects
Edma_IntrObject intrObj_Rangeproc[2];

volatile unsigned long long demoStartTime;

// TODO: ?
uint32_t window1DCoef[NUM_ADC_SAMPLES] __attribute__((section(".l3")));

int16_t radarCube[NUM_ADC_SAMPLES * NUM_CHIRPS_PER_FRAME * NUM_VIRT_ANTENNAS] __attribute((section(".l3")));

void rangeproc_main(void *args)
{
    // Status handle for HWA_open
    int32_t status = SystemP_SUCCESS;
    int32_t retVal = 0;

    DPU_RangeProcHWA_OutParams outParams;

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
    DebugP_log("Hallo Jonas!\r\n");

    RangeProc_config();

    // register Frame Start ISR
    if(registerFrameStartInterrupt() != 0){
        DebugP_log("Error: Failed to register frame start interrupts\n");
        DebugP_assert(0);
    }

    /* give initial trigger for the first frame*/
    retVal = DPU_RangeProcHWA_control(rangeProcHWADpuHandle, DPU_RangeProcHWA_Cmd_triggerProc, NULL, 0);
    if(retVal < 0)
    {
        /* Not Expected */
        DebugP_log("RangeProc DPU control error %d\n", retVal);
        DebugP_assert(0);
    }

    memset((void *)&outParams, 0, sizeof(DPU_RangeProcHWA_OutParams));
    retVal = DPU_RangeProcHWA_process(rangeProcHWADpuHandle, &outParams);

    if(retVal < 0){
        /* Not Expected */
        DebugP_log("RangeProc DPU process error %d\n", retVal);
        DebugP_assert(0);
    }

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

void RangeProc_config()
{
    DPU_RangeProcHWA_HW_Resources *pHwConfig = &rangeProcDpuCfg.hwRes;
    DPU_RangeProcHWA_StaticConfig *params = &rangeProcDpuCfg.staticCfg;
    uint32_t bytesPerRxChan;

    memset((void *)&rangeProcDpuCfg, 0, sizeof(DPU_RangeProcHWA_Config));

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
    // params->ADCBufData.dataSize = NUM_ADC_SAMPLES * NUM_RX_ANTENNAS * sizeof(uint16_t) * 2; // times 2, because of ping and pong C:\ti\mmwave-sdk\docs\MotionPresenceDetectionDemo_documentation.pdf 
    params->ADCBufData.dataProperty.numAdcSamples = NUM_ADC_SAMPLES;
    params->ADCBufData.dataProperty.numRxAntennas = NUM_RX_ANTENNAS;

    /* adc buffer buffer, format fixed, interleave, size will change */
    params->ADCBufData.dataProperty.dataFmt = DPIF_DATAFORMAT_REAL16;
    params->ADCBufData.dataProperty.adcBits = 2U; // 12-bit only
    params->ADCBufData.dataProperty.numChirpsPerChirpEvent = 1U;
    
    /* FFT optimizing params (derived from rangeproc DPU example) */
    params->rangeFFTtuning.fftOutputDivShift = 2;
    params->rangeFFTtuning.numLastButterflyStagesToScale = 0; /* no scaling needed as ADC is 16-bit and we have 8 bits to grow */  

    /* Set Motion Mode (Minor/Major) */
    params->enableMajorMotion = 1;
    params->enableMinorMotion = 0;
    params->numMinorMotionChirpsPerFrame = 0; // obsolete, not using minor motion

    /* bytes per RX channel (each chirp is uint_16) */
    bytesPerRxChan = NUM_ADC_SAMPLES * sizeof(uint16_t);
    bytesPerRxChan = (bytesPerRxChan + 15) / 16 * 16; // ensure that value is multiple of 16 (for EDMA?)
    
    /* initialize RX channel offsets */
    uint32_t index;
    for (index = 0; index < NUM_RX_ANTENNAS; index++)
    {
        params->ADCBufData.dataProperty.rxChanOffset[index] = index * bytesPerRxChan;
    }
    
    /* Further hardware config params (copied from rangeproc config example) */
    params->ADCBufData.dataProperty.interleave = DPIF_RXCHAN_NON_INTERLEAVE_MODE;


    /* Data Input EDMA */
    pHwConfig->edmaInCfg.dataIn.channel         = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_CH;
    pHwConfig->edmaInCfg.dataIn.channelShadow[0]   = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SHADOW_PING;
    pHwConfig->edmaInCfg.dataIn.channelShadow[1]   = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SHADOW_PONG;
    pHwConfig->edmaInCfg.dataIn.eventQueue      = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_EVENT_QUE;
    pHwConfig->edmaInCfg.dataInSignature.channel         = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SIG_CH;
    pHwConfig->edmaInCfg.dataInSignature.channelShadow   = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SIG_SHADOW;
    pHwConfig->edmaInCfg.dataInSignature.eventQueue      = DPC_OBJDET_DPU_RANGEPROC_EDMAIN_SIG_EVENT_QUE;

    // register EDMA interrupt object for rangeproc callback
    // this pointer holds 2 objects. 
    pHwConfig->intrObj = intrObj_Rangeproc;

    /* edma configuration */
    pHwConfig->edmaHandle  = gEdmaHandle[0];
 
    /* Data Output EDMA */
    pHwConfig->edmaOutCfg.path[0].evtDecim.channel = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PING_CH;
    pHwConfig->edmaOutCfg.path[0].evtDecim.channelShadow[0] = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PING_SHADOW_0;
    pHwConfig->edmaOutCfg.path[0].evtDecim.channelShadow[1] = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PING_SHADOW_1;
    pHwConfig->edmaOutCfg.path[0].evtDecim.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PING_EVENT_QUE;
    
    pHwConfig->edmaOutCfg.path[1].evtDecim.channel = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PONG_CH;
    pHwConfig->edmaOutCfg.path[1].evtDecim.channelShadow[0] = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PONG_SHADOW_0;
    pHwConfig->edmaOutCfg.path[1].evtDecim.channelShadow[1] = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PONG_SHADOW_1;
    pHwConfig->edmaOutCfg.path[1].evtDecim.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EVT_DECIM_PONG_EVENT_QUE;
    
    pHwConfig->edmaOutCfg.path[0].dataOutMinor.channel = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PING_CH;
    pHwConfig->edmaOutCfg.path[0].dataOutMinor.channelShadow = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PING_SHADOW;
    pHwConfig->edmaOutCfg.path[0].dataOutMinor.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PING_EVENT_QUE;
    
    pHwConfig->edmaOutCfg.path[1].dataOutMinor.channel = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PONG_CH;
    pHwConfig->edmaOutCfg.path[1].dataOutMinor.channelShadow = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PONG_SHADOW;
    pHwConfig->edmaOutCfg.path[1].dataOutMinor.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MINOR_PONG_EVENT_QUE;
 
    pHwConfig->edmaOutCfg.path[0].dataOutMajor.channel = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PING_CH;
    pHwConfig->edmaOutCfg.path[0].dataOutMajor.channelShadow = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PING_SHADOW;
    pHwConfig->edmaOutCfg.path[0].dataOutMajor.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PING_EVENT_QUE;
    
    pHwConfig->edmaOutCfg.path[1].dataOutMajor.channel = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PONG_CH;
    pHwConfig->edmaOutCfg.path[1].dataOutMajor.channelShadow = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PONG_SHADOW;
    pHwConfig->edmaOutCfg.path[1].dataOutMajor.eventQueue = DPC_OBJDET_DPU_RANGEPROC_EDMAOUT_MAJOR_PONG_EVENT_QUE;
   
    /* radar cube config*/
    /* total size of radar cube in bytes*/
    pHwConfig->radarCube.dataSize = params->numRangeBins * NUM_VIRT_ANTENNAS * sizeof(cmplx16ReIm_t) * params->numDopplerChirpsPerProc;
    pHwConfig->radarCube.datafmt = DPIF_RADARCUBE_FORMAT_6;
        
    /* Further non EDMA related HWA configurations */
    pHwConfig->hwaCfg.paramSetStartIdx = 0;
    pHwConfig->hwaCfg.numParamSet = DPU_RANGEPROCHWA_NUM_HWA_PARAM_SETS;
    pHwConfig->hwaCfg.hwaWinRamOffset  = DPC_OBJDET_HWA_WINDOW_RAM_OFFSET; 
    pHwConfig->hwaCfg.hwaWinSym = 1;
    pHwConfig->hwaCfg.dataInputMode = DPU_RangeProcHWA_InputMode_ISOLATED;
    
    /* windowing buffer is fixed, size will change*/
    rangeProcDpuCfg.staticCfg.window =  (int32_t *)&window1DCoef[0];
    
    /* adc buffer buffer, format fixed, interleave, size will change */
    rangeProcDpuCfg.staticCfg.ADCBufData.dataProperty.dataFmt = DPIF_DATAFORMAT_REAL16;
    rangeProcDpuCfg.staticCfg.ADCBufData.dataProperty.adcBits = 2U; // 12-bit only
    rangeProcDpuCfg.staticCfg.ADCBufData.dataProperty.numChirpsPerChirpEvent = 1U;
    params->ADCBufData.data = (void *)CSL_APP_HWA_ADCBUF_RD_U_BASE;
    
    /* radar cube */
    rangeProcDpuCfg.hwRes.radarCube.data  = (cmplx16ImRe_t *) &radarCube[0];


    /* configure HWA with set parameters */
    int32_t retVal;
    retVal = DPU_RangeProcHWA_config(rangeProcHWADpuHandle, &rangeProcDpuCfg);
 
    if(retVal < 0)
    {
        DebugP_log("DEBUG: RANGE DPU config return error:%d \n", retVal);
        DebugP_assert(0);
    }
}

int32_t registerFrameStartInterrupt(void)
{
    int32_t           retVal = 0;
    int32_t           status = SystemP_SUCCESS;
    HwiP_Params       hwiPrms;

    /* Register interrupt */
    HwiP_Params_init(&hwiPrms);
    hwiPrms.intNum      = 16 + CSL_APPSS_INTR_FECSS_FRAMETIMER_FRAME_START;
    hwiPrms.callback    = frameStartISR;
    /* Use this to change the priority */
    //hwiPrms.priority    = 0;
    status              = HwiP_construct(&gHwiFrameStartHwiObject, &hwiPrms);

    if(SystemP_SUCCESS != status)
    {
        retVal = SystemP_FAILURE;
    }
    else
    {
        HwiP_enableInt((uint32_t)CSL_APPSS_INTR_FECSS_FRAMETIMER_FRAME_START);
    }

    return retVal;
}

static void frameStartISR(void *arg)
{
    uint32_t curCycle;

    /* Clear the interrupt */
    HwiP_clearInt(CSL_APPSS_INTR_FECSS_FRAMETIMER_FRAME_START);

    /* Record the frame start time for profiling or other processing */
    demoStartTime = PRCMSlowClkCtrGet();
    curCycle = Cycleprofiler_getTimeStamp();

    /* Optionally, perform any other frame processing needed here */
    // For example, you might calculate the frame period or process the data further.
}

//The function reads the FRAME_REF_TIMER that runs free at 40MHz
uint32_t Cycleprofiler_getTimeStamp(void)
{
    uint32_t *frameRefTimer;
    frameRefTimer = (uint32_t *) 0x5B000020;
    return *frameRefTimer;
}
