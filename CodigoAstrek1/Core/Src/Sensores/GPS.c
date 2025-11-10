/* GPS.c - Versión corregida con DMA circular robusto */
#include "GPS.h"
#include "Serial.h"

// Variables privadas

extern GPS_Config_t gps_config;
extern UART_HandleTypeDef huart1;
static uint8_t gps_dma_buffer[GPS_BUFFER_SIZE];  // Buffer circular DMA
static uint16_t old_pos = 0;                     // Última posición procesada

// Buffer para ensamblar sentencias incompletas
static char sentence_buffer[GPS_MAX_SENTENCE_LENGTH];
static uint16_t sentence_index = 0;
static uint8_t sentence_started = 0;

// Variables globales
GPS_Data_t g_gps_data = {0};
volatile uint8_t gps_data_ready = 0;

// Funciones privadas
static void GPS_ParseBuffer(uint8_t* data, uint16_t length);
static void GPS_ParseNMEA(char* sentence);
static void GPS_ParseGGA(char* sentence);
static void GPS_ParseRMC(char* sentence);
static uint8_t GPS_ValidateChecksum(char* sentence);

/**
 * @brief Inicializa el módulo GPS
 */
void GPS_Init(GPS_Config_t* config)
{
	gps_config = *config;

	// Inicializar datos GPS
	memset(&g_gps_data, 0, sizeof(GPS_Data_t));

	// Limpiar buffers
	memset(gps_dma_buffer, 0, GPS_BUFFER_SIZE);
	memset(sentence_buffer, 0, GPS_MAX_SENTENCE_LENGTH);
	sentence_index = 0;
	sentence_started = 0;
	old_pos = 0;

	gps_data_ready = 0;

	// Iniciar recepción DMA
	GPS_StartReceive();
}

/**
 * @brief Inicia la recepción DMA circular
 */
void GPS_StartReceive(void)
{
	HAL_UART_Receive_DMA(gps_config.huart, gps_dma_buffer, GPS_BUFFER_SIZE);
}

/**
 * @brief Procesa datos del buffer DMA circular
 * LLAMAR DESDE LA TAREA GPS PERIÓDICAMENTE (cada 100ms recomendado)
 */
void GPS_ProcessData(void)
{
	// Obtener posición actual del DMA
	uint16_t current_pos = GPS_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(gps_config.huart->hdmarx);

	// Si no hay datos nuevos, salir
	if (current_pos == old_pos) {
		return;
	}

	// Calcular cuántos bytes hay para procesar
	uint16_t data_length;

	if (current_pos > old_pos) {
		// Caso normal: sin wrap-around
		data_length = current_pos - old_pos;
		GPS_ParseBuffer(&gps_dma_buffer[old_pos], data_length);
	}
	else {
		// Caso wrap-around: procesar hasta el final del buffer
		data_length = GPS_BUFFER_SIZE - old_pos;
		GPS_ParseBuffer(&gps_dma_buffer[old_pos], data_length);

		// Luego procesar desde el inicio
		if (current_pos > 0) {
			GPS_ParseBuffer(&gps_dma_buffer[0], current_pos);
		}
	}

	// Actualizar última posición procesada
	old_pos = current_pos;
}

/**
 * @brief Parsea buffer buscando sentencias NMEA completas
 */
static void GPS_ParseBuffer(uint8_t* buffer, uint16_t length)
{
	for (uint16_t i = 0; i < length; i++) {
		char c = buffer[i];

		// Detectar inicio de sentencia NMEA
		if (c == '$') {
			sentence_started = 1;
			sentence_index = 0;
			sentence_buffer[sentence_index++] = c;
		}
		// Detectar fin de sentencia
		else if ((c == '\r' || c == '\n') && sentence_started) {
			if (sentence_index > 0) {
				sentence_buffer[sentence_index] = '\0';

				// Validar y parsear
				if (GPS_ValidateChecksum(sentence_buffer)) {
					GPS_ParseNMEA(sentence_buffer);
					g_gps_data.sentences_parsed++;
				} else {
					g_gps_data.checksum_errors++;
				}

				g_gps_data.sentences_received++;
			}

			// Resetear para próxima sentencia
			sentence_started = 0;
			sentence_index = 0;
		}
		// Agregar caracteres a la sentencia actual
		else if (sentence_started) {
			if (sentence_index < GPS_MAX_SENTENCE_LENGTH - 1) {
				sentence_buffer[sentence_index++] = c;
			} else {
				// Overflow - descartar sentencia
				sentence_started = 0;
				sentence_index = 0;
			}
		}
	}
}

