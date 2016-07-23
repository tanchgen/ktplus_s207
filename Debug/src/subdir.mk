################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/_write.c \
../src/buffer.c \
../src/main.c \
../src/my_cli.c \
../src/stm32f2xx_it.c 

OBJS += \
./src/_write.o \
./src/buffer.o \
./src/main.o \
./src/my_cli.o \
./src/stm32f2xx_it.o 

C_DEPS += \
./src/_write.d \
./src/buffer.d \
./src/main.d \
./src/my_cli.d \
./src/stm32f2xx_it.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants -Wall -Wextra  -g3 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_SEMIHOSTING_STDOUT -DSTM32F2XX -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=25000000 -DDEBUG -DUSE_FULL_ASSERT -DTRACE -DOS_USE_TRACE_SEMIHOSTING_STDOUT -DSTM32F2XX -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=25000000 -I"/home/jet/work/thermo/inc" -I"/home/jet/work/thermo/system/inc/cmsis" -I"/home/jet/work/thermo/system/inc/stm32f2-stdperiph" -I"/home/jet/work/thermo/eth/inc" -I/home/jet/work/thermo/eth/inc/ipv4 -I"/home/jet/work/thermo/system/inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


