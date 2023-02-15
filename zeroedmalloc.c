#include "zeroedmalloc.h"
#include <malloc.h>
#include <string.h>

void* zeroedmalloc(size_t size){
    void* ptr = malloc(size);
    if (ptr == NULL){
        return NULL;
    }
    memset(ptr, 0, size);
    return ptr;
}