/**
 * @brief Valida checksum de sentencia NMEA
 */
static uint8_t GPS_ValidateChecksum(char* sentence)
{
	if (sentence[0] != '$') return 0;

	char* asterisk = strchr(sentence, '*');
	if (!asterisk) return 0;

	// Calcular checksum (XOR de todos los caracteres entre $ y *)
	uint8_t checksum = 0;
	for (char* p = sentence + 1; p < asterisk; p++) {
		checksum ^= *p;
	}

	// Convertir checksum recibido de hex string a byte
	uint8_t received_checksum = (uint8_t)strtol(asterisk + 1, NULL, 16);

	return (checksum == received_checksum);
}

/**
 * @brief Identifica y parsea tipo de sentencia NMEA
 */
static void GPS_ParseNMEA(char* sentence)
{
	// Guardar última sentencia para debug
	strncpy(g_gps_data.last_sentence, sentence, GPS_MAX_SENTENCE_LENGTH - 1);

	// Identificar tipo de sentencia (GP o GN prefijo)
	if (strncmp(sentence, "$GPGGA", 6) == 0 || strncmp(sentence, "$GNGGA", 6) == 0) {
		GPS_ParseGGA(sentence);
	}
	else if (strncmp(sentence, "$GPRMC", 6) == 0 || strncmp(sentence, "$GNRMC", 6) == 0) {
		GPS_ParseRMC(sentence);
	}
	// Agregar más tipos si necesitas (GSA, GSV, etc.)
}

/**
 * @brief Parsea sentencia GGA (Global Positioning System Fix Data)
 * Formato: $GPGGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
 */
static void GPS_ParseGGA(char* sentence)
{
	char sentence_copy[GPS_MAX_SENTENCE_LENGTH];
	strncpy(sentence_copy, sentence, GPS_MAX_SENTENCE_LENGTH - 1);

	char* token;
	uint8_t field = 0;

	token = strtok(sentence_copy, ",");

	while (token != NULL) {
		switch (field) {
		case 1: // UTC Time (hhmmss.ss)
			if (strlen(token) >= 6) {
				g_gps_data.utc_time = atof(token);
			}
			break;

		case 2: // Latitude (ddmm.mmmm)
			if (strlen(token) > 0) {
				g_gps_data.latitude = atof(token);
			}
			break;

		case 3: // Latitude Direction (N/S)
			if (strlen(token) > 0) {
				g_gps_data.lat_direction = token[0];
			}
			break;

		case 4: // Longitude (dddmm.mmmm)
			if (strlen(token) > 0) {
				g_gps_data.longitude = atof(token);
			}
			break;

		case 5: // Longitude Direction (E/W)
			if (strlen(token) > 0) {
				g_gps_data.lon_direction = token[0];
			}
			break;

		case 6: // Fix Quality (0=invalid, 1=GPS, 2=DGPS)
			if (strlen(token) > 0) {
				g_gps_data.fix_quality = atoi(token);
			}
			break;

		case 7: // Number of Satellites
			if (strlen(token) > 0) {
				g_gps_data.satellites = atoi(token);
			}
			break;

		case 8: // HDOP (Horizontal Dilution of Precision)
			if (strlen(token) > 0) {
				g_gps_data.hdop = atof(token);
			}
			break;

		case 9: // Altitude above sea level
			if (strlen(token) > 0) {
				g_gps_data.altitude = atof(token);
			}
			break;
		}

		token = strtok(NULL, ",");
		field++;
	}

	// Actualizar siempre el timestamp de la última trama recibida
	g_gps_data.timestamp = HAL_GetTick();

	// --- LÓGICA DE VALIDACIÓN MEJORADA ---
	// Comprobar si el fix es válido (>= 1)
	if (g_gps_data.fix_quality >= 1)
	{
		// El fix es bueno. Marcamos como válido y avisamos a las tareas.
		g_gps_data.is_valid = 1;
		gps_data_ready = 1; // ¡Solo avisar a las tareas SI el dato es válido!
	}
	else
	{
		// El fix es 0 o inválido.
		g_gps_data.is_valid = 0;

		// NO ponemos gps_data_ready = 1.
		// La tarea GPSTask no procesará estos datos
		// y no enviará coordenadas inválidas a la cola de navegación.
	}
}

