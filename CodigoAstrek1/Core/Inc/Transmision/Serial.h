/*
 * Serial.h
 *
 *  Created on: Jul 22, 2025
 *      Author: EdamVelas
 */

#ifndef INC_TRANSMISION_SERIAL_H_
#define INC_TRANSMISION_SERIAL_H_

#include "stm32f4xx_hal.h"
#include "IMU.h"
#include "GPS.h"
#include "cmsis_os.h"
#include "sr04.h"

// Configuración del buffer
#define SERIAL_BUFFER_SIZE 512

// Funciones públicas
void Serial_Init(UART_HandleTypeDef* huart);
void Serial_PrintHCSR04Data(HCSR04_Data_t* data);
void Serial_PrintGPSData(GPS_Data_t* data);
void Serial_PrintIMUData(MPU9250_Data* data);
void Serial_PrintString(const char* str);
void Serial_TxComplete_Callback(UART_HandleTypeDef* huart);


// Variables externas
extern osSemaphoreId_t serial_semaphore;



#endif /* INC_TRANSMISION_SERIAL_H_ */
