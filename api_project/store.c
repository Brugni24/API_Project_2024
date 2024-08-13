#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "store.h"
#include "utils.h"

// Tabella hash che rappresenta il magazzino:
// -> array di puntatori che punta alla merce (ingredienti) presente in magazzino
struct Goods** store = NULL;

struct Batch* create_batch(int quantity, int expiration){
    struct Batch* batch = (struct Batch*)malloc(sizeof(struct Batch));
    batch->quantity = quantity;
    batch->expiration = expiration;
    batch->next = NULL;
    return batch;
}

struct Goods* create_goods(char* name){
    struct Goods* goods = (struct Goods*)malloc(sizeof(struct Goods));
    goods->key = key_function(name);
    strcpy(goods->name, name);
    goods->batches_head = NULL;
    return goods;
}

void create_store(){
    store = (struct Goods**)calloc(TABLE_SIZE, sizeof(struct Goods*));
    for (int i = 0; i < TABLE_SIZE; i++){
        store[i] = NULL;
    }
}

int search_goods(char* name){
    int index = hash_function(name);
    int i = 0;
    while(store[index] != NULL){
        if(strcmp(store[index]->name, name) == 0){
            return index;
        }
        i++;
        index = double_hashing(name, i);
    }
    return -1;
}

void handle_collision_goods(struct Goods* goods, int index){
    int i = 1;
    while(store[index] != NULL){
        index = double_hashing(goods->name, i);
        i++;
    }
    store[index] = goods;
}

void insert_batch(char* name, int quantity, int expiration){
    struct Batch* new_batch = create_batch(quantity, expiration);
    int index = search_goods(name);
    if(store[index]->batches_head == NULL){ // lista vuota
        store[index]->batches_head = new_batch;
    }else{
        struct Batch* current = store[index]->batches_head;
        if(new_batch->expiration < current->expiration){ // new_batch va aggiunto in testa
            new_batch->next = current;
            store[index]->batches_head = new_batch;
        }else{
            while(current->next != NULL && new_batch->expiration > current->next->expiration){
                current = current->next;
            }
            new_batch->next = current->next;
            current->next = new_batch;
        }
    }
}

void insert_goods(char* name, int quantity, int expiration){
    int i = search_goods(name);
    if(i == -1){ // controllo se la merce è già presente
        struct Goods* goods = create_goods(name);
        int index = hash_function(name);
        struct Goods* current_item = store[index];
        if(current_item == NULL){ // cella vuota
            store[index] = goods;
            i = index;
        }else{ // cella occupata -> collisione
            handle_collision_goods(goods, (index+1));
            i = search_goods(name);
        }
    }
    insert_batch(name, quantity, expiration);
    store[i]->total_quantity += quantity;
}

void rifornimento(){
    char name[MAX_LEN];
    int quantity = 0;
    int expiration = 0;
    while(1){
        if(scanf("%s", name) > 0){ // nome goods
            if (scanf("%d", &quantity) > 0) { // quantità goods
                if (scanf("%d", &expiration) > 0) { // data di scadenza goods
                    insert_goods(name, quantity, expiration);
                    printf("rifornito\n");
                }
            }
        }
        // Controlla se il prossimo carattere è un newline o EOF
        int next_char = getchar();
        if (next_char == '\n' || next_char == EOF) {
            break;
        }
    }
}

void print_batch(){
    printf("\nTabella Store\n-------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (store[i]){
            printf("Name: %s ", store[i]->name);
            struct Batch* current = store[i]->batches_head;
            while(current != NULL){
                printf("-> %d %d ", current->quantity, current->expiration);
                current = current->next;
            }
            printf("\n");
        }
    }
    printf("-------------------\n\n");
}

void print_store()
{
    printf("\nTabella Store\n-------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (store[i]){
            printf("Index: %d, Key: %d, Name: %s, Quantity:%d\n", i, store[i]->key, store[i]->name, store[i]->total_quantity);
        }
    }
    printf("-------------------\n\n");
}