/**
 * @brief Parsea sentencia RMC (Recommended Minimum)
 * Formato: $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
 */
static void GPS_ParseRMC(char* sentence)
{
	char sentence_copy[GPS_MAX_SENTENCE_LENGTH];
	strncpy(sentence_copy, sentence, GPS_MAX_SENTENCE_LENGTH - 1);

	char* token;
	uint8_t field = 0;

	token = strtok(sentence_copy, ",");

	while (token != NULL) {
		switch (field) {
		case 1: // UTC Time
			if (strlen(token) >= 6) {
				g_gps_data.utc_time = atof(token);
			}
			break;

		case 2: // Status (A=active/valid, V=void/invalid)
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

		case 9: // Date (ddmmyy)
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
 * @brief Convierte coordenadas NMEA a grados decimales
 */
float GPS_ConvertToDecimalDegrees(float coord, char direction)
{
	int degrees = (int)(coord / 100);
	float minutes = coord - (degrees * 100);
	float decimal_degrees = degrees + (minutes / 60.0f);

	if (direction == 'S' || direction == 'W') {
		decimal_degrees = -decimal_degrees;
	}

	return decimal_degrees;
}

/**
 * @brief Imprime datos GPS (usar Serial.h)
 */
void GPS_PrintData(GPS_Data_t* data)
{
	char buffer[256];

	if (data->is_valid) {
		float lat_decimal = GPS_ConvertToDecimalDegrees(data->latitude, data->lat_direction);
		float lon_decimal = GPS_ConvertToDecimalDegrees(data->longitude, data->lon_direction);

		snprintf(buffer, sizeof(buffer),
				"[GPS] Pos: %.6f°%c, %.6f°%c | Fix:%d Sats:%d Alt:%.1fm Spd:%.1fkn Crs:%.1f° | Rx:%lu Ok:%lu Err:%lu\r\n",
				lat_decimal, data->lat_direction,
				lon_decimal, data->lon_direction,
				data->fix_quality, data->satellites,
				data->altitude, data->speed_knots, data->course,
				data->sentences_received, data->sentences_parsed, data->checksum_errors);
	} else {
		snprintf(buffer, sizeof(buffer),
				"[GPS] NO FIX | Sats:%d | Rx:%lu Ok:%lu Err:%lu | Time:%lu ms\r\n",
				data->satellites,
				data->sentences_received, data->sentences_parsed, data->checksum_errors,
				data->timestamp);
	}

	Serial_PrintString(buffer);
}

/**
 * @brief Callback de DMA Half-Complete (mitad del buffer procesada)
 * LLAMAR desde HAL_UART_RxHalfCpltCallback en stm32f4xx_it.c
 */
void GPS_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == gps_config.huart) {
		// En modo circular, este callback indica que se llenó la primera mitad
		// No hacer nada aquí, el procesamiento se hace en GPS_ProcessData()
	}
}

/**
 * @brief Callback de DMA Complete (buffer completo procesado)
 * LLAMAR desde HAL_UART_RxCpltCallback en stm32f4xx_it.c
 */
void GPS_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == gps_config.huart) {
		// En modo circular, este callback indica que se llenó la segunda mitad
		// No hacer nada aquí, el procesamiento se hace en GPS_ProcessData()
	}
}

/**
 * @brief Callback de error UART
 */
void GPS_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart == gps_config.huart) {
		// Reiniciar DMA en caso de error
		HAL_UART_DMAStop(huart);
		GPS_StartReceive();
	}
}
