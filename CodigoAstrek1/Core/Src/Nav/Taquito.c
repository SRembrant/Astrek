/*
 * Taquito.c
 *
 *  Created on: Sep 25, 2025
 *      Author: ASUS
 */


#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "Taquito.h"
#include "Navegacion.h"
#include "Serial.h"
#include "sr04.h"

// --- Funciones auxiliares ---

/*
 * @brief si el rover esta dentro de un intervalo permitido, no hace ajustes de pared
 * 	      de estar muy lejos o muy cerca, regresa al intervalo permitido
 * @return 0 para rango permitido, 1 para muy cerca, 2 para muy lejos
 */

static float hayPared(float disLateral){
	//float distancia_medida;
	float rangoPosicionMin, rangoPosicionMax;

	//calculo de proximidad a la pared a seguir
	rangoPosicionMin = disLateral - dist_interior_aceptable;
	rangoPosicionMax = disLateral - dist_exterior_aceptable;

	if(rangoPosicionMin >= 0 && rangoPosicionMax <= 0) return 0; //intervalo aceptable
	else if(rangoPosicionMin < 0) return rangoPosicionMin; //muy cerca de la pared (y si retorno la diferencia)
	else if(rangoPosicionMax > 0) return rangoPosicionMax; //muy lejos de la pared (y si retorno la diferencia)

	//error
	return -1.0;
}


/*
 * @brief ajustarPared hace ajustes sobre el rumbo local del rover para continuar con
 *                     el wall following
 * @param direccion Es un valor -1 o 1, indica que pared esta siguiendo el rover
 *                  1 para la pared izquierda, -1 para la pared derecha
 */

static void ajustarPared(int8_t direccion, float disLateral){
	//validamos si estamos siguiendo una pared o una grieta
	if(disLateral>=umbral_grieta) return; // si es una grieta, no hay como hacer ajustes
	float existePared = hayPared(disLateral);
	if(existePared==0) return;
	else if(existePared<0){
		if(direccion == 1){
			//giro hacia la derecha proporcional al valor de retorno
		}
		else{
			//giro hacia la izquierda proporcional al valor de retorno
		}
	}
	else if(existePared>0){
		if(direccion == 1){
			//giro hacia la izquierda proporcional al valor de retorno
		}
		else{
			//giro hacia la derecha proporcional al valor de retorno
		}
	}
}


/**
 * @brief Calcula la distancia perpendicular desde un punto actual a una recta definida por dos puntos GPS.
 *
 * La recta está definida por los puntos 'origen' y 'estacionTerrena', y se calcula en un sistema cartesiano
 * local obtenido mediante proyección plana desde coordenadas GPS.
 *
 * @param posActual Puntero a la estructura GPS_Data que representa la posición actual del rover.
 * @param origen Puntero a la estructura GPS_Data que se toma como origen del sistema cartesiano local.
 * @param estacionTerrena Puntero a la estructura GPS_Data que define un punto en la recta objetivo.
 *
 * @return La distancia en metros desde la posición actual a la recta.
 */
/*
static float distanciaARecta(GPS_Data_t * posActual, GPS_Data_t * origen, GPS_Data_t * estacionTerrena){
	//declaracion de variables
	float a, b, c, numerador, denominador, distancia;
	P_Cartesiano coordPosActual, coordEstTerrena, coordOrigen;

	//tranformamos de coordenagas GPS a coordenadas Cartesianas
	coordEstTerrena = gpsACartesiano(origen, estacionTerrena);
	coordPosActual = gpsACartesiano(origen, posActual);
	coordOrigen = gpsACartesiano(origen, origen);

	//calculamos los coeficientes de la recta
	a = coordEstTerrena.y - coordOrigen.y;         // a = Δy
	b = coordOrigen.x - coordEstTerrena.x;         // b = -Δx
	c = -( (a) * coordOrigen.x + (b) * coordOrigen.y);  // c = -(a·x1 + b·y1)

	//calculamos la distancia desde la posicion actual a la recta
	numerador=fabs(a * coordPosActual.x + b * coordPosActual.y + c);
	denominador = sqrtf(a*a+b*b);

	distancia = numerador/denominador;
	return distancia; //distancia en metros
}
*/

