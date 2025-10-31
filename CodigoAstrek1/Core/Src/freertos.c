/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
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
#include "Control_Rover.h"
#include "GPS.h"
#include "IMU.h"
#include "Serial.h"
#include "sr04.h"

#include "Navegacion.h"
#include "NavGlobal.h"
#include "Taquito.h"

#include "usart.h"
#include "i2c.h"

#include <string.h>
#include <stdio.h>
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
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY); // usa el UART que tengas
    return ch;
}
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/*creation for Navegacion*/
osThreadId_t navegacionHandle;
const osThreadAttr_t navegacion_attributes = {
  .name = "navegacion",
  .stack_size = 144 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/*creation for Taquito*/
osThreadId_t taquitoHandle;
const osThreadAttr_t taquito_attributes = {
  .name = "taquito",
  .stack_size = 311 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/*creation for NavGlobal*/
osThreadId_t navGlobalHandle;
const osThreadAttr_t navGlobal_attributes = {
  .name = "navGlobal",
  .stack_size = 323 * 4, //estaba en 323
  .priority = (osPriority_t) osPriorityBelowNormal,
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
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for Geoposicion */
osThreadId_t GeoposicionHandle;
const osThreadAttr_t Geoposicion_attributes = {
  .name = "Geoposicion",
  .stack_size = 210 * 4, //estaba en 210
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for IMU */
osThreadId_t IMUHandle;
const osThreadAttr_t IMU_attributes = {
  .name = "IMU",
  .stack_size = 182 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal2,
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
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};


/* Definitions for controlDataQueue */
osMessageQueueId_t controlDataQueueHandle;
const osMessageQueueAttr_t controlDataQueue_attributes = {
  .name = "controlDataQueue"
};
/*Definitions for navigationStatesQueue*/
osMessageQueueId_t navStatesQueueHandle;
const osMessageQueueAttr_t navStatesQueue_attributes = {
  .name = "navStatesQueue"
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
/* Definitions for imuDataQueue */
osMessageQueueId_t imuDataQueueHandle;
const osMessageQueueAttr_t imuDataQueue_attributes = {
  .name = "imuDataQueue"
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

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void ControlTask(void *argument);
void UltrasonicTask(void *argument);
void GPSTask(void *argument);
void IMUTask(void *argument);
void TransmisionTask(void *argument);
void SensorsTask(void *argument);

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
  hcsr04SemaphoreHandle = osSemaphoreNew(1, 1, &hcsr04Semaphore_attributes);

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
  sensorDataQueueHandle = osMessageQueueNew (5, sizeof(ultrasonico), &sensorDataQueue_attributes);

  /* creation of gpsDataQueue */
  gpsDataQueueHandle = osMessageQueueNew (5, sizeof(GPS_Data_t), &gpsDataQueue_attributes);

  /* creation of imuDataQueue */
  imuDataQueueHandle = osMessageQueueNew (5, sizeof(uint16_t), &imuDataQueue_attributes);

  /* creation of controlDataQueue */
  controlDataQueueHandle = osMessageQueueNew(5, sizeof(control_command), &controlDataQueue_attributes);

  /* creation of navStatesQueue*/
  navStatesQueueHandle = osMessageQueueNew(5, sizeof(evento_navegacion), &navStatesQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Control */
//  ControlHandle = osThreadNew(ControlTask, NULL, &Control_attributes);

  /* creation of Ultrasonido */
//  UltrasonidoHandle = osThreadNew(UltrasonicTask, NULL, &Ultrasonido_attributes);

  /* creation of Geoposicion */
  GeoposicionHandle = osThreadNew(GPSTask, NULL, &Geoposicion_attributes);

  /* creation of IMU */
//  IMUHandle = osThreadNew(IMUTask, NULL, &IMU_attributes);

  /* creation of Transmision */
//  TransmisionHandle = osThreadNew(TransmisionTask, NULL, &Transmision_attributes);

  /* creation of Sensores_I2C */
//  Sensores_I2CHandle = osThreadNew(SensorsTask, NULL, &Sensores_I2C_attributes);

  /*creation of Navegacion*/
 // navegacionHandle = osThreadNew(navegacion_Task,NULL,&navegacion_attributes);

  /*creation of NavGlobal*/
  navGlobalHandle = osThreadNew(navGlobal_task,NULL,&navGlobal_attributes);

  /*creation of Taquito*/
 // taquitoHandle = osThreadNew(navTaquito_task,NULL,&taquito_attributes);



  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */


  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
  /* USER CODE END StartDefaultTask */
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
	osStatus_t status;
	control_command control_instruction;
  /* USER CODE BEGIN ControlTask */
	/* Infinite loop */
	for(;;)
	{
	//	Serial_PrintString("Tarea de control... ");
		/*Rover_Move(&Rover, ROVER_FORWARD, 400, 5000);
		Rover_Move(&Rover, ROVER_STOP, 400, 5000);
		Rover_Move(&Rover, ROVER_BACKWARD, 400, 5000);*/
		status=osMessageQueueGet(controlDataQueueHandle, &control_instruction, NULL, 10);
		if(status==osOK){
			Rover_Move(&Rover, control_instruction.direccion, control_instruction.velocidad, control_instruction.tiempo);
			//Serial_PrintString("datos de control recibidos...");
	/*		if(control_instruction.direccion==ROVER_FORWARD){
				Serial_PrintString("\nMoverse hacia adelante");
			}
			else if(control_instruction.direccion==ROVER_RIGHT){
				Serial_PrintString("\nMoverse hacia la derecha");
			}
			else if(control_instruction.direccion==ROVER_LEFT){
				Serial_PrintString("\nMoverse hacia la izquierda");
			}
			else if(control_instruction.direccion==ROVER_BACKWARD){
				Serial_PrintString("\nMoverse hacia atras");
			}*/

		}

		osDelay(1000); //delay para las pruebas unicamente
	}
  /* USER CODE END ControlTask */
}

/* USER CODE BEGIN Header_UltrasonicTask */
/**
 * @brief Function implementing the Ultrasonido thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_UltrasonicTask */
__weak void UltrasonicTask(void *argument)
{
  /* USER CODE BEGIN UltrasonicTask */
	//HCSR04_Data_t local_data;
	ultrasonico local_data_t;
	local_data_t.frontal.distance_cm=30;
	local_data_t.izquierdo.distance_cm=30;
	local_data_t.derecho.distance_cm=30;
	/* Infinite loop */
	for(;;)
	{
//		Serial_PrintString("Tarea de HCSR04... ");
		// Leer sensor
	//	if (HCSR04_ReadDistance(&local_data) == HAL_OK) {
			// Enviar datos a la queue
			osMessageQueuePut(sensorDataQueueHandle, &local_data_t, 0, 0);
	//	}

		// Esperar 1 s entre mediciones
		osDelay(1000);
	}
  /* USER CODE END UltrasonicTask */
}

/* USER CODE BEGIN Header_GPSTask */
/**
 * @brief Function implementing the Geoposicion thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_GPSTask */
__weak void GPSTask(void *argument)
{
  /* USER CODE BEGIN GPSTask */
	GPS_Data_t local_gps_data;
/*	local_gps_data.altitude = 1000;
	local_gps_data.is_valid = 1;
	local_gps_data.latitude = 45.53;
	local_gps_data.longitude = 76.89;*/
	/* Infinite loop */
	for(;;)
	{
		Serial_PrintString("Tarea de GPS... ");
		// Procesar datos GPS recibidos
		/*comentar para las pruebas y poder quemar datos*/
		GPS_ProcessData();
		//gps_data_ready = 1;
		/*fin comentar para las pruebas y poder quemar datos*/

		// Si hay datos GPS listos, enviarlos a la queue
		if (gps_data_ready)
		{
			// Copiar datos GPS globales a variable local
			memcpy(&g_gps_data, &local_gps_data, sizeof(GPS_Data_t));

			// Enviar a la queue (sin bloqueo)
			osMessageQueuePut(gpsDataQueueHandle, &local_gps_data, 0, 0);
			Serial_PrintGPSData(&local_gps_data);

			// Limpiar bandera
			gps_data_ready = 0;
		}

		// Procesar cada 100ms
		osDelay(1000); //delay de 1000 para las pruebas, origiinal es solo 100
	}
  /* USER CODE END GPSTask */
}

/* USER CODE BEGIN Header_IMUTask */
/**
 * @brief Function implementing the IMU thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_IMUTask */
__weak void IMUTask(void *argument)
{
  /* USER CODE BEGIN IMUTask */
	MPU9250_Data local_IMU_data;
	BMP280_t bmp;
	/* Infinite loop */
	for(;;)
	{
		Serial_PrintString("Tarea de IMU... ");
		if(MPU9250_ReadAll(&local_IMU_data) || BMP280_Read(&bmp, &local_IMU_data) == HAL_OK)
		{
			osMessageQueuePut(imuDataQueueHandle, &local_IMU_data, 0, 0);
		}



		osDelay(1000); //delay de 1000 para las pruebas, origiinal es solo 100

	}
  /* USER CODE END IMUTask */
}

/* USER CODE BEGIN Header_TransmisionTask */
/**
 * @brief Function implementing the Transmision thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_TransmisionTask */
__weak void TransmisionTask(void *argument)
{
  /* USER CODE BEGIN TransmisionTask */
	HCSR04_Data_t received_ultrasonic_data;
	GPS_Data_t received_gps_data;
	MPU9250_Data received_imu_data;
	osStatus_t status;
	/* Infinite loop */
	for(;;)
	{
		Serial_PrintString("Tarea de transmision... ");
	/*	status = osMessageQueueGet(sensorDataQueueHandle, &received_ultrasonic_data, NULL, 10);
		if (status == osOK)
		{
			// Imprimir datos del ultrasonido
			Serial_PrintHCSR04Data(&received_ultrasonic_data);
			g_hcsr04_data = received_ultrasonic_data;
		}
*/

		/*status = osMessageQueueGet(gpsDataQueueHandle, &received_gps_data, NULL, 10);
		if (status == osOK)
		{
			// Imprimir datos GPS
			Serial_PrintGPSData(&received_gps_data);
		}*/
		/*
		status = osMessageQueueGet(imuDataQueueHandle, &received_imu_data, NULL, 10);
		if (status == osOK)
		{
			Serial_PrintIMUData(&received_imu_data);
		}
*/
		// Si no hay datos disponibles, esperar un poco
		if (osMessageQueueGetCount(sensorDataQueueHandle) == 0 &&
				osMessageQueueGetCount(gpsDataQueueHandle) == 0)
		{
			osDelay(50);
		}
		osDelay(2000);
	}
  /* USER CODE END TransmisionTask */
}

/* USER CODE BEGIN Header_SensorsTask */
/**
 * @brief Function implementing the Sensores_I2C thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_SensorsTask */
__weak void SensorsTask(void *argument)
{
  /* USER CODE BEGIN SensorsTask */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1); // Destinada a todos los del I2C
	}
  /* USER CODE END SensorsTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

