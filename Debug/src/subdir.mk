################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AtomicStampedReference.c \
../src/Block.c \
../src/Chunk.c \
../src/CircularQueue.c \
../src/Client.c \
../src/ClientHP.c \
../src/ClientPC.c \
../src/FreePool.c \
../src/FullPool.c \
../src/HazardPointer.c \
../src/LocalPool.c \
../src/Queue.c \
../src/QueuePool.c \
../src/RandomGenerator.c \
../src/SharedPools.c \
../src/Stack.c \
../src/StackArray.c \
../src/StackPool.c \
../src/WaitFreePool.c \
../src/test.c \
../src/testMulti.c 

OBJS += \
./src/AtomicStampedReference.o \
./src/Block.o \
./src/Chunk.o \
./src/CircularQueue.o \
./src/Client.o \
./src/ClientHP.o \
./src/ClientPC.o \
./src/FreePool.o \
./src/FullPool.o \
./src/HazardPointer.o \
./src/LocalPool.o \
./src/Queue.o \
./src/QueuePool.o \
./src/RandomGenerator.o \
./src/SharedPools.o \
./src/Stack.o \
./src/StackArray.o \
./src/StackPool.o \
./src/WaitFreePool.o \
./src/test.o \
./src/testMulti.o 

C_DEPS += \
./src/AtomicStampedReference.d \
./src/Block.d \
./src/Chunk.d \
./src/CircularQueue.d \
./src/Client.d \
./src/ClientHP.d \
./src/ClientPC.d \
./src/FreePool.d \
./src/FullPool.d \
./src/HazardPointer.d \
./src/LocalPool.d \
./src/Queue.d \
./src/QueuePool.d \
./src/RandomGenerator.d \
./src/SharedPools.d \
./src/Stack.d \
./src/StackArray.d \
./src/StackPool.d \
./src/WaitFreePool.d \
./src/test.d \
./src/testMulti.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -O0 -g3 -Wall -c -fmessage-length=0 -std=c11 -v -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


