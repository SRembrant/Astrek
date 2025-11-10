/*
 * IMU.c
 *
 *  Created on: Jul 22, 2025
 *      Author: EdamVelas
 */

#include "IMU.h"
#include "math.h"
#include <string.h>

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;

// Funciones I2C
HAL_StatusTypeDef MPU9250_WriteRegister(uint8_t reg, uint8_t value) {
    return HAL_I2C_Mem_Write(&hi2c2, DEVICE_ADDRESS << 1, reg, 1, &value, 1, 100);
}

HAL_StatusTypeDef MPU9250_ReadRegister(uint8_t reg, uint8_t *buffer, uint8_t length) {
    return HAL_I2C_Mem_Read(&hi2c2, DEVICE_ADDRESS << 1, reg, 1, buffer, length, 100);
}
static uint16_t read16_LE(BMP280_t *bmp, uint8_t reg) {
    uint8_t data[2];
    HAL_I2C_Mem_Read(bmp->hi2c, bmp->address, reg, 1, data, 2, 100);
    return (uint16_t)(data[0] | (data[1] << 8));
}

static int16_t readS16_LE(BMP280_t *bmp, uint8_t reg) {
    return (int16_t)read16_LE(bmp, reg);
}

// Inicialización
void MPU9250_Init() {
    if (HAL_I2C_IsDeviceReady(&hi2c2, DEVICE_ADDRESS << 1, 1, 100) != HAL_OK) {
        printf("MPU9250 no conectado.\n");
        return;
    }

    // Inicializar acelerómetro y giroscopio
    MPU9250_WriteRegister(REG_CONF_GIROSCOPE, FS_SEL_GIRO_500);
    MPU9250_WriteRegister(REG_CONF_ACCELEROMETER, FS_SEL_ACCEL_16G);


    //MAGNETOMETRO
    HAL_Delay(100);
    // Habilitar modo bypass para acceso al magnetómetro AK8963
        MPU9250_WriteRegister(0x37, 0x02);  // INT_PIN_CFG
        HAL_Delay(10);
        printf("MPU9250 inicializado y modo bypass habilitado.\n");
        uint8_t check = 0;
          HAL_I2C_Mem_Read(&hi2c2, AK8963_ADDRESS << 1, AK8963_WHO_AM_I, 1, &check, 1, 100);
          if (check != 0x48) {
              printf("AK8963 no detectado. WHO_AM_I = 0x%02X\n", check);
              return;
          }
          // Configurar en modo continuo de lectura a 100Hz con resolución 16-bit
            HAL_I2C_Mem_Write(&hi2c2, AK8963_ADDRESS << 1, AK8963_CNTL1, 1, (uint8_t[]){AK8963_MODE_CONTINUOUS}, 1, 100);
            HAL_Delay(10);


}
	void BMP280_Init(BMP280_t *bmp, I2C_HandleTypeDef *hi2c) {
    bmp->hi2c = hi2c;
    bmp->address = BMP280_I2C_ADDR;

    uint8_t id;
    HAL_I2C_Mem_Read(hi2c, bmp->address, BMP280_REG_ID, 1, &id, 1, 100);
    if (id != 0x58) return;

    HAL_I2C_Mem_Write(hi2c, bmp->address, BMP280_REG_RESET, 1, (uint8_t[]){0xB6}, 1, 100);
    HAL_Delay(100);

    bmp->dig_T1 = read16_LE(bmp, 0x88);
    bmp->dig_T2 = readS16_LE(bmp, 0x8A);
    bmp->dig_T3 = readS16_LE(bmp, 0x8C);
    bmp->dig_P1 = read16_LE(bmp, 0x8E);
    bmp->dig_P2 = readS16_LE(bmp, 0x90);
    bmp->dig_P3 = readS16_LE(bmp, 0x92);
    bmp->dig_P4 = readS16_LE(bmp, 0x94);
    bmp->dig_P5 = readS16_LE(bmp, 0x96);
    bmp->dig_P6 = readS16_LE(bmp, 0x98);
    bmp->dig_P7 = readS16_LE(bmp, 0x9A);
    bmp->dig_P8 = readS16_LE(bmp, 0x9C);
    bmp->dig_P9 = readS16_LE(bmp, 0x9E);

    uint8_t ctrl_meas = (1 << 5) | (1 << 2) | 3; // temp x1, press x1, normal mode
    uint8_t config = (5 << 5) | (0 << 2) | 0;    // standby 1000ms, filter off

    HAL_I2C_Mem_Write(hi2c, bmp->address, BMP280_REG_CONFIG, 1, &config, 1, 100);
    HAL_I2C_Mem_Write(hi2c, bmp->address, BMP280_REG_CTRL_MEAS, 1, &ctrl_meas, 1, 100);


}




