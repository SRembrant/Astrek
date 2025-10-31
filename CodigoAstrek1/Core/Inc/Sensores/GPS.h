/*
 * GPS.h
 *
 *  Created on: Jul 22, 2025
 *      Author: EdamVelas
 */

#ifndef INC_SENSORES_GPS_H_
#define INC_SENSORES_GPS_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Tamaño del buffer circular para recepción
#define GPS_BUFFER_SIZE 512
#define GPS_MAX_SENTENCE_LENGTH 128

// Estructura para almacenar datos GPS
typedef struct {
    // Datos de posición
    float latitude;
    float longitude;
    char lat_direction;    // 'N' o 'S'
    char lon_direction;    // 'E' o 'W'

    // Datos de tiempo
    float utc_time;        // HHMMSS.SSS
    uint32_t date;         // DDMMYY

    // Datos de calidad
    uint8_t fix_quality;   // 0=sin fix, 1=GPS fix, 2=DGPS fix
    uint8_t satellites;    // Número de satélites
    float hdop;            // Dilución horizontal de precisión
    float altitude;        // Altitud en metros
    float speed_knots;     // Velocidad en nudos
    float course;          // Rumbo en grados

    // Estado
    uint8_t is_valid;      // 1 si los datos son válidos
    uint32_t timestamp;    // Timestamp del último dato válido

    // Información de la trama
    char last_sentence[GPS_MAX_SENTENCE_LENGTH];
    uint32_t sentences_received;
    uint32_t sentences_parsed;
} GPS_Data_t;

// Estructura de configuración
typedef struct {
    UART_HandleTypeDef* huart;
    uint32_t timeout_ms;
} GPS_Config_t;

// Funciones públicas
void GPS_Init(GPS_Config_t* config);
void GPS_StartReceive(void);
void GPS_ProcessData(void);
void GPS_ParseNMEA(char* sentence);
uint8_t GPS_ValidateChecksum(char* sentence);
void GPS_ParseBuffer(uint8_t* buffer, uint16_t length);

// Funciones de parsing específicas
void GPS_ParseGGA(char* sentence);
void GPS_ParseRMC(char* sentence);
void GPS_ParseGSA(char* sentence);
void GPS_ParseGSV(char* sentence);


// Funciones de utilidad
float GPS_ConvertToDecimalDegrees(float coord, char direction);
void GPS_PrintData(GPS_Data_t* data);

// Variables externas
extern GPS_Data_t g_gps_data;
extern volatile uint8_t gps_data_ready;


#endif /* INC_SENSORES_GPS_H_ */
