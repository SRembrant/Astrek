/*
 * Motor.c
 *
 *  Created on: Jul 16, 2025
 *      Author: EdamVelas
 */
#include "Motor.h"


/*
 * La logica está seteada para el motor izquierdo como principal, el derecho simplemente es la contraria
 * en caso de presentar inconvenientes a la hora de modificar el montaje, ajustarla segun las necesidades
 */
void Motor_SetMovement(Motor_Config *motor, Motor_Movement Movimiento, uint16_t speed)
{
	switch(Movimiento)
	{
	case MOTOR_ADELANTE:
		__HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel_IN1, 0);      //IN1 [LOW]
		__HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel_IN2, speed);  //IN2 [PWM]
		break;
	case MOTOR_ATRAS:
		__HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel_IN1, speed);  //IN1 [PWM]
		__HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel_IN2, 0);      //IN2 [LOW]
		break;
	case MOTOR_PARAR:
		__HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel_IN1, 0);  //IN1 [LOW]
		__HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel_IN2, 0);  //IN2 [LOW]
		break;
	case MOTOR_NEUTRO:
		__HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel_IN1, 1000);  //IN1 [HIGH]
		__HAL_TIM_SET_COMPARE(motor->pwm_timer, motor->pwm_channel_IN2, 1000);  //IN2 [HIGH]
		break;
	}
}



