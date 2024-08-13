#ifndef STORE_H
#define STORE_H

#define MAX_LEN 256

//* Dichiarazione delle strutture dati
struct Batch{ // lotti
    int quantity;
    int expiration;
    struct Batch* next;
};

struct Goods{ // merce presente nel magazzino -> ingredienti
    int key;
    char name[MAX_LEN];
    int total_quantity;
    struct Batch* batches_head;
};


//* Dichiarazione delle funzioni
struct Batch* create_batch(int quantity, int expiration);
struct Goods* create_goods(char* name);
void create_store();
int search_goods(char* name);
void handle_collision_goods(struct Goods* goods, int index);
void insert_batch(char* name, int quantity, int expiration);
void insert_goods(char* name, int quantity, int expiration);
void rifornimento();
void print_batch();
void print_store();

#endif