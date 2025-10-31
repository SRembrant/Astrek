/*
 * Motor.h
 *
 *  Created on: Jul 16, 2025
 *      Author: EdamVelas
 */

#ifndef INC_ACTUADORES_MOTOR_H_
#define INC_ACTUADORES_MOTOR_H_

#include "stm32f4xx_hal.h"
#include "tim.h"

// Uso hecho para el driver DRV8833
typedef struct
{
	TIM_HandleTypeDef* pwm_timer;      // Timer para PWM
	uint32_t pwm_channel_IN1;          // AIN1 y BIN1
	uint32_t pwm_channel_IN2;          // AIN2 y BIN2
} Motor_Config;

typedef enum
{
	MOTOR_ADELANTE, // Hace que el motor vaya hacia adelante
	MOTOR_ATRAS,    // Hace que el motor vaya hacia atrás
	MOTOR_PARAR,    // Hace que el motor frene activamente
	MOTOR_NEUTRO    // Hace que el motor esté en neutro
}Motor_Movement;

// Prototipos de funciones
void Motor_SetMovement(Motor_Config *motor,Motor_Movement Movimiento, uint16_t speed);  // Setea el movimiento del motor


#endif /* INC_ACTUADORES_MOTOR_H_ */
