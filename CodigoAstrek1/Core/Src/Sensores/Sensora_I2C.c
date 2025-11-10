/*
 * Sensors_I2C.c
 *
 *  Created on: Nov 2, 2025
 *      Author: joadj
 */
#include "Sensors_I2C.h"
#include <math.h>
#include <stdio.h>
#include "cmsis_os.h"

// Direcciones de 7 bits encontradas por el escáner, desplazadas 1 bit
#define CCS811_ADDR   (0x5B << 1) // Tu escáner encontró 0x5B, ¡no 0x5A!
#define MPU6050_ADDR  (0x68 << 1)
#define HDC1080_ADDR  (0x40 << 1)
#define LTR390_ADDR   (0x53 << 1)

// Registros que necesitaremos para inicializar
#define MPU6050_REG_PWR_MGMT_1  0x6B
#define CCS811_REG_APP_START    0xF4
#define HDC1080_REG_CONFIG      0x02
#define LTR390_REG_MAIN_CTRL    0x00

#define CCS811_REG_ENV_DATA 0x05

#define CCS811_REG_STATUS 0x00
#define CCS811_REG_ALG_RESULT_DATA 0x02
#define CCS811_STATUS_DATA_READY (1 << 0)


HAL_StatusTypeDef Sensors_I2C_Init(Sensors_I2C_Handle_t *hsensors, I2C_HandleTypeDef *hi2c) {
	hsensors->hi2c = hi2c;
	uint8_t buffer[2];
	HAL_StatusTypeDef status;

	// ----- 1. Despertar MPU6050 -----
	// Escribir 0x00 en el registro PWR_MGMT_1 (0x6B)
	buffer[0] = 0x00;
	status = HAL_I2C_Mem_Write(hsensors->hi2c, MPU6050_ADDR, MPU6050_REG_PWR_MGMT_1, 1, buffer, 1, 100);
	if (status != HAL_OK) return status; // Falla al despertar MPU6050
	osDelay(10);

	// ----- 2. Iniciar CCS811 -----
	// Escribir un comando "vacío" al registro APP_START (0xF4)
	status = HAL_I2C_Master_Transmit(hsensors->hi2c, CCS811_ADDR, (uint8_t[]){CCS811_REG_APP_START}, 1, 100);
	if (status != HAL_OK) return status; // Falla al iniciar CCS811
	osDelay(100); // El CCS811 necesita tiempo para arrancar

	// ----- 3. Configurar HDC1080 -----
	// Configurar para adquirir Temp y Humedad en 14 bits
	// Escribir 0x1000 en el registro de Configuración (0x02)
	buffer[0] = 0x10;
	buffer[1] = 0x00;
	status = HAL_I2C_Mem_Write(hsensors->hi2c, HDC1080_ADDR, HDC1080_REG_CONFIG, 1, buffer, 2, 100);
	if (status != HAL_OK) return status; // Falla al configurar HDC1080
	osDelay(20); // Tiempo de configuración

	// ----- 4. Activar LTR390 -----
	// Poner en modo ALS (Ambient Light Sensor)
	// Escribir 0x02 en el registro MAIN_CTRL (0x00)
	buffer[0] = 0x02; // 0x02 = (Bit 1: Enable = 1, Bit 3: Mode = 0 (ALS))
	status = HAL_I2C_Mem_Write(hsensors->hi2c, LTR390_ADDR, LTR390_REG_MAIN_CTRL, 1, buffer, 1, 100);
	if (status != HAL_OK) return status; // Falla al activar LTR390
	osDelay(10);

	return HAL_OK;
}

// ---------------- CCS811 -----------------
HAL_StatusTypeDef CCS811_Read(Sensors_I2C_Handle_t *hsensors) {
    uint8_t buffer[8];
    HAL_StatusTypeDef status;

    // 1. Leer el registro de STATUS (0x00) para ver si hay datos listos
    status = HAL_I2C_Mem_Read(hsensors->hi2c, CCS811_ADDR, CCS811_REG_STATUS, 1, buffer, 1, 100);
    if (status != HAL_OK) {
        return status; // Falla al leer el status
    }

    // 2. Comprobar el bit DATA_READY
    //    Si no está listo, simplemente salimos sin actualizar los valores.
    //    Los valores antiguos (ej. 65021) se seguirán mostrando
    //    hasta que el sensor reporte datos nuevos.
    if (buffer[0] & CCS811_STATUS_DATA_READY)
    {
        // 3. ¡Datos listos! Leer los 8 bytes del registro de resultados (0x02)
        status = HAL_I2C_Mem_Read(hsensors->hi2c, CCS811_ADDR, CCS811_REG_ALG_RESULT_DATA, 1, buffer, 8, 100);

        if (status == HAL_OK) {
            // Actualizar los valores solo si la lectura fue exitosa
            hsensors->data.eco2 = (buffer[0] << 8) | buffer[1];
            hsensors->data.tvoc = (buffer[2] << 8) | buffer[3];
        }
        return status;
    }

    // Si llegamos aquí, no había datos listos (DATA_READY == 0)
    // Devolvemos HAL_OK porque no hubo un error de I2C,
    // simplemente no había datos nuevos.
    return HAL_OK;
}

