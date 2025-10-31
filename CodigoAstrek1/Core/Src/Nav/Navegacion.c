/*
 * Navegacion.c
 *
 *  Created on: Sep 25, 2025
 *      Author: ASUS
 */



#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "Navegacion.h"
#include "GPS.h"
#include "Serial.h"

//-- VARIABLES GLOBALES DE NAVEGACION --
static GPS_Data_t estTerrena;
GPS_Data_t * estacionTerrena = &estTerrena;
	//quemamos datos para la estacion terrena. Esto hace la prueba de comunicacion, biitacora del 25/09 item Bonus


//--  FUNCIONES GENERALES DE NAVEGACION  --
void init_navegacion(void){
	estacionTerrena->altitude=100;
	estacionTerrena->latitude=0;
	estacionTerrena->longitude=0;
	estacionTerrena->is_valid=1;
}


// Convierte grados a radianes
float deg2rad(float deg) {
    return deg * (M_PI / 180.0f);
}

// Convierte radianes a grados
float rad2deg(float rad) {
    return rad * (180.0f / M_PI);
}

float angle_diff(float a, float b) {
    float d = a - b;
    while (d > 180.0f) d -= 360.0f;
    while (d < -180.0f) d += 360.0f;
    return d;
}

// --- Función para convertir coordenadas GPS a Cartesianas locales ---
/*
 * @brief Convierte una coordenada GPS objetivo a coordenadas cartesianas locales
 * respecto a un punto de origen GPS. Utiliza una aproximación de tierra plana.
 *
 * @param origin  El punto GPS que servirá como (0,0) en el sistema cartesiano local.
 * @param target  El punto GPS cuyas coordenadas cartesianas se desean calcular.
 * @return P_Cartesiano Un struct que contiene las coordenadas (x, y) en metros.
 * 'x' es el desplazamiento Este (+) / Oeste (-)
 * 'y' es el desplazamiento Norte (+) / Sur (-)
 */

P_Cartesiano gpsACartesiano(GPS_Data_t * origin, GPS_Data_t *target){
    P_Cartesiano coordLocales;

    // Convertir latitud y longitud de grados a radianes
    float lat_origin_rad = deg2rad(origin->latitude);
    float lon_origin_rad = deg2rad(origin->longitude);
    float lat_target_rad = deg2rad(target->latitude);
    float lon_target_rad = deg2rad(target->longitude);

    // Calcular las diferencias en latitud y longitud en radianes
    float delta_lat_rad = lat_target_rad - lat_origin_rad;
    float delta_lon_rad = lon_target_rad - lon_origin_rad;

    // Calcular la latitud promedio en radianes para la corrección de longitud
    float avg_lat_rad = (lat_origin_rad + lat_target_rad) / 2.0;
    float cos_avg_lat = cos(avg_lat_rad);

    // Calcular el desplazamiento en Y (Norte-Sur) en metros
    coordLocales.y = delta_lat_rad * EARTH_RADIUS;

    // Calcular el desplazamiento en X (Este-Oeste) en metros
    coordLocales.x = delta_lon_rad * EARTH_RADIUS * cos_avg_lat;

    return coordLocales;
}

/**
 * @brief Calcula la distancia en línea recta entre dos puntos GPS usando la fórmula de Haversine.
 *
 * Esta función utiliza la fórmula de Haversine para calcular la distancia más corta sobre
 * la superficie terrestre (considerando la Tierra como una esfera) entre dos coordenadas GPS.
 *
 * @param nodo1 Puntero a la estructura GPS_Data que representa el primer punto.
 * @param nodo2 Puntero a la estructura GPS_Data que representa el segundo punto.
 *
 * @return Distancia entre ambos puntos en metros.
 */

float distanciaNodos(GPS_Data_t* nodo1, GPS_Data_t* nodo2){
	// Diferencia de latitud y longitud en radianes
    float dLat = deg2rad(nodo2->latitude - nodo1->latitude);
    float dLon = deg2rad(nodo2->longitude - nodo1->longitude);

    // Fórmula de Haversine
    float a = sinf(dLat / 2) * sinf(dLat / 2) +
              cosf(deg2rad(nodo1->latitude)) * cosf(deg2rad(nodo2->latitude)) *
              sinf(dLon / 2) * sinf(dLon / 2);

    float c = 2 * atanf(sqrtf(a) / sqrtf(1 - a));

    // Distancia final en metros
    float distance = EARTH_RADIUS * c;
    return distance;
}


