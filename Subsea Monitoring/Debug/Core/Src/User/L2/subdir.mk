################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/User/L2/Comm_Datalink.c \
../Core/Src/User/L2/DataPlotter.c 

OBJS += \
./Core/Src/User/L2/Comm_Datalink.o \
./Core/Src/User/L2/DataPlotter.o 

C_DEPS += \
./Core/Src/User/L2/Comm_Datalink.d \
./Core/Src/User/L2/DataPlotter.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/User/L2/%.o Core/Src/User/L2/%.su: ../Core/Src/User/L2/%.c Core/Src/User/L2/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-User-2f-L2

clean-Core-2f-Src-2f-User-2f-L2:
	-$(RM) ./Core/Src/User/L2/Comm_Datalink.d ./Core/Src/User/L2/Comm_Datalink.o ./Core/Src/User/L2/Comm_Datalink.su ./Core/Src/User/L2/DataPlotter.d ./Core/Src/User/L2/DataPlotter.o ./Core/Src/User/L2/DataPlotter.su

.PHONY: clean-Core-2f-Src-2f-User-2f-L2