static bool validarCruceRecta(GPS_Data_t * posActual, GPS_Data_t * origen, GPS_Data_t * estacionTerrena, int8_t paredASeguir){

	//P_Cartesiano origenPlano;
	P_Cartesiano posicionActual;
	P_Cartesiano estTerrena;

  //  origenPlano.x=0;
  //  origenPlano.y=0;

	estTerrena = gpsACartesiano(origen, estacionTerrena);
	posicionActual = gpsACartesiano(origen, posActual);

	// Paso 4: Calcular el producto cruz en 2D (z-componente del vector 3D).
	// Formula: (x1 * y2) - (x2 * y1)
	float cross_product = ( estTerrena.x * posicionActual.y) - (posicionActual.x * estTerrena.y);

	// Paso 5: Retornar el resultado basado en el signo del producto cruz.
	if (cross_product > 0 && paredASeguir == 1) { //estoy a la izquierda
		return true;
	} else if (cross_product < 0 && paredASeguir == -1) { //estoy a la derecha
		return true;
	} else {
		return false;
	}
}

/*
 *@brief tarea de navegacion taquito (es un algoritmo bicho tipo II).
 *@brief Se encarga de rodear un obstaculo de tamaño desconocido hasta alinearse de nuevo con su rumbo hacia la estacion terrena
 */
