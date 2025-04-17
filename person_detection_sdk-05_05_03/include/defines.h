/**
*
* @file defines.h
*
* @brief Configuration macros for the radar system.
*
* This file contains macro definitions for antenna settings, chirp configurations, timing parameters, and system-level settings.
*
* The configuration settings can be generated with the "mmWave Sensing Estimator" tool (https://dev.ti.com/gallery/view/mmwave/mmWaveSensingEstimator/ver/2.4.0/) 
*
*/

/*************************************************************
 * MMWAVE System level defines
 *************************************************************/
// #define SYS_COMMON_NUM_RX_CHANNEL                   3U
// #define SYS_COMMON_CQ_MAX_CHIRP_THRESHOLD           8U

// /* This is the size of the Chirp Parameters (CP) in CBUFF Units */
// #define SYS_COMMON_CP_SIZE_CBUFF_UNITS              2U


/*************************************************************
 * MMWAVE Config
 *************************************************************/
#define CLI_NUM_CHIRPS_PER_BURST 8
#define CLI_NUM_CHIRPS_ACCUM 0
#define CLI_BURST_PERIOD 643
#define CLI_W_BURST_PERIOD (10.0 * CLI_BURST_PERIOD) // calculated value, not contained in cfg/json
#define CLI_NUM_BURSTS_PER_FRAME 1
#define CLI_FRAME_PERIOD (((float)(250.0) * 40000000.0)/1000.0) // for reference: 250.0 is the value from the cfg/json
#define CLI_NUM_FRAMES 0

/* chirpTimingCfg */
#define CLI_CHIRP_IDLE_TIME (10.0 * 6) // for reference: 6 is the value from the cfg/json
#define CLI_CHIRP_ADC_START_TIME (28 << 10) // for reference: 28 is the value from the cfg/json
#define CLI_CHIRP_TX_START_TIME (50.0 * 0) // for reference: 0 is the value from the cfg/json
#define CLI_CHIRP_SLOPE 65

#define CLI_START_FREQ              59.75
#define CLI_CHIRP_FREQ_SLOPE        ((CLI_CHIRP_SLOPE * 1048576.0)/(3* 100 * 100)) // calculated value, not contained in cfg/json
#define CLI_CHIRP_START_FREQ        ((CLI_START_FREQ * 1000.0 * 256.0)/(300)) // calculated value, not contained in cfg/json

/* channelCfg */
#define CLI_CHA_CFG_RX_BITMASK          7
#define CLI_CHA_CFG_TX_BITMASK          3
#define CLI_CHA_CFG_MISC_CTRL           0

/* chirpComnCfg */
#define CLI_NUM_ADC_SAMPLES             256
#define CLI_NUM_RBINS                   (mathUtils_pow2roundup(CLI_NUM_ADC_SAMPLES)/2) // calculated value, not contained in cfg/json
#define CLI_DIG_OUT_SAMPLING_RATE       20
#define CLI_ADC_SAMPLING_RATE           (100.0 / CLI_DIG_OUT_SAMPLING_RATE) // calculated value, not contained in cfg/json
#define CLI_DIG_OUT_BITS_SEL            0
#define CLI_DFE_FIR_SEL                 0
#define CLI_MIMO_SEL                    4
#define CLI_CHIRP_RAMP_END_TIME         (10.0 * 60) // for reference: 60 is the value from the cfg/json
#define CLI_CHIRP_RX_HPF_SEL            0

#define CLI_C_MISC_SETTINGS             0
#define CLI_HPF_FAST_INIT_DURATION      15U

/* Factory Calibration */
#define CLI_FACCALCFG_RX_GAIN           40
#define CLI_FACCALCFG_TX_BACKOFF_SEL    0 * 2
#define CLI_FACCALCFG_FLASH_OFFSET      0x1FF000

/* sensorStart */
#define CLI_SENSOR_START_FRM_TRIG       0
#define CLI_SENSOR_START_LB_EN          0
#define CLI_SENSOR_START_MON_EN         0
#define CLI_SENSOR_START_TRIG_TIMER     0


#define CHIRPTIMINGCFG_CHIRP_TX_BPM_EN_SEL        0x0U // MIMO BPM enable (hardcoded to 0 in demo project)


#define NUM_TX_ANTENNAS 2
#define NUM_RX_ANTENNAS 3
#define NUM_VIRT_ANTENNAS (NUM_TX_ANTENNAS * NUM_RX_ANTENNAS)
#define NUM_CHIRPS_PER_FRAME (CLI_NUM_BURSTS_PER_FRAME * CLI_NUM_CHIRPS_PER_BURST)
#define NUM_DOPPLER_CHIRPS_PER_FRAME (NUM_CHIRPS_PER_FRAME / NUM_TX_ANTENNAS)
#define NUM_DOPPLER_CHIRPS_PER_PROC NUM_DOPPLER_CHIRPS_PER_FRAME

