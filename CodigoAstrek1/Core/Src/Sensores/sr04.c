/*
 * sr04.c
 *
 * Creado: Jul 22, 2025
 * Autor: Joad
 * (Versión corregida usando la lógica de "ambos flancos" del tutorial
 * y conteo de overflow robusto)
 */

#include "sr04.h"
#include "main.h"

// --- Variables Privadas ---
extern osSemaphoreId_t hcsr04SemaphoreHandle;

static HCSR04_Config_t hcsr04_conf;

// Volatile es clave para variables usadas en Tareas e ISRs
static volatile uint8_t  capture_index = 0;
static volatile uint32_t timer_overflows = 0;
static uint32_t capture_values[2] = {0}; // cap[0] = Rising, cap[1] = Falling

// Esta variable SÍ es pública (declarada en .h)
HCSR04_Data_t g_hcsr04_data = {0};


/**
 * @brief Genera un delay preciso en microsegundos usando el DWT.
 * (Asegúrate de habilitar el DWT en main.c)
 */
static void delay_us(uint32_t us)
{
    uint32_t start_tick = DWT->CYCCNT;
    uint32_t ticks_por_us = (HAL_RCC_GetHCLKFreq() / 1000000); // ej. 100MHz / 1M = 100 ticks/us
    uint32_t delay_ticks = us * ticks_por_us;

    while ((DWT->CYCCNT - start_tick) < delay_ticks);
}


/**
 * @brief Inicializa el sensor HC-SR04
 */
void HCSR04_Init(HCSR04_Config_t* config) {
	hcsr04_conf = *config;

	// Iniciar el timer y sus interrupciones
	HAL_TIM_Base_Start(hcsr04_conf.htim);             // Inicia el contador
	HAL_TIM_Base_Start_IT(hcsr04_conf.htim);         // Habilita la interrupción de overflow
	HAL_TIM_IC_Start_IT(hcsr04_conf.htim, hcsr04_conf.tim_channel); // Habilita la interrupción de Input Capture

	// Inicializar pin TRIG en LOW
	HAL_GPIO_WritePin(hcsr04_conf.trig_port, hcsr04_conf.trig_pin, GPIO_PIN_RESET);
}


/**
 * @brief Lee la distancia del sensor (Función de Tarea)
 */
HAL_StatusTypeDef HCSR04_ReadDistance(HCSR04_Data_t* data) {

    // --- 1. Resetear estado ---
    // Usamos una sección crítica para asegurarnos de que una interrupción
    // no ocurra mientras reseteamos las variables.
	taskENTER_CRITICAL();
	capture_index = 0;     // Esperando el flanco ascendente
	timer_overflows = 0;   // Reseteamos el contador de overflows
	taskEXIT_CRITICAL();

	// Asegurar que empezamos en flanco ascendente
	__HAL_TIM_SET_CAPTUREPOLARITY(hcsr04_conf.htim, hcsr04_conf.tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);
	// (Re)iniciar la captura
	HAL_TIM_IC_Start_IT(hcsr04_conf.htim, hcsr04_conf.tim_channel);

	// --- 2. Enviar pulso de trigger (10us) ---
	HAL_GPIO_WritePin(hcsr04_conf.trig_port, hcsr04_conf.trig_pin, GPIO_PIN_SET);
	delay_us(10); // Esta es la versión DWT que no interfiere con el TIM2
	HAL_GPIO_WritePin(hcsr04_conf.trig_port, hcsr04_conf.trig_pin, GPIO_PIN_RESET);

	// --- 3. Esperar a que la ISR complete (libere el semáforo) ---
    // Esperamos 100ms. Si no hay eco, reporta timeout.
	if (osSemaphoreAcquire(hcsr04SemaphoreHandle, 100) != osOK) {
		data->is_valid = 0;
		// Detener la captura si falló para no gastar CPU
		HAL_TIM_IC_Stop_IT(hcsr04_conf.htim, hcsr04_conf.tim_channel);
		return HAL_TIMEOUT;
	}

	// --- 4. Calcular y copiar los datos ---
	uint32_t echo_time;
    uint32_t timer_period = hcsr04_conf.htim->Init.Period + 1; // 1000

    // Cálculo robusto (maneja 0, 1 o N overflows)
    // (ValorFinal + (Overlows * Periodo)) - ValorInicial
	echo_time = (capture_values[1] + (timer_overflows * timer_period)) - capture_values[0];

	// Guardar en la estructura de datos
	g_hcsr04_data.echo_time_us = echo_time;
	float distance_cm = (echo_time * 0.0343f) / 2.0f;

	if (distance_cm >= 2.0f && distance_cm <= 400.0f) {
		g_hcsr04_data.distance_cm = distance_cm;
		g_hcsr04_data.distance_mm = distance_cm * 10.0f;
		g_hcsr04_data.is_valid = 1;
	} else {
		g_hcsr04_data.is_valid = 0; // Falla si está fuera de rango
	}
	g_hcsr04_data.timestamp = HAL_GetTick();

	// 5. Copiar el resultado a la variable del caller
	*data = g_hcsr04_data;

	return HAL_OK;
}

/**
 * @brief Callback de Input Capture (ISR)
 * LLAMAR DESDE HAL_TIM_IC_CaptureCallback en stm32f4xx_it.c
 */
void HCSR04_InputCaptureCallback(TIM_HandleTypeDef* htim) {
	if (htim->Channel == hcsr04_conf.tim_channel) { // Comprobar el canal

		if (capture_index == 0) {
			// --- 1. Flanco Ascendente (Inicio del Eco) ---
			capture_values[0] = HAL_TIM_ReadCapturedValue(hcsr04_conf.htim, hcsr04_conf.tim_channel);

            // Reseteamos el contador de overflows *después* de capturar el inicio
            // Esto es lo que hace la lógica del tutorial (implícitamente)
			timer_overflows = 0;
			capture_index = 1; // Ahora esperamos el flanco descendente

			// Cambiar a flanco descendente
			__HAL_TIM_SET_CAPTUREPOLARITY(hcsr04_conf.htim, hcsr04_conf.tim_channel, TIM_INPUTCHANNELPOLARITY_FALLING);

		} else if (capture_index == 1) {
			// --- 2. Flanco Descendente (Fin del Eco) ---
			capture_values[1] = HAL_TIM_ReadCapturedValue(hcsr04_conf.htim, hcsr04_conf.tim_channel);


			// ¡Detener la captura! Ya tenemos los dos valores.
			HAL_TIM_IC_Stop_IT(hcsr04_conf.htim, hcsr04_conf.tim_channel);

			// Resetear estado para la próxima vez
			capture_index = 0;

            // Volver a configurar para flanco ascendente (para la próxima llamada a ReadDistance)
            __HAL_TIM_SET_CAPTUREPOLARITY(hcsr04_conf.htim, hcsr04_conf.tim_channel, TIM_INPUTCHANNELPOLARITY_RISING);

			// ¡Despertar a la tarea!
            // Usamos 'FromISR' para seguridad en la interrupción
			osSemaphoreRelease(hcsr04SemaphoreHandle);
		}
	}
}

/**
 * @brief Callback de Overflow del Timer (ISR)
 * LLAMAR DESDE HAL_TIM_PeriodElapsedCallback
 */
void HCSR04_TimerOverflowCallback(TIM_HandleTypeDef* htim) {
	if (htim == hcsr04_conf.htim) {
        // Solo contar si la medición está activa
        // (es decir, después del flanco ascendente)
		if (capture_index == 1) {
			timer_overflows++;
		}
	}
}
