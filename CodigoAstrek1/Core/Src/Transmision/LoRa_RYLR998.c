/*
 * LoRa_RYLR998.c
 *
 *  Created on: Nov 3, 2025
 *      Author: joadj
 */
#include "LoRa_RYLR998.h"
#include "cmsis_os.h"

HAL_StatusTypeDef LoRa_Init(LoRa_t *lora, UART_HandleTypeDef *huart) {
	lora->huart = huart;
	memset(lora->buffer, 0, sizeof(lora->buffer));

	// Prueba de comunicación
	const char *cmd = "AT\r\n";
	HAL_UART_Transmit(lora->huart, (uint8_t*)cmd, strlen(cmd), 100);
	return LoRa_ReadResponse(lora, 500);
}

HAL_StatusTypeDef LoRa_ReadResponse(LoRa_t *lora, uint32_t timeout) {
	memset(lora->buffer, 0, sizeof(lora->buffer));
	return HAL_UART_Receive(lora->huart, (uint8_t*)lora->buffer, sizeof(lora->buffer)-1, timeout);
}

HAL_StatusTypeDef LoRa_SendString(LoRa_t *lora, const char *data) {
	char cmd[160];
	snprintf(cmd, sizeof(cmd), "AT+SEND=0,%d,%s\n\r", (int)strlen(data), data);

	// Transmitimos (esto sigue siendo un bloqueo, pero es corto)
	HAL_StatusTypeDef status = HAL_UART_Transmit(lora->huart, (uint8_t*)cmd, sizeof(cmd), 200 );

	// No esperamos la respuesta "+OK". Asumimos que funcionó.
	return status;
}

HAL_StatusTypeDef LoRa_Setup(LoRa_t *lora, const char *addr, const char *netid, const char *band) {
	char cmd[64];

	// Dirección local
	snprintf(cmd, sizeof(cmd), "AT+ADDRESS=%s\r\n", addr);
	HAL_UART_Transmit(lora->huart, (uint8_t*)cmd, strlen(cmd), 100);
	LoRa_ReadResponse(lora, 200);

	// Red
	snprintf(cmd, sizeof(cmd), "AT+NETWORKID=%s\r\n", netid);
	HAL_UART_Transmit(lora->huart, (uint8_t*)cmd, strlen(cmd), 100);
	LoRa_ReadResponse(lora, 200);

	// Banda de frecuencia
	snprintf(cmd, sizeof(cmd), "AT+BAND=%s\r\n", band);
	HAL_UART_Transmit(lora->huart, (uint8_t*)cmd, strlen(cmd), 100);
	LoRa_ReadResponse(lora, 200);

	// Configurar parámetros RF (SF=9, BW=125kHz, CR=4/5, Preamble=12)
	snprintf(cmd, sizeof(cmd), "AT+PARAMETER=9,7,1,12\r\n");
	HAL_UART_Transmit(lora->huart, (uint8_t*)cmd, strlen(cmd), 100);
	LoRa_ReadResponse(lora, 200);

	return HAL_OK;
}



