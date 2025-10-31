################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Transmision/Serial.c 

OBJS += \
./Core/Src/Transmision/Serial.o 

C_DEPS += \
./Core/Src/Transmision/Serial.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Transmision/%.o Core/Src/Transmision/%.su Core/Src/Transmision/%.cyclo: ../Core/Src/Transmision/%.c Core/Src/Transmision/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Actuadores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Actuadores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Sensores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Transmision" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Sensores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Transmision" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Nav" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Nav" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Herramientas" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Herramientas" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Transmision

clean-Core-2f-Src-2f-Transmision:
	-$(RM) ./Core/Src/Transmision/Serial.cyclo ./Core/Src/Transmision/Serial.d ./Core/Src/Transmision/Serial.o ./Core/Src/Transmision/Serial.su

.PHONY: clean-Core-2f-Src-2f-Transmision

