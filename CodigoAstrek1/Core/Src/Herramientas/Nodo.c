/*
 * Nodo.c
 *
 *  Created on: Sep 27, 2025
 *      Author: ASUS
 */
#include "Nodo.h"
#include <stdlib.h>
/**
 * @brief Crea un nuevo nodo con el dato especificado.
 * En un microcontrolador, es preferible usar un pool de memoria estático
 * en lugar de malloc/free para evitar fragmentación y ser determinista.
 * @param data El dato a almacenar en el nuevo nodo.
 * @return Puntero al nuevo nodo, o NULL si la asignación falla.
 */
NodoG * crearNodo(uint8_t data) {
	/*** CONSIDERACIÓN CRÍTICA PARA STM32 ***
    // En lugar de malloc, aquí deberías usar un pool de memoria pre-asignado.
    // Ejemplo (pseudocódigo para un pool simple):
    // static Node_t node_pool[MAX_NODES];
    // static int next_free_node_idx = 0;
    // if (next_free_node_idx < MAX_NODES) {
    //     Node_t *newNode = &node_pool[next_free_node_idx++];
    //     newNode->data = data;
    //     newNode->prev = NULL;
    //     newNode->next = NULL;
    //     return newNode;
    // }
    // return NULL; // No hay nodos disponibles
	 *
	 */

	NodoG *newNode = (NodoG *)malloc(sizeof(NodoG));
	if (newNode != NULL) {
		newNode->nodoID = data;
		newNode->anterior = NULL;
		newNode->siguiente = NULL;
	}
	return newNode;
}



