#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 256
#define TABLE_SIZE 50000
#define PRIME 49999 // numero primo per la funzione hash 2

int time = 0;

//* DEFINIZIONE STRUTTURE DATI
struct Ingredient{
    int key;
    char name[MAX_LEN];
    int quantity;
};

struct Recipe{
    int key;
    char name[MAX_LEN];
    struct Ingredient** ingredients;
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

struct Order{
    int arrival_time;
    int quantity;
    char recipe[MAX_LEN];
    struct Order* next;
    struct Order* prev;
};

//* ISTANZE GLOBALI
struct Recipe** recipe_book = NULL; // Tabella hash per la raccolta delle ricette
struct Goods** store = NULL; // Tabella hash che rappresenta il magazzino: array di puntatori che punta alla merce (ingredienti) presente in magazzino
struct Order* prepared_orders_head = NULL; // Ordini pronti ad essere spediti
struct Order* prepared_orders_tail = NULL;
struct Order* pending_orders_head = NULL; // Ordini in attesa di ricevere gli ingredienti
struct Order* pending_orders_tail = NULL;

//* DEFINIZIONE FUNZIONI GENERALI
unsigned int key_function(char *name){ // Algoritmo DJB2
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

unsigned int double_hashing(char* name, int i){ // Open addressing -> double hashing
    return (hash_function(name) + i*hash_function2(name)) % TABLE_SIZE;
}


//* FUNZIONI DI GESTIONE DEGLI INGREDIENTI
struct Ingredient* create_ingredient(char* name, int quantity){
    struct Ingredient* ingredient = (struct Ingredient*)malloc(sizeof(struct Ingredient)); // Alloca la memoria per il puntatore ad un nuovo ingrediente
    ingredient->key = key_function(name);
    strcpy(ingredient->name, name);
    ingredient->quantity = quantity;
    return ingredient;
}

struct Ingredient** create_ingredients_table(){
    struct Ingredient** tb_ingredients = (struct Ingredient**)calloc(TABLE_SIZE, sizeof(struct Ingredient*)); // alloca spazio per le celle della tabella
    for (int i = 0; i < TABLE_SIZE; i++){
        tb_ingredients[i] = NULL;
    }
    return tb_ingredients;
}

void handle_collision_ingredient(struct Ingredient** tb_ingredients, struct Ingredient* ingredient, int index){
    int i = 1;
    while(tb_ingredients[index] != NULL){
        index = double_hashing(ingredient->name, i);
        i++;
    }
    tb_ingredients[index] = ingredient;
}

void insert_ingredient(struct Ingredient** tb_ingredients, char* name, int quantity){
    struct Ingredient* ingredient = create_ingredient(name, quantity);
    int index = hash_function(name);
    struct Ingredient* current = tb_ingredients[index];
    if (current == NULL){ // Cella vuota
        tb_ingredients[index] = ingredient;
    }else{ // Cella occupata --> collisione
        handle_collision_ingredient(tb_ingredients, ingredient, (index+1));
        return;
    }
}

int search_ingredient(struct Ingredient** tb_ingredients, char* name){
    int index = hash_function(name);
    struct Ingredient* ingredient = tb_ingredients[index];
    int i = 0;
    while(ingredient != NULL){
        if(strcmp(ingredient->name, name) == 0){
            return index;
        }
        i++;
        index = double_hashing(name, i);
        ingredient = tb_ingredients[index];
    }
    return -1;
}

void free_ingredients(struct Ingredient** tb_ingredients){
    for(int i = 0; i < TABLE_SIZE; i++){
        if(tb_ingredients[i] != NULL){
            free(tb_ingredients[i]); 
        }
    }
    free(tb_ingredients);
}

//* FUNZIONI DI GESTIONE DELLE RICETTE
struct Recipe* create_recipe(char* name, struct Ingredient** tb_ingredients){
    struct Recipe* recipe = (struct Recipe*)malloc(sizeof(struct Recipe));
    recipe->key = key_function(name);
    strcpy(recipe->name, name);
    recipe->ingredients = tb_ingredients;
    return recipe;
}

void create_recipe_book(){
    recipe_book = (struct Recipe**)calloc(TABLE_SIZE, sizeof(struct Recipe*)); // alloca spazio per le celle della tabella
    for (int i = 0; i < TABLE_SIZE; i++){
        recipe_book[i] = NULL;
    }
}

void handle_collision_recipe(struct Recipe* recipe, int index){
    int i = 1;
    while(recipe_book[index] != NULL){
        index = double_hashing(recipe->name, i);
        i++;
    }
    recipe_book[index] = recipe;
}

void insert_recipe(struct Recipe* recipe){
    int index = hash_function(recipe->name);
    struct Recipe* current = recipe_book[index];

    if (current == NULL){ // Cella vuota
        recipe_book[index] = recipe;
    }else{ // Cella occupata --> collisione
        handle_collision_recipe(recipe, (index+1));
        return;
    }
}

int search_recipe(char* name){
    int index = hash_function(name);
    struct Recipe* recipe = recipe_book[index];
    int i = 0;
    while(recipe != NULL){
        if(strcmp(recipe->name, name) == 0){
            return index;
        }
        i++;
        index = double_hashing(name, i);
        recipe = recipe_book[index];
    }
    return -1;
}

void read_recipe(){
    struct Recipe* recipe = NULL;
    struct Ingredient** tb_ingredients = create_ingredients_table();
    char name[MAX_LEN];
    int quantity = 0;
    if(scanf("%s", name)>0){ // nome ricetta
        if(search_recipe(name) == -1){
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
            insert_recipe(recipe);
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

void free_recipe(struct Recipe* recipe){
    free_ingredients(recipe->ingredients);
    free(recipe);
}

int search_recipe_orders(struct Order* head, char* name){
    struct Order* current = head;
    while(current != NULL){
        if(strcmp(current->recipe, name) == 0){
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void remove_recipe(){
    char name[MAX_LEN];
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta
        if(search_recipe(name) == -1){ // la ricetta non è presente
            printf("non presente\n");
        }else{ // la ricetta è presente
            if(search_recipe_orders(prepared_orders_head, name) || search_recipe_orders(pending_orders_head, name)){  // è presente un ordine con la ricetta in questione
                printf("ordini in sospeso\n");
                return;
            }else{
                int index = search_recipe(name);
                free_recipe(recipe_book[index]);
                recipe_book[index] = NULL;
                printf("rimossa\n");
            }
        }
    }
}

//* FUNZIONI DI GESTIONE DEL MAGAZZINO E DEI LOTTI
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
                }
            }
        }
        // Controlla se il prossimo carattere è un newline o EOF
        int next_char = getchar();
        if (next_char == '\n' || next_char == EOF) {
            break;
        }
    }
    printf("rifornito\n");
}

void delete_batch_head(int goods_index){
    struct Batch* current = store[goods_index]->batches_head;
    store[goods_index]->batches_head = current->next;
    free(current);
}

void remove_batch(int goods_index, int total_quantity_needed){
    while(total_quantity_needed > 0 && store[goods_index]->batches_head != NULL){
        total_quantity_needed -= store[goods_index]->batches_head->quantity;
        if(total_quantity_needed >= 0){ // mi servono altri batch
            delete_batch_head(goods_index);
        }else{ // devo sottrare la quantità dell'ingrediente usato dalla batch in testa
            store[goods_index]->batches_head->quantity -= total_quantity_needed;
        }
    }
}

void delete_expired_batch(int goods_index){
    while(store[goods_index]->batches_head != NULL){
        if(store[goods_index]->batches_head->expiration <= time){
            delete_batch_head(goods_index);
        }else{
            return;
        }
    }
}

//* FUNZIONI DI GESTIONE DEGLI ORDINI
struct Order* create_order(int arrival_time, char* recipe, int quantity){
    struct Order* new_order = (struct Order*)malloc(sizeof(struct Order));
    new_order->arrival_time = arrival_time + 1;
    strcpy(new_order->recipe, recipe);
    new_order->quantity = quantity;
    new_order->next = NULL;
    new_order->prev = NULL;
    return new_order;
}

void insert_prepared_order(int arrival_time, char* recipe, int quantity){
    struct Order* new_order = create_order(arrival_time, recipe, quantity);
    if(prepared_orders_head == NULL){ // lista vuota
        prepared_orders_head = new_order;
        prepared_orders_tail = new_order;
    }else{
        new_order->next = prepared_orders_head;
        prepared_orders_head->prev = new_order;
        prepared_orders_head = new_order;
    }
}

void insert_pending_order(int arrival_time, char* recipe, int quantity){
    struct Order* new_order = create_order(arrival_time, recipe, quantity);
    if(pending_orders_head == NULL){ // lista vuota
        pending_orders_head = new_order;
        pending_orders_tail = new_order;
    }else{ // aggiungi in ordine di arrivo
        struct Order* current = pending_orders_head;
        while(current != NULL && current->arrival_time < arrival_time){
            current = current->next;
        }
        
        if(current == NULL){
            pending_orders_tail->next = new_order;
            new_order->prev = pending_orders_tail;
            pending_orders_tail = new_order;
        }else{
            if(current == pending_orders_head){
                new_order->next = pending_orders_head;
                pending_orders_head->prev = new_order;
                pending_orders_head = new_order;
            }else{
                new_order->next = current;
                new_order->prev = current->prev;
                current->prev->next = new_order;
                current->prev = new_order;
            }
        }
    }
}

int check_ingredients_availability(char* name, int quantity){
    struct Recipe* recipe = recipe_book[search_recipe(name)];
    int goods_index;
    int total_quantity_needed;
    for (int i = 0; i < TABLE_SIZE; i++){
        if(recipe->ingredients[i] != NULL){
            goods_index = search_goods(recipe->ingredients[i]->name);
            if(goods_index == -1){
                return 0; // ingrediente non trovato nel magazzino
            }
            delete_expired_batch(goods_index);
            total_quantity_needed = recipe->ingredients[i]->quantity * quantity;
            if(store[goods_index]->total_quantity < total_quantity_needed){
                return 0; // scorte insufficienti
            }
        }
    }
    return 1;
}

void order_preparation(char* name, int quantity){
    if(check_ingredients_availability(name, quantity)){ // ci sono ingredienti a sufficienza per preparare l'ordine
        insert_prepared_order(time, name, quantity);
        // eliminare ingredienti usati
        int recipe_index = search_recipe(name);
        int goods_index = 0;
        int total_quantity_needed = 0;
        for(int i = 0; i < TABLE_SIZE; i++){ // scorro gli ingredienti della ricetta
            if(recipe_book[recipe_index]->ingredients[i] != NULL){
                goods_index = search_goods(recipe_book[recipe_index]->ingredients[i]->name); // indice ingrediente nel magazzino
                total_quantity_needed = recipe_book[recipe_index]->ingredients[i]->quantity*quantity;
                remove_batch(goods_index, total_quantity_needed);
            }
        }
    }else{
        insert_pending_order(time, name, quantity);
    }
}

void prepare_pending_order(){
    struct Order* current = pending_orders_tail;
    while(current != pending_orders_head){
        if(check_ingredients_availability(current->recipe, current->quantity)){
            order_preparation(current->recipe, current->quantity);
        }
        current = current->prev;
    }
}

void delete_order_tail(){
    if(prepared_orders_tail == NULL){
        return;
    }
    struct Order* temp = prepared_orders_tail;
    if(prepared_orders_head == prepared_orders_tail){
        prepared_orders_head = NULL;
        prepared_orders_tail = NULL;
    }else{
        prepared_orders_tail = prepared_orders_tail->prev;
        prepared_orders_tail->next = NULL;
    }
    free(temp);
}

void ordine(){ // legge nome della ricetta e la quantità da preparare
    char name[MAX_LEN];
    int quantity = 0;
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta dell'ordine
        if(search_recipe(name) == -1){ // la ricetta non è presente
            printf("rifiutato\n");
            return;
        }else{ // la ricetta è presente
            if(scanf("%d", &quantity) > 0){
                order_preparation(name, quantity);
                printf("accettato\n");
            }
        }
    }
}

//* CARICO FURGONE
void load_truck(int capacity){
    int i = 0;
    while(prepared_orders_tail != NULL && capacity > 0){
        if(prepared_orders_tail->quantity <= capacity){
            printf("%d %s %d\n", prepared_orders_tail->arrival_time, prepared_orders_tail->recipe, prepared_orders_tail->quantity);
            capacity -= prepared_orders_tail->quantity;
            delete_order_tail();
            i++;
        }else{
            break;
        }
    }
    if(i == 0){
        printf("camioncino vuoto\n");
    }
}

//* FUNZIONI PER DEBUGGING
void print_ingredients_table(struct Ingredient** tb_ingredients){
    printf("Ingredienti: ");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (tb_ingredients[i]){
            printf("%s, %d  -  ", tb_ingredients[i]->name, tb_ingredients[i]->quantity);
        }
    }
    printf("\n");
}

