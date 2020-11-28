#include "hash.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "stdio.h"

// ++
#define CANT_INICIAL 0
#define CAPACIDAD_INICIAL 20
#define VALOR_CARGA 0.7

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
        if (hash->tabla[pos].estado != BORRADO && strcmp( (char*) hash->tabla[pos].clave, clave) == 0){
            return true;  
        } 
        pos++;
        if (pos == hash->capacidad) pos = 0;
    }
    return false;
}

void *hash_obtener(const hash_t *hash, const char *clave) {
    size_t pos = hash_f((char*) clave) % hash->capacidad;

    while (hash->tabla[pos].estado != VACIO) {
        if (hash->tabla[pos].estado != BORRADO && strcmp( (char*) hash->tabla[pos].clave, clave) == 0) {
            return hash->tabla[pos].valor;
        }
        pos++;
        if (pos == hash->capacidad) pos = 0;
    }

    return NULL;
}


bool redimensionar(hash_t *hash, size_t nuevaCapacidad){

    campo_t* nuevaTabla = malloc(nuevaCapacidad * sizeof(campo_t));
    if(!nuevaTabla) return false;

    for(int i = 0; i < nuevaCapacidad; i++){
        campo_t campo;
        campo.clave = NULL;
        campo.valor = NULL;
        campo.estado = VACIO;
        nuevaTabla[i] = campo;
    }

    for(int i = 0; i < hash->capacidad; i++){
        if(hash->tabla[i].estado == OCUPADO){
            size_t pos = hash_f((char*) hash->tabla[i].clave) % nuevaCapacidad;
            size_t len = strlen(hash->tabla[i].clave);

            char* copia_clave = malloc(sizeof(char) * len + 1);
            if(!copia_clave) return false;
            strcpy(copia_clave,hash->tabla[i].clave);

            while (nuevaTabla[pos].estado == OCUPADO){
                pos++;
                if (pos == nuevaCapacidad) pos = 0;
            }

            nuevaTabla[pos].clave = copia_clave;
            nuevaTabla[pos].valor = hash->tabla[i].valor;
            nuevaTabla[pos].estado = OCUPADO;

            free(hash->tabla[i].clave);
        }
    }
    free(hash->tabla);
    hash->tabla = nuevaTabla;
    hash->capacidad = nuevaCapacidad;

    return true;
}


bool hash_guardar(hash_t *hash, const char *clave, void *dato){

    if((float)hash->cantidad / (float)hash->capacidad >= VALOR_CARGA){
        if(!redimensionar(hash, hash->capacidad*2)) return false;
    }
    
    size_t pos = hash_f((char*) clave) % hash->capacidad;

    if (hash_pertenece(hash, clave)) {
        while (strcmp( (char*) hash->tabla[pos].clave, clave) != 0) {
            pos++;
            if (pos == hash->capacidad) pos = 0;
        }

        if (hash->funcion_destruccion) hash->funcion_destruccion(hash->tabla[pos].valor);
        hash->tabla[pos].valor = dato;
        return true;

    }
    
    size_t len = strlen(clave);
    char* copia_clave = malloc(sizeof(char) * len + 1);
    if (!copia_clave) return false;
    strcpy(copia_clave,clave);

    while (hash->tabla[pos].estado == OCUPADO){
        pos++;
        if (pos == hash->capacidad) pos = 0;
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

        if (hash->tabla[pos].estado != BORRADO && strcmp( (char*) hash->tabla[pos].clave, clave) == 0) {
            void * devolver = hash->tabla[pos].valor;
            if (hash->funcion_destruccion) hash->funcion_destruccion(hash->tabla[pos].valor);
            free(hash->tabla[pos].clave);
            hash->tabla[pos].clave = NULL;
            hash->tabla[pos].estado = BORRADO;
            hash->cantidad--;
            return devolver;
        }

        pos++;        
        if (pos == hash->capacidad) pos = 0;
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

hash_iter_t *hash_iter_crear(const hash_t *hash){

    hash_iter_t *hash_iter = malloc(sizeof(hash_iter_t));

    if(!hash_iter) return NULL;

    hash_iter->hash = hash;

    size_t pos = 0;
    while(hash->tabla[pos].estado != OCUPADO){
        pos++;
        if( pos == hash->capacidad) break;
    }
    if(pos != hash->capacidad) hash_iter->campo_act = hash->tabla[pos];
    hash_iter->posicion = pos;

    return hash_iter;

}

bool hash_iter_avanzar(hash_iter_t *iter){
    if(hash_iter_al_final(iter)) return false;
    while (!hash_iter_al_final(iter))
    {
        iter->posicion++;
        if(hash_iter_al_final(iter)) break;
        if(iter->hash->tabla[iter->posicion].estado == OCUPADO) break;
    }
    return true;
    
}

const char *hash_iter_ver_actual(const hash_iter_t *iter){
    if(hash_iter_al_final(iter)) return NULL;
    return iter->hash->tabla[iter->posicion].clave;
}

bool hash_iter_al_final(const hash_iter_t *iter){
    return iter->posicion == iter->hash->capacidad;
}

void hash_iter_destruir(hash_iter_t *iter){
    free(iter);
}