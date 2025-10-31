/*
 * Control_Rover.h
 *
 *  Created on: Jul 16, 2025
 *      Author: EdamVelas
 */

#ifndef INC_ACTUADORES_CONTROL_ROVER_H_
#define INC_ACTUADORES_CONTROL_ROVER_H_

#include "Motor.h"
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "cmsis_os.h"

typedef enum { //Crea los casos a usar
    ROVER_STOP, // Para el Rover
    ROVER_FORWARD, // Mueve el Rover hacia adelante
    ROVER_BACKWARD, // Mueve el Rover hacia atrás
    ROVER_LEFT, // Gira el Rover hacia la izquierda
    ROVER_RIGHT // Gira el Rover hacia la derecha
} Rover_Direccion;

// Configuracion de los dos motores
typedef struct {
    Motor_Config f_left_motor; // Motor izquierdo del frente
    Motor_Config f_right_motor; // Motor derecho del frente
    Motor_Config b_left_motor; // Motor izquierdo de atras
    Motor_Config b_right_motor; // Motor derecho de atras
} Rover_Config;

//estructura para comunicacion con control
typedef struct{
	Rover_Direccion direccion;
	uint16_t velocidad;
	uint32_t tiempo;
}control_command;

//Funciones a usar
void Rover_Init(Rover_Config *Rover); //Inicializa el movimiento del Rover
void Rover_Move(Rover_Config *Rover, Rover_Direccion direccion, uint16_t velocidad,uint32_t tiempo );

#endif /* INC_ACTUADORES_CONTROL_ROVER_H_ */
