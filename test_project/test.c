#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 256
#define TABLE_SIZE 50000
#define PRIME 49999 // numero primo per la funzione hash 2

int collisioni = 0;

struct Ingredient{
    int key;
    char name[MAX_LEN];
    int quantity;
};

struct Ingredients{
    struct Ingredient** ingredients; // Array di puntatori
    int count;
};

struct Recipe{
    int key;
    char name[MAX_LEN];
    struct Ingredients* ingredients;
};

struct Recipes{
    struct Recipe** recipes; // array di puntatori per le ricette
    int count;
};

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

// Tabella hash che rappresenta il magazzino:
// -> array di puntatori che punta alla merce (ingredienti) presente in magazzino
struct Goods** store = NULL;

//* Funzione per trasformare un stringa in un intero
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

struct Ingredient* create_ingredient(char* name, int quantity){
    struct Ingredient* item = (struct Ingredient*) malloc(sizeof(struct Ingredient)); // Alloca la memoria per il puntatore ad un nuovo ingrediente
    item->key = key_function(name);
    strcpy(item->name, name);
    item->quantity = quantity;

    return item;
}

struct Ingredients* create_table_ingredients(){
    // Creazione tabella hash per la gestione degli ingredienti
    struct Ingredients* table = (struct Ingredients*) malloc(sizeof(struct Ingredients)); // alloca spazio per la tabella
    table->count = 0;
    table->ingredients = (struct Ingredient**) calloc(TABLE_SIZE, sizeof(struct Ingredient*)); // alloca spazio per le celle della tabella
    for (int i = 0; i < TABLE_SIZE; i++){
        table->ingredients[i] = NULL;
    }
    return table;
}

struct Recipes* create_table_recipes(){
    // Creazione tabella hash per la gestione delle ricette
    struct Recipes* table = (struct Recipes*) malloc(sizeof(struct Recipes)); // alloca spazio per la tabella
    table->count = 0;
    table->recipes = (struct Recipe**) calloc(TABLE_SIZE, sizeof(struct Recipe*)); // alloca spazio per le celle della tabella
    for (int i = 0; i < TABLE_SIZE; i++){
        table->recipes[i] = NULL;
    }
    return table;
}

struct Recipe* create_recipe(char* name, struct Ingredients* tb_ingredient){
    struct Recipe* recipe = (struct Recipe*)malloc(sizeof(struct Recipe));
    recipe->key = key_function(name);
    strcpy(recipe->name, name);
    recipe->ingredients = tb_ingredient;
    return recipe;
}

void print_table(struct Ingredients* table)
{
    printf("\nTabella Ingredienti\n-------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (table->ingredients[i]){
            printf("Index: %d, Key: %d, Value: %s, Quantity: %d\n", i, table->ingredients[i]->key, table->ingredients[i]->name, table->ingredients[i]->quantity);
        }
    }
    printf("-------------------\n\n");
}

void print_table_recipe(struct Recipes* table)
{
    printf("\nTabella Ricette\n-------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (table->recipes[i]){
            printf("Index: %d, Key: %d, Value: %s\n", i, table->recipes[i]->key, table->recipes[i]->name);
        }
    }
    printf("-------------------\n\n");
}

void handle_collision_ingredient(struct Ingredients* table, struct Ingredient* ingredient, int index){
    int i = 1;
    while(table->ingredients[index] != NULL){
        index = double_hashing(ingredient->name, i);
        i++;
    }
    collisioni += i;
    table->ingredients[index] = ingredient;
    table->count++;
}

void handle_collision_recipe(struct Recipes* table, struct Recipe* recipe, int index){
    int i = 1;
    while(table->recipes[index] != NULL){
        index = double_hashing(recipe->name, i);
        i++;
    }
    collisioni += i;
    table->recipes[index] = recipe;
    table->count++;
}

void insert_ingredient(struct Ingredients* table, char* name, int quantity){
    // Crea l'item
    struct Ingredient* item = create_ingredient(name, quantity);

    // Calcola l'indice
    int index = hash_function(name);

    struct Ingredient* current_item = table->ingredients[index];

    if (current_item == NULL){ // Cella vuota
        if (table->count == TABLE_SIZE){ // Tabella piena
            printf("Insert Error: Hash Table is full\n");
            return;
        }
        table->ingredients[index] = item;
        table->count++;
    }else{ // Cella occupata --> collisione
        handle_collision_ingredient(table, item, (index+1));
        return;
    }
}

void insert_recipe(struct Recipes* table, struct Recipe* recipe){
    // Calcola l'indice
    int index = hash_function(recipe->name);

    struct Recipe* current_item = table->recipes[index];

    if (current_item == NULL){ // Cella vuota
        if (table->count == TABLE_SIZE){ // Tabella piena
            printf("Insert Error: Hash Table is full\n");
            return;
        }
        table->recipes[index] = recipe;
        table->count++;
    }else{ // Cella occupata --> collisione
        handle_collision_recipe(table, recipe, (index+1));
        return;
    }
}

