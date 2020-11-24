#include "hash.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#define CANTINICIAL 0
#define CAPACIDADINICIAL 20

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

    
unsigned long hash_f(unsigned char *str){
    unsigned long hash = 5381;
    int c;

    while (c = *str++){
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}


hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t *hash = malloc(sizeof(hash_t));
    if(!hash) return NULL;

    hash->tabla = malloc(CAPACIDADINICIAL * sizeof(campo_t*));
    if(!hash->tabla){
        free(hash);
        return NULL;
    }

    hash->cantidad = CANTINICIAL;
    hash->capacidad = CAPACIDADINICIAL;
    if(destruir_dato) hash->funcion_destruccion = destruir_dato;
    return hash;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){

    char* copia_clave = malloc(sizeof(char) * strlen(clave));
    if (!copia_clave) return false;
    strcpy(copia_clave,clave);

    size_t pos = hash_f(copia_clave) % hash->capacidad;


    campo_t campo;
    campo.clave = copia_clave;
    campo.valor = dato;
    campo.estado = OCUPADO;

    while (hash->tabla[pos].estado == OCUPADO){
        pos++;
    }
    
    hash->tabla[pos] = campo;
    return true;
}