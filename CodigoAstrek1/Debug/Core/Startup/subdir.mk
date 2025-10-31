################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f411ceux.s 

OBJS += \
./Core/Startup/startup_stm32f411ceux.o 

S_DEPS += \
./Core/Startup/startup_stm32f411ceux.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Actuadores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Actuadores" -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Sensores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Transmision" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Sensores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Transmision" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Nav" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Nav" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Herramientas" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Herramientas" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32f411ceux.d ./Core/Startup/startup_stm32f411ceux.o

.PHONY: clean-Core-2f-Startup

