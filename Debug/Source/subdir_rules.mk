################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Source/%.obj: ../Source/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"/home/h2l/Downloads/ccstudio/src/installdir/opt/ccstudio/ccs/tools/compiler/ti-cgt-arm_20.2.0.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me -O2 --opt_for_speed=4 --fp_mode=relaxed --include_path="/home/h2l/Documents/Semesterarbeit/Oszilloscope_project" --include_path="/opt/tivaware" --include_path="/home/h2l/Downloads/ccstudio/src/installdir/opt/ccstudio/ccs/tools/compiler/ti-cgt-arm_20.2.0.LTS/include" --define=ccs="ccs" --define=PART_TM4C123GH6PM -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="Source/$(basename $(<F)).d_raw" --obj_directory="Source" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