// ---------------- MPU6050 -----------------
HAL_StatusTypeDef MPU6050_Read(Sensors_I2C_Handle_t *hsensors) {
	uint8_t buffer[14];
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hsensors->hi2c, MPU6050_ADDR, 0x3B, 1, buffer, 14, 100);
	if (status == HAL_OK) {
		hsensors->data.accel[0] = (int16_t)((buffer[0] << 8) | buffer[1]) / 16384.0f;
		hsensors->data.accel[1] = (int16_t)((buffer[2] << 8) | buffer[3]) / 16384.0f;
		hsensors->data.accel[2] = (int16_t)((buffer[4] << 8) | buffer[5]) / 16384.0f;
		hsensors->data.gyro[0]  = (int16_t)((buffer[8] << 8) | buffer[9]) / 131.0f;
		hsensors->data.gyro[1]  = (int16_t)((buffer[10] << 8) | buffer[11]) / 131.0f;
		hsensors->data.gyro[2]  = (int16_t)((buffer[12] << 8) | buffer[13]) / 131.0f;
	}
	return status;
}

// ---------------- HDC1080 -----------------
HAL_StatusTypeDef HDC1080_Read(Sensors_I2C_Handle_t *hsensors) {
	uint8_t buffer[4];
	HAL_StatusTypeDef status;

	// 1. Apuntar al registro de temperatura (0x00)
	buffer[0] = 0x00;
	status = HAL_I2C_Master_Transmit(hsensors->hi2c, HDC1080_ADDR, buffer, 1, 100);
	if (status != HAL_OK) {
		return status;
	}

	// 2. Esperar el tiempo de conversión (aprox 15ms para T+H)
	osDelay(20);

	// 3. Leer los 4 bytes (Temp MSB, Temp LSB, Hum MSB, Hum LSB)
	status = HAL_I2C_Master_Receive(hsensors->hi2c, HDC1080_ADDR, buffer, 4, 100);
	if (status == HAL_OK) {
		uint16_t raw_temp = (buffer[0] << 8) | buffer[1];
		uint16_t raw_hum  = (buffer[2] << 8) | buffer[3];

		hsensors->data.temperature = ((raw_temp / 65536.0f) * 165.0f) - 40.0f;
		hsensors->data.humidity    = (raw_hum / 65536.0f) * 100.0f;
	}
	return status;
}

// ---------------- LTR390 -----------------
/* HAL_StatusTypeDef LTR390_Read(Sensors_I2C_Handle_t *hsensors) {
	uint8_t buffer[3];
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hsensors->hi2c, LTR390_ADDR, 0x0D, 1, buffer, 3, 100);
	if (status == HAL_OK) {
		uint32_t raw_light = (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];
		hsensors->data.light = raw_light * 0.6f; // factor de conversión estimado
	}
	return status;
}*/

HAL_StatusTypeDef LTR390_Read(Sensors_I2C_Handle_t *hsensors) {
	extern uint32_t raw_light_debug; // Declara que usará la variable global
	uint8_t buffer[3];
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hsensors->hi2c, LTR390_ADDR, 0x0D, 1, buffer, 3, 100);
	if (status == HAL_OK) {
		uint32_t raw_light = (buffer[2] << 16) | (buffer[1] << 8) | buffer[0];
		raw_light_debug = raw_light; // <- AÑADE ESTA LÍNEA
		hsensors->data.light = raw_light * 0.6f; // factor de conversión estimado
	} else {
		raw_light_debug = 999999; // Error
	}
	return status;
}

/**
 * @brief Escribe los datos de temperatura y humedad en el CCS811
 * para compensación de lecturas.
 * @param hsensors: Handle de sensores
 * @param temp: Temperatura en °C
 * @param hum: Humedad relativa en %
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef CCS811_WriteEnvData(Sensors_I2C_Handle_t *hsensors, float temp, float hum)
{
	uint8_t buffer[4];

	// El sensor CCS811 espera los datos en un formato específico.
	// Humedad: (humedad * 512)
	// Temperatura: ((temperatura + 25) * 512)

	uint16_t hum_data = (uint16_t)(hum * 512.0f);
	uint16_t temp_data = (uint16_t)((temp + 25.0f) * 512.0f);

	// Formato: [HUM_MSB, HUM_LSB, TEMP_MSB, TEMP_LSB]
	buffer[0] = (hum_data >> 8) & 0xFF;
	buffer[1] = hum_data & 0xFF;
	buffer[2] = (temp_data >> 8) & 0xFF;
	buffer[3] = temp_data & 0xFF;

	// Escribir los 4 bytes en el registro ENV_DATA (0x05)
	return HAL_I2C_Mem_Write(hsensors->hi2c, CCS811_ADDR, CCS811_REG_ENV_DATA, 1, buffer, 4, 100);
}




