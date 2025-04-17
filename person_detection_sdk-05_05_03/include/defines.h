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
#define CLI_NUM_RBINS                   (mathUtils_pow2roundup(CLI_NUM_ADC_SAMPLES) / 2) // calculated value, not contained in cfg/json
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