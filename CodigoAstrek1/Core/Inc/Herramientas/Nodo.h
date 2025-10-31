/*
 * Nodo.h
 *
 *  Created on: Sep 27, 2025
 *      Author: ASUS
 */

#ifndef INC_HERRAMIENTAS_NODO_H_
#define INC_HERRAMIENTAS_NODO_H_

#include<stdint.h>
#include "GPS.h"

typedef struct NodoG {
    uint8_t nodoID;
  //  struct GPS_Data *datosGPS;
    struct NodoG *siguiente;
    struct NodoG *anterior;
} NodoG;

NodoG * crearNodo(uint8_t data);

#endif /* INC_HERRAMIENTAS_NODO_H_ */
