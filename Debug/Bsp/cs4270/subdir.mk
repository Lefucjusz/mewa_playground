################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bsp/cs4270/cs4270.c 

OBJS += \
./Bsp/cs4270/cs4270.o 

C_DEPS += \
./Bsp/cs4270/cs4270.d 


# Each subdirectory must supply rules for building sources it contributes
Bsp/cs4270/%.o Bsp/cs4270/%.su: ../Bsp/cs4270/%.c Bsp/cs4270/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Bsp-2f-cs4270

clean-Bsp-2f-cs4270:
	-$(RM) ./Bsp/cs4270/cs4270.d ./Bsp/cs4270/cs4270.o ./Bsp/cs4270/cs4270.su

.PHONY: clean-Bsp-2f-cs4270

