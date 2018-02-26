################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/libs/radio_prot_a.c \
../src/libs/radio_prot_ri.c 

OBJS += \
./src/libs/radio_prot_a.o \
./src/libs/radio_prot_ri.o 

C_DEPS += \
./src/libs/radio_prot_a.d \
./src/libs/radio_prot_ri.d 


# Each subdirectory must supply rules for building sources it contributes
src/libs/%.o: ../src/libs/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


