/*
 * I2C_Scanner.c
 *
 *  Created on: Oct 29, 2025
 *      Author: joadj
 */

/*
 * I2C_Scanner.c
 *
 * Escanea el bus I2C para detectar dispositivos conectados
 * Usar en main.c ANTES de inicializar IMU
 */

#include "main.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;  // I2C1 (PB6/PB7)
extern UART_HandleTypeDef huart1; // Para printf

/**
 * @brief Escanea el bus I2C buscando dispositivos
 * Imprime direcciones de dispositivos encontrados
 */
void I2C_Scanner(void)
{
    char msg[128];
    uint8_t devices_found = 0;

    sprintf(msg, "\r\n=== I2C Scanner ===\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    sprintf(msg, "Escaneando bus I2C...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    for (uint8_t addr = 1; addr < 128; addr++)
    {
        // Intentar comunicación con dispositivo
        HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(
            &hi2c1,  // I2C1
            addr << 1,  // Dirección de 7 bits shifted
            1,          // Número de intentos
            100         // Timeout en ms
        );

        if (result == HAL_OK)
        {
            sprintf(msg, "  Dispositivo encontrado en 0x%02X\r\n", addr);
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            devices_found++;
        }

        HAL_Delay(10);  // Delay entre escaneos
    }

    if (devices_found == 0)
    {
        sprintf(msg, "\r\n❌ No se encontraron dispositivos\r\n");
        sprintf(msg + strlen(msg), "Verificar:\r\n");
        sprintf(msg + strlen(msg), "  - Conexiones SDA/SCL\r\n");
        sprintf(msg + strlen(msg), "  - Alimentación del módulo\r\n");
        sprintf(msg + strlen(msg), "  - Pull-ups en SDA/SCL\r\n");
    }
    else
    {
        sprintf(msg, "\r\n✓ Total dispositivos: %d\r\n", devices_found);
    }

    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    sprintf(msg, "===================\r\n\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}

/**
 * @brief Verifica específicamente el MPU6050
 * @return 1 si se detectó, 0 si no
 */
uint8_t I2C_Check_MPU6050(void)
{
    char msg[128];

    // Dirección estándar del MPU6050
    uint8_t mpu6050_addr = 0x68;

    sprintf(msg, "Verificando MPU6050 en 0x68...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    HAL_StatusTypeDef result = HAL_I2C_IsDeviceReady(
        &hi2c1,  // I2C1
        mpu6050_addr << 1,
        3,
        100
    );

    if (result == HAL_OK)
    {
        sprintf(msg, "✓ MPU6050 detectado en 0x68\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        return 1;
    }

    // Intentar con dirección alternativa (AD0=HIGH)
    mpu6050_addr = 0x69;
    sprintf(msg, "Intentando 0x69...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    result = HAL_I2C_IsDeviceReady(
        &hi2c1,  // I2C1
        mpu6050_addr << 1,
        3,
        100
    );

    if (result == HAL_OK)
    {
        sprintf(msg, "✓ MPU6050 detectado en 0x69\r\n");
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
        return 1;
    }

    sprintf(msg, "❌ MPU6050 NO detectado\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    return 0;
}

/**
 * @brief Test básico de lectura del WHO_AM_I register
 */
void I2C_Test_MPU6050_WhoAmI(void)
{
    char msg[128];
    uint8_t who_am_i = 0;
    uint8_t mpu6050_addr = 0x68;
    uint8_t who_am_i_reg = 0x75;  // WHO_AM_I register del MPU6050

    sprintf(msg, "\r\nLeyendo WHO_AM_I register...\r\n");
    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

    // Leer registro WHO_AM_I
    HAL_StatusTypeDef result = HAL_I2C_Mem_Read(
        &hi2c1,  // I2C1
        mpu6050_addr << 1,
        who_am_i_reg,
        1,
        &who_am_i,
        1,
        100
    );

    if (result == HAL_OK)
    {
        sprintf(msg, "WHO_AM_I = 0x%02X ", who_am_i);
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);

        if (who_am_i == 0x68)
        {
            sprintf(msg, "✓ (MPU6050 correcto)\r\n");
        }
        else if (who_am_i == 0x71)
        {
            sprintf(msg, "✓ (MPU6000/MPU9150)\r\n");
        }
        else if (who_am_i == 0x73)
        {
            sprintf(msg, "✓ (MPU9250)\r\n");
        }
        else
        {
            sprintf(msg, "❌ (Valor inesperado)\r\n");
        }
    }
    else
    {
        sprintf(msg, "❌ Error leyendo WHO_AM_I (HAL error: %d)\r\n", result);
    }

    HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
}