void print_recipe_book(){
    printf("-------------------\nTABELLA RICETTE:\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (recipe_book[i]){
            printf("Name: %s\n", recipe_book[i]->name);
            print_ingredients_table(recipe_book[i]->ingredients);
        }
    }
    printf("-------------------\n");
}

void print_batch(struct Goods* goods){
    printf("Lotti: ");
    struct Batch* current = goods->batches_head;
    while(current != NULL){
        printf("Quantity: %d, Expiration: %d  -  ", current->quantity, current->expiration);
        current = current->next;
    }   
    printf("\n");
}

void print_store(){
    printf("-------------------\nTABELLA MAGAZZINO:\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (store[i]){
            printf("Name: %s, Quantity: %d\n", store[i]->name, store[i]->total_quantity);
            print_batch(store[i]);
        }
    }
    printf("-------------------\n");
}

void print_order(){
    struct Order* current = prepared_orders_head;
    printf("-------------------\nTABELLA ORDINI:\n");
    while(current != NULL){
        printf("Name: %s, Quantity: %d, Time: %d -> ", current->recipe, current->quantity, current->arrival_time);
        current = current->next;
    }
    current = pending_orders_head;
    printf("\n-------------------\n");
    while(current != NULL){
        printf("Name: %s, Quantity: %d, Time: %d -> ", current->recipe, current->quantity, current->arrival_time);
        current = current->next;
    }
    printf("\n-------------------\n");
}

