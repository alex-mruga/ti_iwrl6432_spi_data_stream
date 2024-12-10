# Project Setup and Configuration

## 1. Theia Setup
- When installing Theia, select the following options:
  - mmWave
  - Cortex M4 (ARM)

## 2. Install mmWave SDK
- Ensure the mmWave SDK is installed.

## 3. Use Resource Explorer `hello_world` RTOS as the Basis
- **Note**: The NORTOS project leads to a crash. A forum entry has been created regarding the issue:
  [Unable to run hello_world_nortos example - Missing C files](https://e2e.ti.com/support/sensors-group/sensors/f/sensors-forum/1448966/iwrl6432boost-unable-to-run-hello_world_nortos-example-missing-c-files)

## 4. Include `.libs` in the Linker
- Right-click on the project -> Properties -> ARM Linker
- Specify the paths to mmWave SDK and `.lib` file names.

## 5. Enable HWA in SysConfig
- HWA must be enabled in SysConfig.
- Default values can be used.