// Lectura de todos los sensores

HAL_StatusTypeDef MPU9250_ReadAll(MPU9250_Data *data) {
    uint8_t buffer[14];
    if (MPU9250_ReadRegister(REG_DATA_ACCEL_X_H, buffer, 14) != HAL_OK) return HAL_ERROR;

    data->accel_x = (int16_t)(buffer[0] << 8 | buffer[1]) * ACCEL_SCALE_16G;
    data->accel_y = (int16_t)(buffer[2] << 8 | buffer[3]) * ACCEL_SCALE_16G;
    data->accel_z = (int16_t)(buffer[4] << 8 | buffer[5]) * ACCEL_SCALE_16G;

    data->gyro_x  = (int16_t)(buffer[8] << 8 | buffer[9]) * GYRO_SCALE_500DPS;
    data->gyro_y  = (int16_t)(buffer[10] << 8 | buffer[11]) * GYRO_SCALE_500DPS;
    data->gyro_z  = (int16_t)(buffer[12] << 8 | buffer[13]) * GYRO_SCALE_500DPS;

    data->AccTotal = sqrtf(data->accel_x * data->accel_x +
                           data->accel_y * data->accel_y +
                           data->accel_z * data->accel_z);
    return HAL_OK;

    // Leer magnetómetro si encontramos forma

    /*
    uint8_t mag_status;
    HAL_I2C_Mem_Read(&hi2c2, AK8963_ADDRESS << 1, AK8963_ST1, 1, &mag_status, 1, 100);

    if (mag_status & 0x01) { // Datos nuevos disponibles
        uint8_t mag_buffer[7];
        HAL_I2C_Mem_Read(&hi2c2, AK8963_ADDRESS << 1, AK8963_DATA, 1, mag_buffer, 7, 100);

        int16_t mag_x = (int16_t)(mag_buffer[1] << 8 | mag_buffer[0]);
        int16_t mag_y = (int16_t)(mag_buffer[3] << 8 | mag_buffer[2]);
        int16_t mag_z = (int16_t)(mag_buffer[5] << 8 | mag_buffer[4]);

        data->mag_x = mag_x * 0.15f;  // Conversión a µT
        data->mag_y = mag_y * 0.15f;
        data->mag_z = mag_z * 0.15f;

        // Leer ST2 para liberar el buffer
        uint8_t dummy;
        HAL_I2C_Mem_Read(&hi2c2, AK8963_ADDRESS << 1, AK8963_ST2, 1, &dummy, 1, 100);
    }*/


    // 	Descomentar para ver aceleracion sin filtros por uart 1

    //	printf("Accel: X=%.2f Y=%.2f Z=%.2f | Gyro: X=%.2f Y=%.2f Z=%.2f  Accel Total=%.2f\n",
    //  data->accel_x, data->accel_y, data->accel_z,
    //           data->gyro_x, data->gyro_y, data->gyro_z,data->AccTotal);

   // char px1[300];
   // sprintf(px1,"Datos sin filtro Acel %.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",data->accel_x,data->accel_y,data->accel_z,data->gyro_x,data->gyro_y,data->gyro_z,data->AccTotal);
   //	HAL_UART_Transmit(&huart1, (uint8_t*)px1, strlen(px1), HAL_MAX_DELAY);
}



