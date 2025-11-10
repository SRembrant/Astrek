/*
 * Sensors_I2C.h
 *
 *  Created on: Nov 2, 2025
 *      Author: joadj
 */

#ifndef INC_SENSORES_SENSORS_I2C_H_
#define INC_SENSORES_SENSORS_I2C_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#define CCS811_ADDR     (0x5B << 1)
#define MPU6050_ADDR    (0x68 << 1)
#define HDC1080_ADDR    (0x40 << 1)
#define LTR390_ADDR     (0x53 << 1)

typedef struct {
    float temperature;
    float humidity;
    uint16_t eco2;
    uint16_t tvoc;
    float accel[3];
    float gyro[3];
    float light;
    float uva;
    float uvb;
} SensorData_t;

typedef struct {
    I2C_HandleTypeDef *hi2c;
    SensorData_t data;
} Sensors_I2C_Handle_t;

// Inicialización general
HAL_StatusTypeDef Sensors_I2C_Init(Sensors_I2C_Handle_t *hsensors, I2C_HandleTypeDef *hi2c);

// Lecturas individuales
HAL_StatusTypeDef CCS811_Read(Sensors_I2C_Handle_t *hsensors);
HAL_StatusTypeDef MPU6050_Read(Sensors_I2C_Handle_t *hsensors);
HAL_StatusTypeDef HDC1080_Read(Sensors_I2C_Handle_t *hsensors);
HAL_StatusTypeDef LTR390_Read(Sensors_I2C_Handle_t *hsensors);
HAL_StatusTypeDef CCS811_WriteEnvData(Sensors_I2C_Handle_t *hsensors, float temp, float hum);


#endif /* INC_SENSORES_SENSORS_I2C_H_ */
