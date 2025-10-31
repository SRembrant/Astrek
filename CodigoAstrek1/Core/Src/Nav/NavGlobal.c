/*
 * NavGlobal.c
 *
 *  Created on: Sep 25, 2025
 *      Author: ASUS
 */


#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "Navegacion.h"
#include "NavGlobal.h"
#include "Serial.h"




// --- Funciones auxiliares ---

/**
 * @brief Convierte millas náuticas a grados decimales (latitud/longitud)
 *        1 milla náutica = 1 minuto de arco
 *        60 minutos = 1 grado
 *
 * @param millas_nauticas Distancia en millas náuticas
 * @return Valor en grados decimales
 */

static float millas_nauticas_a_grados(float millas_nauticas) {
    // Paso 1: Convertir a minutos (es 1 a 1)
    float minutos = millas_nauticas;

    // Paso 2: Separar los minutos en grados y minutos restantes
    int grados = (int)(minutos / 60);               // Parte entera en grados
    float minutos_restantes = fmod(minutos, 60);   // Lo que sobra

    // Paso 3: Separar los minutos restantes en segundos
    int minutos_enteros = (int)minutos_restantes;
    float segundos = (minutos_restantes - minutos_enteros) * 60;

    // Paso 4: Convertir tod a grados decimales
    float grados_decimales = grados + (minutos_enteros / 60.0) + (segundos / 3600.0);

    return grados_decimales;
}


/*
 * @brief Esta funcion calcula el rumbo (en grados) que debe seguir el rover para alcanzar una posicion GPS de destino
 * 		  parte de la trignometria esferica y el concepto de rumbo en nautica y aeronautica, que se mide desde el norte del meridiano
 * 		  actual hacia el este. Se hacen calculos siguiendo la ortodromica (distancia mas corta entre dos puntos sobre una esfera).
 * @param current -> es la posicion GPS donde se encuentra actualmente
 * @param target -> es la posicion GPS de destino
 * @return Valor en grados del rumbo
 */

static float calculate_bearing(GPS_Data_t* current, GPS_Data_t *target) {
    float lat1 = deg2rad(current->latitude);
    float lat2 = deg2rad(target->latitude);
    float dLon = deg2rad(target->longitude - current->longitude);

    float y = sinf(dLon) * cosf(lat2);
    float x = cosf(lat1) * sinf(lat2) - sinf(lat1) * cosf(lat2) * cosf(dLon);

    float bearing = atan2f(y, x);
    bearing = rad2deg(bearing);
    return fmodf((bearing + 360.0f), 360.0f);  // Normaliza a [0, 360)
}



/**
 * @Brief Determina si ya se alcanzo o no la estacion terrena (verdadero o falso respectivamente)
 * De no ser el caso, determina cual es el siguiente target de control
 * @Param actual -> es la posicion actual del rover
 * @Param estacionTerrena -> es la ubicacionn de la estacion terrena
 * @Param target -> es una referencia variable al siguiente target de control
 * */
void navGlobal_task(void *argument)
{
	//Declaracion de variables
	float rumbo, delta_lng, delta_ltd, distancia_Nodos;
	osStatus_t status;
	GPS_Data_t received_gps_data;
	static GPS_Data_t target;
	evento_navegacion evento;

	for(;;)
	{
	//	HAL_Delay(5000); //para que me de tiempo de hacer pruebas
	//	Serial_PrintString("NavGlobal en ejecucion...\n");
		//lectura de datos de las colas
		status = osMessageQueueGet(gpsDataQueueHandle, &received_gps_data, NULL, 10);
		if(status == osOK){

	//		Serial_PrintGPSData(&received_gps_data);


			//1. Calcular la magnitud del vector
			distancia_Nodos = distanciaNodos(&received_gps_data, estacionTerrena);
			//2. Verificar si ya llegamos a la estacion terrena
			if(distancia_Nodos<=3){
				evento = META_ALCANZADA;
				osMessageQueuePut(navStatesQueueHandle,&evento, 0, 0); //ya no hay mas targets por alcanzar. hemos llegado
		//		Serial_PrintString("Ya llegamos a la meta");
				osThreadResume(navegacionHandle);
				osThreadSuspend(osThreadGetId());
			}
			else if(distancia_Nodos<=10){ //si no hemos llegado a la estacion terrena pero estamos a un grid map o menos de distancia, nuestro ultimo target es la estacion en si
				target = *estacionTerrena; //revisar
				// enviar datos a la cola
				//osMessageQueuePut(targetDataQueueHandle, &target, 0, 10);
				evento = TARGET_GENERADO;
				osMessageQueuePut(navStatesQueueHandle,&evento, 0, 0);
		//		Serial_PrintString("El siguiente target es la estacion terrena");
				osThreadResume(navegacionHandle);
				osThreadSuspend(osThreadGetId());
			}
			else{ //definir cual es el siguiente target de Control

				//3. Calcular el rumbo a seguir
				rumbo = calculate_bearing(&received_gps_data, estacionTerrena);

				//4. Calcular el cambio de longitud y latitud (para que la distancia sea 10 metros)

				delta_lng = 0.00539957  * sinf(rumbo); //0.00539957 son 10 metros expresados en millas nauticas
				delta_ltd = 0.00539957 * cosf(rumbo);

				//5. Cambiar de millas nauticas a grados

				delta_lng = millas_nauticas_a_grados(delta_lng);
				delta_ltd = millas_nauticas_a_grados(delta_ltd);

				//6. sumar los grados a actual y listo, ese es el nuevo target
				target.latitude=received_gps_data.latitude+delta_ltd;
				target.longitude=received_gps_data.longitude+delta_lng;

				//quema de datos solo para la prueba
			//	target.lon_direction='W';
			//	target.lat_direction='N';
				//fin quema de datos
				target.is_valid=1;

				//7. enviar datos a la cola
				//osMessageQueuePut(targetDataQueueHandle, &target, 0, 10);
				evento = TARGET_GENERADO;
				osMessageQueuePut(navStatesQueueHandle,&evento, 0, 0);
		//		Serial_PrintString("Target Generado");
		//		Serial_PrintGPSData(&target);

				//vTaskSuspend(NULL);
				//Serial_PrintString("target generado:   ");
				//Serial_PrintGPSData(&target);
				osThreadResume(navegacionHandle);
				osThreadSuspend(osThreadGetId());
			}
		}
	//	osDelay(500);
	}
}


