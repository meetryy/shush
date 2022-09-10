################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../shush/drivers/si5351.c 

OBJS += \
./shush/drivers/si5351.o 

C_DEPS += \
./shush/drivers/si5351.d 


# Each subdirectory must supply rules for building sources it contributes
shush/drivers/%.o shush/drivers/%.su: ../shush/drivers/%.c shush/drivers/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/github/shush/cubeide/shush/drivers" -I"D:/github/shush/cubeide/shush/inc" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-shush-2f-drivers

clean-shush-2f-drivers:
	-$(RM) ./shush/drivers/si5351.d ./shush/drivers/si5351.o ./shush/drivers/si5351.su

.PHONY: clean-shush-2f-drivers

