/*
 * Serial.c
 *
 *  Created on: Jul 22, 2025
 *      Author: EdamVelas
 */
#include "Serial.h"
#include "main.h"
#include "semphr.h"
#include "GPS.h"
#include "IMU.h"
#include <stdio.h>
#include <string.h>

// Variables privadas
extern osSemaphoreId_t serialSemaphoreHandle;

static UART_HandleTypeDef* huart_serial;
static char serial_buffer[SERIAL_BUFFER_SIZE];

/**
 * @brief Inicializa el módulo de salida serial
 * @param huart Handle de UART
 */
void Serial_Init(UART_HandleTypeDef* huart) {
    huart_serial = huart;

    // Mensaje de inicio
    Serial_PrintString("HC-SR04, GPS Monitor & GY-91\r\n");
    Serial_PrintString("Initializing sensors...\r\n");
}

/**
 * @brief Imprime los datos del sensor HC-SR04
 * @param data Estructura con los datos del sensor
 */
void Serial_PrintHCSR04Data(HCSR04_Data_t* data) {
    if (data->is_valid) {
        snprintf(serial_buffer, SERIAL_BUFFER_SIZE,
                "\n[Ultrasonico] -> Distance: %.2f cm (%.1f mm) | Echo: %lu us | Time: %lu ms\r\n",
                data->distance_cm,
                data->distance_mm,
                data->echo_time_us,
                data->timestamp);
    } else {
        snprintf(serial_buffer, SERIAL_BUFFER_SIZE,
                "\nInvalid measurement | Time: %lu ms\r\n",
                HAL_GetTick());
    }

    Serial_PrintString(serial_buffer);
}

/**
 * @brief Imprime los datos GPS formateados
 * @param data Estructura con datos GPS
 */
void Serial_PrintGPSData(GPS_Data_t* data) {
	 if (data->is_valid) {
	        float lat_decimal = GPS_ConvertToDecimalDegrees(data->latitude, data->lat_direction);
	        float lon_decimal = GPS_ConvertToDecimalDegrees(data->longitude, data->lon_direction);

	        snprintf(serial_buffer, SERIAL_BUFFER_SIZE,
	                "[GPS] -> Lat: %.6f°%c, Lon: %.6f°%c | Fix: %d | Sats: %d | Alt: %.1fm | Speed: %.1f kn \r\n",
	                lat_decimal, data->lat_direction,
	                lon_decimal, data->lon_direction,
	                data->fix_quality,
	                data->satellites,
	                data->altitude,
	                data->speed_knots);
	    } else {
	        snprintf(serial_buffer, SERIAL_BUFFER_SIZE,
	                "[GPS] -> No Fix | Sats: %d | Rx: %lu/%lu | Time: %lu ms\r\n",
	                data->satellites,
	                data->sentences_parsed,
	                data->sentences_received,
	                data->timestamp);
	    }

	    Serial_PrintString(serial_buffer);
}

/**
 * @brief Imprime los datos del IMU formateados
 * @param data Estructura con los datos del IMU
 */
void Serial_PrintIMUData(MPU9250_Data* data) {

    snprintf(serial_buffer, SERIAL_BUFFER_SIZE,
    		"Acel: %.2f, %.2f, %.2f | Gyro: %.2f, %.2f, %.2f | MAG: %.2f, %.2f, %.2f | Temp: %.2f | Presion: %.2f | AccTot: %.2f",
    		 data->accel_x, data->accel_y, data->accel_z,
    		 data->gyro_x, data->gyro_y, data->gyro_z,
    		 data->mag_x, data->mag_y, data->mag_z,
    		 data->Temperature, data->Presure, data->AccTotal);

    Serial_PrintString(serial_buffer);
}


/**
 * @brief Envía una cadena por UART usando DMA
 * @param str Cadena a enviar
 */
void Serial_PrintString(const char* str) {
    // Esperar por el semáforo (bloqueo hasta que la transmisión anterior termine)
    if (osSemaphoreAcquire(serialSemaphoreHandle, 100) == osOK) {
        // Enviar por DMA
        HAL_UART_Transmit_DMA(huart_serial, (uint8_t*)str, strlen(str));
        // El semáforo se libera en el callback de transmisión completa
    }
}

/**
 * @brief Callback de transmisión completa (llamado desde la interrupción)
 * @param huart Handle de UART
 */
void Serial_TxComplete_Callback(UART_HandleTypeDef* huart) {
    if (huart == huart_serial) {
        // Liberar semáforo para permitir próxima transmisión
        osSemaphoreRelease(serialSemaphoreHandle);
    }
}



