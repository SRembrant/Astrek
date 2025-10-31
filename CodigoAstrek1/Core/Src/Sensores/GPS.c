/*
 * GPS.c
 *
 *  Created on: Jul 22, 2025
 *      Author: EdamVelas
 */

/* gps_neo6m.c */
#include "GPS.h"
#include "main.h"
#include "Serial.h"

// Variables privadas
static GPS_Config_t gps_config;
static uint8_t gps_buffer[GPS_BUFFER_SIZE];
static uint16_t gps_buffer_index = 0;
static char gps_sentence[GPS_MAX_SENTENCE_LENGTH];
static uint8_t sentence_ready = 0;

// Variables globales
GPS_Data_t g_gps_data = {0};
volatile uint8_t gps_data_ready = 0;

/**
 * @brief Inicia la recepción de datos GPS por DMA
 */
void GPS_StartReceive(void) {
	HAL_UART_Receive_DMA(gps_config.huart, gps_buffer, GPS_BUFFER_SIZE);
}




void GPS_InitData() {
	memset(&g_gps_data, 0, sizeof(GPS_Data_t));

	// Inicializar con valores por defecto
	g_gps_data.latitude = 0.0;
	g_gps_data.longitude = 0.0;
	g_gps_data.lat_direction = 'N';
	g_gps_data.lon_direction = 'E';
	g_gps_data.utc_time = 0.0;
	g_gps_data.date = 0;
	g_gps_data.fix_quality = 0;
	g_gps_data.satellites = 0;
	g_gps_data.hdop = 0.0;
	g_gps_data.altitude = 0.0;
	g_gps_data.speed_knots = 0.0;
	g_gps_data.course = 0.0;
	g_gps_data.is_valid = 0;
	g_gps_data.timestamp = 0;
	g_gps_data.sentences_received = 0;
	g_gps_data.sentences_parsed = 0;
}

/*
 * @brief Inicializa el módulo GPS (versión corregida)
 * @param config Estructura de configuración
 */
void GPS_Init(GPS_Config_t* config) {
	gps_config = *config;

	// Inicializar datos GPS correctamente
	GPS_InitData();

	// Limpiar buffer
	memset(gps_buffer, 0, GPS_BUFFER_SIZE);
	gps_buffer_index = 0;

	// Inicializar recepción DMA circular
	GPS_StartReceive();
}
/**
 * @brief Procesa los datos recibidos del GPS
 */
void GPS_ProcessData(void) {
	static uint16_t last_pos = 0;
	uint16_t current_pos;

	// Obtener posición actual del DMA
	current_pos = GPS_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(gps_config.huart->hdmarx);

	// Procesar solo si hay datos nuevos
	if (current_pos != last_pos) {
		uint16_t data_length;

		if (current_pos > last_pos) {
			// Caso normal: sin wrap-around
			data_length = current_pos - last_pos;
			GPS_ParseBuffer(&gps_buffer[last_pos], data_length);
		} else {
			// Caso wrap-around: procesar hasta el final del buffer
			data_length = GPS_BUFFER_SIZE - last_pos;
			GPS_ParseBuffer(&gps_buffer[last_pos], data_length);

			// Procesar desde el inicio del buffer
			if (current_pos > 0) {
				GPS_ParseBuffer(&gps_buffer[0], current_pos);
			}
		}

		last_pos = current_pos;
	}
}

/**
 * @brief Parsea el buffer buscando sentencias NMEA completas
 * @param buffer Puntero al buffer
 * @param length Longitud de datos a procesar
 */
void GPS_ParseBuffer(uint8_t* buffer, uint16_t length) {
	static char temp_sentence[GPS_MAX_SENTENCE_LENGTH];
	static uint8_t sentence_index = 0;

	for (uint16_t i = 0; i < length; i++) {
		char c = buffer[i];

		if (c == '$') {
			// Inicio de nueva sentencia
			sentence_index = 0;
			temp_sentence[sentence_index++] = c;
		} else if (c == '\r' || c == '\n') {
			// Final de sentencia
			if (sentence_index > 0) {
				temp_sentence[sentence_index] = '\0';

				// Validar y parsear la sentencia
				if (GPS_ValidateChecksum(temp_sentence)) {
					GPS_ParseNMEA(temp_sentence);
					g_gps_data.sentences_parsed++;
				}
				g_gps_data.sentences_received++;

				sentence_index = 0;
			}
		} else {
			// Agregar caracter a la sentencia
			if (sentence_index < GPS_MAX_SENTENCE_LENGTH - 1) {
				temp_sentence[sentence_index++] = c;
			}
		}
	}
}

