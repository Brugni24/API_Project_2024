#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

unsigned int key_function(char *name){
    unsigned int k = 5381;
    int c;
    while ((c = *name++))
        k = ((k << 5) + k) + c;
    return k;
}

unsigned int hash_function(char *name){
    return key_function(name) % TABLE_SIZE;
}

unsigned int hash_function2(char *name){
    return PRIME - (key_function(name) % PRIME);
}

//* Open addressing -> double hashing
unsigned int double_hashing(char* name, int i){
    return (hash_function(name) + i*hash_function2(name)) % TABLE_SIZE;
}
