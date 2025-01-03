#define LOW_POWER_MODE 2

/* basic configuration ( and frameCfg)*/
#define NUM_TX_ANTENNAS 2
#define NUM_RX_ANTENNAS 3
#define NUM_VIRT_ANTENNAS (NUM_TX_ANTENNAS * NUM_RX_ANTENNAS)
#define NUM_ADC_SAMPLES 128 // number of adc samples per chirp hardcoded in https://dev.ti.com/gallery/view/mmwave/mmWaveSensingEstimator/ver/2.4.0/ and also used in MOTION_AND_PRESENCE_DETECTION_DEMO
#define NUM_BURSTS_PER_FRAME 1 // from MOTION_AND_PRESENCE_DETECTION_DEMO
#define NUM_CHIRPS_PER_BURST 8 // from MOTION_AND_PRESENCE_DETECTION_DEMO
#define NUM_CHIRPS_PER_FRAME (NUM_BURSTS_PER_FRAME * NUM_CHIRPS_PER_BURST)

// calculated defines, not in config
#define NUM_RANGE_BINS (NUM_ADC_SAMPLES / 2)
#define NUM_DOPPLER_CHIRPS_PER_FRAME (NUM_ADC_SAMPLES / NUM_TX_ANTENNAS)
#define NUM_DOPPLER_CHIRPS_PER_PROC NUM_DOPPLER_CHIRPS_PER_FRAME
///////////////////

/* chirpComnCfg */
#define CHIRPCOMNCFG_DIG_OUTPUT_SAMP_RATE         20  // M_RL_SENS_DIG_OUT_SAMP_RATE_MAX_12P5M
#define CHIRPCOMNCFG_DIG_OUTPUT_BITS_SEL          0   // M_RL_SENS_DIG_OUT_12BITS_4LSB_ROUND
#define CHIRPCOMNCFG_DFE_FIR_SEL                  0   // M_RL_SENS_DFE_FIR_LONG_FILT
#define CHIRPCOMNCFG_NUM_OF_ADC_SAMPLES           NUM_ADC_SAMPLES // 256U; /* 2.56us */
#define CHIRPCOMNCFG_CHIRP_TX_MIMO_PAT_SEL        4   // 0; M_RL_SENS_TX_MIMO_PATRN_DIS
// not in .cfg file:
#define CHIRPCOMNCFG_MISC_SETTINGS                M_RL_SENS_MISC_HPF_FAST_INIT_DIS_BIT   // 0U; /* HPF FINIT, CRD ena, PA blank dis */
#define CHIRPCOMNCFG_HPF_FAST_INIT_DURATION       15U  // 15U; /* 1.5us */
#define CHIRPCOMNCFG_CHIRP_RAMP_END_TIME          30.0 // 600; 250U; /* 25us low res */
#define CHIRPCOMNCFG_CHIRP_RX_HPF_SEL             M_RL_SENS_RX_HPF_SEL_300KHZ   // M_RL_SENS_RX_HPF_SEL_350KHZ
///////////////////

/* chirpTimingCfg */
#define CHIRPTIMINGCFG_CHIRP_IDLE_TIME            6  // 400; 65U; /* 6.5us low res */
#define CHIRPTIMINGCFG_CHIRP_ADC_START_TIME       28 // 30770;
#define CHIRPTIMINGCFG_CHIRP_TX_START_TIME        0   // -10; /* -0.2us */
#define CHIRPTIMINGCFG_CHIRP_RF_FREQ_SLOPE        90 // 699; 3495; /* 100MHz/us , 77G - 2621 */
#define CHIRPTIMINGCFG_CHIRP_RF_FREQ_START        M_RL_SENS_CHIRP_RFFREQ_LR_59G
// not in .cfg file:
#define CHIRPTIMINGCFG_CHIRP_TX_EN_SEL            0x3U // 2 TX enable in chirp
#define CHIRPTIMINGCFG_CHIRP_TX_BPM_EN_SEL        0x3U // 0; 0x2U; /* TX1 BPM enable in chirp */
////////////////////
#define NUM_CHIRPS_ACCUM                          0
#define BURST_PERIODICITY                         403
#define FRAME_PERIODICITY                         250
#define NUM_FRAMES                                0

/* sensorStart */
#define SENSOR_START_FRAME_TRIG_MODE              0
#define SENSOR_START_CHIRP_START_SIG_LB_ENABLE    0
#define SENSOR_START_FRAME_LIVE_MON_ENABLE        0
#define SENSOR_START_FRAME_TRIG_TIMER_VAL         0


// derived from common/syscommon.h
/*************************************************************
 * MMWAVE System level defines
 *************************************************************/
#define SYS_COMMON_NUM_RX_CHANNEL                   3U
#define SYS_COMMON_CQ_MAX_CHIRP_THRESHOLD           8U

/* This is the size of the Chirp Parameters (CP) in CBUFF Units */
#define SYS_COMMON_CP_SIZE_CBUFF_UNITS              2U





// DMA channel defines
#define DMA_TRIG_SRC_CHAN_0 0
#define DMA_TRIG_SRC_CHAN_1 1
