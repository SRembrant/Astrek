/*
 * Gases.c
 *
 *  Created on: Jul 26, 2025
 *      Author: joadj
 */

#include "Gases.h"
#include <string.h>

/* Private function prototypes */
static CCS811_Status_t CCS811_WriteRegister(CCS811_Handle_t *handle, uint8_t reg, uint8_t *data, uint8_t len);
static CCS811_Status_t CCS811_ReadRegister(CCS811_Handle_t *handle, uint8_t reg, uint8_t *data, uint8_t len);
static CCS811_Status_t CCS811_CheckHardwareID(CCS811_Handle_t *handle);
static CCS811_Status_t CCS811_StartApp(CCS811_Handle_t *handle);
static CCS811_Status_t CCS811_SetMeasurementMode(CCS811_Handle_t *handle, uint8_t mode);

/**
 * @brief Initialize the CCS811 sensor
 */
CCS811_Status_t CCS811_Init(CCS811_Handle_t *handle, CCS811_Config_t *config)
{
    if (handle == NULL || config == NULL || config->hi2c == NULL) {
        return CCS811_ERROR_INIT;
    }

    // Copy configuration
    handle->config = *config;
    handle->initialized = false;
    handle->last_measurement_time = 0;

    // Reset data structure
    memset(&handle->data, 0, sizeof(CCS811_Data_t));

    // Wait for sensor startup
    HAL_Delay(CCS811_STARTUP_DELAY);

    // Check hardware ID
    if (CCS811_CheckHardwareID(handle) != CCS811_OK) {
        return CCS811_ERROR_DEVICE_ID;
    }

    // Check if app is valid
    uint8_t status;
    if (CCS811_ReadRegister(handle, CCS811_REG_STATUS, &status, 1) != CCS811_OK) {
        return CCS811_ERROR_I2C;
    }

    if (!(status & CCS811_STATUS_APP_VALID)) {
        return CCS811_ERROR_APP_NOT_VALID;
    }

    // Start the app
    if (CCS811_StartApp(handle) != CCS811_OK) {
        return CCS811_ERROR_INIT;
    }

    // Wait for app to start
    HAL_Delay(CCS811_APP_START_DELAY);

    // Set measurement mode
    if (CCS811_SetMeasurementMode(handle, config->measurement_mode) != CCS811_OK) {
        return CCS811_ERROR_INIT;
    }

    // Set baseline if provided
    if (config->baseline != 0) {
        CCS811_SetBaseline(handle, config->baseline);
    }

    handle->initialized = true;
    return CCS811_OK;
}

/**
 * @brief Read measurement data from CCS811
 */
CCS811_Status_t CCS811_ReadData(CCS811_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return CCS811_ERROR_INIT;
    }

    uint8_t status;
    uint8_t data[8];

    // Check status
    if (CCS811_ReadRegister(handle, CCS811_REG_STATUS, &status, 1) != CCS811_OK) {
        return CCS811_ERROR_I2C;
    }

    handle->data.status = status;

    // Check for errors
    if (status & CCS811_STATUS_ERROR) {
        uint8_t error_id;
        CCS811_ReadRegister(handle, CCS811_REG_ERROR_ID, &error_id, 1);
        handle->data.error_id = error_id;

        if (handle->config.error_callback != NULL) {
            handle->config.error_callback(error_id);
        }
        return CCS811_ERROR_SENSOR_ERROR;
    }

    // Check if data is ready
    if (!(status & CCS811_STATUS_DATA_READY)) {
        handle->data.data_ready = false;
        return CCS811_ERROR_NO_DATA;
    }

    // Read algorithm results
    if (CCS811_ReadRegister(handle, CCS811_REG_ALG_RESULT, data, 8) != CCS811_OK) {
        return CCS811_ERROR_I2C;
    }

    // Parse data
    handle->data.co2_ppm = (uint16_t)((data[0] << 8) | data[1]);
    handle->data.tvoc_ppb = (uint16_t)((data[2] << 8) | data[3]);
    handle->data.status = data[4];
    handle->data.error_id = data[5];
    handle->data.raw_current = (uint16_t)((data[6] >> 2));
    handle->data.raw_voltage = (uint16_t)(((data[6] & 0x03) << 8) | data[7]);
    handle->data.data_ready = true;

    handle->last_measurement_time = HAL_GetTick();

    return CCS811_OK;
}

/**
 * @brief Check if new data is available
 */
bool CCS811_IsDataReady(CCS811_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return false;
    }

    uint8_t status;
    if (CCS811_ReadRegister(handle, CCS811_REG_STATUS, &status, 1) != CCS811_OK) {
        return false;
    }

    return (status & CCS811_STATUS_DATA_READY) ? true : false;
}

/**
 * @brief Get the latest CO2 reading
 */
uint16_t CCS811_GetCO2(CCS811_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0;
    }
    return handle->data.co2_ppm;
}

/**
 * @brief Get the latest TVOC reading
 */
uint16_t CCS811_GetTVOC(CCS811_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0;
    }
    return handle->data.tvoc_ppb;
}

/**
 * @brief Set environmental data for compensation
 */
CCS811_Status_t CCS811_SetEnvironmentalData(CCS811_Handle_t *handle, float humidity, float temperature)
{
    if (handle == NULL || !handle->initialized) {
        return CCS811_ERROR_INIT;
    }

    // Convert to CCS811 format
    uint16_t hum = (uint16_t)(humidity * 512);  // 0.5% resolution
    uint16_t temp = (uint16_t)((temperature + 25) * 512);  // 0.5°C resolution, offset by 25°C

    uint8_t env_data[4] = {
        (uint8_t)(hum >> 8),
        (uint8_t)(hum & 0xFF),
        (uint8_t)(temp >> 8),
        (uint8_t)(temp & 0xFF)
    };

    return CCS811_WriteRegister(handle, CCS811_REG_ENV_DATA, env_data, 4);
}