struct Ingredient* search_ingredient(struct Ingredients* table, char* name){
    int index = hash_function(name);
    struct Ingredient* item = table->ingredients[index];
    int i = 0;
    while(item != NULL){
        if(strcmp(item->name, name) == 0){
            return item;
        }
        i++;
        index = double_hashing(name, i);
        item = table->ingredients[index];
    }
    return NULL;
}

int search_recipe(struct Recipes* table, char* name){
    int index = hash_function(name);
    struct Recipe* item = table->recipes[index];
    int i = 0;
    while(item != NULL){
        if(strcmp(item->name, name) == 0){
            return index;
        }
        i++;
        index = double_hashing(name, i);
        item = table->recipes[index];
    }
    return -1;
}

void print_search_ingredient(struct Ingredients* table, char* name)
{
    struct Ingredient* res = search_ingredient(table, name);
    if (res == NULL)
    {
        printf("Key:%s does not exist\n", name);
        return;
    }
    else {
        printf("Key:%d, Value:%s\n", res->key, res->name);
    }
}

void read_recipe(struct Recipes* table){
    struct Recipe* recipe = NULL;
    struct Ingredients* tb_ingredients = create_table_ingredients();
    char name[MAX_LEN];
    int quantity = 0;
    if(scanf("%s", name)>0){ // nome ricetta
        if(search_recipe(table, name) == -1){
            recipe = create_recipe(name, tb_ingredients);
            while(1){
                if (scanf("%s", name) > 0) { // nome ingrediente
                    if (scanf("%d", &quantity) > 0) { // quantità ingrediente
                        insert_ingredient(tb_ingredients, name, quantity);
                    }
                } 
                // Controlla se il prossimo carattere è un newline o EOF
                int next_char = getchar();
                if (next_char == '\n' || next_char == EOF) {
                    break;
                }
            }
            insert_recipe(table, recipe);
            printf("aggiunta\n");
        }else{
            printf("ignorato\n");
            while(1){ // consumo la riga
                int next_char = getchar();
                if (next_char == '\n' || next_char == EOF) {
                    break;
                }
            }
        }  
    }
}

void free_ingredients(struct Ingredients* table){
    for(int i = 0; i < TABLE_SIZE; i++){
        if(table->ingredients[i] != NULL){
            free(table->ingredients[i]); 
        }
    }
    free(table->ingredients);
    free(table);
}

void free_recipe(struct Recipe* recipe){
    free_ingredients(recipe->ingredients);
    free(recipe);
}

void remove_recipe(struct Recipes* table){
    char name[MAX_LEN];
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta
        if(search_recipe(table, name) == -1){ // la ricetta non è presente
            printf("non presente\n");
        }else{ // la ricetta è presente
            if(1 < 0){  // è presente un ordine con la ricetta in questione

            }else{
                int index = search_recipe(table, name);
                free_recipe(table->recipes[index]);
                table->recipes[index] = NULL;
                printf("rimossa\n");
            }
        }
    }
}

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
    collisioni += i;
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

int main(){
    // int t_corriere = 0;
    // int q_corriere = 0;

    //* Lettura dati corriere: periodo e quantità
    // if(scanf("%i", &t_corriere)<1){
    //     printf("Errore nella lettura di t_corriere");
    //     return 0;
    // }else{
    //     printf("Periodo corriere: %i\n", t_corriere);
    // }
    // if(scanf("%i", &q_corriere)<1){
    //     printf("Errore nella lettura di q_corriere");
    //     return 0;
    // }else{
    //     printf("Quantità corriere: %i\n", q_corriere);
    // }

    //* Lettura comandi
    char comando[MAX_LEN];
    struct Recipes* tb_recipe = create_table_recipes();
    create_store(); // alloca memoria per il vettore di puntatori

    while(scanf("%s", comando) > 0){
        // Determino il comando
        if(strcmp(comando, "aggiungi_ricetta") == 0){
            read_recipe(tb_recipe);
        }else{
            if(strcmp(comando, "rimuovi_ricetta") == 0){
                remove_recipe(tb_recipe);
            }else{
                if(strcmp(comando, "rifornimento") == 0){
                    rifornimento();
                }else{
                    if(strcmp(comando, "ordine")==0){
                        printf("Comando: %s\n", comando);
                    }else{
                        printf("Comando non riconosciuto.\n");
                    }
                }
            }
        }
    }
    print_table_recipe(tb_recipe);
    print_store();
    print_batch();
    return 0;
}