void navTaquito_task(void *argument){
	//creacion de variables a usar
		//variables de la logica de taquito
		static GPS_Data_t inicioTaquito, posActual, anteriorTaquito;
		static float  distRecta, disNodosTaq, disLateral;
		static bool banderaCallejon = true;
		static int8_t paredASeguir=-1, anteriorTaqID, contEsqExt=0;
		static uint8_t iDNodoTaquito=0; //hacemos el cast para el id nodo taquito nuevo (deberia  ser un solo nodo y ya???????
		static estado estadoTaq = NUEVO_TAQUITO;

		//varables de comunicacion
		evento_navegacion evento;
		static osStatus_t status;
		static control_command comunicacionControl_t;
		static ultrasonico distancias; //ultrasonico es una estructura que guarda los 3 sensores. hablar esto con joad
		//static HCSR04_Data_t received_ultrasonic_data; //-- variable de la prueba de comunicacion entre tareas, ultra-->taq,, Prueba 1. Despues reemplazar por la linea de arriba

		//estructuras de datos
		static Lista pilaNodosTaquito;
		static HashMap map;


		inicializarLista(&pilaNodosTaquito);
		hashmap_init(&map);

	for(;;){
		Serial_PrintString("Nav taquito en ejecucion...");
		switch(estadoTaq)
		{
			case NUEVO_TAQUITO:
				status = osMessageQueueGet(gpsDataQueueHandle, &inicioTaquito, NULL, osWaitForever);
				if(status==osOK){
					// Verificamos que el nodo que estemos creando no haya sido creado antes o este muy cerca del anterior.
					//   Si esta muy cerca, dimos una vuelta en circulo (hormiga pensante, sigue por la otra pared), sino, es un taquito nuevo*/
					if(LSize(&pilaNodosTaquito) > 0){
						anteriorTaqID= LpopFront(&pilaNodosTaquito)->nodoID;
						anteriorTaquito=hashmap_get(&map, anteriorTaqID);
						disNodosTaq=distanciaNodos(&inicioTaquito, &anteriorTaquito);
						if(disNodosTaq<=2.5){
							paredASeguir = 1; //--> esta muy cerca, dimos una vuelta en circulo, pasamos a seguir la otra pared
						}
					}

					//si la pared a seguir es -1 (es un taquito nuevo, agregamos el nodo a la pila)
					if(paredASeguir == -1){
						iDNodoTaquito++;
						LPushFront(&pilaNodosTaquito, iDNodoTaquito); //ID arbitrario para navTaquito
						hashmap_put(&map, iDNodoTaquito, inicioTaquito);
					}

					estadoTaq=SEGUIMIENTO_PARED;
					contEsqExt=0;

				}
			break;

			case SEGUIMIENTO_PARED:
				// transiciones primarias
				if(contEsqExt == 4) {
					estadoTaq=ESTADO_DE_ESCAPE;//si hicimos esquina exterior 4 veces seguidas, dimos una vuelta en circulo--> estamos perdidos --> caso de escape
					break;
				}
				if(banderaCallejon==false){
					estadoTaq = CALLEJON_SIN_SALIDA;
					break;
				}

				//revisar con joad la lectura de los tres sensores al mismo time
				status = osMessageQueueGet(sensorDataQueueHandle, &distancias/*&received_ultrasonic_data*/, NULL, osWaitForever);
				if(status==osOK){

					//validaciones de obstaculo frontal


					if(distancias.izquierdo.distance_cm < dist_interior_aceptable && distancias.derecho.distance_cm < dist_interior_aceptable)
					{
						contEsqExt = 0;
						estadoTaq = CALLEJON_SIN_SALIDA;
					}
					else if(distancias.izquierdo.distance_cm > umbral_grieta && distancias.derecho.distance_cm > umbral_grieta){
						contEsqExt = 0;
						estadoTaq = CALLEJON_SIN_SALIDA;
					}
					else if(paredASeguir==-1 && ((distancias.derecho.distance_cm >= dist_suelo_estandar && distancias.derecho.distance_cm <= umbral_grieta)/*||(distancias.derecho <=umbral_grieta)*/ ))
						estadoTaq = ESQUINA_EXTERIOR;
					else if (paredASeguir == 1 && ((distancias.izquierdo.distance_cm >= dist_suelo_estandar && distancias.izquierdo.distance_cm <= umbral_grieta)/*||(distancias.izquierdo <= umbral_grieta)*/))
						estadoTaq = ESQUINA_EXTERIOR;
					else if(distancias.frontal.distance_cm <= umbral_pared || distancias.frontal.distance_cm >= umbral_grieta){
						contEsqExt = 0;  //la contadora de esquinas exteriores vuelve a cero siempre que se ejecuten otros casos diferentes
						estadoTaq = ESQUINA_INTERIOR;
					}
					else
					{
						contEsqExt = 0;
						if(paredASeguir==-1) disLateral = distancias.derecho.distance_cm;
						else disLateral = distancias.izquierdo.distance_cm;
						ajustarPared(distRecta, disLateral);

						//mandar comandos de control a control
						comunicacionControl_t.direccion = ROVER_FORWARD;
						comunicacionControl_t.velocidad = 500;
						comunicacionControl_t.tiempo=10;
						status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);
						//Rover_Move(Rover, ROVER_FORWARD, 500);
					}
					break;

				}
				break;
			case ESQUINA_INTERIOR:
				if(paredASeguir==-1) {

					comunicacionControl_t.direccion = ROVER_LEFT;
					comunicacionControl_t.velocidad = 500;
					status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);
					//Rover_Move(Rover, ROVER_LEFT, 500); //AQUI nececito un giro de 90°
				}
				else{
					comunicacionControl_t.direccion = ROVER_RIGHT;
					comunicacionControl_t.velocidad = 500;
					status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);

				//	Rover_Move(Rover, ROVER_RIGHT, 500); //AQUI nececito un giro de 90°
				}
				estadoTaq = SEGUIMIENTO_PARED;
				break;

			case ESQUINA_EXTERIOR:
				contEsqExt++;
				//Para terminaciones abruptas
				comunicacionControl_t.direccion = ROVER_STOP;
				comunicacionControl_t.velocidad = 500;
				status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);
				//Rover_Move(Rover, ROVER_STOP, 500);
				if(paredASeguir == -1) {

					comunicacionControl_t.direccion = ROVER_RIGHT;
					comunicacionControl_t.velocidad = 500;
					status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);

				//	Rover_Move(Rover, ROVER_RIGHT, 500); //mirar como hace el movimiento, si sobre su eje central o como
				}
				else{
					comunicacionControl_t.direccion = ROVER_LEFT;
					comunicacionControl_t.velocidad = 500;
					status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);

				//	Rover_Move(Rover, ROVER_LEFT, 500);
				}

				//la idea es avanzar mas alla de dist_exterior_aceptable
				comunicacionControl_t.direccion = ROVER_FORWARD;
				comunicacionControl_t.velocidad = 600;
				status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);

				//Rover_Move(Rover, ROVER_FORWARD, 600);
				estadoTaq = SEGUIMIENTO_PARED;
				break;

			case CALLEJON_SIN_SALIDA:
				status = osMessageQueueGet(sensorDataQueueHandle,&distancias/*&received_ultrasonic_data*/, NULL, osWaitForever);
				//moverse hacia atras hasta encontrar un sensor con lectura libre
				banderaCallejon = false;

				comunicacionControl_t.direccion = ROVER_BACKWARD;
				comunicacionControl_t.velocidad = 300;
				status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);

				// Rover_Move(Rover, ROVER_BACKWARD, 300); //--> aqui como es con el prametro velocidad???
				if(paredASeguir==-1){ //si la pared a seguir es la derecha, debo revisar el sensor izquierdo
					if(distancias.izquierdo.distance_cm > dist_exterior_aceptable){
						banderaCallejon = true;

						comunicacionControl_t.direccion = ROVER_LEFT;
						comunicacionControl_t.velocidad = 500;
						status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);
						//Rover_Move(Rover, ROVER_LEFT, 500);

						comunicacionControl_t.direccion = ROVER_FORWARD;
						comunicacionControl_t.velocidad = 500;
						status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);
						//Rover_Move(Rover, ROVER_FORWARD, 500);
						estadoTaq = SEGUIMIENTO_PARED;
					}
				}else{
					if(distancias.derecho.distance_cm > dist_exterior_aceptable) {
						banderaCallejon = true;

						comunicacionControl_t.direccion = ROVER_RIGHT;
						comunicacionControl_t.velocidad = 500;
						status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);
						//Rover_Move(Rover, ROVER_RIGHT, 500);

						comunicacionControl_t.direccion = ROVER_FORWARD;
						comunicacionControl_t.velocidad = 500;
						status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);
						//Rover_Move(Rover, ROVER_FORWARD, 500);
						estadoTaq = SEGUIMIENTO_PARED;
					}
				}
				break;

			case ESTADO_DE_ESCAPE:
				status = osMessageQueueGet(sensorDataQueueHandle, &distancias/*&received_ultrasonic_data*/, NULL, osWaitForever);

				//si tiene espacio al frente, escapa
				if(distancias.frontal.distance_cm >= umbral_pared && distancias.frontal.distance_cm <= umbral_grieta){
					comunicacionControl_t.direccion = ROVER_FORWARD;
					comunicacionControl_t.velocidad = 1000;
					status = osMessageQueuePut(controlDataQueueHandle, &comunicacionControl_t, 0, 10);
					//Rover_Move(Rover, ROVER_FORWARD, 1000); //-----> enviar datos con la queue

				}

				evento = ERROR_DESCONOCIDO;
				osMessageQueuePut(navStatesQueueHandle,&evento, 0, 0);
				estadoTaq = NUEVO_TAQUITO;
				//vTaskSuspend(NULL);
				osThreadResume(navegacionHandle);
				osThreadSuspend(osThreadGetId());
				//de no haber espacio suficiente, simplemente rompe tod y hace relocalizacion con navGlobal
				break;

			default:
				evento = ERROR_DESCONOCIDO;
				osMessageQueuePut(navStatesQueueHandle,&evento, 0, 0);
				estadoTaq = NUEVO_TAQUITO;
				//vTaskSuspend(NULL);
				osThreadResume(navegacionHandle);
				osThreadSuspend(osThreadGetId());
				break;


		}

		//revisamos si ya cruzamos o no la recta, mientras no la hayams cruzado, seguimos siguiendo la pared
		status = osMessageQueueGet(gpsDataQueueHandle, &posActual, NULL, 10);
		if(status==osOK){
//			distRecta = distanciaARecta(&posActual, &inicioTaquito ,estacionTerrena);
			bool cruce = validarCruceRecta(&posActual, &inicioTaquito ,estacionTerrena, paredASeguir);
			if(cruce){
				//la proxima ejecucion de taquito se reinicia el algoritmo
				estadoTaq = NUEVO_TAQUITO;
				//comunicamos con el selector que se completo taquito correctamente
				evento = OBSTACULO_RODEADO;
				osMessageQueuePut(navStatesQueueHandle,&evento, 0, 0);
				//le cedemos el control al selector
				osThreadResume(navegacionHandle);
				osThreadSuspend(osThreadGetId());
			}
		}
	/*	//medir stakkk

		uint32_t free_words = osThreadGetStackSpace(osThreadGetId()); // stack libre en "words" (4 bytes)
		uint32_t free_bytes = free_words * sizeof(uint32_t);

		printf("Stack libre (min): %lu words = %lu bytes\r\n", free_words, free_bytes);
*/
//		Serial_PrintString("  NavTaquito en ejecucion...  ");
		osDelay(500);
	}
}
