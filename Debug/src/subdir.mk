################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AtomicStampedRef.c \
../src/AtomicStampedReference.c \
../src/Block.c \
../src/Chunk.c \
../src/CircularQueue.c \
../src/FreePool.c \
../src/FullPool.c \
../src/HazardPointer.c \
../src/LocalPool.c \
../src/Queue.c \
../src/QueuePool.c \
../src/SharedPools.c \
../src/Stack.c \
../src/StackArray.c \
../src/StackPool.c \
../src/WaitFreePool.c \
../src/utils.c 

OBJS += \
./src/AtomicStampedRef.o \
./src/AtomicStampedReference.o \
./src/Block.o \
./src/Chunk.o \
./src/CircularQueue.o \
./src/FreePool.o \
./src/FullPool.o \
./src/HazardPointer.o \
./src/LocalPool.o \
./src/Queue.o \
./src/QueuePool.o \
./src/SharedPools.o \
./src/Stack.o \
./src/StackArray.o \
./src/StackPool.o \
./src/WaitFreePool.o \
./src/utils.o 

C_DEPS += \
./src/AtomicStampedRef.d \
./src/AtomicStampedReference.d \
./src/Block.d \
./src/Chunk.d \
./src/CircularQueue.d \
./src/FreePool.d \
./src/FullPool.d \
./src/HazardPointer.d \
./src/LocalPool.d \
./src/Queue.d \
./src/QueuePool.d \
./src/SharedPools.d \
./src/Stack.d \
./src/StackArray.d \
./src/StackPool.d \
./src/WaitFreePool.d \
./src/utils.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/Users/architaagarwal/hpc-gcc/lib/gcc/x86_64-apple-darwin14.0.0/4.9.2/include -I"/Users/architaagarwal/Documents/workspace/Logger" -I/Users/architaagarwal/hpc-gcc/include/c++/4.9.2 -O3 -g3 -Wall -Wextra -Werror -c -fmessage-length=0 -std=c11 -pg -O3 -finline-limit=20000 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


