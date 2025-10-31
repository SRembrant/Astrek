/*
 * Lista.c
 *
 *  Created on: Sep 27, 2025
 *      Author: ASUS
 */


#include <stdint.h>
#include "Lista.h"
#include "Nodo.h"
#include <stdlib.h>

void inicializarLista(struct Lista *list) {
    if (list != NULL) {
        list->inicio = NULL;
        list->ultimo = NULL;
        list->size = 0;
    }
}


/**
 * @brief Agrega un nodo al final de la lista.
 * @param list Puntero a la estructura de la lista.
 * @param data El dato a agregar.
 * @return true si se agregó correctamente, false en caso contrario.
 */
int LPushBack(Lista *list, uint8_t data) {
    if (list == NULL) return 0;

    NodoG *newNode = crearNodo(data);
    if (newNode == NULL) return 0; // Falló la asignación de memoria

    if (list->inicio == NULL) { // La lista está vacía
        list->inicio = newNode;
        list->ultimo = newNode;
    } else {
        list->ultimo->siguiente = newNode;
        newNode->anterior = list->ultimo;
        list->ultimo = newNode;
    }
    list->size++;
    return 1;
}

/**
 * @brief Agrega un nodo al principio de la lista.
 * @param list Puntero a la estructura de la lista.
 * @param data El dato a agregar.
 * @return true si se agregó correctamente, false en caso contrario.
 */
int LPushFront(Lista *list, uint8_t data) {
    if (list == NULL) return 0;

    NodoG *newNode = crearNodo(data);
    if (newNode == NULL) return 0; // Falló la asignación de memoria

    if (list->inicio == NULL) { // La lista está vacía
        list->inicio = newNode;
        list->ultimo = newNode;
    } else {
        newNode->siguiente = list->inicio;
        list->inicio->anterior = newNode;
        list->inicio = newNode;
    }
    list->size++;
    return 1;
}

/**
 * @brief Elimina el nodo con el dato especificado (primera ocurrencia).
 * @param list Puntero a la estructura de la lista.
 * @param data El dato a eliminar.
 * @return true si se eliminó correctamente, false en caso contrario.
 */
int LRemove(Lista *list, uint8_t data) {
    if (list == NULL || list->inicio == NULL) return 0;

    NodoG *current = list->inicio;
    while (current != NULL) {
        if (current->nodoID == data) {
            if (current->anterior != NULL) {
                current->anterior->siguiente = current->siguiente;
            } else {
                list->inicio = current->siguiente; // Era el primer nodo
            }

            if (current->siguiente != NULL) {
                current->siguiente->anterior = current->anterior;
            } else {
                list->ultimo = current->anterior; // Era el último nodo
            }

            // *** CONSIDERACIÓN CRÍTICA PARA STM32 ***
            // En lugar de free, aquí deberías "devolver" el nodo al pool de memoria.
            // Ejemplo:
            // return_node_to_pool(current);
            free(current);
            list->size--;
            return 1;
        }
        current = current->siguiente;
    }
    return 0; // Dato no encontrado
}

/**
 * @brief Busca un dato en la lista y retorna un puntero al nodo si lo encuentra.
 * @param list Puntero a la estructura de la lista.
 * @param data El dato a buscar.
 * @return Puntero al nodo que contiene el dato, o NULL si no se encuentra.
 */
NodoG* LSearch(Lista *list, uint8_t data) {
    if (list == NULL || list->inicio == NULL) return NULL;

    NodoG *current = list->inicio;
    while (current != NULL) {
        if (current->nodoID == data) {
            return current;
        }
        current = current->siguiente;
    }
    return NULL;
}

/**
 * @brief Obtiene el dato en una posición específica de la lista (0-indexed).
 * @param list Puntero a la estructura de la lista.
 * @param index La posición del dato a obtener.
 * @param data_out Puntero donde se almacenará el dato encontrado.
 * @return true si se encontró el dato, false en caso contrario.
 */
int LGetAt(Lista *list, size_t index, int *data_out) {
    if (list == NULL || list->inicio == NULL || index >= list->size || data_out == NULL) return 0;

    NodoG *current = list->inicio;
    // Optimización: recorrer desde head o tail dependiendo de la cercanía al índice
    if (index < list->size / 2) {
        for (size_t i = 0; i < index; i++) {
            current = current->siguiente;
        }
    } else {
        current = list->ultimo;
        for (size_t i = list->size - 1; i > index; i--) {
            current = current->anterior;
        }
    }
    *data_out = current->nodoID;
    return 1;
}

NodoG* LpopFront(Lista * list){
	return list->inicio;
}

/**
 * @brief Vacía la lista y libera toda la memoria de los nodos.
 * En un microcontrolador, esto implicaría devolver los nodos al pool.
 * @param list Puntero a la estructura de la lista.
 */
void LClear(Lista *list) {
    if (list == NULL) return;

    NodoG *current = list->inicio;
    NodoG *next;
    while (current != NULL) {
        next = current->siguiente;
        // *** CONSIDERACIÓN CRÍTICA PARA STM32 ***
        // En lugar de free, aquí deberías "devolver" el nodo al pool de memoria.
        // Ejemplo:
        // return_node_to_pool(current);
        free(current);
        current = next;
    }
    list->inicio = NULL;
    list->ultimo = NULL;
    list->size = 0;
}

/**
 * @brief Retorna el tamaño actual de la lista.
 * @param list Puntero a la estructura de la lista.
 * @return El número de elementos en la lista.
 */
uint8_t LSize(Lista *list) {
    if (list == NULL) return 0;
    return list->size;
}

