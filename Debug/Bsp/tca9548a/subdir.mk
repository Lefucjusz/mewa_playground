################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bsp/tca9548a/tca9548a.c 

OBJS += \
./Bsp/tca9548a/tca9548a.o 

C_DEPS += \
./Bsp/tca9548a/tca9548a.d 


# Each subdirectory must supply rules for building sources it contributes
Bsp/tca9548a/%.o Bsp/tca9548a/%.su: ../Bsp/tca9548a/%.c Bsp/tca9548a/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Bsp-2f-tca9548a

clean-Bsp-2f-tca9548a:
	-$(RM) ./Bsp/tca9548a/tca9548a.d ./Bsp/tca9548a/tca9548a.o ./Bsp/tca9548a/tca9548a.su

.PHONY: clean-Bsp-2f-tca9548a

