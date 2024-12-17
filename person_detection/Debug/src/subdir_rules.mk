################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"C:/ti/ti_cgt_tiarmclang_3.2.2.LTS/bin/tiarmclang.exe" -c -mcpu=cortex-m4 -mfloat-abi=hard -mlittle-endian -mthumb -I"C:/ti/ti_cgt_tiarmclang_3.2.2.LTS/include/c" -I"C:/Users/jonas/repos/tin/ti_iwrl6432boost_dsp/person_detection/include" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/source" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/source/kernel/freertos/FreeRTOS-Kernel/include" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/source/kernel/freertos/portable/TI_ARM_CLANG/ARM_CM4F" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/source/kernel/freertos/config/xwrL64xx/m4f" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/firmware/mmwave_dfp" -I"C:/ti/MMWAVE_L_SDK_05_05_02_00/firmware/mmwave_dfp/mmwavelink" -DSOC_XWRL64XX -D_DEBUG_=1 -g -Wall -Wno-gnu-variable-sized-type-not-at-end -Wno-unused-function -mno-unaligned-access -MMD -MP -MF"src/$(basename $(<F)).d_raw" -MT"$(@)" -I"C:/Users/jonas/repos/tin/ti_iwrl6432boost_dsp/person_detection/Debug/syscfg"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


