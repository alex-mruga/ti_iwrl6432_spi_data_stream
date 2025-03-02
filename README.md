# TI IWRL6432 mmWave Radar

## Overview  

This project implements a bare-minimum digital signal processing pipeline for mmWave radar data using the Texas Instruments [IWRL6432BOOST](https://www.ti.com/tool/IWRL6432BOOST) evaluation module. It is based on TI’s [Motion and Presence Detection Demo](https://dev.ti.com/tirex/explore/node?node=A__AGKSp6XJSIVonQK9nNyYLg__MMWAVE-L-SDK__BHQ90AU__LATEST) but strips away unnecessary components, using only essential SDK functions to achieve a working FMCW setup.  

The project utilizes **MMWAVE-L-SDK version 05.05.03.00**. Download [here](https://www.ti.com/tool/download/MMWAVE-L-SDK).  

### Key Features  

- **Range Measurement via Range-FFT**  
  - Extracts range bins from one antenna and one chirp.  
  - Data is streamed via UART to a host application for visualization.  

- **Minimal Standalone Implementation**  
  - No CLI-based reconfiguration, all parameters set in `defines.h`.  
  - Only includes necessary SDK function calls for FMCW processing.  

- **Embedded System Considerations**  
  - Factory calibration data restored from flash.  
  - Task management using FreeRTOS with semaphore synchronization. 

## Project Structure

```
/person_detecton_sdk-05_05_03
├── src/                    # Source files
├── include/                # Header files
├── doxygen/                # Doxygen documentation
├── example.syscfg          # configuration file for configuring the MCU drivers
/docs                       # project report in LateX         
/uart_range_plotter 
├── uart_range_plotter.py   # python script to visualize sent range radar cube data
```

### Project files

Detailed documentation can be found in form of Doxygen comments in the source files, as well as in exported form in the `doxygen/` directory of the repository.

#### **Brief overview of important source files:**


| `/person_detection_sdk-05_05_03/src/`                  |  |
|-----------------------|-------------|
| [`main.c`](/person_detection_sdk-05_05_03/src/main.c)             | Initializes hardware, configures the radar sensor, sets up DPUs, and starts FreeRTOS. |
| [`factory_cal.c`](/person_detection_sdk-05_05_03/src/factory_cal.c)      | Restores and applies factory calibration data from flash memory. |
| [`mem_pool.c`](/person_detection_sdk-05_05_03/src/mem_pool.c)        | Implements memory pool management functions and data structures. |
| [`mmwave_basic.c`](/person_detection_sdk-05_05_03/src/mmwave_basic.c)    | Handles mmWave sensor initialization, configuration, and control. |
| [`mmwave_control_config.c`](/person_detection_sdk-05_05_03/src/mmwave_control_config.c) | Configures chirp and profile settings for TI mmWave radar. |
| [`rangeproc_dpc.c`](/person_detection_sdk-05_05_03/src/rangeproc_dpc.c)   | Implements the Range Processing DPU (FFT, object detection, UART transmission). |
| [`uart_transmit.c`](/person_detection_sdk-05_05_03/src/uart_transmit.c)   | Manages UART transmission of radar cube data, synchronized via semaphores. |


| `/person_detection_sdk-05_05_03/include/`           |  |
|--------------|-------------|
| [`defines.h`](./person_detection_sdk-05_05_03/include/defines.h)  | Defines system parameters (antenna settings, chirp configurations, timing). Configurations can be generated using the [mmWave Sensing Estimator](https://dev.ti.com/gallery/view/mmwave/mmWaveSensingEstimator/ver/2.4.0/). |
