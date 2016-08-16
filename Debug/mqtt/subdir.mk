################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../mqtt/MQTTClient.c \
../mqtt/MQTTConnectClient.c \
../mqtt/MQTTConnectServer.c \
../mqtt/MQTTDeserializePublish.c \
../mqtt/MQTTFormat.c \
../mqtt/MQTTPacket.c \
../mqtt/MQTTSerializePublish.c \
../mqtt/MQTTSubscribeClient.c \
../mqtt/MQTTSubscribeServer.c \
../mqtt/MQTTUnsubscribeClient.c \
../mqtt/MQTTUnsubscribeServer.c \
../mqtt/MQTTlwip.c \
../mqtt/mqtt.c 

OBJS += \
./mqtt/MQTTClient.o \
./mqtt/MQTTConnectClient.o \
./mqtt/MQTTConnectServer.o \
./mqtt/MQTTDeserializePublish.o \
./mqtt/MQTTFormat.o \
./mqtt/MQTTPacket.o \
./mqtt/MQTTSerializePublish.o \
./mqtt/MQTTSubscribeClient.o \
./mqtt/MQTTSubscribeServer.o \
./mqtt/MQTTUnsubscribeClient.o \
./mqtt/MQTTUnsubscribeServer.o \
./mqtt/MQTTlwip.o \
./mqtt/mqtt.o 

C_DEPS += \
./mqtt/MQTTClient.d \
./mqtt/MQTTConnectClient.d \
./mqtt/MQTTConnectServer.d \
./mqtt/MQTTDeserializePublish.d \
./mqtt/MQTTFormat.d \
./mqtt/MQTTPacket.d \
./mqtt/MQTTSerializePublish.d \
./mqtt/MQTTSubscribeClient.d \
./mqtt/MQTTSubscribeServer.d \
./mqtt/MQTTUnsubscribeClient.d \
./mqtt/MQTTUnsubscribeServer.d \
./mqtt/MQTTlwip.d \
./mqtt/mqtt.d 


# Each subdirectory must supply rules for building sources it contributes
mqtt/%.o: ../mqtt/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DTRACE -DOS_USE_TRACE_SEMIHOSTING_STDOUT -DSTM32F2XX -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=25000000 -DNO_SYS=1 -I"/home/jet/work/thermo/inc" -I"/home/jet/work/thermo/system/inc/cmsis" -I"/home/jet/work/thermo/system/inc/stm32f2-stdperiph" -I"/home/jet/work/thermo/eth/inc" -I/home/jet/work/thermo/eth/inc/ipv4 -I"/home/jet/work/thermo/system/inc" -I"/home/jet/work/thermo/mqtt" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


