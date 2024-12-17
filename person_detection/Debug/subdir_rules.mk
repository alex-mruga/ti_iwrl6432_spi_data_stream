################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
build-1884568399: ../example.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/sysconfig_1.20.0/sysconfig_cli.bat" --script "C:/Users/jonas/repos/tin/ti_iwrl6432boost_dsp/person_detection/example.syscfg" -o "syscfg" -s "C:/ti/MMWAVE_L_SDK_05_05_02_00/.metadata/product.json" --context "m4fss0-0" --part Default --package FCCSP --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

syscfg/ti_dpl_config.c: build-1884568399 ../example.syscfg
syscfg/ti_dpl_config.h: build-1884568399
syscfg/ti_drivers_config.c: build-1884568399
syscfg/ti_drivers_config.h: build-1884568399
syscfg/ti_drivers_open_close.c: build-1884568399
syscfg/ti_drivers_open_close.h: build-1884568399
syscfg/ti_pinmux_config.c: build-1884568399
syscfg/ti_power_clock_config.c: build-1884568399
syscfg/ti_board_config.c: build-1884568399
syscfg/ti_board_config.h: build-1884568399
syscfg/ti_board_open_close.c: build-1884568399
syscfg/ti_board_open_close.h: build-1884568399
syscfg/ti_cli_mpd_demo_config.h: build-1884568399
syscfg/ti_cli_mmwave_demo_config.h: build-1884568399
syscfg: build-1884568399

syscfg/%.o: ./syscfg/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ti_cgt_tiarmclang_3.2.2.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mlittle-endian -mthumb -I"C:/ti/ti_cgt_tiarmclang_3.2.2.LTS/include/c" -I"C:/Users/jonas/repos/tin/ti_iwrl6432boost_dsp/person_detection/include" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/source" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CM4F" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/source/kernel/freertos/config/xwrL64xx/m4f" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/firmware/mmwave_dfp" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/firmware/mmwave_dfp/mmwavelink" -DSOC_XWRL64XX -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -mno-unaligned-access -MMD -MP -MF"syscfg/$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/jonas/repos/tin/ti_iwrl6432boost_dsp/person_detection/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


