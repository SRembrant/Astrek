/*
 * Taquito.h
 *
 *  Created on: Sep 25, 2025
 *      Author: ASUS
 */

#ifndef INC_NAV_TAQUITO_H_
#define INC_NAV_TAQUITO_H_

#define WALL_FOLLOW_VX_BASE     0.3f // Velocidad de avance para wall-following [m/s]
// --- Alfabeto de la maquina de estados ---

typedef enum{
	NUEVO_TAQUITO, //es el estado inicial
	SEGUIMIENTO_PARED, //avanza en linea recta y hace ajustes pequeños
	ESQUINA_INTERIOR,
	ESQUINA_EXTERIOR,
	CALLEJON_SIN_SALIDA,
	ESTADO_DE_ESCAPE,
	AVANCE,
	AJUSTE
}estado;






void navTaquito_task(void *argument);

#endif /* INC_NAV_TAQUITO_H_ */
