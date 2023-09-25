################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Bsp/ssd1306/ssd1306.c \
../Bsp/ssd1306/ssd1306_fonts.c 

OBJS += \
./Bsp/ssd1306/ssd1306.o \
./Bsp/ssd1306/ssd1306_fonts.o 

C_DEPS += \
./Bsp/ssd1306/ssd1306.d \
./Bsp/ssd1306/ssd1306_fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Bsp/ssd1306/%.o Bsp/ssd1306/%.su: ../Bsp/ssd1306/%.c Bsp/ssd1306/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx -c -I../Core/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc -I../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Bsp-2f-ssd1306

clean-Bsp-2f-ssd1306:
	-$(RM) ./Bsp/ssd1306/ssd1306.d ./Bsp/ssd1306/ssd1306.o ./Bsp/ssd1306/ssd1306.su ./Bsp/ssd1306/ssd1306_fonts.d ./Bsp/ssd1306/ssd1306_fonts.o ./Bsp/ssd1306/ssd1306_fonts.su

.PHONY: clean-Bsp-2f-ssd1306

