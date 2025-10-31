/*
 * Gases.h
 *
 *  Created on: Jul 26, 2025
 *      Author: joadj
 */

#ifndef INC_SENSORES_GASES_H_
#define INC_SENSORES_GASES_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* CCS811 I2C Addresses */
#define CCS811_I2C_ADDR_LOW     0x5A    // ADDR pin connected to GND
#define CCS811_I2C_ADDR_HIGH    0x5B    // ADDR pin connected to VDD

/* CCS811 Register Addresses */
#define CCS811_REG_STATUS       0x00
#define CCS811_REG_MEAS_MODE    0x01
#define CCS811_REG_ALG_RESULT   0x02
#define CCS811_REG_RAW_DATA     0x03
#define CCS811_REG_ENV_DATA     0x05
#define CCS811_REG_NTC          0x06
#define CCS811_REG_THRESHOLDS   0x10
#define CCS811_REG_BASELINE     0x11
#define CCS811_REG_HW_ID        0x20
#define CCS811_REG_HW_VERSION   0x21
#define CCS811_REG_FW_BOOT_VER  0x23
#define CCS811_REG_FW_APP_VER   0x24
#define CCS811_REG_ERROR_ID     0xE0
#define CCS811_REG_APP_START    0xF4
#define CCS811_REG_SW_RESET     0xFF

/* CCS811 Status Register Bits */
#define CCS811_STATUS_ERROR         (1 << 0)
#define CCS811_STATUS_DATA_READY    (1 << 3)
#define CCS811_STATUS_APP_VALID     (1 << 4)
#define CCS811_STATUS_FW_MODE       (1 << 7)

/* CCS811 Measurement Modes */
#define CCS811_MODE_IDLE            0x00
#define CCS811_MODE_1SEC            0x10    // Measurement every second
#define CCS811_MODE_10SEC           0x20    // Measurement every 10 seconds
#define CCS811_MODE_60SEC           0x30    // Measurement every 60 seconds
#define CCS811_MODE_250MS           0x40    // Measurement every 250ms

/* CCS811 Error Codes */
#define CCS811_ERROR_WRITE_REG      (1 << 0)
#define CCS811_ERROR_READ_REG       (1 << 1)
#define CCS811_ERROR_MEASURE_MODE   (1 << 2)
#define CCS811_ERROR_MAX_RESISTANCE (1 << 3)
#define CCS811_ERROR_HEATER_FAULT   (1 << 4)
#define CCS811_ERROR_HEATER_SUPPLY  (1 << 5)

/* Hardware ID and expected values */
#define CCS811_HW_ID_VALUE          0x81

/* Timeout values */
#define CCS811_I2C_TIMEOUT          1000
#define CCS811_STARTUP_DELAY        20      // 20ms startup delay
#define CCS811_APP_START_DELAY      1       // 1ms delay after app start

/**
 * @brief CCS811 Status enumeration
 */
typedef enum {
    CCS811_OK = 0,
    CCS811_ERROR_INIT,
    CCS811_ERROR_I2C,
    CCS811_ERROR_DEVICE_ID,
    CCS811_ERROR_APP_NOT_VALID,
    CCS811_ERROR_NO_DATA,
    CCS811_ERROR_SENSOR_ERROR,
    CCS811_TIMEOUT
} CCS811_Status_t;

/**
 * @brief CCS811 measurement data structure
 */
typedef struct {
    uint16_t co2_ppm;           // CO2 concentration in ppm
    uint16_t tvoc_ppb;          // TVOC concentration in ppb
    uint8_t status;             // Status register value
    uint8_t error_id;           // Error ID if any
    uint16_t raw_current;       // Raw ADC reading for current
    uint16_t raw_voltage;       // Raw ADC reading for voltage
    bool data_ready;            // Flag indicating if new data is available
} CCS811_Data_t;

/**
 * @brief CCS811 configuration structure
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;           // I2C handle
    uint8_t device_address;            // I2C device address (7-bit)
    uint8_t measurement_mode;          // Measurement mode
    uint16_t baseline;                 // Baseline value for calibration
    bool interrupt_enabled;            // Enable/disable interrupt
    void (*error_callback)(uint8_t error_code);  // Error callback function
} CCS811_Config_t;

/**
 * @brief CCS811 handle structure
 */
typedef struct {
    CCS811_Config_t config;            // Configuration
    CCS811_Data_t data;                // Latest measurement data
    bool initialized;                  // Initialization flag
    uint32_t last_measurement_time;    // Last measurement timestamp
} CCS811_Handle_t;

/* Function Prototypes */

/**
 * @brief Initialize the CCS811 sensor
 * @param handle Pointer to CCS811 handle structure
 * @param config Pointer to configuration structure
 * @return CCS811_Status_t Status of initialization
 */
CCS811_Status_t CCS811_Init(CCS811_Handle_t *handle, CCS811_Config_t *config);

/**
 * @brief Read measurement data from CCS811
 * @param handle Pointer to CCS811 handle
 * @return CCS811_Status_t Status of the operation
 */
CCS811_Status_t CCS811_ReadData(CCS811_Handle_t *handle);

/**
 * @brief Check if new data is available
 * @param handle Pointer to CCS811 handle
 * @return bool True if data is ready, false otherwise
 */
bool CCS811_IsDataReady(CCS811_Handle_t *handle);

/**
 * @brief Get the latest CO2 reading
 * @param handle Pointer to CCS811 handle
 * @return uint16_t CO2 concentration in ppm
 */
uint16_t CCS811_GetCO2(CCS811_Handle_t *handle);

/**
 * @brief Get the latest TVOC reading
 * @param handle Pointer to CCS811 handle
 * @return uint16_t TVOC concentration in ppb
 */
uint16_t CCS811_GetTVOC(CCS811_Handle_t *handle);

/**
 * @brief Set environmental data for compensation
 * @param handle Pointer to CCS811 handle
 * @param humidity Relative humidity (0.5% resolution, 0-100%)
 * @param temperature Temperature in Celsius (0.5°C resolution)
 * @return CCS811_Status_t Status of the operation
 */
CCS811_Status_t CCS811_SetEnvironmentalData(CCS811_Handle_t *handle, float humidity, float temperature);

/**
 * @brief Get baseline value for calibration
 * @param handle Pointer to CCS811 handle
 * @param baseline Pointer to store baseline value
 * @return CCS811_Status_t Status of the operation
 */
CCS811_Status_t CCS811_GetBaseline(CCS811_Handle_t *handle, uint16_t *baseline);

/**
 * @brief Set baseline value for calibration
 * @param handle Pointer to CCS811 handle
 * @param baseline Baseline value to set
 * @return CCS811_Status_t Status of the operation
 */
CCS811_Status_t CCS811_SetBaseline(CCS811_Handle_t *handle, uint16_t baseline);

/**
 * @brief Software reset of the CCS811
 * @param handle Pointer to CCS811 handle
 * @return CCS811_Status_t Status of the operation
 */
CCS811_Status_t CCS811_SoftwareReset(CCS811_Handle_t *handle);

/**
 * @brief Get error status from the sensor
 * @param handle Pointer to CCS811 handle
 * @return uint8_t Error code
 */
uint8_t CCS811_GetErrorStatus(CCS811_Handle_t *handle);

/**
 * @brief Check if sensor has an error
 * @param handle Pointer to CCS811 handle
 * @return bool True if error present, false otherwise
 */
bool CCS811_HasError(CCS811_Handle_t *handle);



#endif /* INC_SENSORES_GASES_H_ */
