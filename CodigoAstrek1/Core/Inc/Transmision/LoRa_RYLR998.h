/*
 * LoRa_RYLR998.h
 *
 *  Created on: Nov 2, 2025
 *      Author: joadj
 */

#ifndef INC_TRANSMISION_LORA_RYLR998_H_
#define INC_TRANSMISION_LORA_RYLR998_H_

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>

typedef struct {
    UART_HandleTypeDef *huart;
    char buffer[256];
} LoRa_t;

HAL_StatusTypeDef LoRa_Init(LoRa_t *lora, UART_HandleTypeDef *huart);
HAL_StatusTypeDef LoRa_SendString(LoRa_t *lora, const char *data);
HAL_StatusTypeDef LoRa_ReadResponse(LoRa_t *lora, uint32_t timeout);
HAL_StatusTypeDef LoRa_Setup(LoRa_t *lora, const char *addr, const char *netid, const char *band);




#endif /* INC_TRANSMISION_LORA_RYLR998_H_ */
