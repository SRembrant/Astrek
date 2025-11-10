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

#include "Control_Pose.h"
#include "Taquito.h"
#include "Navegacion.h" //libreria de funciones de navegacion
#include "Serial.h"
#include "sr04.h"

// --- Funciones auxiliares (Propias de taquito) ---

// Variable estática para el controlador de pared dentro del archivo taquito.c
static WallFollow_Controller_t wall_ctrl;

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
 * el wall following
 * @param direccion Es un valor -1 o 1, indica que pared esta siguiendo el rover
 * 1 para la pared izquierda, -1 para la pared derecha
 * @param disLateral Distancia lateral medida (cm)
 * @return float Velocidad angular de corrección (wz_correction)
 */
static float ajustarPared(int8_t direccion, float disLateral_cm){
	// No hacemos ajustes si estamos en modo grieta.
	if(disLateral_cm >= umbral_grieta) {
        PID_Reset(&wall_ctrl.pid_wall);
        return 0.0f;
    }

    // Convertir a metros, que es la unidad estándar del sistema
    float disLateral_m = disLateral_cm / 100.0f;

    // Calcular el error: (Distancia Medida - Distancia Deseada)
    // Error positivo = muy lejos; Error negativo = muy cerca.
    float error = disLateral_m - wall_ctrl.dist_lateral_deseada;

    // Calcular la corrección angular (wz_correction) usando el PID
    float wz_correction = PID_Update(&wall_ctrl.pid_wall, error, POSE_CONTROL_DT);

    // La salida del PID es 'wz' para corregir, pero el signo debe reflejar
    // la pared que se está siguiendo.
    // Error (+) (Lejos) -> Pared DERECHA (-1): wz debe ser NEGATIVO (girar a la derecha)
    // Error (+) (Lejos) -> Pared IZQUIERDA (+1): wz debe ser POSITIVO (girar a la izquierda)

    // Se invierte la corrección para la pared derecha
    if (direccion == -1) { // Pared Derecha
        wz_correction = -wz_correction;
    }
    // Si direccion == 1 (Pared Izquierda), se mantiene el signo (más lejos -> más positivo wz)

    // La función que llama a ajustarPared deberá usar este wz_correction para el rover

    return wz_correction;
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

/**
 * @brief determina si se ha cruzado la linea recta que pasa entre la estacion terrena y el lugar donde empezo a ejecutarse taquito
 * @param posActual es la posicion actual del rover
 * @param origen es la posicion GPS donde se empezo a ejecutar taquito
 * @param estacionTerrena es la posicion GPS donde se ubica la estacion terrena
 * @param paredASeguir indica que lado de la pared se estaba siguiendo, es -1 o 1. Se valida que el valor calculado sea el contrario al recibido aqui
 * @return vedadero si se cruzo la linea, falso en caso contrario
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
	//	evento_navegacion evento;
		static osStatus_t status;
		static ControlCommand_t cmd;
		static ultrasonico distancias; //ultrasonico es una estructura que guarda los 3 sensores. hablar esto con joad
		//static HCSR04_Data_t received_ultrasonic_data; //-- variable de la prueba de comunicacion entre tareas, ultra-->taq,, Prueba 1. Despues reemplazar por la linea de arriba

		//estructuras de datos
		static Lista pilaNodosTaquito;
		static HashMap map;

		inicializarLista(&pilaNodosTaquito);
		hashmap_init(&map);

		//variables de la maquina de estados avance / ajuste
		float bearing; //bearing desde la posicion actual a la estacion terrena
		float heading; //heading del rover respecto al norte
		float diff; //diferencia entre el bearing y el heading

		//
		wall_ctrl.dist_lateral_deseada = dist_objetivo;

		    // Ganancias PID para el ajuste de pared (ejemplo de ajuste)
		    // Se recomienda que sea un P o PI con una Kp dominante
		    PID_Init(&wall_ctrl.pid_wall,
		             0.5f,     // Kp (Respuesta directa al error de distancia)
		             0.0f,     // Ki (Puede introducir inestabilidad, empezar en 0)
		             0.05f,    // Kd (Para amortiguar giros bruscos)
		             0.1f,     // Integral max (anti-windup)
		             -POSE_MAX_ANGULAR_VELOCITY,
					 POSE_MAX_ANGULAR_VELOCITY);

	for(;;){
		Serial_PrintString("Nav taquito en ejecucion...");
		switch(estadoTaq)
		{
			case AVANCE: //avanza en linea recta
				status = osMessageQueueGet(sensorDataQueueHandle,&distancias/*&received_ultrasonic_data*/, NULL, osWaitForever);

				if(status == osOK){
					if(distancias.frontal < umbral_pared || distancias.frontal > umbral_grieta){
						estadoTaq = NUEVO_TAQUITO;
					}
					else{

						// solo hay que mandar una estrcutura con la poscion actual x,y y el heading.
						cmd.mode = MODE_POSE_TARGET;
						cmd.target_x = 0.0; //si en la conversion del controlTask se usa la estacion terrena como orige su referenciia cartesiana siempre sera (0,0)
						cmd.target_y = 0.0;
						status = osMessageQueuePut(controlDataQueueHandle, &cmd, 0, 10);
					}
				}
				break;

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

						// 2. Calcular la corrección angular con el PID
						float wz_cmd_correction = ajustarPared(paredASeguir, disLateral);

						// 3. Crear Comando Cinémativo para ControlTask

						cmd.mode = MODE_WALL_FOLLOW;
						cmd.target_vx = WALL_FOLLOW_VX_BASE; // Velocidad de avance constante
						cmd.target_wz = wz_cmd_correction;       // Corrección angular del PID lateral
						// (Otros campos no son necesarios para este modo)
						status = osMessageQueuePut(controlDataQueueHandle, &cmd, 0, 10);
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

				estadoTaq = AVANCE;

				//de no haber espacio suficiente, simplemente rompe tod y hace relocalizacion con navGlobal
				break;

			default:

				estadoTaq = AVANCE;

				break;
		}

		//Antes del if otro if verficando que este en ajuste o avance para no entrar a estado fin, fin
		if(estadoTaq != AVANCE)
		{
			//revisamos si ya cruzamos o no la recta, mientras no la hayams cruzado, seguimos siguiendo la pared
			status = osMessageQueueGet(gpsDataQueueHandle, &posActual, NULL, 10);
			if(status==osOK)
			{
	//			distRecta = distanciaARecta(&posActual, &inicioTaquito ,estacionTerrena);
				bool cruce = validarCruceRecta(&posActual, &inicioTaquito ,estacionTerrena, paredASeguir);
				if(cruce)
				{
					//la proxima ejecucion de taquito se reinicia el algoritmo
					estadoTaq = AVANCE;
				}
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
