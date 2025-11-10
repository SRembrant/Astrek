/*
 * Control_Pose.c
 *
 *  Created on: Oct 24, 2025
 *      Author: joadj
 */
#include "Control_Pose.h"

// Funciones privadas para PID
static void PID_Init(PID_Controller_t *pid, float Kp, float Ki, float Kd,
                     float integral_max, float output_min, float output_max);
static float PID_Update(PID_Controller_t *pid, float error, float dt);
static void PID_Reset(PID_Controller_t *pid);

/**
 * @brief Inicializa el controlador de pose con parámetros por defecto
 */
void PoseController_Init(PoseController_t *ctrl)
{
    // Inicializar PID lineal (control de distancia)
    // Ganancias ajustadas para rover lento
    PID_Init(&ctrl->pid_linear,
             0.8f,    // Kp - respuesta proporcional moderada
             0.05f,   // Ki - integral baja (evitar overshoot)
             0.1f,    // Kd - derivativa para suavizar
             0.5f,    // Integral max (anti-windup)
             0.0f,    // Output min (no retroceso)
             POSE_MAX_LINEAR_VELOCITY);

    // Inicializar PID angular (control de orientación)
    PID_Init(&ctrl->pid_angular,
             1.2f,    // Kp - respuesta más agresiva para girar
             0.01f,   // Ki - integral muy baja
             0.15f,   // Kd - derivativa para evitar oscilaciones
             0.3f,    // Integral max
             -POSE_MAX_ANGULAR_VELOCITY,
             POSE_MAX_ANGULAR_VELOCITY);

    // Inicializar estado
    ctrl->x_actual = 0.0f;
    ctrl->y_actual = 0.0f;
    ctrl->theta_actual = 0.0f;

    ctrl->x_target = 0.0f;
    ctrl->y_target = 0.0f;
    ctrl->theta_target = 0.0f;

    ctrl->vx_cmd = 0.0f;
    ctrl->wz_cmd = 0.0f;

    ctrl->target_reached = false;
    ctrl->orientation_aligned = false;
    ctrl->distance_to_target = 0.0f;
    ctrl->angle_error = 0.0f;

    ctrl->pure_pursuit_mode = true;
}

/**
 * @brief Establece un nuevo target para el controlador
 */
void PoseController_SetTarget(PoseController_t *ctrl, float x_target, float y_target)
{
    ctrl->x_target = x_target;
    ctrl->y_target = y_target;
    ctrl->target_reached = false;
    ctrl->orientation_aligned = false;

    // Resetear integrales al cambiar de target
    ctrl->pid_linear.integral = 0.0f;
    ctrl->pid_angular.integral = 0.0f;
}

/**
 * @brief Actualiza el controlador con la pose actual
 * LLAMAR A 20Hz (cada 50ms) desde ControlTask
 */
