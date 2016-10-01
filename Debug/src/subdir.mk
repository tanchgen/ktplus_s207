################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/buffer.c \
../src/can.c \
../src/fmt_translate.c \
../src/main.c \
../src/mqtt.c \
../src/mqttApp.c \
../src/mqtt_codec.c \
../src/my_cli.c \
../src/stm32f2xx_it.c \
../src/time.c 

OBJS += \
./src/buffer.o \
./src/can.o \
./src/fmt_translate.o \
./src/main.o \
./src/mqtt.o \
./src/mqttApp.o \
./src/mqtt_codec.o \
./src/my_cli.o \
./src/stm32f2xx_it.o \
./src/time.o 

C_DEPS += \
./src/buffer.d \
./src/can.d \
./src/fmt_translate.d \
./src/main.d \
./src/mqtt.d \
./src/mqttApp.d \
./src/mqtt_codec.d \
./src/my_cli.d \
./src/stm32f2xx_it.d \
./src/time.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DTRACE -DOS_USE_TRACE_SEMIHOSTING_STDOUT -DSTM32F2XX -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=25000000 -DRTL8201=1 -DCAN_TEST=0 -I"/home/jet/work/thermo/inc" -I"/home/jet/work/thermo/system/inc/cmsis" -I"/home/jet/work/thermo/system/inc/stm32f2-stdperiph" -I"/home/jet/work/thermo/eth/inc" -I/home/jet/work/thermo/eth/inc/ipv4 -I"/home/jet/work/thermo/system/inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