/**
 * @brief Valida el checksum de una sentencia NMEA
 * @param sentence Sentencia a validar
 * @return 1 si es válida, 0 si no
 */
uint8_t GPS_ValidateChecksum(char* sentence) {
	if (sentence[0] != '$') return 0;

	char* asterisk = strchr(sentence, '*');
	if (!asterisk) return 0;

	// Calcular checksum
	uint8_t checksum = 0;
	for (char* p = sentence + 1; p < asterisk; p++) {
		checksum ^= *p;
	}

	// Convertir checksum recibido
	uint8_t received_checksum = strtol(asterisk + 1, NULL, 16);

	return (checksum == received_checksum);
}

/**
 * @brief Función de debug para imprimir sentencia raw
 * @param sentence Sentencia NMEA recibida
 */
void GPS_PrintRawSentence(char* sentence) {
	char debug_buffer[256];
	snprintf(debug_buffer, sizeof(debug_buffer), "RAW GPS: %s\r\n", sentence);
	Serial_PrintString(debug_buffer);
}

/**
 * @brief Parsea una sentencia NMEA
 * @param sentence Sentencia a parsear
 */
void GPS_ParseNMEA(char* sentence) {
	// Guardar última sentencia
	strncpy(g_gps_data.last_sentence, sentence, GPS_MAX_SENTENCE_LENGTH - 1);
	g_gps_data.last_sentence[GPS_MAX_SENTENCE_LENGTH - 1] = '\0';

	// AGREGAR ESTA LÍNEA PARA DEBUG
	GPS_PrintRawSentence(sentence);  // Para ver qué sentencias llegan

	// Identificar tipo de sentencia
	if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0) {
		GPS_ParseGGA(sentence);
	} else if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0) {
		GPS_ParseRMC(sentence);
	} else if (strncmp(sentence, "$GPGSA", 6) == 0 || strncmp(sentence, "$GNGSA", 6) == 0) {
		GPS_ParseGSA(sentence);
	}
}

/**
 * @brief Parsea sentencia GGA (Global Positioning System Fix Data)
 * @param sentence Sentencia GGA
 */
void GPS_ParseGGA(char* sentence) {
	char sentence_copy[GPS_MAX_SENTENCE_LENGTH];
	char* token;
	uint8_t field = 0;

	// Copiar la sentencia para no modificar la original
	strncpy(sentence_copy, sentence, GPS_MAX_SENTENCE_LENGTH - 1);
	sentence_copy[GPS_MAX_SENTENCE_LENGTH - 1] = '\0';

	token = strtok(sentence_copy, ",");

	while (token != NULL && field < 15) {
		switch (field) {
		case 1: // UTC Time
			if (strlen(token) >= 6) {  // Verificar longitud mínima
				g_gps_data.utc_time = atof(token);
			}
			break;
		case 2: // Latitude
			if (strlen(token) > 0) {
				g_gps_data.latitude = atof(token);
			}
			break;
		case 3: // Latitude Direction
			if (strlen(token) > 0) {
				g_gps_data.lat_direction = token[0];
			}
			break;
		case 4: // Longitude
			if (strlen(token) > 0) {
				g_gps_data.longitude = atof(token);
			}
			break;
		case 5: // Longitude Direction
			if (strlen(token) > 0) {
				g_gps_data.lon_direction = token[0];
			}
			break;
		case 6: // Fix Quality
			if (strlen(token) > 0) {
				g_gps_data.fix_quality = atoi(token);
			}
			break;
		case 7: // Number of Satellites
			if (strlen(token) > 0) {
				g_gps_data.satellites = atoi(token);
			}
			break;
		case 8: // Horizontal Dilution of Precision
			if (strlen(token) > 0) {
				g_gps_data.hdop = atof(token);
			}
			break;
		case 9: // Altitude
			if (strlen(token) > 0) {
				g_gps_data.altitude = atof(token);
			}
			break;
		}

		token = strtok(NULL, ",");
		field++;
	}

	// Actualizar validez solo si fix_quality es válido (1 o 2)
	g_gps_data.is_valid = (g_gps_data.fix_quality >= 1 && g_gps_data.fix_quality <= 2);
	g_gps_data.timestamp = HAL_GetTick();
	gps_data_ready = 1;
}


