/*
 * IMU.h
 *
 *  Created on: Jul 22, 2025
 *      Author: EdamVelas
 */

#ifndef INC_SENSORES_IMU_H_
#define INC_SENSORES_IMU_H_

#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define DEVICE_ADDRESS  0b1101000 // MPU9250
#define AK8963_ADDRESS  0x0C      // Dirección del magnetómetro

// MPU9250 Registers
#define REG_CONF_GIROSCOPE        27
#define REG_CONF_ACCELEROMETER   28
#define REG_POWER_MANAGEMENT_1  107
#define REG_DATA_ACCEL_X_H       59


// Configuración
#define FS_SEL_GIRO_500   0x08
#define FS_SEL_ACCEL_16G  0x18

#define ACCEL_SCALE_16G   (16.0 / 32768.0 * 9.80665)
#define GYRO_SCALE_500DPS  (500.0 / 32768.0 * (M_PI / 180.0))
#define MAG_SCALE_16BIT   (4912.0 / 32760.0)

//AK8963 MAGNETOMETRO
#define AK8963_ADDRESS         0x0C
#define AK8963_WHO_AM_I        0x00
#define AK8963_CNTL1           0x0A
#define AK8963_ST1             0x02
#define AK8963_DATA            0x03
#define AK8963_ST2             0x09
#define AK8963_MODE_CONTINUOUS 0x16

//BMP280

#define BMP280_I2C_ADDR      0x76 << 1
#define BMP280_REG_ID         0xD0
#define BMP280_REG_RESET      0xE0
#define BMP280_REG_CTRL_MEAS  0xF4
#define BMP280_REG_CONFIG     0xF5
#define BMP280_REG_PRESS_MSB  0xF7
#define BMP280_REG_TEMP_MSB   0xFA
#define BMP280_REG_CALIB      0x88

// Estructura para los datos


typedef struct {
	float accel_x, accel_y, accel_z;
	float gyro_x, gyro_y, gyro_z;
	float mag_x, mag_y, mag_z;
	float AccTotal;
	float Temperature, Presure;
} MPU9250_Data;

// Filtro Kalman
typedef struct {
	float Q, R, X, P, K;
} KalmanFilter;

typedef struct {
	I2C_HandleTypeDef *hi2c;
	uint8_t address;

	uint16_t dig_T1;
	int16_t  dig_T2, dig_T3;
	uint16_t dig_P1;
	int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
	int32_t  t_fine;
} BMP280_t;



// Funciones públicas
void MPU9250_Init(void);
HAL_StatusTypeDef MPU9250_ReadAll(MPU9250_Data *data);
void MPU9250_EnviarDatos(void);
void MPU9250_Reset(void);


//Calman filter
void KalmanFilter_Init(KalmanFilter *kf, float Q, float R, float P, float initial_value);
float KalmanFilter_Update(KalmanFilter *kf, float measurement);
void MPU9250_ReadAllWithKalman(MPU9250_Data *data);
void MPU9250_InitWithKalman(void);






void BMP280_Init(BMP280_t *bmp, I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BMP280_Read(BMP280_t *bmp, MPU9250_Data *data);
char* GY91_printfData(MPU9250_Data *data);



#endif /* INC_SENSORES_IMU_H_ */
