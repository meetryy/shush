################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../shush/src/eeprom.c \
../shush/src/gps.c \
../shush/src/rtc.c \
../shush/src/schedule.c \
../shush/src/tools.c \
../shush/src/transmitter.c \
../shush/src/wspr.c 

OBJS += \
./shush/src/eeprom.o \
./shush/src/gps.o \
./shush/src/rtc.o \
./shush/src/schedule.o \
./shush/src/tools.o \
./shush/src/transmitter.o \
./shush/src/wspr.o 

C_DEPS += \
./shush/src/eeprom.d \
./shush/src/gps.d \
./shush/src/rtc.d \
./shush/src/schedule.d \
./shush/src/tools.d \
./shush/src/transmitter.d \
./shush/src/wspr.d 


# Each subdirectory must supply rules for building sources it contributes
shush/src/%.o shush/src/%.su: ../shush/src/%.c shush/src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Core/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -I"D:/github/shush/cubeide/shush/drivers" -I"D:/github/shush/cubeide/shush/inc" -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/DFU/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-shush-2f-src

clean-shush-2f-src:
	-$(RM) ./shush/src/eeprom.d ./shush/src/eeprom.o ./shush/src/eeprom.su ./shush/src/gps.d ./shush/src/gps.o ./shush/src/gps.su ./shush/src/rtc.d ./shush/src/rtc.o ./shush/src/rtc.su ./shush/src/schedule.d ./shush/src/schedule.o ./shush/src/schedule.su ./shush/src/tools.d ./shush/src/tools.o ./shush/src/tools.su ./shush/src/transmitter.d ./shush/src/transmitter.o ./shush/src/transmitter.su ./shush/src/wspr.d ./shush/src/wspr.o ./shush/src/wspr.su

.PHONY: clean-shush-2f-src

