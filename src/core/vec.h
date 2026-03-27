#ifndef BABA_CORE_VEC_H
#define BABA_CORE_VEC_H

#include <stdlib.h>
#include <string.h>
#include "types.h"

typedef struct {
    void* data;
    size_t size;
    size_t capacity;
    size_t element_size;
} BabaVec;

BABA_INLINE int baba_vec_init(BabaVec* vec, size_t element_size, size_t initial_capacity) {
    vec->element_size = element_size;
    vec->size = 0;
    vec->capacity = initial_capacity;
    if (initial_capacity > 0) {
        vec->data = malloc(element_size * initial_capacity);
        if (!vec->data) return BABA_ERROR_OUT_OF_MEMORY;
    } else {
        vec->data = NULL;
    }
    return BABA_SUCCESS;
}

BABA_INLINE void baba_vec_destroy(BabaVec* vec) {
    free(vec->data);
    vec->data = NULL;
    vec->size = 0;
    vec->capacity = 0;
}

BABA_INLINE int baba_vec_push(BabaVec* vec, const void* element) {
    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity == 0 ? 8 : vec->capacity * 2;
        void* new_data = realloc(vec->data, vec->element_size * new_capacity);
        if (!new_data) return BABA_ERROR_OUT_OF_MEMORY;
        vec->data = new_data;
        vec->capacity = new_capacity;
    }
    memcpy((char*)vec->data + vec->size * vec->element_size, element, vec->element_size);
    vec->size++;
    return BABA_SUCCESS;
}

BABA_INLINE void* baba_vec_get(BabaVec* vec, size_t index) {
    if (index >= vec->size) return NULL;
    return (char*)vec->data + index * vec->element_size;
}

BABA_INLINE void baba_vec_pop(BabaVec* vec) {
    if (vec->size > 0) vec->size--;
}

BABA_INLINE void baba_vec_clear(BabaVec* vec) {
    vec->size = 0;
}

#define BABA_VEC_FOREACH(vec, ptr) \
    for (size_t _i = 0; _i < (vec).size && ((ptr) = baba_vec_get(&(vec), _i)); _i++)

#endif