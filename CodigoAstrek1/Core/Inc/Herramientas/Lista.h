/*
 * Lista.h
 *
 *  Created on: Sep 27, 2025
 *      Author: ASUS
 */

#ifndef INC_HERRAMIENTAS_LISTA_H_
#define INC_HERRAMIENTAS_LISTA_H_

#include <stdint.h>
#include "Nodo.h"

 typedef struct Lista {
    struct NodoG *inicio;        // Puntero al primer nodo de la lista
    struct NodoG *ultimo;        // Puntero al último nodo de la lista
    uint8_t size;         // Número de elementos en la lista
} Lista;

void inicializarLista(struct Lista *list);
int LPushBack(Lista *list, uint8_t data);
int LPushFront(Lista *list, uint8_t data);
int LRemove(Lista *list, uint8_t data);
NodoG* LSearch(Lista *list, uint8_t data);
int LGetAt(Lista *list, size_t index, int *data_out);
NodoG* LpopFront(Lista * list);
void LClear(Lista *list);
uint8_t LSize(Lista *list);

#endif /* INC_HERRAMIENTAS_LISTA_H_ */