HAL_StatusTypeDef BMP280_Read(BMP280_t *bmp, MPU9250_Data *data) {
    uint8_t dataaux[6];
    HAL_I2C_Mem_Read(bmp->hi2c, bmp->address, BMP280_REG_PRESS_MSB, 1, dataaux, 6, 100);

    int32_t adc_P = (int32_t)((dataaux[0] << 12) | (dataaux[1] << 4) | (dataaux[2] >> 4));
    int32_t adc_T = (int32_t)((dataaux[3] << 12) | (dataaux[4] << 4) | (dataaux[5] >> 4));

    int32_t var1 = ((((adc_T >> 3) - ((int32_t)bmp->dig_T1 << 1))) * ((int32_t)bmp->dig_T2)) >> 11;
    int32_t var2 = (((((adc_T >> 4) - ((int32_t)bmp->dig_T1)) * ((adc_T >> 4) - ((int32_t)bmp->dig_T1))) >> 12) * ((int32_t)bmp->dig_T3)) >> 14;
    bmp->t_fine = var1 + var2;

    data->Temperature = (bmp->t_fine * 5 + 128) >> 8;
    data->Temperature /= 100.0f;

    int64_t var1p = ((int64_t)bmp->t_fine) - 128000;
    int64_t var2p = var1p * var1p * (int64_t)bmp->dig_P6;
    var2p = var2p + ((var1p * (int64_t)bmp->dig_P5) << 17);
    var2p = var2p + (((int64_t)bmp->dig_P4) << 35);
    var1p = ((var1p * var1p * (int64_t)bmp->dig_P3) >> 8) + ((var1p * (int64_t)bmp->dig_P2) << 12);
    var1p = (((((int64_t)1) << 47) + var1p)) * ((int64_t)bmp->dig_P1) >> 33;

    if (var1p == 0) return HAL_OK;

    int64_t p = 1048576 - adc_P;
    p = (((p << 31) - var2p) * 3125) / var1p;
    var1p = (((int64_t)bmp->dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2p = (((int64_t)bmp->dig_P8) * p) >> 19;

    data->Presure = ((p + var1p + var2p) >> 8) + (((int64_t)bmp->dig_P7) << 4);
    data->Presure /= 256.0f;
    return HAL_OK;
}

//Immplementacion del filtro de kalman


static KalmanFilter kf_accel_x, kf_accel_y, kf_accel_z;

void KalmanFilter_Init(KalmanFilter *kf, float Q, float R, float P, float initial_value) {
    kf->Q = Q;
    kf->R = R;
    kf->P = P;
    kf->X = initial_value;
}

float KalmanFilter_Update(KalmanFilter *kf, float measurement) {
    // Predicción
    kf->P += kf->Q;

    // Actualización
    kf->K = kf->P / (kf->P + kf->R);
    kf->X += kf->K * (measurement - kf->X);
    kf->P *= (1 - kf->K);

    return kf->X;
}

void MPU9250_InitWithKalman() {
    KalmanFilter_Init(&kf_accel_x, 0.001, 0.1, 0.1, 0);
    KalmanFilter_Init(&kf_accel_y, 0.001, 0.1, 0.1, 0);
    KalmanFilter_Init(&kf_accel_z, 0.001, 0.1, 0.1, 0);
}

void MPU9250_ReadAllWithKalman(MPU9250_Data *data) {
  //Mejorar podemos hacer una estructura de datos filtrados
    float accel_x_filtered = KalmanFilter_Update(&kf_accel_x, data->accel_x);
    float accel_y_filtered = KalmanFilter_Update(&kf_accel_y, data->accel_y);
    float accel_z_filtered = KalmanFilter_Update(&kf_accel_z, data->accel_z);
    float Aceltotal = sqrt(accel_x_filtered * accel_x_filtered
                         + accel_y_filtered * accel_y_filtered
                         + accel_z_filtered * accel_z_filtered);

    // 📡 Enviar datos por UART
   // char buffer[100];
   // sprintf(buffer, "Datos con filro Acel 1   %.2f,%.2f,%.2f,%.2f\n", accel_x_filtered, accel_y_filtered, accel_z_filtered,Aceltotal);
   // HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}


char* GY91_printfData(MPU9250_Data *data) {
    static char msg[128];  // 'static' mantiene el array después de que la función termine

    snprintf(msg, sizeof(msg),
        "Acel: %.2f, %.2f, %.2f | Gyro: %.2f, %.2f, %.2f | MAG: %.2f, %.2f, %.2f | Temp: %.2f | Presion: %.2f | AccTot: %.2f",
        data->accel_x, data->accel_y, data->accel_z,
        data->gyro_x, data->gyro_y, data->gyro_z,
        data->mag_x, data->mag_y, data->mag_z,
        data->Temperature, data->Presure, data->AccTotal
    );

    return msg;
}


