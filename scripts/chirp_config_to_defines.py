import argparse
import json
import time
import os
import sys

# structure defining the expected configuration parameter order for each command
CONFIG_STRUCTURE = {
    'channelCfg': ['rxChCtrlBitMask', 'txChCtrlBitMask', 'miscCtrl'],
    'chirpComnCfg': ['digOutputSampRate', 'digOutputBitsSel', 'dfeFirSel', 'numOfAdcSamples',
                     'chirpTxMimoPatSel', 'chirpRampEndTime', 'chirpRxHpfSel'],
    'chirpTimingCfg': ['chirpIdleTime', 'chirpAdcSkipSamples', 'chirpTxStartTime',
                       'chirpRfFreqSlope', 'chirpRfFreqStart'],
    'frameCfg': ['numOfChirpsInBurst', 'numOfChirpsAccum', 'burstPeriodicity', 'numOfBurstsInFrame',
                 'framePeriodicity', 'numOfFrames'],
    'factoryCalibCfg': ['saveEnable', 'restoreEnable', 'rxGain', 'txBackoff', 'flashOffset'],
    'sensorStart': ['frameTrigMode', 'chirpStartSigLbEn', 'frameLivMonEn', 'frameTrigTimerVal']
}

# name of the output file
DEFINES_HEADER_NAME= "defines.h"

def print_basic_config_info(data):
    """
    Print basic info calculated from parameters
    """

    c = 3e8  # speed of light in m/s

    # get slope (MHz/us) and ramp duration (us) from data
    slope_mhz_per_us = float(data['chirpTimingCfg']['chirpRfFreqSlope'])
    ramp_time_us     = float(data['chirpComnCfg']['chirpRampEndTime'])

    # calculate bandwidth in Hz
    slope_hz_per_s   = slope_mhz_per_us * 1e12
    ramp_time_s      = ramp_time_us * 1e-6
    bandwidth_hz     = slope_hz_per_s * ramp_time_s

    # calculate range resolution in meters
    range_res        = c / (2 * bandwidth_hz)

    msg = f"""
Some basic information on the configuration:
  - n range bins (N_bins): {int(data['chirpComnCfg']['numOfAdcSamples']) // 2}
  - bandwidth (B):         {bandwidth_hz * 1e-9} GHz
  - range resolution (ΔR): {(range_res * 100):.2f} cm
"""
    print(msg)


def parse_cfg_file(file_path):
    """
    Parse .cfg file and extract supported commands and parameters
    """
    data = {}
    with open(file_path, 'r') as f:
        for line in f:
            # remove spaces
            line = line.strip()  
            # ignore comments and empty lines
            if not line or line.startswith('%'):
                continue
            
            # split each config line into command and parameter values
            parts = line.split()
            cmd = parts[0]
            vals = parts[1:]

            # if it is a supported command extract the values
            if cmd in list(CONFIG_STRUCTURE.keys()):
                expected = CONFIG_STRUCTURE[cmd]
                # check if command config values meet the expected number of values
                if len(vals) != len(expected):
                    raise ValueError(
                        f"incorrect number of arguments for command '{cmd}': "
                        f"expected {len(expected)}, got {len(vals)}")
                # if everything is ok, write parameter values to data[]
                data[cmd] = dict(zip(expected, vals))
    return data


def parse_json_file(file_path):
    """
    Parse .json file and extract supported commands and parameters
    """
    data = {}
    # read data from json file
    with open(file_path, 'r') as f:
        cfg = json.load(f)

    # iterate over the expected config commands and parameter values
    for cmd, expected in CONFIG_STRUCTURE.items():
        # check if the command is contained in the provided json
        if cmd in cfg:
            params = cfg[cmd]

            # ensure that the json contains all expected parameter values
            if not isinstance(params, dict) or set(params.keys()) != set(expected):
                raise ValueError(
                    f"incorrect parameter keys for command '{cmd}': "
                    f"expected {expected}, got {list(params.keys())}")

            # if everything is ok, write parameter values to data[] in the order
            #   of the expected structure
            data[cmd] = {p: params[p] for p in expected}
    return data


def parse_config_file(file_path):
    """
    Parse a .cfg or .json file and return supported command data
    """
    if file_path.endswith('.cfg'):
        return parse_cfg_file(file_path)
    if file_path.endswith('.json'):
        return parse_json_file(file_path)
    raise ValueError("unsupported file format. provide .cfg or .json file.")