/**
 * @brief Parsea sentencia RMC (Recommended Minimum)
 * @param sentence Sentencia RMC
 */
void GPS_ParseRMC(char* sentence) {
	char sentence_copy[GPS_MAX_SENTENCE_LENGTH];
	char* token;
	uint8_t field = 0;

	// Copiar la sentencia para no modificar la original
	strncpy(sentence_copy, sentence, GPS_MAX_SENTENCE_LENGTH - 1);
	sentence_copy[GPS_MAX_SENTENCE_LENGTH - 1] = '\0';

	token = strtok(sentence_copy, ",");

	while (token != NULL && field < 12) {
		switch (field) {
		case 1: // UTC Time
			if (strlen(token) >= 6) {  // Verificar longitud mínima
				g_gps_data.utc_time = atof(token);
			}
			break;
		case 2: // Status (A = valid, V = invalid)
			if (strlen(token) > 0) {
				g_gps_data.is_valid = (token[0] == 'A') ? 1 : 0;
			}
			break;
		case 3: // Latitude
			if (strlen(token) > 0) {
				g_gps_data.latitude = atof(token);
			}
			break;
		case 4: // Latitude Direction
			if (strlen(token) > 0) {
				g_gps_data.lat_direction = token[0];
			}
			break;
		case 5: // Longitude
			if (strlen(token) > 0) {
				g_gps_data.longitude = atof(token);
			}
			break;
		case 6: // Longitude Direction
			if (strlen(token) > 0) {
				g_gps_data.lon_direction = token[0];
			}
			break;
		case 7: // Speed over ground (knots)
			if (strlen(token) > 0) {
				g_gps_data.speed_knots = atof(token);
			}
			break;
		case 8: // Course over ground (degrees)
			if (strlen(token) > 0) {
				g_gps_data.course = atof(token);
			}
			break;
		case 9: // Date
			if (strlen(token) > 0) {
				g_gps_data.date = atol(token);
			}
			break;
		}

		token = strtok(NULL, ",");
		field++;
	}

	g_gps_data.timestamp = HAL_GetTick();
}


/**
 * @brief Parsea sentencia GSA (GPS DOP and active satellites)
 * @param sentence Sentencia GSA
 */
void GPS_ParseGSA(char* sentence) {
	// Implementar si necesitas información adicional de DOP
}

/**
 * @brief Convierte coordenadas a grados decimales
 * @param coord Coordenada en formato DDMM.MMMM
 * @param direction Dirección ('N', 'S', 'E', 'W')
 * @return Coordenada en grados decimales
 */
float GPS_ConvertToDecimalDegrees(float coord, char direction) {
	int degrees = (int)(coord / 100);
	float minutes = coord - (degrees * 100);
	float decimal_degrees = degrees + (minutes / 60.0);

	if (direction == 'S' || direction == 'W') {
		decimal_degrees = -decimal_degrees;
	}

	return decimal_degrees;
}

/**
 * @brief Imprime los datos GPS formateados
 * @param data Estructura con datos GPS
 */
void GPS_PrintData(GPS_Data_t* data) {
	char buffer[512];

	if (data->is_valid) {
		float lat_decimal = GPS_ConvertToDecimalDegrees(data->latitude, data->lat_direction);
		float lon_decimal = GPS_ConvertToDecimalDegrees(data->longitude, data->lon_direction);

		snprintf(buffer, sizeof(buffer),
				"GPS: %.6f°%c, %.6f°%c | Fix: %d | Sats: %d | Alt: %.1fm | Speed: %.1fknots | Time: %.3f seg | Course: %.2f\r\n",
				lat_decimal, data->lat_direction,
				lon_decimal, data->lon_direction,
				data->fix_quality,
				data->satellites,
				data->altitude,
				data->speed_knots,
				data->utc_time,
				data->course);
	} else {
		snprintf(buffer, sizeof(buffer),
				"GPS: No Fix | Sats: %d | Sentences: %lu/%lu | Time: %lu ms\r\n",
				data->satellites,
				data->sentences_parsed,
				data->sentences_received,
				data->timestamp);
	}

	Serial_PrintString(buffer);
}






