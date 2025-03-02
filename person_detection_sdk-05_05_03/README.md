# TI IWRL6432BOOST mmWave minimum

## Description

This project implements the bare-minimum digital signal processing on mmWave radar data using the Texas Instruments [IWRL6432BOOST](https://www.ti.com/tool/IWRL6432BOOST) evaluation module.

The project utilizes MMWAVE-L-SDK version 05.05.03.00. Download [here](https://www.ti.com/tool/download/MMWAVE-L-SDK).

## Features

It implements range measurements, utilizing a Range-FFT and sends the resulting data via UART to a Host application.


## Project Structure

```
/repo
├── src/            # Source files
├── indlude/        # Header files
├── doxygen/        # Doxygen documentation
├── example.syscfg  # configuration file for configuring the MCU drivers
```

### Project files

Detailed documentation can be found in form of Doxygen comments in the source files, as well as in exported form in the `doxygen/` directory of the repository.

**Brief overview of the project files:**

`main.c` – Initializes hardware, configures the radar sensor, sets up Data Processing Units (DPUs), and starts FreeRTOS for radar processing and UART communication.

`factory_cal.c` – Restores and applies factory calibration data by reading from flash memory and configuring the radar front-end (FECSS) to ensure optimal performance.

`mem_pool.c` – Implements memory pool management functions and data structures.

`mmwave_basic.c` – Handles mmWave sensor initialization, configuration, and control, including memory pool setup and HWA initialization.

`mmwave_control_config.c` – Configures chirp and profile settings for TI mmWave radar.

`rangeproc_dpc.c` - implements the Range Processing DPU for radar signal processing, including FFT, object detection, and UART transmission, using the TI mmWave SDK and HWA.

`uart_transmit.c` – Implements UART transmission for radar cube data, managing synchronization via semaphores and enabling communication between the radar processing unit and external systems.