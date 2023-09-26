################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Logger/logger.c 

OBJS += \
./Logger/logger.o 

C_DEPS += \
./Logger/logger.d 


# Each subdirectory must supply rules for building sources it contributes
Logger/%.o Logger/%.su: ../Logger/%.c Logger/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -I"/home/lefucjusz/STM32CubeIDE/workspace_1.11.2/mewa_playground/Bsp/tca9548a" -I"/home/lefucjusz/STM32CubeIDE/workspace_1.11.2/mewa_playground/Bsp/ssd1306" -I"/home/lefucjusz/STM32CubeIDE/workspace_1.11.2/mewa_playground/Logger" -I"/home/lefucjusz/STM32CubeIDE/workspace_1.11.2/mewa_playground/Bsp/cs4270" -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Logger

clean-Logger:
	-$(RM) ./Logger/logger.d ./Logger/logger.o ./Logger/logger.su

.PHONY: clean-Logger

