################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/test/CodeCorrectness.c \
../src/test/RandomGenerator.c \
../src/test/SingleThread.c \
../src/test/TestAtomicStampedRef.c \
../src/test/TestStackArray.c \
../src/test/TestUtils.c \
../src/test/testCircularQueue.c \
../src/test/testWaitFreePool.c 

OBJS += \
./src/test/CodeCorrectness.o \
./src/test/RandomGenerator.o \
./src/test/SingleThread.o \
./src/test/TestAtomicStampedRef.o \
./src/test/TestStackArray.o \
./src/test/TestUtils.o \
./src/test/testCircularQueue.o \
./src/test/testWaitFreePool.o 

C_DEPS += \
./src/test/CodeCorrectness.d \
./src/test/RandomGenerator.d \
./src/test/SingleThread.d \
./src/test/TestAtomicStampedRef.d \
./src/test/TestStackArray.d \
./src/test/TestUtils.d \
./src/test/testCircularQueue.d \
./src/test/testWaitFreePool.d 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -DLOGGING_LEVEL=LOG_LEVEL_NONE -O3 -Wall -c -fmessage-length=0 -std=c11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


