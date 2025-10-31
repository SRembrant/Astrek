/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Control_Rover.h"
#include "GPS.h"
#include "IMU.h"
#include "Serial.h"
#include "sr04.h"
#include "Gases.h"
#include "Navegacion.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
Rover_Config Rover; // A corregir, que los de diseño hpta se decidan si 4x4 o solo traccion delantera
HCSR04_Config_t hcsr04_config;
GPS_Config_t gps_config;
MPU9250_Data IMU_Data;
BMP280_t bmp;
CCS811_Handle_t ccs811_handle;

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define CCS811_I2C_ADDRESS         0x5A  // Dirección I2C del CCS811 (ADDR a GND)
#define SENSOR_READ_INTERVAL_MS    2000  // Intervalo de lectura de sensores en ms
#define UART_BUFFER_SIZE           256   // Tamaño del buffer UART

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void AllInit(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_I2C2_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  AllInit();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void AllInit(void){ // COLOCAR LOS QUE SON BRRRRRRRRRRR

	BMP280_Init(&bmp, &hi2c2);
	MPU9250_Init();

	// Configuración del sensor de gases CCS811
	CCS811_Config_t ccs811_config;
	//ccs811_config.hi2c = &hi2c1;  // Usar I2C1 para el CCS811
	ccs811_config.device_address = CCS811_I2C_ADDRESS;
	ccs811_config.measurement_mode = CCS811_MODE_1SEC;  // Medición cada segundo
	ccs811_config.baseline = 0;  // Sin baseline inicial
	ccs811_config.interrupt_enabled = false;  // Sin interrupciones
	//ccs811_config.error_callback = CCS811_ErrorHandler;

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET); // Prende el StandBy

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2); // motor izquierdo adelante
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4); // Motor derecho adelante
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // motor izquierdo atras
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4); // Motor derecho atras
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	// Configurar pines y timer para el motor derecho delantero
	Rover.f_right_motor.pwm_timer = &htim1;
	Rover.f_right_motor.pwm_channel_IN1 = TIM_CHANNEL_4; // AIN1
	Rover.f_right_motor.pwm_channel_IN2 = TIM_CHANNEL_1; // AIN2

	// Configurar pines y timer para el motor izquierdo delantero
	Rover.f_left_motor.pwm_timer = &htim1;
	Rover.f_left_motor.pwm_channel_IN1 = TIM_CHANNEL_2; // BIN1
	Rover.f_left_motor.pwm_channel_IN2 = TIM_CHANNEL_3; // BIN2

	// Configurar pines y timer para el motor derecho trasero
	Rover.b_right_motor.pwm_timer = &htim3;
	Rover.b_right_motor.pwm_channel_IN1 = TIM_CHANNEL_4; // CIN1
	Rover.b_right_motor.pwm_channel_IN2 = TIM_CHANNEL_3; // CIN2

	// Configurar pines y timer para el motor izquierdo trasero
	Rover.b_left_motor.pwm_timer = &htim3;
	Rover.b_left_motor.pwm_channel_IN1 = TIM_CHANNEL_1; // DIN1
	Rover.b_left_motor.pwm_channel_IN2 = TIM_CHANNEL_2; // DIN2


	hcsr04_config.htim = &htim2;
	hcsr04_config.tim_channel = TIM_CHANNEL_1;
	hcsr04_config.trig_port = GPIOA;          // Ajustar según tu configuración
	hcsr04_config.trig_pin = GPIO_PIN_6;     // Ajustar según tu configuración
	hcsr04_config.timeout_ms = 100;
	HCSR04_Init(&hcsr04_config);

	gps_config.huart = &huart2;  // colocar la que es aqui y arriba en la definicion externa
	gps_config.timeout_ms = 1000;
	GPS_Init(&gps_config);

	Serial_Init(&huart1);

	init_navegacion();
}

/**
 *  @brief Callback de Input Capture
 * @param htim Handle del timer
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	HCSR04_InputCaptureCallback(htim);
}

/**
 * @brief Callback de transmisión UART completa
 * @param huart Handle de UART
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	Serial_TxComplete_Callback(huart);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2) {
		// Reiniciar recepción en caso de error
		GPS_StartReceive();
	}
}

void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
	// El procesamiento se hace en la task GPS
	// No es necesario hacer nada aquí ya que usamos DMA circular
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM11 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM11)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
