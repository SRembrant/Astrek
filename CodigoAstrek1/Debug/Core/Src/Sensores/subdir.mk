################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Sensores/GPS.c \
../Core/Src/Sensores/Gases.c \
../Core/Src/Sensores/IMU.c \
../Core/Src/Sensores/sr04.c 

OBJS += \
./Core/Src/Sensores/GPS.o \
./Core/Src/Sensores/Gases.o \
./Core/Src/Sensores/IMU.o \
./Core/Src/Sensores/sr04.o 

C_DEPS += \
./Core/Src/Sensores/GPS.d \
./Core/Src/Sensores/Gases.d \
./Core/Src/Sensores/IMU.d \
./Core/Src/Sensores/sr04.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Sensores/%.o Core/Src/Sensores/%.su Core/Src/Sensores/%.cyclo: ../Core/Src/Sensores/%.c Core/Src/Sensores/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Actuadores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Actuadores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Sensores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Transmision" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Sensores" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Transmision" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Nav" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Nav" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Inc/Herramientas" -I"D:/EQUIPO ATREK/VERSIONADO/ROVER ASTRE V.4.2/CodigoAstrek1/Core/Src/Herramientas" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Sensores

clean-Core-2f-Src-2f-Sensores:
	-$(RM) ./Core/Src/Sensores/GPS.cyclo ./Core/Src/Sensores/GPS.d ./Core/Src/Sensores/GPS.o ./Core/Src/Sensores/GPS.su ./Core/Src/Sensores/Gases.cyclo ./Core/Src/Sensores/Gases.d ./Core/Src/Sensores/Gases.o ./Core/Src/Sensores/Gases.su ./Core/Src/Sensores/IMU.cyclo ./Core/Src/Sensores/IMU.d ./Core/Src/Sensores/IMU.o ./Core/Src/Sensores/IMU.su ./Core/Src/Sensores/sr04.cyclo ./Core/Src/Sensores/sr04.d ./Core/Src/Sensores/sr04.o ./Core/Src/Sensores/sr04.su

.PHONY: clean-Core-2f-Src-2f-Sensores

