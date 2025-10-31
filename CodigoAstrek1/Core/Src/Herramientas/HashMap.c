/*
 * HashMap.c
 *
 *  Created on: Sep 27, 2025
 *      Author: ASUS
 */

#include "HashMap.h"
#include <stddef.h>

// Siguiendo las recomendaciones de Gemini, se está usando un memory pool para gestion de memoria


// --- Funciones auxiliares para el pool de memoria --- //

/**
 * @brief Asigna una nueva entrada del pool de memoria estático.
 * @param map Puntero al HashMap que contiene el pool.
 * @return Puntero a un NodoHash libre o NULL si el pool está lleno.
 */
static NodoHash* allocate_node(HashMap* map) {
    for (uint8_t i = 0; i < MAX_NODOS_HASH; i++) {
        if (map->node_pool[i].in_use == 0) {
            map->node_pool[i].in_use = 1;
            map->node_pool[i].siguiente = NULL; // Limpiar el puntero para el nuevo uso
            return &map->node_pool[i];
        }
    }
    return NULL; // No hay nodos disponibles
}

/**
 * @brief Libera un nodo al pool de memoria estático (marcar como no usado).
 * @param node Puntero al NodoHash a liberar.
 */
static void free_node(NodoHash* node) {
    if (node != NULL) {
        node->in_use = 0;
        // Opcional: limpiar datos para seguridad/depuración
        // node->clave = 0;
        // node->valor = NULL;
        // node->siguiente = NULL;
    }
}

// --- Implementación de las funciones del HashMap --- //

void hashmap_init(HashMap* map) {
    if (map == NULL) return;
    for (uint8_t i = 0; i < HASHMAP_SIZE; i++) {
        map->buckets[i] = NULL;
    }
    map->size = 0;

    // Inicializa el pool de memoria: todos los nodos libres
    for (uint8_t i = 0; i < MAX_NODOS_HASH; i++) {
        map->node_pool[i].in_use = 0;
    }
}

/**
 * @brief Función hash para claves de tipo uint8_t.
 * Utiliza una simple operación bit a bit si HASHMAP_SIZE es potencia de 2.
 * @param clave La clave de tipo uint8_t.
 * @return El índice del cubo.
 */

uint32_t hash_function(const uint8_t clave) {
    // Esta es la forma más eficiente y común para hashear un entero
    // cuando el tamaño del array de buckets es una potencia de 2.
    return (uint32_t)clave & (HASHMAP_SIZE - 1);
    // Por ejemplo, si HASHMAP_SIZE es 64 (0b01000000), entonces HASHMAP_SIZE - 1 es 63 (0b00111111).
    // clave & 63 toma los 6 bits menos significativos de la clave.
}


/**
 * @brief Inserta o actualiza un par clave-valor en el HashMap.
 * @param map Puntero al HashMap.
 * @param clave La clave (uint8_t).
 * @param valor Puntero al valor (void*).
 * @return 0 si falla (ej. pool lleno), 1 si inserta, 2 si actualiza.
 */
int hashmap_put(HashMap* map, const uint8_t clave, GPS_Data_t valor) {
    if (map == NULL) return 0;

    uint32_t index = hash_function(clave);
    NodoHash* current = map->buckets[index];

    // 1. Buscar si la clave ya existe (actualización)
    while (current != NULL) {
        if (current->clave == clave) { // Comparación directa de uint8_t
            current->valor = valor;
            return 2; // Actualizado
        }
        current = current->siguiente;
    }

    // 2. Si no se encontró la clave, insertar una nueva entrada
    NodoHash* new_node = allocate_node(map); // Usar el pool de memoria
    if (new_node == NULL) {
        // Pool de memoria lleno, no se puede insertar
        return 0; // Falló
    }

    new_node->clave = clave;
    new_node->valor = valor;

    // Insertar al principio de la lista enlazada del cubo (simple y rápido)
    new_node->siguiente = map->buckets[index];
    map->buckets[index] = new_node;

    map->size++;
    return 1; // Insertado
}

/**
 * @brief Obtiene el valor asociado a una clave.
 * @param map Puntero al HashMap.
 * @param clave La clave a buscar (uint8_t).
 * @return Puntero al valor si se encuentra, o NULL si no.
 */
GPS_Data_t hashmap_get(HashMap* map, const uint8_t clave) {
    if (map == NULL) return;

    uint32_t index = hash_function(clave);
    NodoHash* node = map->buckets[index];

    while (node != NULL) {
        if (node->clave == clave) { // Comparación directa de uint8_t
            return node->valor;
        }
        node = node->siguiente;
    }
    return; // No encontrado
}

/**
 * @brief Elimina una entrada del HashMap.
 * @param map Puntero al HashMap.
 * @param clave La clave a eliminar (uint8_t).
 * @return 0 si la clave no se encuentra, 1 si se elimina.
 */
int hashmap_remove(HashMap* map, const uint8_t clave) {
    if (map == NULL) return 0;

    uint32_t index = hash_function(clave);
    NodoHash* node = map->buckets[index];
    NodoHash* prev = NULL;

    while (node != NULL) {
        if (node->clave == clave) { // Comparación directa de uint8_t
            if (prev != NULL) {
                prev->siguiente = node->siguiente;
            } else {
                map->buckets[index] = node->siguiente; // Era el primer nodo del cubo
            }
            free_node(node); // Devolver al pool de memoria
            map->size--;
            return 1; // Eliminado
        }
        prev = node;
        node = node->siguiente;
    }
    return 0; // Clave no encontrada
}

/**
 * @brief Retorna el número de elementos en el HashMap.
 * @param map Puntero al HashMap.
 * @return El número de elementos.
 */
uint8_t hashmap_size(HashMap* map) {
    if (map == NULL) return 0;
    return map->size;
}




