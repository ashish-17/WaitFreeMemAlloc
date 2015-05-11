################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/AtomicStampedReference.c \
../src/Block.c \
../src/Chunk.c \
../src/FreePool.c \
../src/FullPool.c \
../src/LocalPool.c \
../src/Stack.c \
../src/WaitFreePool.c \
../src/pool.c \
../src/test.c 

OBJS += \
./src/AtomicStampedReference.o \
./src/Block.o \
./src/Chunk.o \
./src/FreePool.o \
./src/FullPool.o \
./src/LocalPool.o \
./src/Stack.o \
./src/WaitFreePool.o \
./src/pool.o \
./src/test.o 

C_DEPS += \
./src/AtomicStampedReference.d \
./src/Block.d \
./src/Chunk.d \
./src/FreePool.d \
./src/FullPool.d \
./src/LocalPool.d \
./src/Stack.d \
./src/WaitFreePool.d \
./src/pool.d \
./src/test.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c11 -O0 -g3 -Wall -c -fmessage-length=0 -std=c11 -v -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