/**
 * @brief vTask de navegacion, funciona como una maquina de estados finitos para ejecutar el
 *        algoritmo de navegacion mas apropiado segun sea el caso
 *
 *
 * --revisar como se hace la lectura de opcion, idealmente seria un while, sin embargo, es lo mas apropieado?
 *   o quizas es mejor retornar un valor de opcion y asi mismo recibirlo por parametro, para que el while sea externo;
 */

void navegacion_Task(void *argument){
	//seccion de variables e inicializacion

	static estado_navegacion estadoNavActual = NAV_DSTAR;

	//static osStatus_t statusMachine; //usar al momento de hacer debug
	static evento_navegacion evenNav = TARGET_ALCANZADO; //para la prueba

	static osStatus_t status=osOK;
	static bool banderaPrimeraEjecucion = true;
	  osThreadSuspend(navGlobalHandle);
	  osThreadSuspend(taquitoHandle);
	for(;;){

		osDelay(5000); //solo para las pruebas
		//Serial_PrintString("selector...");
		//falta: comunicar datos globales con Transmision_Task

		//si es la primera ejecucion, no tiene sentido leer la cola
		if(banderaPrimeraEjecucion){
			status = osOK;
			banderaPrimeraEjecucion = false;
		//	Serial_PrintString("primera ejecucion...");
		}
		else{
			status=osMessageQueueGet(navStatesQueueHandle, &evenNav, NULL, 10);
		//	if(evenNav == TARGET_GENERADO) 	Serial_PrintString("RECIIBIIIDO TARGET...");
		}

		if(status == osOK){
			//	Serial_PrintString("Status okk...");
			switch(estadoNavActual){
				case NAV_GLOBAL: //Navegacion Global
				//	Serial_PrintString("  caso global  ");
					if(evenNav==META_ALCANZADA){
					//	Serial_PrintString("se acabo navegaciion");
						osThreadSuspend(osThreadGetId());
					}
					else if(evenNav==TARGET_GENERADO){
					//	osThreadResume(/*navDStarHandle*/);
					//	Serial_PrintString("  target_generado reciibiido  ");
						estadoNavActual = NAV_DSTAR;

						/*/solo para probar GPS
						estadoNavActual =NAV_TAQUITO;
						evenNav=CAMINO_NO_ENCONTRADO;
						osThreadResume(taquitoHandle);
						osThreadSuspend(osThreadGetId());
						//fin solo para probar GPS*/

				//		banderaPrimeraEjecucion=true; //esto es solo para la prueba
					}
					break;
				case NAV_DSTAR: //D*
				//	Serial_PrintString("  caso Dstar  ");
					if(evenNav==TARGET_ALCANZADO){
						//enviar copia del gridMap a transmision
				//		Serial_PrintString("evento: Target alcanzado. Pasando a Nav global \n\n");
						estadoNavActual=NAV_GLOBAL;
						osThreadResume(navGlobalHandle);
						osThreadSuspend(osThreadGetId());
					}
					else if(evenNav==CAMINO_NO_ENCONTRADO){
						estadoNavActual=NAV_TAQUITO;
						osThreadResume(taquitoHandle);
						osThreadSuspend(osThreadGetId());
					}
					break;
				case NAV_TAQUITO: //Taquito
				//	Serial_PrintString("  caso taquiito  ");
					if(evenNav==OBSTACULO_RODEADO){
						//enviar copia del mapaHash a transmision
						osThreadResume(navGlobalHandle);
						estadoNavActual=NAV_GLOBAL;
						osThreadSuspend(osThreadGetId());
					}
					else if(evenNav == ERROR_DESCONOCIDO){
						osThreadResume(navGlobalHandle);
						estadoNavActual=NAV_GLOBAL;
						osThreadSuspend(osThreadGetId());
					}
					break;
				default: //bajo ningun concepto deberia ejecutarse esta linea... pero por si algo
					estadoNavActual=NAV_GLOBAL;
					osThreadResume(navGlobalHandle);
					osThreadSuspend(osThreadGetId());
					break;
			}
		}

			/*//medir stakkk

			uint32_t free_words = osThreadGetStackSpace(osThreadGetId()); // stack libre en "words" (4 bytes)
			uint32_t free_bytes = free_words * sizeof(uint32_t);

			printf("Stack libre (min): %lu words = %lu bytes\r\n", free_words, free_bytes);
*/

	}
}


