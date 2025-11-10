/*
 * Control_Rover.c
 *
 *  Created on: Jul 16, 2025
 *      Author: EdamVelas
 */
#include "Control_Rover.h"
#include "Motor.h"

void Rover_Move(Rover_Config *rover, Rover_Direccion direction, uint16_t speed , uint32_t tiempo)
{
	if (speed>1000) speed =1000;

    switch (direction) {
        case ROVER_FORWARD:
        	// La logica del motor derecho es la contraria al izquierdo
            Motor_SetMovement(&rover->f_left_motor, MOTOR_ATRAS, speed);   // Se mueve hacia adelante
            Motor_SetMovement(&rover->f_right_motor, MOTOR_ADELANTE, speed);  // Se mueve hacia adelante
            Motor_SetMovement(&rover->b_left_motor, MOTOR_ADELANTE, speed);   // Se mueve hacia adelante
            Motor_SetMovement(&rover->b_right_motor, MOTOR_ADELANTE, speed);  // Se mueve hacia adelante
            break;
        case ROVER_BACKWARD:
        	Motor_SetMovement(&rover->f_left_motor, MOTOR_ADELANTE, speed );     // Se mueve hacia atras
        	Motor_SetMovement(&rover->f_right_motor, MOTOR_ATRAS, speed); // Se mueve hacia atras
          	Motor_SetMovement(&rover->b_left_motor, MOTOR_ATRAS, speed );     // Se mueve hacia atras
            Motor_SetMovement(&rover->b_right_motor, MOTOR_ATRAS, speed); // Se mueve hacia atras
        	break;
        case ROVER_LEFT:
        	Motor_SetMovement(&rover->f_left_motor, MOTOR_ATRAS, speed);      // Se mueve hacia atras
        	Motor_SetMovement(&rover->f_right_motor, MOTOR_ADELANTE, speed);  // Se mueve hacia adelante
        	Motor_SetMovement(&rover->b_left_motor, MOTOR_NEUTRO, speed);     // Neutro
        	Motor_SetMovement(&rover->b_right_motor, MOTOR_NEUTRO, speed);    // Neutro
            break;
        case ROVER_RIGHT:
        	Motor_SetMovement(&rover->f_left_motor, MOTOR_ADELANTE, speed);   // Se mueve hacia adelante
        	Motor_SetMovement(&rover->f_right_motor, MOTOR_ATRAS, speed);     // Se mueve hacia atrás
        	Motor_SetMovement(&rover->b_left_motor, MOTOR_NEUTRO, speed);     // Neutro
        	Motor_SetMovement(&rover->b_right_motor, MOTOR_NEUTRO, speed);    // Neutro
            break;
        case ROVER_STOP:
        	Motor_SetMovement(&rover->f_left_motor, MOTOR_PARAR, speed); // speed es irrelevante en este caso
        	Motor_SetMovement(&rover->f_right_motor, MOTOR_PARAR, speed);
        	Motor_SetMovement(&rover->b_left_motor, MOTOR_PARAR, speed);
        	Motor_SetMovement(&rover->b_right_motor, MOTOR_PARAR, speed);
    break;
    }
    osDelay(tiempo);
}



