################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/User/main_user.c \
../Core/Src/User/util.c 

OBJS += \
./Core/Src/User/main_user.o \
./Core/Src/User/util.o 

C_DEPS += \
./Core/Src/User/main_user.d \
./Core/Src/User/util.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/User/%.o Core/Src/User/%.su: ../Core/Src/User/%.c Core/Src/User/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-User

clean-Core-2f-Src-2f-User:
	-$(RM) ./Core/Src/User/main_user.d ./Core/Src/User/main_user.o ./Core/Src/User/main_user.su ./Core/Src/User/util.d ./Core/Src/User/util.o ./Core/Src/User/util.su

.PHONY: clean-Core-2f-Src-2f-User

