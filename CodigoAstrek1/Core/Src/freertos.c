/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c (VERSIÓN CORREGIDA)
 * Description        : Code for freertos applications
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#define _USE_MATH_DEFINES
#include <math.h>

#include "GPS.h"
#include "IMU.h"
#include "Serial.h"
#include "sr04.h"

#include "Navegacion.h"
//#include "NavGlobal.h"
#include "Taquito.h"

#include "usart.h"
#include "i2c.h"

#include <string.h>
#include <stdio.h>

#include "Control_Rover.h"
#include "Control_Pose.h"
#include "Control_Kinematics.h"
#include "Control_PWM.h"

#include "Sensors_I2C.h"
#include "LoRa_RYLR998.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern Rover_Config Rover;
extern HCSR04_Config_t hcsr04_config;
extern GPS_Config_t gps_config;
extern MPU9250_Data IMU_Data;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

uint32_t raw_light_debug = 0;
static PoseController_t pose_controller;
static PWM_Compensation_t pwm_compensation;
static uint8_t control_initialized = 0;

/* USER CODE END Variables */
/* Definitions for Navegacion */
osThreadId_t NavegacionHandle;
const osThreadAttr_t Navegacion_attributes = {
		.name = "Navegacion",
		.stack_size = 144 * 4,
		.priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for Taquito */
osThreadId_t TaquitoHandle;
const osThreadAttr_t Taquito_attributes = {
		.name = "Taquito",
		.stack_size = 311 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for NavGlobal */
osThreadId_t NavGlobalHandle;
const osThreadAttr_t NavGlobal_attributes = {
		.name = "NavGlobal",
		.stack_size = 323 * 4,
		.priority = (osPriority_t) osPriorityBelowNormal1,
};
/* Definitions for Control */
osThreadId_t ControlHandle;
const osThreadAttr_t Control_attributes = {
		.name = "Control",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for Ultrasonido */
osThreadId_t UltrasonidoHandle;
const osThreadAttr_t Ultrasonido_attributes = {
		.name = "Ultrasonido",
		.stack_size = 210 * 4,
		.priority = (osPriority_t) osPriorityBelowNormal2,
};
/* Definitions for Geoposicion */
osThreadId_t GeoposicionHandle;
const osThreadAttr_t Geoposicion_attributes = {
		.name = "Geoposicion",
		.stack_size = 210 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Transmision */
osThreadId_t TransmisionHandle;
const osThreadAttr_t Transmision_attributes = {
		.name = "Transmision",
		.stack_size = 512 * 4,
		.priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Sensores_I2C */
osThreadId_t Sensores_I2CHandle;
const osThreadAttr_t Sensores_I2C_attributes = {
		.name = "Sensores_I2C",
		.stack_size = 512 * 4,
		.priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for sensorDataQueue */
osMessageQueueId_t sensorDataQueueHandle;
const osMessageQueueAttr_t sensorDataQueue_attributes = {
		.name = "sensorDataQueue"
};
/* Definitions for gpsDataQueue */
osMessageQueueId_t gpsDataQueueHandle;
const osMessageQueueAttr_t gpsDataQueue_attributes = {
		.name = "gpsDataQueue"
};
/* Definitions for controlDataQueue */
osMessageQueueId_t controlDataQueueHandle;
const osMessageQueueAttr_t controlDataQueue_attributes = {
		.name = "controlDataQueue"
};
/* Definitions for navStatesQueue */
osMessageQueueId_t navStatesQueueHandle;
const osMessageQueueAttr_t navStatesQueue_attributes = {
		.name = "navStatesQueue"
};
/* Definitions for hcsr04DataQueue */
osMessageQueueId_t hcsr04DataQueueHandle;
const osMessageQueueAttr_t hcsr04DataQueue_attributes = {
		.name = "hcsr04DataQueue"
};
/* Definitions for hcsr04Semaphore */
osSemaphoreId_t hcsr04SemaphoreHandle;
const osSemaphoreAttr_t hcsr04Semaphore_attributes = {
		.name = "hcsr04Semaphore"
};
/* Definitions for serialSemaphore */
osSemaphoreId_t serialSemaphoreHandle;
const osSemaphoreAttr_t serialSemaphore_attributes = {
		.name = "serialSemaphore"
};
/* Definitions for imuSemaphore */
osSemaphoreId_t imuSemaphoreHandle;
const osSemaphoreAttr_t imuSemaphore_attributes = {
		.name = "imuSemaphore"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void Rover_SetPWM_Differential(Rover_Config *rover, PWM_Commands_t commands)
{
	// ========== MOTORES DERECHOS ==========
	if (commands.dir_right == 0) {
		__HAL_TIM_SET_COMPARE(rover->f_right_motor.pwm_timer,
				rover->f_right_motor.pwm_channel_IN1,
				commands.pwm_right);
		__HAL_TIM_SET_COMPARE(rover->f_right_motor.pwm_timer,
				rover->f_right_motor.pwm_channel_IN2, 0);

		__HAL_TIM_SET_COMPARE(rover->b_right_motor.pwm_timer,
				rover->b_right_motor.pwm_channel_IN1,
				commands.pwm_right);
		__HAL_TIM_SET_COMPARE(rover->b_right_motor.pwm_timer,
				rover->b_right_motor.pwm_channel_IN2, 0);
	} else {
		__HAL_TIM_SET_COMPARE(rover->f_right_motor.pwm_timer,
				rover->f_right_motor.pwm_channel_IN1, 0);
		__HAL_TIM_SET_COMPARE(rover->f_right_motor.pwm_timer,
				rover->f_right_motor.pwm_channel_IN2,
				commands.pwm_right);

		__HAL_TIM_SET_COMPARE(rover->b_right_motor.pwm_timer,
				rover->b_right_motor.pwm_channel_IN1, 0);
		__HAL_TIM_SET_COMPARE(rover->b_right_motor.pwm_timer,
				rover->b_right_motor.pwm_channel_IN2,
				commands.pwm_right);
	}

	// ========== MOTORES IZQUIERDOS ==========
	if (commands.dir_left == 0) {
		__HAL_TIM_SET_COMPARE(rover->f_left_motor.pwm_timer,
				rover->f_left_motor.pwm_channel_IN1,
				commands.pwm_left);
		__HAL_TIM_SET_COMPARE(rover->f_left_motor.pwm_timer,
				rover->f_left_motor.pwm_channel_IN2, 0);

		__HAL_TIM_SET_COMPARE(rover->b_left_motor.pwm_timer,
				rover->b_left_motor.pwm_channel_IN1,
				commands.pwm_left);
		__HAL_TIM_SET_COMPARE(rover->b_left_motor.pwm_timer,
				rover->b_left_motor.pwm_channel_IN2, 0);
	} else {
		__HAL_TIM_SET_COMPARE(rover->f_left_motor.pwm_timer,
				rover->f_left_motor.pwm_channel_IN1, 0);
		__HAL_TIM_SET_COMPARE(rover->f_left_motor.pwm_timer,
				rover->f_left_motor.pwm_channel_IN2,
				commands.pwm_left);

		__HAL_TIM_SET_COMPARE(rover->b_left_motor.pwm_timer,
				rover->b_left_motor.pwm_channel_IN1, 0);
		__HAL_TIM_SET_COMPARE(rover->b_left_motor.pwm_timer,
				rover->b_left_motor.pwm_channel_IN2,
				commands.pwm_left);
	}
}

/* USER CODE END FunctionPrototypes */

void NavegacionTask(void *argument);
void TaquitoTask(void *argument);
void Navegacion_Global(void *argument);
void ControlTask(void *argument);
void UltrasonidoTask(void *argument);
void GPSTask(void *argument);
void TransmisionTask(void *argument);
void SensoresTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
void MX_FREERTOS_Init(void) {
	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* Create the semaphores(s) */
	/* creation of hcsr04Semaphore */
	hcsr04SemaphoreHandle = osSemaphoreNew(1, 0, &hcsr04Semaphore_attributes);

	/* creation of serialSemaphore */
	serialSemaphoreHandle = osSemaphoreNew(1, 1, &serialSemaphore_attributes);

	/* creation of imuSemaphore */
	imuSemaphoreHandle = osSemaphoreNew(1, 1, &imuSemaphore_attributes);

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* Create the queue(s) */
	/* creation of sensorDataQueue */
	sensorDataQueueHandle = osMessageQueueNew (5, sizeof(SensorData_t), &sensorDataQueue_attributes);

	/* creation of gpsDataQueue */
	gpsDataQueueHandle = osMessageQueueNew (5, sizeof(GPS_Data_t), &gpsDataQueue_attributes);

	/* creation of controlDataQueue */
	controlDataQueueHandle = osMessageQueueNew (5, sizeof(ControlCommand_t), &controlDataQueue_attributes);

	/* creation of navStatesQueue */
	//navStatesQueueHandle = osMessageQueueNew (5, sizeof(evento_navegacion), &navStatesQueue_attributes);

	/* creation of hcsr04DataQueue */
	hcsr04DataQueueHandle = osMessageQueueNew (5, sizeof(uint16_t), &hcsr04DataQueue_attributes);

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of Navegacion */
	//NavegacionHandle = osThreadNew(NavegacionTask, NULL, &Navegacion_attributes);

	/* creation of Taquito */
	TaquitoHandle = osThreadNew(TaquitoTask, NULL, &Taquito_attributes);

	/* creation of NavGlobal */
	//NavGlobalHandle = osThreadNew(Navegacion_Global, NULL, &NavGlobal_attributes);

	/* creation of Control */
	ControlHandle = osThreadNew(ControlTask, NULL, &Control_attributes);

	/* creation of Ultrasonido */
	UltrasonidoHandle = osThreadNew(UltrasonidoTask, NULL, &Ultrasonido_attributes);

	/* creation of Geoposicion */
	GeoposicionHandle = osThreadNew(GPSTask, NULL, &Geoposicion_attributes);

	/* creation of Transmision */
	TransmisionHandle = osThreadNew(TransmisionTask, NULL, &Transmision_attributes);

	/* creation of Sensores_I2C */
	Sensores_I2CHandle = osThreadNew(SensoresTask, NULL, &Sensores_I2C_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	/* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_NavegacionTask */
/**
 * @brief  Function implementing the Navegacion thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_NavegacionTask */
void NavegacionTask(void *argument)
{
	/* USER CODE BEGIN NavegacionTask */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
	/* USER CODE END NavegacionTask */
}

/* USER CODE BEGIN Header_TaquitoTask */
/**
 * @brief Function implementing the Taquito thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_TaquitoTask */
void TaquitoTask(void *argument)
{
	/* USER CODE BEGIN TaquitoTask */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
	/* USER CODE END TaquitoTask */
}

/* USER CODE BEGIN Header_Navegacion_Global */
/**
 * @brief Function implementing the NavGlobal thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Navegacion_Global */
void Navegacion_Global(void *argument)
{
	/* USER CODE BEGIN Navegacion_Global */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
	/* USER CODE END Navegacion_Global */
}

/* USER CODE BEGIN Header_ControlTask */
/**
 * @brief Function implementing the Control thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_ControlTask */
void ControlTask(void *argument)
{
    PoseController_t pose_ctrl;
    PoseController_Init(&pose_ctrl);

    GPS_Data_t gps_data;
    ControlCommand_t cmd;
    osStatus_t status;

    // Variables de trabajo para el control
	float vx_cmd = 0.0f;
	float wz_cmd = 0.0f;

    // Target inicial (hardcoded para prueba)
  /*  float target_x = 10.0f;  // 10 metros al norte
    float target_y = 0.0f;*/

    for(;;)
    {
        // 1. Leer GPS actual
        status = osMessageQueueGet(gpsDataQueueHandle, &gps_data, NULL, 10);
        if (status != osOK) continue;

        // Intentar leer un comando de la cola de Taquito (no bloqueante, timeout 0)
        status = osMessageQueueGet(controlDataQueueHandle, &cmd, NULL, 0);
        if (status != osOK) continue;

        // 2. Convertir GPS a coordenadas locales (usar gpsACartesiano)
        P_Cartesiano pos_actual = gpsACartesiano(estacionTerrena, &gps_data);

        // 3. Obtener orientación (temporalmente de GPS course)
        float theta_actual = deg2rad(gps_data.course);

        // 3. SELECCIÓN DE LÓGICA DE CONTROL BASADA EN EL MODO
		switch (cmd.mode)
		{
			case MODE_WALL_FOLLOW: //control PID para el seguimiento de pared
				// Taquito ya calculó el PID lateral y dio el vx y wz (corrección)
				vx_cmd = cmd.target_vx;
				wz_cmd = cmd.target_wz;

				// Resetear el PID de pose (para que no interfiera)
				PoseController_Reset(&pose_ctrl);
				break;

			case MODE_POSE_GIRO: //pendiente de ajustar, es para los giros de 90°
				// Giro de 90 grados:
				// 1. Usamos el PID angular de PoseController
				PoseController_Update_Theta(&pose_ctrl,
											pos_actual.theta,
											cmd.target_theta);

				// 2. Tomamos el wz del controlador angular y forzamos vx=0
				vx_cmd = 0.0f;
				wz_cmd = pose_ctrl.wz_cmd;

				// Lógica de finalización (debe estar aquí o en PoseController_Update_Theta)
				// if (fabsf(theta_actual - cmd.target_theta) < THETA_TOLERANCE) {
				//     // Notificar a Taquito y cambiar el modo de control.
				// }
				break;

			case MODE_POSE_TARGET: //la logica que ya se tenia, avanza hacia un target x,y
				// Comportamiento de control de pose original (ir a X, Y)
				PoseController_Update(&pose_ctrl,
									 pos_actual.x, pos_actual.y, theta_actual,
									 cmd.target_x, cmd.target_y); // Usar targets del comando

				vx_cmd = pose_ctrl.vx_cmd;
				wz_cmd = pose_ctrl.wz_cmd;
				break;

			case MODE_FORWARD:
				//avance "en bruto" desde taquito
				vx_cmd = cmd.target_vx;
				wz_cmd = cmd.target_wz;
				break;
		}


        // 5. Cinemática inversa
        WheelVelocities_t wheels = Kinematics_Inverse(vx_cmd,
                                                      wz_cmd);
///
        // 6. Modelo PWM
        PWM_Mapping_t pwm_right = PWM_VelocityToPWM(wheels.vR_cmd, 1);
        PWM_Mapping_t pwm_left = PWM_VelocityToPWM(wheels.vL_cmd, 0);

        // 7. Aplicar PWM
        Rover_SetPWM(&Rover, pwm_right.pwm_duty, pwm_left.pwm_duty);

        osDelay(50);  // Control a 20Hz
    }
}

/* USER CODE BEGIN Header_UltrasonidoTask */
/**
 * @brief Function implementing the Ultrasonido thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_UltrasonidoTask */
void UltrasonidoTask(void *argument)
{
	/* USER CODE BEGIN UltrasonidoTask */

	HCSR04_Data_t data_ultrasonico;

	/* Infinite loop */
	for(;;)
	{
		// 1. Llama a la función de lectura.
		// Esta función bloqueará la tarea (espera el semáforo)
		// hasta que la interrupción del timer reciba el eco.
		HAL_StatusTypeDef status = HCSR04_ReadDistance(&data_ultrasonico);
		// 2. Comprobar si la lectura fue exitosa
		if (status == HAL_OK)
		{
			// 3. Imprimir el dato usando la función de Serial.c
			// Esta función ya comprueba si data.is_valid
			Serial_PrintHCSR04Data(&data_ultrasonico);
		}
		else
		{
			// 4. (Opcional) Imprimir si hubo un error (timeout)
			//Serial_PrintString("[Ultrasonico] Error: Timeout (eco no recibido)\r\n");
		}
		// 5. Esperar antes de la próxima medición
		// No medir demasiado rápido para evitar ecos fantasmas.
		osDelay(200); // 5 lecturas por segundo es más que suficiente.
	}
	/* USER CODE END UltrasonidoTask */
}

/* USER CODE BEGIN Header_GPSTask */
/**
 * @brief Function implementing the Geoposicion thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_GPSTask */
void GPSTask(void *argument)
{
	/* USER CODE BEGIN GPSTask */
	GPS_Data_t local_gps_data;

	/* Infinite loop */
	for(;;)
	{
		GPS_ProcessData();
		if (gps_data_ready) {
			// Copiar datos globales a local
			memcpy(&local_gps_data, &g_gps_data, sizeof(GPS_Data_t));

			// Enviar a cola para otras tareas
			osMessageQueuePut(gpsDataQueueHandle, &local_gps_data, 0, 0);

			// Imprimir para debug
			GPS_PrintData(&local_gps_data);

			// Limpiar bandera
			gps_data_ready = 0;
		}
		osDelay(100);
	}
	/* USER CODE END GPSTask */
}

/* USER CODE BEGIN Header_TransmisionTask */
/**
 * @brief Function implementing the Transmision thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_TransmisionTask */
void TransmisionTask(void *argument)
{
	/* USER CODE BEGIN TransmisionTask */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
	/* USER CODE END TransmisionTask */
}

/* USER CODE BEGIN Header_SensoresTask */
/**
 * @brief Function implementing the Sensores_I2C thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_SensoresTask */
void SensoresTask(void *argument)
{
	/* USER CODE BEGIN SensoresTask */
	Sensors_I2C_Handle_t hsensors = {0};
	LoRa_t lora_module;

	if (Sensors_I2C_Init(&hsensors, &hi2c1)==HAL_OK)
	{
		Serial_PrintString("Sensores I2C Inicializados \n\r");
	}
	else
	{
		Serial_PrintString("No se logró inicializar los sensores \n\r");
	}
	LoRa_Init(&lora_module, &huart1);
	LoRa_Setup(&lora_module,
			"1",  // Dirección del dispositivo
			"18", // ID de Red
			"915000000"); // Banda (ej. 915 MHz)

	/* Infinite loop */
	for(;;)
	{
		// 1. Leer Temperatura y Humedad
		HDC1080_Read(&hsensors);

		// 2. Enviar datos de T/H al CCS811 para compensación
		CCS811_WriteEnvData(&hsensors, hsensors.data.temperature, hsensors.data.humidity);

		// 3. Leer Calidad de Aire (ahora compensada)
		CCS811_Read(&hsensors);

		// 4. Leer el resto de sensores
		MPU6050_Read(&hsensors);
		LTR390_Read(&hsensors);

		char msg[128];

		// ----- MODIFICACIÓN DEL SNPRINTF -----
		snprintf(msg, sizeof(msg),
				"%.1f,%.1f,%u,%u,%.2f,%.2f,%.2f,%.1f",
				hsensors.data.temperature,
				hsensors.data.humidity,
				hsensors.data.eco2,
				hsensors.data.tvoc,
				hsensors.data.accel[0],
				hsensors.data.accel[1],
				hsensors.data.accel[2],
				hsensors.data.light); // Volvemos a imprimir el valor float

		char cmd[160];
		snprintf(cmd, sizeof(cmd), "AT+SEND=0,%d,%s\r\n", (int)strlen(msg), msg);
		Serial_PrintString(cmd);
		osDelay(200);

	}

	/* USER CODE END SensoresTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