void PoseController_Update(PoseController_t *ctrl,
                          float x_actual, float y_actual, float theta_actual)
{
    // Actualizar estado actual
    ctrl->x_actual = x_actual;
    ctrl->y_actual = y_actual;
    ctrl->theta_actual = normalize_angle(theta_actual);

    // Calcular error de posición
    float dx = ctrl->x_target - ctrl->x_actual;
    float dy = ctrl->y_target - ctrl->y_actual;
    ctrl->distance_to_target = sqrtf(dx*dx + dy*dy);

    // Calcular ángulo deseado hacia el target
    float theta_desired = atan2f(dy, dx);
    ctrl->angle_error = angle_difference(theta_desired, ctrl->theta_actual);

    // Verificar si llegamos al target
    if (ctrl->distance_to_target <= POSE_DISTANCE_THRESHOLD) {
        ctrl->target_reached = true;
        ctrl->vx_cmd = 0.0f;
        ctrl->wz_cmd = 0.0f;
        return;
    }

    // ESTRATEGIA DE CONTROL: Orientar primero, luego avanzar

    // Fase 1: Si el error angular es grande, solo girar
    if (fabsf(ctrl->angle_error) > POSE_ANGLE_THRESHOLD) {
        ctrl->orientation_aligned = false;
        ctrl->vx_cmd = 0.0f;  // No avanzar
        ctrl->wz_cmd = PID_Update(&ctrl->pid_angular, ctrl->angle_error, POSE_CONTROL_DT);
    }
    // Fase 2: Orientación aceptable, avanzar con corrección angular
    else {
        ctrl->orientation_aligned = true;

        // Control lineal proporcional a la distancia
        ctrl->vx_cmd = PID_Update(&ctrl->pid_linear, ctrl->distance_to_target, POSE_CONTROL_DT);

        // Corrección angular suave mientras avanza
        if (fabsf(ctrl->angle_error) > POSE_ANGLE_TOLERANCE) {
            ctrl->wz_cmd = PID_Update(&ctrl->pid_angular, ctrl->angle_error, POSE_CONTROL_DT) * 0.5f;
        } else {
            ctrl->wz_cmd = 0.0f;
        }
    }

    // Saturar comandos (redundante pero seguro)
    if (ctrl->vx_cmd > POSE_MAX_LINEAR_VELOCITY) ctrl->vx_cmd = POSE_MAX_LINEAR_VELOCITY;
    if (ctrl->vx_cmd < 0.0f) ctrl->vx_cmd = 0.0f;

    if (ctrl->wz_cmd > POSE_MAX_ANGULAR_VELOCITY) ctrl->wz_cmd = POSE_MAX_ANGULAR_VELOCITY;
    if (ctrl->wz_cmd < -POSE_MAX_ANGULAR_VELOCITY) ctrl->wz_cmd = -POSE_MAX_ANGULAR_VELOCITY;
}

/**
 * @brief Resetea el controlador
 */
void PoseController_Reset(PoseController_t *ctrl)
{
    PID_Reset(&ctrl->pid_linear);
    PID_Reset(&ctrl->pid_angular);

    ctrl->vx_cmd = 0.0f;
    ctrl->wz_cmd = 0.0f;
    ctrl->target_reached = false;
    ctrl->orientation_aligned = false;
}

/**
 * @brief Normaliza ángulo a rango [-π, π]
 */
float normalize_angle(float angle)
{
    while (angle > M_PI) angle -= 2.0f * M_PI;
    while (angle < -M_PI) angle += 2.0f * M_PI;
    return angle;
}

/**
 * @brief Calcula diferencia angular normalizada
 */
float angle_difference(float target, float current)
{
    float diff = target - current;
    return normalize_angle(diff);
}

// ============================================================================
// FUNCIONES PRIVADAS DE PID
// ============================================================================

/**
 * @brief Inicializa un controlador PID
 */
static void PID_Init(PID_Controller_t *pid, float Kp, float Ki, float Kd,
                    float integral_max, float output_min, float output_max)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral_max = integral_max;
    pid->output_min = output_min;
    pid->output_max = output_max;

    pid->error_prev = 0.0f;
    pid->integral = 0.0f;
}

/**
 * @brief Actualiza PID y retorna salida
 */
static float PID_Update(PID_Controller_t *pid, float error, float dt)
{
    // Término proporcional
    float P = pid->Kp * error;

    // Término integral con anti-windup
    pid->integral += error * dt;
    if (pid->integral > pid->integral_max) pid->integral = pid->integral_max;
    if (pid->integral < -pid->integral_max) pid->integral = -pid->integral_max;
    float I = pid->Ki * pid->integral;

    // Término derivativo
    float derivative = (error - pid->error_prev) / dt;
    float D = pid->Kd * derivative;

    // Salida total
    float output = P + I + D;

    // Saturación
    if (output > pid->output_max) output = pid->output_max;
    if (output < pid->output_min) output = pid->output_min;

    // Guardar error para próxima iteración
    pid->error_prev = error;

    return output;
}

/**
 * @brief Resetea estados internos del PID
 */
static void PID_Reset(PID_Controller_t *pid)
{
    pid->error_prev = 0.0f;
    pid->integral = 0.0f;
}

