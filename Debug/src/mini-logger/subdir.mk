################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/mini-logger/logger.c \
../src/mini-logger/main.c 

OBJS += \
./src/mini-logger/logger.o \
./src/mini-logger/main.o 

C_DEPS += \
./src/mini-logger/logger.d \
./src/mini-logger/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/mini-logger/%.o: ../src/mini-logger/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -O0 -g3 -Wall -c -fmessage-length=0 -std=c11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


