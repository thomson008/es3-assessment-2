################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
LD_SRCS += \
../src/src/lscript.ld 

C_SRCS += \
../src/src/gpio_init.c \
../src/src/interrupt_handler.c \
../src/src/main.c \
../src/src/platform.c \
../src/src/seg7_display.c \
../src/src/xinterruptES3.c 

OBJS += \
./src/src/gpio_init.o \
./src/src/interrupt_handler.o \
./src/src/main.o \
./src/src/platform.o \
./src/src/seg7_display.o \
./src/src/xinterruptES3.o 

C_DEPS += \
./src/src/gpio_init.d \
./src/src/interrupt_handler.d \
./src/src/main.d \
./src/src/platform.d \
./src/src/seg7_display.d \
./src/src/xinterruptES3.d 


# Each subdirectory must supply rules for building sources it contributes
src/src/%.o: ../src/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MicroBlaze gcc compiler'
	mb-gcc -Wall -O0 -g3 -c -fmessage-length=0 -MT"$@" -I../../TrafficExtra_bsp/microblaze_0/include -mlittle-endian -mcpu=v9.5 -mxl-soft-mul -Wl,--no-relax -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


