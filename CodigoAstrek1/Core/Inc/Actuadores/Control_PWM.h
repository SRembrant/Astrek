/*
 * Control_PWM.h
 *
 *  Created on: Oct 24, 2025
 *      Author: joadj
 */

#ifndef INC_ACTUADORES_CONTROL_PWM_H_
#define INC_ACTUADORES_CONTROL_PWM_H_

#include <stdint.h>
#include <math.h>
#include "Control_Kinematics.h"

// ============================================================================
// CONFIGURACIÓN PWM
// ============================================================================

#define PWM_TIMER_ARR        1000     // Auto-reload del timer (0-1000)
#define PWM_MIN_DUTY         300      // 30% - zona muerta mínima
#define PWM_MAX_DUTY         800      // 80% - límite de seguridad

// Voltaje de referencia de caracterización
#define PWM_VOLTAGE_REF      7.6f     // Voltaje de tu tabla

// ============================================================================
// LOOK-UP TABLE (LUT) - BASADA EN TU CARACTERIZACIÓN
// ============================================================================

// Puntos de calibración [RPM, PWM_duty]
#define LUT_SIZE 12

typedef struct {
    float rpm;
    uint16_t pwm;
} LUT_Point_t;

// Tabla de calibración (extraída de tu caracterización)
static const LUT_Point_t PWM_LUT[LUT_SIZE] = {
    {0.0f,    0},      // Parado
    {84.0f,   300},    // Inicio de movimiento (30%)
    {200.0f,  350},    // 35%
    {310.0f,  400},    // 40%
    {400.0f,  450},    // 45%
    {470.0f,  500},    // 50%
    {650.0f,  550},    // 55%
    {750.0f,  600},    // 60%
    {830.0f,  650},    // 65%
    {870.0f,  700},    // 70%
    {1000.0f, 800},    // 80%
    {1150.0f, 900}     // 90% (extrapolación)
};

// ============================================================================
// ESTRUCTURAS
// ============================================================================

/**
 * @brief Comandos PWM para motores
 */
typedef struct {
    uint16_t pwm_right;    // PWM rueda derecha [0-1000]
    uint16_t pwm_left;     // PWM rueda izquierda [0-1000]
    uint8_t dir_right;     // Dirección: 0=adelante, 1=atrás
    uint8_t dir_left;      // Dirección: 0=adelante, 1=atrás
} PWM_Commands_t;

/**
 * @brief Configuración de compensaciones
 */
typedef struct {
    float battery_voltage;      // Voltaje actual de batería
    float pitch_angle;          // Ángulo de inclinación (rad)
    float compensation_factor;  // Factor de compensación total
    uint8_t enable_battery_comp;
    uint8_t enable_slope_comp;
} PWM_Compensation_t;

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

/**
 * @brief Convierte velocidades de ruedas a comandos PWM
 *
 * @param wheels Velocidades deseadas [m/s]
 * @param comp Compensaciones (puede ser NULL)
 * @return Comandos PWM calculados
 */
PWM_Commands_t PWM_FromWheelVelocities(WheelVelocities_t wheels,
                                       PWM_Compensation_t *comp);

/**
 * @brief Mapea velocidad [m/s] a PWM usando LUT con interpolación
 *
 * @param velocity Velocidad deseada [m/s]
 * @return PWM duty cycle [0-1000]
 */
uint16_t PWM_VelocityToDuty(float velocity);

/**
 * @brief Mapea RPM a PWM usando LUT con interpolación lineal
 *
 * @param rpm RPM deseado
 * @return PWM duty cycle [0-1000]
 */
uint16_t PWM_RPMToDuty(float rpm);

/**
 * @brief Aplica compensación por voltaje de batería
 *
 * @param pwm PWM base
 * @param voltage Voltaje actual de batería
 * @return PWM compensado
 */
uint16_t PWM_CompensateBattery(uint16_t pwm, float voltage);

/**
 * @brief Aplica compensación por pendiente (pitch)
 *
 * @param pwm PWM base
 * @param pitch Ángulo de inclinación en radianes
 * @return PWM compensado
 */
uint16_t PWM_CompensateSlope(uint16_t pwm, float pitch);

/**
 * @brief Inicializa configuración de compensaciones por defecto
 */
void PWM_InitCompensation(PWM_Compensation_t *comp);


#endif /* INC_ACTUADORES_CONTROL_PWM_H_ */