/**
 * @brief Get baseline value for calibration
 */
CCS811_Status_t CCS811_GetBaseline(CCS811_Handle_t *handle, uint16_t *baseline)
{
    if (handle == NULL || !handle->initialized || baseline == NULL) {
        return CCS811_ERROR_INIT;
    }

    uint8_t data[2];
    CCS811_Status_t status = CCS811_ReadRegister(handle, CCS811_REG_BASELINE, data, 2);

    if (status == CCS811_OK) {
        *baseline = (uint16_t)((data[0] << 8) | data[1]);
    }

    return status;
}

/**
 * @brief Set baseline value for calibration
 */
CCS811_Status_t CCS811_SetBaseline(CCS811_Handle_t *handle, uint16_t baseline)
{
    if (handle == NULL || !handle->initialized) {
        return CCS811_ERROR_INIT;
    }

    uint8_t data[2] = {
        (uint8_t)(baseline >> 8),
        (uint8_t)(baseline & 0xFF)
    };

    return CCS811_WriteRegister(handle, CCS811_REG_BASELINE, data, 2);
}

/**
 * @brief Software reset of the CCS811
 */
CCS811_Status_t CCS811_SoftwareReset(CCS811_Handle_t *handle)
{
    if (handle == NULL) {
        return CCS811_ERROR_INIT;
    }

    uint8_t reset_sequence[4] = {0x11, 0xE5, 0x72, 0x8A};
    CCS811_Status_t status = CCS811_WriteRegister(handle, CCS811_REG_SW_RESET, reset_sequence, 4);

    if (status == CCS811_OK) {
        HAL_Delay(CCS811_STARTUP_DELAY);
        handle->initialized = false;
    }

    return status;
}

/**
 * @brief Get error status from the sensor
 */
uint8_t CCS811_GetErrorStatus(CCS811_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return 0xFF;
    }
    return handle->data.error_id;
}

/**
 * @brief Check if sensor has an error
 */
bool CCS811_HasError(CCS811_Handle_t *handle)
{
    if (handle == NULL || !handle->initialized) {
        return true;
    }
    return (handle->data.status & CCS811_STATUS_ERROR) ? true : false;
}

/* Private Functions */

/**
 * @brief Write data to CCS811 register
 */
static CCS811_Status_t CCS811_WriteRegister(CCS811_Handle_t *handle, uint8_t reg, uint8_t *data, uint8_t len)
{
    HAL_StatusTypeDef hal_status;

    if (len == 0) {
        // Write only register address (for commands like APP_START)
        hal_status = HAL_I2C_Master_Transmit(handle->config.hi2c,
                                           handle->config.device_address << 1,
                                           &reg, 1, CCS811_I2C_TIMEOUT);
    } else {
        // Write register address followed by data
        uint8_t buffer[len + 1];
        buffer[0] = reg;
        memcpy(&buffer[1], data, len);

        hal_status = HAL_I2C_Master_Transmit(handle->config.hi2c,
                                           handle->config.device_address << 1,
                                           buffer, len + 1, CCS811_I2C_TIMEOUT);
    }

    return (hal_status == HAL_OK) ? CCS811_OK : CCS811_ERROR_I2C;
}

/**
 * @brief Read data from CCS811 register
 */
static CCS811_Status_t CCS811_ReadRegister(CCS811_Handle_t *handle, uint8_t reg, uint8_t *data, uint8_t len)
{
    HAL_StatusTypeDef hal_status;

    // Write register address
    hal_status = HAL_I2C_Master_Transmit(handle->config.hi2c,
                                        handle->config.device_address << 1,
                                        &reg, 1, CCS811_I2C_TIMEOUT);
    if (hal_status != HAL_OK) {
        return CCS811_ERROR_I2C;
    }

    // Read data
    hal_status = HAL_I2C_Master_Receive(handle->config.hi2c,
                                       handle->config.device_address << 1,
                                       data, len, CCS811_I2C_TIMEOUT);

    return (hal_status == HAL_OK) ? CCS811_OK : CCS811_ERROR_I2C;
}

/**
 * @brief Check CCS811 hardware ID
 */
static CCS811_Status_t CCS811_CheckHardwareID(CCS811_Handle_t *handle)
{
    uint8_t hw_id;

    if (CCS811_ReadRegister(handle, CCS811_REG_HW_ID, &hw_id, 1) != CCS811_OK) {
        return CCS811_ERROR_I2C;
    }

    return (hw_id == CCS811_HW_ID_VALUE) ? CCS811_OK : CCS811_ERROR_DEVICE_ID;
}

/**
 * @brief Start the CCS811 application
 */
static CCS811_Status_t CCS811_StartApp(CCS811_Handle_t *handle)
{
    return CCS811_WriteRegister(handle, CCS811_REG_APP_START, NULL, 0);
}

/**
 * @brief Set measurement mode
 */
static CCS811_Status_t CCS811_SetMeasurementMode(CCS811_Handle_t *handle, uint8_t mode)
{
    uint8_t meas_mode = mode;

    if (handle->config.interrupt_enabled) {
        meas_mode |= (1 << 3);  // Enable interrupt
    }

    return CCS811_WriteRegister(handle, CCS811_REG_MEAS_MODE, &meas_mode, 1);
}



