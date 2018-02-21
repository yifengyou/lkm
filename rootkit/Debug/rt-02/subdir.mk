################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../rt-02/RTmain.o \
../rt-02/RTsyscalltable.o \
../rt-02/rootkit.mod.o \
../rt-02/rootkit.o 

C_SRCS += \
../rt-02/RTmain.c \
../rt-02/RTsyscalltable.c 

OBJS += \
./rt-02/RTmain.o \
./rt-02/RTsyscalltable.o 

C_DEPS += \
./rt-02/RTmain.d \
./rt-02/RTsyscalltable.d 


# Each subdirectory must supply rules for building sources it contributes
rt-02/%.o: ../rt-02/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


