/*
 * HashMap.h
 *
 *  Created on: Sep 27, 2025
 *      Author: ASUS
 */

#ifndef INC_HERRAMIENTAS_HASHMAP_H_
#define INC_HERRAMIENTAS_HASHMAP_H_

#include <stdint.h>
#include <stddef.h>
#include "GPS.h"

// --- CONFIGURACIÓN DEL HASH MAP ---
#define HASHMAP_SIZE 16

// MAX_NODOS_HASH define el número máximo total de elementos clave-valor que el HashMap puede almacenar.
// Es el tamaño de tu pool de memoria estático.
#define MAX_NODOS_HASH      32

// Definición de la estructura del nodo del HashMap
typedef struct NodoHash {
    uint8_t clave;                // La clave (ID único de 8 bits)
    GPS_Data_t valor;                  // Dato GPS asociado a la clave
    struct NodoHash* siguiente;   // Puntero al siguiente nodo en caso de colisión (lista enlazada)
    uint8_t in_use;               // Bandera para el pool de memoria (0=libre, 1=en uso)
} NodoHash;

// Definición de la estructura del HashMap
typedef struct HashMap {
    NodoHash* buckets[HASHMAP_SIZE]; // Array de punteros a la cabeza de cada lista enlazada (cubo)
    uint8_t size;                     // Número actual de elementos en el HashMap
    NodoHash node_pool[MAX_NODOS_HASH]; // Pool de memoria estático para los nodos
} HashMap;

// Declaraciones de funciones
void hashmap_init(HashMap* map);
uint32_t hash_function(const uint8_t clave); // La función hash ahora es adecuada para uint8_t
int hashmap_put(HashMap* map, const uint8_t clave, GPS_Data_t valor); // Retorna 0=fallo, 1=insertado, 2=actualizado
GPS_Data_t hashmap_get(HashMap* map, const uint8_t clave);
int hashmap_remove(HashMap* map, const uint8_t clave);
uint8_t hashmap_size(HashMap* map); // Función para obtener el tamaño actual


#endif /* INC_HERRAMIENTAS_HASHMAP_H_ */
