/*
 * Control_Pose.h
 *
 *  Created on: Oct 24, 2025
 *      Author: joadj
 *      Descripcion: Controlador PID de alto nivel para seguimiento de waypoint
 */

#ifndef INC_ACTUADORES_CONTROL_POSE_H_
#define INC_ACTUADORES_CONTROL_POSE_H_

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

// Constantes de configuración
#define POSE_CONTROL_FREQUENCY_HZ    20.0f    // Frecuencia de control (20Hz)
#define POSE_CONTROL_DT              0.05f    // Periodo (1/20 = 50ms)

// Umbrales de control
#define POSE_DISTANCE_THRESHOLD      0.3f     // 30cm - considerado "llegado"
#define POSE_ANGLE_THRESHOLD         0.17f    // ~10 grados en radianes
#define POSE_ANGLE_TOLERANCE         0.087f   // ~5 grados - tolerancia para avanzar

// Límites de comandos
#define POSE_MAX_LINEAR_VELOCITY     0.5f     // 0.5 m/s máximo
#define POSE_MAX_ANGULAR_VELOCITY    0.52f    // ~30 deg/s en rad/s

// Estructura de PID
typedef struct {
    // Ganancias
    float Kp;
    float Ki;
    float Kd;

    // Estados internos
    float error_prev;
    float integral;
    float integral_max;  // Anti-windup

    // Límites de salida
    float output_min;
    float output_max;
} PID_Controller_t;

// Estructura del controlador de pose
typedef struct {
    // PIDs
    PID_Controller_t pid_linear;    // Control de distancia
    PID_Controller_t pid_angular;   // Control de orientación

    // Estado actual (entrada del controlador)
    float x_actual;          // metros
    float y_actual;          // metros
    float theta_actual;      // radianes (-π a π)

    // Target (objetivo)
    float x_target;
    float y_target;
    float theta_target;      // Opcional, para orientación final

    // Comandos de salida
    float vx_cmd;           // Velocidad lineal (m/s)
    float wz_cmd;           // Velocidad angular (rad/s)

    // Estado del controlador
    bool target_reached;
    bool orientation_aligned;
    float distance_to_target;
    float angle_error;

    // Modo de control
    bool pure_pursuit_mode;  // true: sigue waypoint, false: orientación fija

} PoseController_t;

//estructura para el PID del seguimiento de pared
typedef struct {
    PID_Controller_t pid_wall;
    float dist_lateral_deseada;
} WallFollow_Controller_t;

// Tipos de comando que Taquito puede enviar
typedef enum {
    MODE_POSE_TARGET,   // Ir a un (X, Y) fijo (comportamiento actual en ControlTask)
    MODE_WALL_FOLLOW,   // Moverse con vx y corrección wz (Nuevo modo para Taquito)
    MODE_POSE_GIRO,     // Giro de 90 grados (Theta Target)
	MODE_FORWARD
} Rover_Control_Mode_t;

// Estructura de comunicación ampliada
typedef struct {
    Rover_Control_Mode_t mode;  // El modo de operación
    //WALL FOLLOW
    float target_vx;            // Velocidad lineal deseada (para Wall Follow)
    float target_wz;            // Velocidad angular deseada (para giros puros o Wall Follow)
    float distance_cm;          // Distancia lateral (solo relevante para Wall Follow)
    int8_t wall_direction;      // Dirección de pared (1 o -1)
    //WAYPOIINTS
    float target_x;             // X objetivo (para Pose Target o Giro)
    float target_y;             // Y objetivo (para Pose Target o Giro)
    //GIROS ESTATICOS
    float target_theta;         // Theta objetivo (para Giro de 90 grados)

} ControlCommand_t;

// Funciones públicas
void PoseController_Init(PoseController_t *ctrl);
void PoseController_SetTarget(PoseController_t *ctrl, float x_target, float y_target);
void PoseController_Update(PoseController_t *ctrl, float x_actual, float y_actual, float theta_actual);
void PoseController_Reset(PoseController_t *ctrl);

// Utilidades
float normalize_angle(float angle);
float angle_difference(float target, float current);

#endif /* INC_ACTUADORES_CONTROL_POSE_H_ */