def generate_defines_file(data, script_name, base_input, output_path):
    """
    Generate a C header file 'defines.h' from parsed data
    """
    timestamp = time.strftime("%Y-%m-%d %H:%M:%S")

    guard_macro = os.path.splitext(DEFINES_HEADER_NAME)[0].upper() + "_H"

    # skeleton for defines.h file
    content = f"""
#ifndef {guard_macro}
#define {guard_macro}

/**
 * @file {DEFINES_HEADER_NAME}
 *
 * @brief Configuration macros for the radar system.
 *
 * This file contains the sensor front-end parameters.
 * For reference regarding the parameters, please refer to https://software-dl.ti.com/ra-processors/esd/MMWAVE-L-SDK/05_05_00_02/exports/api_guide_xwrL64xx/MOTION_AND_PRESENCE_DETECTION_DEMO.html
 *   section \"Configuration (.cfg) File Format (CLI INTERFACE)\"
 *
 * The configuration settings can be generated with the \"mmWave Sensing Estimator\" tool (https://dev.ti.com/gallery/view/mmwave/mmWaveSensingEstimator/ver/2.4.0/)
 *
 * This file was auto-generated by the script '{script_name}' from the config file '{base_input}' on {timestamp}
 */

/* frameCfg */
#define CLI_NUM_CHIRPS_PER_BURST     {data['frameCfg']['numOfChirpsInBurst']}
#define CLI_NUM_CHIRPS_ACCUM         {data['frameCfg']['numOfChirpsAccum']}
#define CLI_BURST_PERIOD             {data['frameCfg']['burstPeriodicity']}
#define CLI_W_BURST_PERIOD           (10.0 * CLI_BURST_PERIOD)   // calculated value, not contained in config
#define CLI_NUM_BURSTS_PER_FRAME     {data['frameCfg']['numOfBurstsInFrame']}
#define CLI_FRAME_PERIOD             (((float)({data['frameCfg']['framePeriodicity']}) * 40000000.0) / 1000.0)   // for reference: value {data['frameCfg']['framePeriodicity']} is from config
#define CLI_NUM_FRAMES               {data['frameCfg']['numOfFrames']}

/* chirpTimingCfg */
#define CLI_CHIRP_IDLE_TIME          (10.0 * {data['chirpTimingCfg']['chirpIdleTime']})        // for reference: value {data['chirpTimingCfg']['chirpIdleTime']} is from config
#define CLI_CHIRP_ADC_START_TIME     ({data['chirpTimingCfg']['chirpAdcSkipSamples']} << 10)        // for reference: value {data['chirpTimingCfg']['chirpAdcSkipSamples']} is from config
#define CLI_CHIRP_TX_START_TIME      (50.0 * {data['chirpTimingCfg']['chirpTxStartTime']})        // for reference: value {data['chirpTimingCfg']['chirpTxStartTime']} is from config
#define CLI_CHIRP_SLOPE              {data['chirpTimingCfg']['chirpRfFreqSlope']}
#define CLI_START_FREQ               {data['chirpTimingCfg']['chirpRfFreqStart']}
#define CLI_CHIRP_FREQ_SLOPE         ((CLI_CHIRP_SLOPE * 1048576.0) / (3 * 100 * 100))         // calculated value, not contained in config
#define CLI_CHIRP_START_FREQ         ((CLI_START_FREQ * 1000.0 * 256.0) / (300))               // calculated value, not contained in config

/* channelCfg */
#define CLI_CHA_CFG_RX_BITMASK       {data['channelCfg']['rxChCtrlBitMask']}
#define CLI_CHA_CFG_TX_BITMASK       {data['channelCfg']['txChCtrlBitMask']}
#define CLI_CHA_CFG_MISC_CTRL        {data['channelCfg']['miscCtrl']}

/* chirpComnCfg */
#define CLI_NUM_ADC_SAMPLES          {data['chirpComnCfg']['numOfAdcSamples']}
#define CLI_NUM_RBINS                (mathUtils_pow2roundup(CLI_NUM_ADC_SAMPLES) / 2)    // calculated value, not contained in config
#define CLI_DIG_OUT_SAMPLING_RATE    {data['chirpComnCfg']['digOutputSampRate']}
#define CLI_ADC_SAMPLING_RATE        (100.0 / CLI_DIG_OUT_SAMPLING_RATE)                 // calculated value, not contained in config
#define CLI_DIG_OUT_BITS_SEL         {data['chirpComnCfg']['digOutputBitsSel']}
#define CLI_DFE_FIR_SEL              {data['chirpComnCfg']['dfeFirSel']}
#define CLI_MIMO_SEL                 {data['chirpComnCfg']['chirpTxMimoPatSel']}
#define CLI_CHIRP_RAMP_END_TIME      (10.0 * {data['chirpComnCfg']['chirpRampEndTime']})       // for reference: value {data['chirpComnCfg']['chirpRampEndTime']} is from config
#define CLI_CHIRP_RX_HPF_SEL         {data['chirpComnCfg']['chirpRxHpfSel']}

/* Factory Calibration */
#define CLI_FACCALCFG_RX_GAIN        {data['factoryCalibCfg']['rxGain']}
#define CLI_FACCALCFG_TX_BACKOFF_SEL {data['factoryCalibCfg']['txBackoff']} * 2             // for reference: value {data['factoryCalibCfg']['txBackoff']} is from config
#define CLI_FACCALCFG_FLASH_OFFSET   {data['factoryCalibCfg']['flashOffset']}

/* sensorStart */
#define CLI_SENSOR_START_FRM_TRIG    {data['sensorStart']['frameTrigMode']}
#define CLI_SENSOR_START_LB_EN       {data['sensorStart']['chirpStartSigLbEn']}
#define CLI_SENSOR_START_MON_EN      {data['sensorStart']['frameLivMonEn']}
#define CLI_SENSOR_START_TRIG_TIMER  {data['sensorStart']['frameTrigTimerVal']}

/* other (derived from header generated through CLI_REMOVAL from motion and presence detection demo) */
#define CLI_C_MISC_SETTINGS          0
#define CLI_HPF_FAST_INIT_DURATION   15U

#endif /* {guard_macro} */
"""
    # write file to output path
    with open(output_path, 'w') as f:
        f.write(content)


