/*
 * sr04.h
 *
 *  Created on: Jul 22, 2025
 *      Author: EdamVelas
 */

#ifndef INC_SENSORES_SR04_H_
#define INC_SENSORES_SR04_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os.h"

// Estructura para almacenar datos del sensor
typedef struct {
    float distance_cm;
    float distance_mm;
    uint32_t echo_time_us;
    uint8_t is_valid;
    uint32_t timestamp;
} HCSR04_Data_t;

// Estructura de configuración del sensor
typedef struct {
    TIM_HandleTypeDef* htim;
    uint32_t tim_channel;
    GPIO_TypeDef* trig_port;
    uint16_t trig_pin;
    uint32_t timeout_ms;
} HCSR04_Config_t;

// Estructura para almacenar los datos de los 3 sensores ultrasonicos
typedef struct {
	HCSR04_Data_t frontal;
	HCSR04_Data_t izquierdo;
	HCSR04_Data_t derecho;
} ultrasonico;

// Funciones públicas
void HCSR04_Init(HCSR04_Config_t* config);
HAL_StatusTypeDef HCSR04_ReadDistance(HCSR04_Data_t* data);
void HCSR04_TriggerMeasurement(void);
void HCSR04_InputCaptureCallback(TIM_HandleTypeDef* htim);
void HCSR04_TimerOverflowCallback(TIM_HandleTypeDef* htim);

// Variables externas
extern HCSR04_Data_t g_hcsr04_data;
extern osSemaphoreId_t hcsr04_semaphore;


#endif /* INC_SENSORES_SR04_H_ */
