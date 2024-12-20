################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/User/L1/USART_Driver.c 

OBJS += \
./Core/Src/User/L1/USART_Driver.o 

C_DEPS += \
./Core/Src/User/L1/USART_Driver.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/User/L1/%.o Core/Src/User/L1/%.su: ../Core/Src/User/L1/%.c Core/Src/User/L1/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-User-2f-L1

clean-Core-2f-Src-2f-User-2f-L1:
	-$(RM) ./Core/Src/User/L1/USART_Driver.d ./Core/Src/User/L1/USART_Driver.o ./Core/Src/User/L1/USART_Driver.su

.PHONY: clean-Core-2f-Src-2f-User-2f-L1

