# TI IWRL6432 mmWave Radar data streaming and capture in Python


![Work in Progress](https://img.shields.io/badge/status–work%20in%20progress-yellow)

> **Warning:** This repository is work in progress. **As of now none of the goals have been implemented.**

## Overview 
This project is a work-in-progress fork of the [TI IWRL6432 mmWave Radar Minimum Rangeproc DPU Implementation](https://github.com/95lux/ti_iwrl6432boost_dsp) repository which was created as part of a university assignment. 
Goal of this repo is to:
- **Implement SPI data streaming of IWRL6432 ADC and Radar Cube (Range-FFT) data** 
    - streamed data (format and which data) can then easily be adapted to your needs
    - more minimalistic approach than running the full mmwave demo project on the MCU
- **Implement an easy-to-use Python wrapper for receiving the data**
    - capture data
    - build your own experimental real-time DSP chain on top of it

Please note, that there already is an easy way to stream raw ADC data via SPI using the demo project. Please refer to TI's tutorial [here](https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/1023/Steps-for-Raw-ADC-Data-Streaming-in-IWRL6432.pdf).

#### **Brief overview of important source files:**


| `/minimal_rangeproc_impl/src/`                  |  |
|-----------------------|-------------|
| [`main.c`](/minimal_rangeproc_impl/src/main.c)             | Initializes hardware, configures the radar sensor, sets up DPUs, and starts FreeRTOS. |
| [`factory_cal.c`](/minimal_rangeproc_impl/src/factory_cal.c)      | Restores and applies factory calibration data from flash memory. |
| [`mem_pool.c`](/minimal_rangeproc_impl/src/mem_pool.c)        | Implements memory pool management functions and data structures. |
| [`mmwave_basic.c`](/minimal_rangeproc_impl/src/mmwave_basic.c)    | Handles mmWave sensor initialization, configuration, and control. |
| [`mmwave_control_config.c`](/minimal_rangeproc_impl/src/mmwave_control_config.c) | Configures chirp and profile settings for TI mmWave radar. |
| [`rangeproc_dpc.c`](/minimal_rangeproc_impl/src/rangeproc_dpc.c)   | Implements the Range Processing DPU (FFT, object detection, SPI transmission). |
| [`spi_transmit.c`](/minimal_rangeproc_impl/src/spi_transmit.c)   | Manages SPI transmission of radar cube and ADC data, synchronized via semaphores. |


| `/minimal_rangeproc_impl/include/`           |  |
|--------------|-------------|
| [`system.h`](./minimal_rangeproc_impl/include/system.h)  | Holds most global handles and configs. |
| [`defines.h`](./minimal_rangeproc_impl/include/defines.h)  | Defines chirp parameters (antenna settings, chirp configurations, timing). Configurations can be generated using the [mmWave Sensing Estimator](https://dev.ti.com/gallery/view/mmwave/mmWaveSensingEstimator/ver/2.4.0/) and the [chirp_config_to_defines.py](/scripts/chirp_config_to_defines.py) script. |
