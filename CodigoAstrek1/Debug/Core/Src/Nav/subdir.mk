################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Nav/NavGlobal.c \
../Core/Src/Nav/Navegacion.c \
../Core/Src/Nav/Taquito.c 

OBJS += \
./Core/Src/Nav/NavGlobal.o \
./Core/Src/Nav/Navegacion.o \
./Core/Src/Nav/Taquito.o 

C_DEPS += \
./Core/Src/Nav/NavGlobal.d \
./Core/Src/Nav/Navegacion.d \
./Core/Src/Nav/Taquito.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Nav/%.o Core/Src/Nav/%.su Core/Src/Nav/%.cyclo: ../Core/Src/Nav/%.c Core/Src/Nav/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Actuadores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Actuadores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Sensores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Transmision" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Sensores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Transmision" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Nav" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Nav" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Herramientas" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Herramientas" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Nav

clean-Core-2f-Src-2f-Nav:
	-$(RM) ./Core/Src/Nav/NavGlobal.cyclo ./Core/Src/Nav/NavGlobal.d ./Core/Src/Nav/NavGlobal.o ./Core/Src/Nav/NavGlobal.su ./Core/Src/Nav/Navegacion.cyclo ./Core/Src/Nav/Navegacion.d ./Core/Src/Nav/Navegacion.o ./Core/Src/Nav/Navegacion.su ./Core/Src/Nav/Taquito.cyclo ./Core/Src/Nav/Taquito.d ./Core/Src/Nav/Taquito.o ./Core/Src/Nav/Taquito.su

.PHONY: clean-Core-2f-Src-2f-Nav

