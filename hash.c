#include "hash.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "stdio.h"


#define CANT_INICIAL 0
#define CAPACIDAD_INICIAL 20

typedef enum estado {
    VACIO, OCUPADO, BORRADO
} estado_t;

typedef struct campo{
  char* clave;
  void* valor;
  estado_t estado;
} campo_t;

struct hash{
  size_t capacidad;
  size_t cantidad;
  hash_destruir_dato_t funcion_destruccion;
  campo_t* tabla;
};

struct hash_iter{
  const hash_t* hash;
  campo_t campo_act;
  size_t posicion;
};

    
unsigned long hash_f(char *str){
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}


hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t *hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;

    hash->tabla = malloc(CAPACIDAD_INICIAL * sizeof(campo_t));
    if(!hash->tabla){
        free(hash);
        return NULL;
    }

    for (int i = 0; i < CAPACIDAD_INICIAL; i++) {
        campo_t campo;
        campo.clave = NULL;
        campo.valor = NULL;
        campo.estado = VACIO;
        hash->tabla[i] = campo;
    }

    hash->cantidad = CANT_INICIAL;
    hash->capacidad = CAPACIDAD_INICIAL;
    if(destruir_dato) hash->funcion_destruccion = destruir_dato;
    else hash->funcion_destruccion = NULL;
    return hash;
}

bool hash_pertenece(const hash_t *hash, const char *clave) {
    size_t pos = hash_f( (char*) clave) % hash->capacidad;
    while (hash->tabla[pos].estado != VACIO) {
        if (strcmp( (char*) hash->tabla[pos].clave, clave) == 0) return true;
        if (pos == hash->capacidad) pos = 0;
        else pos++;
    }
    return false;
}

void *hash_obtener(const hash_t *hash, const char *clave) {
    size_t pos = hash_f( (char*) clave) % hash->capacidad;
    while (hash->tabla[pos].estado != VACIO) {
        if (strcmp( (char*) hash->tabla[pos].clave, clave) == 0) {
            return (char*) hash->tabla[pos].valor;
        }
        if (pos == hash->capacidad) pos = 0;
        else pos++;
    }
    return NULL;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){

    size_t pos = hash_f((char*) clave) % hash->capacidad;

    if (hash_pertenece(hash, clave)) {
        while (strcmp( (char*) hash->tabla[pos].clave, clave) != 0) {
            if (pos == hash->capacidad) pos = 0;
            else pos++;
        }
        hash->tabla[pos].valor = dato;
        return true;
    }
    
    size_t len = strlen(clave);
    char* copia_clave = malloc(sizeof(char) * len + 1);
    
    if (!copia_clave) return false;
    strcpy(copia_clave,clave);

    while (hash->tabla[pos].estado == OCUPADO){
        if (pos == hash->capacidad) pos = 0;
        else pos++;
    }
    
    hash->tabla[pos].clave = copia_clave;
    hash->tabla[pos].valor = dato;
    hash->tabla[pos].estado = OCUPADO;
    hash->cantidad++;

    return true;
}

size_t hash_cantidad(const hash_t *hash) {
    return hash->cantidad;
}

void *hash_borrar(hash_t *hash, const char *clave) {
    size_t pos = hash_f( (char*) clave) % hash->capacidad;
    while (hash->tabla[pos].estado != VACIO) {
        if (strcmp( (char*) hash->tabla[pos].clave, clave) == 0) {
            free(hash->tabla[pos].clave);
            void* dev = hash->tabla[pos].valor;
            campo_t campo;
            //campo.clave = NULL;
            campo.valor = NULL;
            campo.estado = BORRADO;
            hash->tabla[pos] = campo;
            hash->cantidad--;
            return dev;
        }
        if (pos == hash->capacidad) pos = 0;
        else pos++;        
    }
    return NULL;
}

void hash_destruir(hash_t *hash) {
    for (int i = 0; i < hash->capacidad; i++) {
        if (hash->tabla[i].estado == OCUPADO) {
            if (hash->funcion_destruccion) hash->funcion_destruccion(hash->tabla[i].valor);
            free(hash->tabla[i].clave);
        }
    }
    free(hash->tabla);
    free(hash);
}