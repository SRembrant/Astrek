/*
 * sr04.c
 *
 *  Created on: Jul 22, 2025
 *      Author: EdamVelas
 */
/* hcsr04.c */
#include "sr04.h"
#include "main.h"

// Variables privadas
extern osSemaphoreId_t hcsr04SemaphoreHandle;
extern osMessageQueueId_t sensorDataQueueHandle;

static HCSR04_Config_t hcsr04_conf;
static uint32_t capture_values[2] = {0};
static uint8_t capture_index = 0;
static uint8_t measurement_complete = 0;

// Variables globales
HCSR04_Data_t g_hcsr04_data = {0};

/**
 * @brief Inicializa el sensor HC-SR04
 * @param config Estructura de configuración
 */
void HCSR04_Init(HCSR04_Config_t* config) {
    hcsr04_conf = *config;

    // Configurar el timer para Input Capture
    HAL_TIM_Base_Start(hcsr04_conf.htim);
    HAL_TIM_IC_Start_IT(hcsr04_conf.htim, hcsr04_conf.tim_channel);

    // Inicializar pin TRIG en LOW
    HAL_GPIO_WritePin(hcsr04_conf.trig_port, hcsr04_conf.trig_pin, GPIO_PIN_RESET);
}

/**
 * @brief Dispara una medición del sensor
 */
void HCSR04_TriggerMeasurement(void) {
    // Reset variables
    capture_index = 0;
    measurement_complete = 0;

    // Configurar para detectar flanco ascendente
    __HAL_TIM_SET_CAPTUREPOLARITY(hcsr04_conf.htim, hcsr04_conf.tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);

    // Enviar pulso de trigger (10us)
    HAL_GPIO_WritePin(hcsr04_conf.trig_port, hcsr04_conf.trig_pin, GPIO_PIN_SET);
    osDelay(1); // Delay de 10us mínimo
    HAL_GPIO_WritePin(hcsr04_conf.trig_port, hcsr04_conf.trig_pin, GPIO_PIN_RESET);
}
/**
 * @brief Reinicia el estado de las variables de captura del HC-SR04.
 * Debe llamarse antes de cada nueva medición.
 */
static void HCSR04_ResetState(void) {
    capture_index = 0;
    capture_values[0] = 0;
    capture_values[1] = 0;
    // Si la polaridad se restablece en la interrupción, no es necesario aquí.
    // Pero es una buena práctica asegurar un estado inicial conocido.
    __HAL_TIM_SET_CAPTUREPOLARITY(hcsr04_conf.htim, hcsr04_conf.tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);
}

/**
 * @brief Lee la distancia del sensor
 * @param data Estructura donde se almacenarán los datos
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef HCSR04_ReadDistance(HCSR04_Data_t* data) {
	HCSR04_ResetState();
    // Disparar medición
    HCSR04_TriggerMeasurement();

    // Esperar por el semáforo con timeout usando el handle del IOC
    if (osSemaphoreAcquire(hcsr04SemaphoreHandle, hcsr04_conf.timeout_ms) != osOK) {
        data->is_valid = 0;
        return HAL_TIMEOUT;
    }

    // Calcular tiempo en microsegundos
    uint32_t echo_time = 0;
    if (capture_values[1] > capture_values[0]) {
        echo_time = capture_values[1] - capture_values[0];
    } else {
        // Overflow del timer
        echo_time = (0xFFFF - capture_values[0]) + capture_values[1];
    }

    // Calcular distancia (velocidad del sonido = 343 m/s = 0.0343 cm/us)
    // Distancia = (tiempo * velocidad) / 2
    float distance_cm = (echo_time * 0.0343f) / 2.0f;

    // Validar rango (2cm - 400cm)
    if (distance_cm >= 2.0f && distance_cm <= 400.0f) {
        data->distance_cm = distance_cm;
        data->distance_mm = distance_cm * 10.0f;
        data->echo_time_us = echo_time;
        data->is_valid = 1;
        data->timestamp = HAL_GetTick();
    } else {
        data->is_valid = 0;
    }

    return HAL_OK;
}

/**
 * @brief Callback de Input Capture (llamado desde la interrupción)
 * @param htim Handle del timer
 */
void HCSR04_InputCaptureCallback(TIM_HandleTypeDef* htim) {
    if (htim == hcsr04_conf.htim) {
        if (capture_index == 0) {
            // Primera captura (flanco ascendente)
            capture_values[0] = HAL_TIM_ReadCapturedValue(hcsr04_conf.htim, hcsr04_conf.tim_channel);
            capture_index = 1;

            // Cambiar a flanco descendente
            __HAL_TIM_SET_CAPTUREPOLARITY(hcsr04_conf.htim, hcsr04_conf.tim_channel, TIM_INPUTCHANNELPOLARITY_FALLING);
        } else {
            // Segunda captura (flanco descendente)
            capture_values[1] = HAL_TIM_ReadCapturedValue(hcsr04_conf.htim, hcsr04_conf.tim_channel);
            capture_index = 0;
            measurement_complete = 1;


            // Volver a flanco ascendente para próxima medición
            __HAL_TIM_SET_CAPTUREPOLARITY(hcsr04_conf.htim, hcsr04_conf.tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);


            // Liberar semáforo usando el handle del IOC
            osSemaphoreRelease(hcsr04SemaphoreHandle);
        }
    }
}