def print_usage(script_name):
    """
    Print usage instructions / information
    """
    msg = f"""\
Description:
  This script takes config files from the MMWAVE-L-SDK or TI mmWave Sensing Estimator and generates a {DEFINES_HEADER_NAME}
  file from it. Please note that it only processes the commands and parameters which are used within the minimal
  RangeProc DPU implementation in this repo and ignores all the others. 
  TI mmWave Sensing Estimator: https://dev.ti.com/gallery/view/mmwave/mmWaveSensingEstimator/ver/2.4.1/

Usage:
  {script_name} <path to config .cfg or .json> [-o <output header file or directory>]

"""
    print(msg)


def main():
    # set up arg parser
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument('input_file', nargs='?', help="path to config file (.cfg or .json)")
    parser.add_argument('-o', '--output', help="path to output header file or directory", default=None)
    parser.add_argument('-h', '--help', action='store_true', help="show help message and exit")
    args = parser.parse_args()

    script_name = os.path.basename(sys.argv[0])

    # print usage and exit if help arg is given or input file is not provided
    if args.help or not args.input_file:
        print_usage(script_name)
        sys.exit(0)

    # read and parse config from input file and ensure file is not empty
    try:
        data = parse_config_file(args.input_file)
    except Exception as e:
        print(f"error: {e}")
        sys.exit(1)

    if not data:
        print("Provided file is empty.")
        sys.exit(1)

    # determine desired output path
    script_dir  = os.path.dirname(os.path.abspath(sys.argv[0]))
    repo_root   = os.path.dirname(script_dir)
    default_dir = os.path.join(repo_root, 'minimal_rangeproc_impl', 'include')

    # if output file arg is provided, ensure that it is an actual file and not a dir
    #   otherwise use the defined default dir
    if args.output:
        if os.path.isdir(args.output):
            # if a dir path is provided as output file arg, change it to defines.h in provided dir
            desired = os.path.join(args.output, DEFINES_HEADER_NAME)
        else:
            desired = args.output
    else:
        desired = os.path.join(default_dir, DEFINES_HEADER_NAME)

    # verify directory exists
    dir_path = os.path.dirname(desired)
    if not os.path.isdir(dir_path):
        final = os.path.join(os.getcwd(), DEFINES_HEADER_NAME)
        print(f"directory '{dir_path}' not found, saving to working directory: {final}")
    else:
        final = desired
        if os.path.exists(final):
            resp = input(f"file '{final}' exists. overwrite? [y/N]: ")
            if resp.lower() != 'y':
                sys.exit(1)

    # generate output file
    generate_defines_file(data, script_name, os.path.basename(args.input_file), final)
    print(f"generated header: {final}")

    # output some basic info about the config
    print_basic_config_info(data)


if __name__ == '__main__':
    main()