// #define LOW_POWER_MODE 2

/* basic configuration (frameCfg and others)*/

// #define NUM_ADC_SAMPLES 256 // 128 // number of adc samples per chirp hardcoded in https://dev.ti.com/gallery/view/mmwave/mmWaveSensingEstimator/ver/2.4.0/ and also used in MOTION_AND_PRESENCE_DETECTION_DEMO
// #define NUM_BURSTS_PER_FRAME 1 // from MOTION_AND_PRESENCE_DETECTION_DEMO
// #define NUM_CHIRPS_PER_BURST 8 // from MOTION_AND_PRESENCE_DETECTION_DEMO


// channelCfg
// #define RX_CH_CTRL_BITMASK 7 // all 3 RX antennas active => 7 (0b111)
// #define TX_CH_CTRL_BITMASK 3 // all 2 TX antennas active => 3 (0b11)
// #define CHANNEL_CFG_MISC_CTRL 0

// calculated defines, not in config
// #define NUM_RANGE_BINS (NUM_ADC_SAMPLES / 2)

///////////////////

/* chirpComnCfg */
// #define CHIRPCOMNCFG_DIG_OUTPUT_SAMP_RATE         20  // 5 MHz // M_RL_SENS_DIG_OUT_SAMP_RATE_MAX_12P5M
// #define CHIRPCOMNCFG_DIG_OUTPUT_BITS_SEL          0   // M_RL_SENS_DIG_OUT_12BITS_4LSB_ROUND
// #define CHIRPCOMNCFG_DFE_FIR_SEL                  0   // M_RL_SENS_DFE_FIR_LONG_FILT
// #define CHIRPCOMNCFG_NUM_OF_ADC_SAMPLES           NUM_ADC_SAMPLES // 256U; /* 2.56us */
// #define CHIRPCOMNCFG_CHIRP_TX_MIMO_PAT_SEL        4   // 0; M_RL_SENS_TX_MIMO_PATRN_DIS
// not in .cfg file:
// #define CHIRPCOMNCFG_MISC_SETTINGS                M_RL_SENS_MISC_HPF_FAST_INIT_DIS_BIT   // 0U; /* HPF FINIT, CRD ena, PA blank dis */
// #define CHIRPCOMNCFG_HPF_FAST_INIT_DURATION       15U  // 15U; /* 1.5us */
// #define CHIRPCOMNCFG_CHIRP_RAMP_END_TIME          60.0 //30.0 // 600; 250U; /* 25us low res */
// #define CHIRPCOMNCFG_CHIRP_RX_HPF_SEL             M_RL_SENS_RX_HPF_SEL_300KHZ   // M_RL_SENS_RX_HPF_SEL_350KHZ
///////////////////

/* chirpTimingCfg */
// #define CHIRPTIMINGCFG_CHIRP_IDLE_TIME            6  // 400; 65U; /* 6.5us low res */
// #define CHIRPTIMINGCFG_CHIRP_ADC_START_TIME       (28 << 10)// 30770;
// #define CHIRPTIMINGCFG_CHIRP_TX_START_TIME        0   // -10; /* -0.2us */
// #define CHIRPTIMINGCFG_CHIRP_RF_FREQ_SLOPE        65 // 699; 3495; /* 100MHz/us , 77G - 2621 */
// #define CHIRPTIMINGCFG_CHIRP_RF_FREQ_START        59.75 // calculation from defines (M_RL_SENS_CHIRP_RFFREQ_LR_59G) are unclear, so use fix value 
// not in .cfg file:
// #define CHIRPTIMINGCFG_CHIRP_TX_EN_SEL            0x3U // 2 TX enable in chirp

////////////////////
//#define NUM_CHIRPS_ACCUM                          0
// #define BURST_PERIOD                              643 // 403
// #define W_BURST_PERIOD                            (10.0 * BURST_PERIOD)
// #define FRAME_PERIOD                              (((float)(250.0) * 40000000.0)/1000.0) 
// #define NUM_FRAMES                                0

/* sensorStart */
// #define SENSOR_START_FRAME_TRIG_MODE              0
// #define SENSOR_START_CHIRP_START_SIG_LB_ENABLE    0
// #define SENSOR_START_FRAME_LIVE_MON_ENABLE        0
// #define SENSOR_START_FRAME_TRIG_TIMER_VAL         0

/* Factory Calibration */
// #define CLI_FACCALCFG_RES_EN 0
// #define CLI_FACCALCFG_RX_GAIN 40
// #define CLI_FACCALCFG_TX_BACKOFF_SEL 0
// #define CLI_FACCALCFG_FLASH_OFFSET 0x1FF000

// derived from common/syscommon.h





