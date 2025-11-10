/*
 * Control_PWM.c
 *
 *  Created on: Oct 24, 2025
 *      Author: joadj
 */

#include "Control_PWM.h"

// Funciones privadas
static uint16_t interpolate_LUT(float rpm);
static uint16_t saturate_PWM(uint16_t pwm);

/**
 * @brief Convierte velocidades de ruedas a comandos PWM
 */
PWM_Commands_t PWM_FromWheelVelocities(WheelVelocities_t wheels,
                                       PWM_Compensation_t *comp)
{
    PWM_Commands_t commands;

    // Determinar dirección y valor absoluto de velocidades
    commands.dir_right = (wheels.vR >= 0) ? 0 : 1;  // 0=adelante, 1=atrás
    commands.dir_left = (wheels.vL >= 0) ? 0 : 1;

    float abs_vR = fabsf(wheels.vR);
    float abs_vL = fabsf(wheels.vL);

    // Convertir velocidades a PWM base
    commands.pwm_right = PWM_VelocityToDuty(abs_vR);
    commands.pwm_left = PWM_VelocityToDuty(abs_vL);

    // Aplicar compensaciones si están habilitadas
    if (comp != NULL) {
        if (comp->enable_battery_comp) {
            commands.pwm_right = PWM_CompensateBattery(commands.pwm_right,
                                                        comp->battery_voltage);
            commands.pwm_left = PWM_CompensateBattery(commands.pwm_left,
                                                       comp->battery_voltage);
        }

        if (comp->enable_slope_comp) {
            commands.pwm_right = PWM_CompensateSlope(commands.pwm_right,
                                                      comp->pitch_angle);
            commands.pwm_left = PWM_CompensateSlope(commands.pwm_left,
                                                     comp->pitch_angle);
        }
    }

    // Saturar a límites seguros
    commands.pwm_right = saturate_PWM(commands.pwm_right);
    commands.pwm_left = saturate_PWM(commands.pwm_left);

    return commands;
}

/**
 * @brief Convierte velocidad [m/s] a PWM usando LUT
 */
uint16_t PWM_VelocityToDuty(float velocity)
{
    // Manejar caso de velocidad cero
    if (fabsf(velocity) < 0.01f) {
        return 0;
    }

    // Convertir velocidad a RPM
    float rpm = Kinematics_VelocityToRPM(velocity);

    // Mapear RPM a PWM usando LUT
    return PWM_RPMToDuty(rpm);
}

/**
 * @brief Mapea RPM a PWM usando interpolación lineal en LUT
 */
uint16_t PWM_RPMToDuty(float rpm)
{
    // Caso especial: RPM muy bajo (zona muerta)
    if (rpm < PWM_LUT[1].rpm) {
        if (rpm < 10.0f) {
            return 0;  // Completamente parado
        } else {
            return PWM_MIN_DUTY;  // PWM mínimo para vencer fricción
        }
    }

    // Caso especial: RPM muy alto (saturación)
    if (rpm >= PWM_LUT[LUT_SIZE - 1].rpm) {
        return PWM_MAX_DUTY;
    }

    // Interpolación lineal
    return interpolate_LUT(rpm);
}

/**
 * @brief Interpola linealmente en la LUT
 */
static uint16_t interpolate_LUT(float rpm)
{
    // Buscar segmento de la LUT donde cae el RPM
    for (int i = 0; i < LUT_SIZE - 1; i++) {
        if (rpm >= PWM_LUT[i].rpm && rpm < PWM_LUT[i + 1].rpm) {
            // Interpolación lineal entre puntos i e i+1
            float rpm_low = PWM_LUT[i].rpm;
            float rpm_high = PWM_LUT[i + 1].rpm;
            uint16_t pwm_low = PWM_LUT[i].pwm;
            uint16_t pwm_high = PWM_LUT[i + 1].pwm;

            // Factor de interpolación
            float factor = (rpm - rpm_low) / (rpm_high - rpm_low);

            // PWM interpolado
            uint16_t pwm = pwm_low + (uint16_t)(factor * (pwm_high - pwm_low));

            return pwm;
        }
    }

    // Fallback (no debería llegar aquí)
    return PWM_MIN_DUTY;
}

/**
 * @brief Compensación por voltaje de batería
 *
 * Modelo: Si el voltaje baja, aumentar PWM proporcionalmente
 * PWM_compensado = PWM_base * (V_ref / V_actual)
 */
uint16_t PWM_CompensateBattery(uint16_t pwm, float voltage)
{
    // Validar voltaje (rango típico Li-Po 3S: 9.0V - 12.6V)
    if (voltage < 9.0f) voltage = 9.0f;
    if (voltage > 12.6f) voltage = 12.6f;

    // Factor de compensación
    float factor = PWM_VOLTAGE_REF / voltage;

    // Aplicar compensación
    uint16_t compensated = (uint16_t)(pwm * factor);

    return saturate_PWM(compensated);
}

/**
 * @brief Compensación por pendiente (pitch del IMU)
 *
 * Modelo simplificado:
 * - Subiendo (pitch > 0): Aumentar PWM
 * - Bajando (pitch < 0): Reducir PWM (evitar acelerar cuesta abajo)
 */
uint16_t PWM_CompensateSlope(uint16_t pwm, float pitch)
{
    // Limitar ángulos extremos (±30 grados = ±0.52 rad)
    if (pitch > 0.52f) pitch = 0.52f;
    if (pitch < -0.52f) pitch = -0.52f;

    // Factor de compensación lineal
    // pitch = 0° → factor = 1.0 (sin cambio)
    // pitch = +15° → factor = 1.2 (20% más PWM subiendo)
    // pitch = -15° → factor = 0.8 (20% menos PWM bajando)
    float factor = 1.0f + (pitch * 0.8f);

    // Aplicar compensación
    uint16_t compensated = (uint16_t)(pwm * factor);

    return saturate_PWM(compensated);
}

/**
 * @brief Satura PWM a límites seguros
 */
static uint16_t saturate_PWM(uint16_t pwm)
{
    if (pwm > PWM_MAX_DUTY) {
        return PWM_MAX_DUTY;
    } else if (pwm < PWM_MIN_DUTY && pwm > 0) {
        return PWM_MIN_DUTY;  // Evitar zona muerta
    }
    return pwm;
}

/**
 * @brief Inicializa compensaciones por defecto
 */
void PWM_InitCompensation(PWM_Compensation_t *comp)
{
    comp->battery_voltage = PWM_VOLTAGE_REF;
    comp->pitch_angle = 0.0f;
    comp->compensation_factor = 1.0f;
    comp->enable_battery_comp = 0;  // Deshabilitado por defecto
    comp->enable_slope_comp = 0;    // Deshabilitado por defecto
}