void print_data(){
    print_recipe_book();
    print_store();
    print_order();
}

int main(){
    int t_corriere = 0;
    int q_corriere = 0;

    //* Lettura dati corriere: periodo e quantità
    if(scanf("%i", &t_corriere)<1){
        printf("Errore nella lettura di t_corriere");
        return 0;
    }
    if(scanf("%i", &q_corriere)<1){
        printf("Errore nella lettura di q_corriere");
        return 0;
    }

    //* Lettura comandi
    char comando[MAX_LEN];
    create_recipe_book(); // creo ricettario
    create_store(); // creo magazzino

    while(scanf("%s", comando) > 0){
        if(strcmp(comando, "aggiungi_ricetta") == 0){
            read_recipe(recipe_book);
            time++;
        }else{
            if(strcmp(comando, "rimuovi_ricetta") == 0){
                remove_recipe(recipe_book);
                time++;
            }else{
                if(strcmp(comando, "rifornimento") == 0){
                    rifornimento();
                    prepare_pending_order();
                    time++;
                }else{
                    if(strcmp(comando, "ordine") == 0){
                        ordine();
                        time++;
                    }
                }
            }
        }
        print_data();
        if(time % t_corriere == 0){ // verifico se devo caricare il camion con gli ordini terminati
            printf("Camioncino: ");
            load_truck(q_corriere);
        }
    }
    return 0;
}