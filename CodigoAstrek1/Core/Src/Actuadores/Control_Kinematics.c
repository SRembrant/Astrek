/*
 * Control_Kinematics.c
 *
 *  Created on: Oct 24, 2025
 *      Author: joadj
 */


#include "Control_Kinematics.h"

/**
 * @brief Cinemática inversa - Modelo diferencial
 *
 * Ecuaciones:
 *   vR = vx + (L/2) * wz
 *   vL = vx - (L/2) * wz
 *
 * Donde:
 *   L = WHEEL_BASE (distancia entre ruedas)
 *   vx = velocidad lineal del centro del rover
 *   wz = velocidad angular del rover (positivo = giro antihorario)
 */
WheelVelocities_t Kinematics_Inverse(float vx, float wz)
{
    WheelVelocities_t wheels;

    // Cálculo del modelo diferencial
    float half_base = WHEEL_BASE / 2.0f;

    wheels.vR = vx + (half_base * wz);
    wheels.vL = vx - (half_base * wz);

    // Saturar velocidades a límites físicos
    Kinematics_SaturateWheels(&wheels);

    return wheels;
}

/**
 * @brief Cinemática directa - Para odometría
 *
 * Ecuaciones:
 *   vx = (vR + vL) / 2
 *   wz = (vR - vL) / L
 */
RoverVelocity_t Kinematics_Forward(float vR, float vL)
{
    RoverVelocity_t rover;

    // Velocidad lineal del centro
    rover.vx = (vR + vL) / 2.0f;

    // Velocidad angular
    rover.wz = (vR - vL) / WHEEL_BASE;

    return rover;
}

/**
 * @brief Convierte velocidad lineal [m/s] a RPM
 *
 * Fórmula:
 *   RPM = (velocidad / perímetro_rueda) * 60
 */
float Kinematics_VelocityToRPM(float velocity)
{
    // velocity [m/s] = (RPM / 60) * circumference
    // RPM = (velocity / circumference) * 60

    float rpm = (velocity / WHEEL_CIRCUMFERENCE) * 60.0f;
    return rpm;
}

/**
 * @brief Convierte RPM a velocidad lineal [m/s]
 */
float Kinematics_RPMToVelocity(float rpm)
{
    // velocity [m/s] = (RPM / 60) * circumference

    float velocity = (rpm / 60.0f) * WHEEL_CIRCUMFERENCE;
    return velocity;
}

/**
 * @brief Satura velocidades de ruedas a límites físicos
 */
void Kinematics_SaturateWheels(WheelVelocities_t *wheels)
{
    // Saturar rueda derecha
    if (wheels->vR > MAX_WHEEL_VELOCITY) {
        wheels->vR = MAX_WHEEL_VELOCITY;
    } else if (wheels->vR < -MAX_WHEEL_VELOCITY) {
        wheels->vR = -MAX_WHEEL_VELOCITY;
    }

    // Aplicar zona muerta
    if (fabsf(wheels->vR) < MIN_WHEEL_VELOCITY) {
        wheels->vR = 0.0f;
    }

    // Saturar rueda izquierda
    if (wheels->vL > MAX_WHEEL_VELOCITY) {
        wheels->vL = MAX_WHEEL_VELOCITY;
    } else if (wheels->vL < -MAX_WHEEL_VELOCITY) {
        wheels->vL = -MAX_WHEEL_VELOCITY;
    }

    // Aplicar zona muerta
    if (fabsf(wheels->vL) < MIN_WHEEL_VELOCITY) {
        wheels->vL = 0.0f;
    }
}
