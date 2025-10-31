/*
 * Navegacion.h
 *
 *  Created on: Sep 25, 2025
 *      Author: ASUS
 */

#ifndef INC_NAV_NAVEGACION_H_
#define INC_NAV_NAVEGACION_H_

#include "GPS.h"
#include "HashMap.h"
#include "Lista.h"
#include "Control_Rover.h"

// -- VARIABLES GLOBALES --
extern GPS_Data_t * estacionTerrena;

// -- HANDLES GLOBALES --
extern osThreadId_t navGlobalHandle;
extern osThreadId_t taquitoHandle;
extern osThreadId_t navegacionHandle;

// -- COLAS DE TAREAS --
	//colas de sensores
extern osMessageQueueId_t gpsDataQueueHandle;
extern osMessageQueueId_t sensorDataQueueHandle;
	//colas de navegacion
extern osMessageQueueId_t controlDataQueueHandle;
extern osMessageQueueId_t navStatesQueueHandle;

// -- CONSTANTES DEL SISTEMA --
#define EARTH_RADIUS 6371000.0f // Radio de la Tierra en metros

//-- DEFINICION DE UMBRALES --
//por lo pronto son arbitrarios ... solo para paredes
#define dist_interior_aceptable 10.0f //10 cm
#define dist_exterior_aceptable 30.0f //30 cm
#define dist_objetivo 20.0f // distancia de 20 cm del obstaculo, con zona muerta entre 10 y 30 centimetros
#define dist_suelo_estandar 40.0f // la lectura del sensor al plano que sopota al rover

//lecturas entre los (15 y 45) centimetros se consideran sorteables y no son obstaculos, es decir, el rover puede transitar por ahi
#define umbral_pared 15.0f //cuando se considera un obstaculo?
#define umbral_grieta 45.0f // umbral grieta

// Umbrales para la detección de obstáculos (DStar, mirar los umbrales mas despacio con Val)
#define UMBRAL_OBSTACULO_PEQUENO_M 2.0f // Distancia máxima para considerar un obstáculo "pequeño"
#define UMBRAL_OBSTACULO_GRANDE_M  1.0f // Distancia máxima para considerar un obstáculo "grande" (bloquea la celda)

// -- ESTRUCTURAS DE NAVEGACION --

// estructura para un punto en un plano cartesiano local
typedef struct{
	float x;
	float y;
}P_Cartesiano;

// mensajes de eventos de navegacion
typedef enum{
	//eventos de navGlobal
	META_ALCANZADA = 0,
	TARGET_GENERADO = 1,
	//eventos de DStar
	TARGET_ALCANZADO = 2,
	CAMINO_NO_ENCONTRADO = 3,
	//eventos de navTaquito
	OBSTACULO_RODEADO = 4,
	ERROR_DESCONOCIDO = 5
} evento_navegacion;

// estructura para la maquina de estados principal de la navegacion
typedef enum{
	NAV_GLOBAL,
	NAV_DSTAR,
	NAV_TAQUITO,
	FIN_NAV
}estado_navegacion;

// -- FUNCIONES GENERALES DE NAVEGACION --
void init_navegacion(void);
//grados a radianes
float deg2rad(float deg);
//radianes a grados
float rad2deg(float rad);

float angle_diff(float a, float b);
// coordenadas GPS a plano cartesiano local (Y - Norte, X - este)
P_Cartesiano gpsACartesiano(GPS_Data_t * origin, GPS_Data_t *target);
// distancia entre dos posiciones GPS
float distanciaNodos(GPS_Data_t* nodo1, GPS_Data_t* nodo2);

// -- TAREA DE NAVEGACION (Maquina de estados) --
void navegacion_Task(void *argument);


#endif /* INC_NAV_NAVEGACION_H_ */
