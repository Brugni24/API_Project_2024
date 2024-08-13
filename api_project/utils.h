#ifndef UTILS_H
#define UTILS_H

#define TABLE_SIZE 50000
#define PRIME 49999 // numero primo per la funzione hash 2

unsigned int key_function(char *name);
unsigned int hash_function(char *name);
unsigned int hash_function2(char *name);
unsigned int double_hashing(char* name, int i);

#endif