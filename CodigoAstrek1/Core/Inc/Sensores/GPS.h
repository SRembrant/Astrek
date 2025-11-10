/* GPS.h - Versión corregida con DMA circular */
#ifndef GPS_H_
#define GPS_H_

#include "stm32f4xx.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Configuración
#define GPS_BUFFER_SIZE            512   // Buffer circular DMA
#define GPS_MAX_SENTENCE_LENGTH    128   // Máxima longitud de sentencia NMEA
#define GPS_SENTENCE_QUEUE_SIZE    4     // Cola de sentencias procesadas

// Estructura de configuración
typedef struct {
    UART_HandleTypeDef* huart;
    uint32_t timeout_ms;
} GPS_Config_t;

// Estructura de datos GPS
typedef struct {
    // Coordenadas
    float latitude;
    float longitude;
    char lat_direction;
    char lon_direction;

    // Información temporal
    float utc_time;
    uint32_t date;

    // Calidad de señal
    uint8_t fix_quality;
    uint8_t satellites;
    float hdop;

    // Navegación
    float altitude;
    float speed_knots;
    float course;

    // Estado
    uint8_t is_valid;
    uint32_t timestamp;

    // Estadísticas
    uint32_t sentences_received;
    uint32_t sentences_parsed;
    uint32_t checksum_errors;

    // Debug
    char last_sentence[GPS_MAX_SENTENCE_LENGTH];
} GPS_Data_t;

// Variables globales
extern GPS_Data_t g_gps_data;
extern volatile uint8_t gps_data_ready;

// Funciones públicas
void GPS_Init(GPS_Config_t* config);
void GPS_StartReceive(void);
void GPS_ProcessData(void);

// Callbacks (llamar desde stm32f4xx_it.c o freertos.c)
void GPS_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart);
void GPS_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void GPS_UART_ErrorCallback(UART_HandleTypeDef *huart);

// Utilidades
float GPS_ConvertToDecimalDegrees(float coord, char direction);
void GPS_PrintData(GPS_Data_t* data);

#endif /* GPS_H_ */
