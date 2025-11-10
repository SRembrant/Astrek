/*
 * Control_Kinematics.h
 *
 *  Created on: Oct 24, 2025
 *      Author: joadj
 */

#ifndef INC_ACTUADORES_CONTROL_KINEMATICS_H_
#define INC_ACTUADORES_CONTROL_KINEMATICS_H_

#include <stdint.h>
#include <math.h>

// ============================================================================
// PARÁMETROS FÍSICOS DEL ROVER
// ============================================================================
// TODO: MEDIR Y ACTUALIZAR CON VALORES REALES

#define WHEEL_BASE           0.25f    // Distancia entre ruedas izq-der [m]
#define WHEEL_RADIUS         0.065f   // Radio de las ruedas [m]
#define WHEEL_CIRCUMFERENCE  (2.0f * M_PI * WHEEL_RADIUS)  // Perímetro rueda

// Límites físicos del rover
#define MAX_WHEEL_VELOCITY   1.0f     // Velocidad máxima de rueda [m/s]
#define MIN_WHEEL_VELOCITY   0.05f    // Velocidad mínima detectable [m/s]

// ============================================================================
// ESTRUCTURAS
// ============================================================================

/**
 * @brief Velocidades de las ruedas
 */
typedef struct {
    float vR;  // Velocidad rueda derecha [m/s]
    float vL;  // Velocidad rueda izquierda [m/s]
} WheelVelocities_t;

/**
 * @brief Comandos de velocidad del rover
 */
typedef struct {
    float vx;  // Velocidad lineal [m/s]
    float wz;  // Velocidad angular [rad/s]
} RoverVelocity_t;

/**
 * @brief RPM de motores (para debug/telemetría)
 */
typedef struct {
    float rpm_right;
    float rpm_left;
} MotorRPM_t;

// ============================================================================
// FUNCIONES PÚBLICAS
// ============================================================================

/**
 * @brief Cinemática inversa: (vx, wz) → (vR, vL)
 * Convierte velocidades del rover a velocidades de ruedas
 *
 * @param vx Velocidad lineal deseada [m/s]
 * @param wz Velocidad angular deseada [rad/s]
 * @return Velocidades de ruedas calculadas
 */
WheelVelocities_t Kinematics_Inverse(float vx, float wz);

/**
 * @brief Cinemática directa: (vR, vL) → (vx, wz)
 * Convierte velocidades de ruedas a velocidades del rover
 * Útil para odometría
 *
 * @param vR Velocidad rueda derecha [m/s]
 * @param vL Velocidad rueda izquierda [m/s]
 * @return Velocidades del rover calculadas
 */
RoverVelocity_t Kinematics_Forward(float vR, float vL);

/**
 * @brief Convierte velocidad lineal [m/s] a RPM de motor
 *
 * @param velocity Velocidad lineal [m/s]
 * @return RPM del motor
 */
float Kinematics_VelocityToRPM(float velocity);

/**
 * @brief Convierte RPM de motor a velocidad lineal [m/s]
 *
 * @param rpm RPM del motor
 * @return Velocidad lineal [m/s]
 */
float Kinematics_RPMToVelocity(float rpm);

/**
 * @brief Valida y satura velocidades de ruedas
 *
 * @param wheels Puntero a velocidades de ruedas a validar
 */
void Kinematics_SaturateWheels(WheelVelocities_t *wheels);

#endif /* INC_ACTUADORES_CONTROL_KINEMATICS_H_ */
