#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 256
#define TABLE_SIZE 50000
#define PRIME 49999 // numero primo per la funzione hash 2

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

//* DEFINIZIONE FUNZIONI PER LA GESTIONE DELL'HASHING
unsigned int key_function(char *name){ // Algoritmo DJB2
    unsigned int k = 5381;
    int c;
    while ((c = *name++))
        k = ((k << 5) + k) + c;
    return k;
}
unsigned int hash_function(char *name){return key_function(name) % TABLE_SIZE;}
unsigned int hash_function2(char *name){return PRIME - (key_function(name) % PRIME);}
// Open addressing -> double hashing
unsigned int double_hashing(char* name, int i){return (hash_function(name) + i*hash_function2(name)) % TABLE_SIZE;}


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

struct Recipe** create_recipe_book(){
    struct Recipe** recipe_book = NULL;
    recipe_book = (struct Recipe**)calloc(TABLE_SIZE, sizeof(struct Recipe*)); // alloca spazio per le celle della tabella
    for (int i = 0; i < TABLE_SIZE; i++){
        recipe_book[i] = NULL;
    }
    return recipe_book;
}

void handle_collision_recipe(struct Recipe** recipe_book, struct Recipe* recipe, int index){
    int i = 1;
    while(recipe_book[index] != NULL){
        index = double_hashing(recipe->name, i);
        i++;
    }
    recipe_book[index] = recipe;
}

void insert_recipe(struct Recipe** recipe_book, char* name, struct Ingredient** tb_ingredients){
    struct Recipe* recipe = create_recipe(name, tb_ingredients);

    int index = hash_function(recipe->name);
    struct Recipe* current = recipe_book[index];

    if (current == NULL){ // Cella vuota
        recipe_book[index] = recipe;
    }else{ // Cella occupata --> collisione
        handle_collision_recipe(recipe_book, recipe, (index+1));
        return;
    }
}

int search_recipe(struct Recipe** recipe_book, char* name){
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

void free_recipe(struct Recipe* recipe){
    free_ingredients(recipe->ingredients);
    free(recipe);
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

struct Goods** create_store(){
    struct Goods** store = (struct Goods**)calloc(TABLE_SIZE, sizeof(struct Goods*));
    for (int i = 0; i < TABLE_SIZE; i++){
        store[i] = NULL;
    }

    return store;
}

int search_goods(struct Goods** store, char* name){
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

void handle_collision_goods(struct Goods** store, struct Goods* goods, int index){
    int i = 1;
    while(store[index] != NULL){
        index = double_hashing(goods->name, i);
        i++;
    }
    store[index] = goods;
}

void delete_batch_head(struct Goods** store, int goods_index){
    struct Batch* current = store[goods_index]->batches_head;
    store[goods_index]->batches_head = current->next;
    free(current);
}

void delete_expired_batch(struct Goods** store, int goods_index, int time){
    while(store[goods_index]->batches_head != NULL){
        if(store[goods_index]->batches_head->expiration <= time){
            store[goods_index]->total_quantity -= store[goods_index]->batches_head->quantity;
            delete_batch_head(store, goods_index);
        }else{
            return;
        }
    }
}

void insert_batch(struct Goods** store, char* name, int quantity, int expiration, int time){
    struct Batch* new_batch = create_batch(quantity, expiration);
    int index = search_goods(store, name);
    delete_expired_batch(store, index, time);

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

void insert_goods(struct Goods** store, char* name, int quantity, int expiration, int time){
    int i = search_goods(store, name);
    if(i == -1){ // controllo se la merce è già presente
        struct Goods* goods = create_goods(name);

        int index = hash_function(name);
        struct Goods* current_item = store[index];
        if(current_item == NULL){ // cella vuota
            store[index] = goods;
            i = index;
        }else{ // cella occupata -> collisione
            handle_collision_goods(store, goods, (index+1));
            i = search_goods(store, name);
        }
    }
    insert_batch(store, name, quantity, expiration, time);
    store[i]->total_quantity += quantity;
}

void remove_batch(struct Goods** store, int goods_index, int total_quantity_needed){
    while(total_quantity_needed > 0 && store[goods_index]->batches_head != NULL){
        if(total_quantity_needed - store[goods_index]->batches_head->quantity >= 0){ // mi servono altri batch
            total_quantity_needed -= store[goods_index]->batches_head->quantity;
            delete_batch_head(store, goods_index);
        }else{ // devo sottrare la quantità dell'ingrediente usato dalla batch in testa
            store[goods_index]->batches_head->quantity -= total_quantity_needed;
            total_quantity_needed = 0;
        }
    }
}

//* FUNZIONI DI GESTIONE DEGLI ORDINI
struct Order* create_order(int arrival_time, char* recipe, int quantity){
    struct Order* new_order = (struct Order*)malloc(sizeof(struct Order));
    new_order->arrival_time = arrival_time;
    strcpy(new_order->recipe, recipe);
    new_order->quantity = quantity;
    new_order->next = NULL;
    new_order->prev = NULL;
    
    return new_order;
}

void insert_order(struct Order** head, struct Order** tail, int arrival_time, char* recipe, int quantity){ // inserimento fatto rispetto al tempo di arrivo dell'ordine
    struct Order* new_order = create_order(arrival_time, recipe, quantity);
    if(*head == NULL){ // lista vuota
        *head = new_order;
        *tail = new_order;
    }else{ // aggiungi in ordine di arrivo
        struct Order* current = *head;
        while(current != NULL && current->arrival_time < arrival_time){
            current = current->next;
        }
        
        if(current == NULL){
            (*tail)->next = new_order;
            new_order->prev = *tail;
            *tail = new_order;
        }else{
            if(current == *head){
                new_order->next = *head;
                (*head)->prev = new_order;
                *head = new_order;
            }else{
                new_order->next = current;
                new_order->prev = current->prev;
                current->prev->next = new_order;
                current->prev = new_order;
            }
        }
    }
}

void delete_order(struct Order** head, struct Order* order_to_delete){
    if(*head == NULL || order_to_delete == NULL){ // se la testa o l'ordine da eliminare sono NULL
        return;
    }

    if(*head == order_to_delete){ // se devo eliminare la testa
        *head = order_to_delete->next;
    }

    if(order_to_delete->prev != NULL){ // se il nodo da eliminare ha un predecessore
        order_to_delete->prev->next = order_to_delete->next;
    }

    if(order_to_delete->next != NULL){  // se il nodo da eliminare ha un successore
        order_to_delete->next->prev = order_to_delete->prev;
    }

    free(order_to_delete);
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

int check_ingredients_availability(struct Recipe** recipe_book, struct Goods** store, char* recipe_name, int quantity, int time){
    struct Recipe* recipe = recipe_book[search_recipe(recipe_book, recipe_name)];
    int goods_index;
    int total_quantity_needed;
    for (int i = 0; i < TABLE_SIZE; i++){
        if(recipe->ingredients[i] != NULL){
            goods_index = search_goods(store, recipe->ingredients[i]->name);
            if(goods_index == -1){
                return 0; // ingrediente non trovato nel magazzino
            }
            delete_expired_batch(store, goods_index, time); // elimino i lotti scaduti
            total_quantity_needed = recipe->ingredients[i]->quantity * quantity;
            if(store[goods_index]->total_quantity < total_quantity_needed){
                return 0; // scorte insufficienti
            }
        }
    }
    return 1;
}

void order_preparation(struct Recipe** recipe_book, struct Goods** store, struct Order** prepared_orders_head, struct Order** prepared_orders_tail, struct Order** pending_orders_head, struct Order** pending_orders_tail, char* name, int quantity, int time, int arrival_time){
    if(check_ingredients_availability(recipe_book, store, name, quantity, time)){ // ci sono ingredienti a sufficienza per preparare l'ordine
        insert_order(prepared_orders_head, prepared_orders_tail, arrival_time, name, quantity);
        // eliminare ingredienti usati
        int recipe_index = search_recipe(recipe_book, name);
        int goods_index = 0;
        int total_quantity_needed = 0;
        for(int i = 0; i < TABLE_SIZE; i++){ // scorro gli ingredienti della ricetta
            if(recipe_book[recipe_index]->ingredients[i] != NULL){
                goods_index = search_goods(store, recipe_book[recipe_index]->ingredients[i]->name); // indice ingrediente nel magazzino
                total_quantity_needed = recipe_book[recipe_index]->ingredients[i]->quantity*quantity;
                remove_batch(store, goods_index, total_quantity_needed);
                store[goods_index]->total_quantity -= total_quantity_needed; // aggiorno indice total_quantity
            }
        }
    }else{
        insert_order(pending_orders_head, pending_orders_tail, arrival_time, name, quantity);
    }
}

void prepare_pending_order(struct Recipe** recipe_book, struct Goods** store, struct Order** prepared_orders_head, struct Order** prepared_orders_tail, struct Order** pending_orders_head, struct Order** pending_orders_tail, int time){
    struct Order* current = *pending_orders_head;
    while(current != NULL){ // scorro la lista dalla testa per trovare il più vecchio ordine che posso evadere
        if(check_ingredients_availability(recipe_book, store, current->recipe, current->quantity, time)){
            order_preparation(recipe_book, store, prepared_orders_head, prepared_orders_tail, pending_orders_head, pending_orders_tail, current->recipe, current->quantity, time, current->arrival_time);
            // cancellare l'ordine che è stato evaso dalla lista degli ordini in attesa
            delete_order(pending_orders_head, current);
        }
        current = current->next;
    }
}

int calculate_total_quantity_order(struct Recipe** recipe_book, char* recipe_name, int order_quantity){
    int total_quantity_order = 0;
    int recipe_index = search_recipe(recipe_book, recipe_name);
    struct Ingredient** tb_ingredients = recipe_book[recipe_index]->ingredients;
    for(int i = 0; i < TABLE_SIZE; i++){
        if(tb_ingredients[i] != NULL){
            total_quantity_order += order_quantity * tb_ingredients[i]->quantity;
        }
    }
    return total_quantity_order;
}

void load_order_truck(struct Order** head, struct Order** tail, int arrival_time, char* recipe, int quantity, int total_quantity_order, struct Recipe** recipe_book){ // inserimento fatto rispetto al tempo di arrivo dell'ordine
    struct Order* new_order = create_order(arrival_time, recipe, quantity);
    
    if(*head == NULL){ // lista vuota
        *head = new_order;
        *tail = new_order;
    }else{ // aggiungi in ordine di arrivo
        struct Order* current = *head;
        while(current != NULL && total_quantity_order <= calculate_total_quantity_order(recipe_book, current->recipe, current->quantity)){
            current = current->next;
        }
        
        if(current == NULL){
            (*tail)->next = new_order;
            new_order->prev = *tail;
            *tail = new_order;
        }else{
            if(current == *head){
                new_order->next = *head;
                (*head)->prev = new_order;
                *head = new_order;
            }else{
                new_order->next = current;
                new_order->prev = current->prev;
                current->prev->next = new_order;
                current->prev = new_order;
            }
        }
    }
}


//* FUNZIONI STRUTTURALI
void read_recipe(struct Recipe** recipe_book){
    struct Ingredient** tb_ingredients = create_ingredients_table();
    char recipe_name[MAX_LEN];
    char name[MAX_LEN];
    int quantity = 0;
    if(scanf("%s", recipe_name) > 0){ // nome ricetta
        if(search_recipe(recipe_book, name) == -1){
            while(1){ // creo la lista degli ingredienti
                if (scanf("%s", name) > 0) {
                    if (scanf("%d", &quantity) > 0) {
                        insert_ingredient(tb_ingredients, name, quantity);
                    }
                } 
                // Controlla se il prossimo carattere è un newline o EOF
                int next_char = getchar();
                if (next_char == '\n' || next_char == EOF) {
                    break;
                }
            }
            insert_recipe(recipe_book, recipe_name, tb_ingredients);
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

void remove_recipe(struct Recipe** recipe_book, struct Order* prepared_orders_head, struct Order* pending_orders_head){
    char name[MAX_LEN];
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta
        if(search_recipe(recipe_book, name) == -1){ // la ricetta non è presente nel ricettario
            printf("non presente\n");
        }else{ // la ricetta è presente
            if(search_recipe_orders(prepared_orders_head, name) || search_recipe_orders(pending_orders_head, name)){  // è presente un ordine con la ricetta in questione
                printf("ordini in sospeso\n");
                return;
            }else{
                int index = search_recipe(recipe_book, name);
                free_recipe(recipe_book[index]);
                recipe_book[index] = NULL;
                printf("rimossa\n");
            }
        }
    }
}

void rifornimento(struct Goods** store, int time){
    char name[MAX_LEN];
    int quantity = 0;
    int expiration = 0;
    while(1){
        if(scanf("%s", name) > 0){ // nome goods
            if (scanf("%d", &quantity) > 0) { // quantità goods
                if (scanf("%d", &expiration) > 0) { // data di scadenza goods
                    insert_goods(store, name, quantity, expiration, time);
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

void ordine(struct Recipe** recipe_book, struct Goods** store, struct Order** prepared_orders_head, struct Order** prepared_orders_tail, struct Order** pending_orders_head, struct Order** pending_orders_tail, int time){ // legge nome della ricetta e la quantità da preparare
    char name[MAX_LEN];
    int quantity = 0;
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta dell'ordine
        if(search_recipe(recipe_book, name) == -1){ // la ricetta non è presente
            printf("%d, rifiutato\n", search_recipe(recipe_book, name));
            return;
        }else{ // la ricetta è presente
            if(scanf("%d", &quantity) > 0){
                order_preparation(recipe_book, store, prepared_orders_head, prepared_orders_tail, pending_orders_head, pending_orders_tail, name, quantity, time, time);
                printf("accettato\n");
            }
        }
    }
}

void load_truck(struct Recipe** recipe_book, struct Order** prepared_orders_head, struct Order** prepared_orders_tail, int capacity){
    struct Order* truck_head = NULL;
    struct Order* truck_tail = NULL;
    int total_quantity_order;

    while(*prepared_orders_head != NULL && capacity > 0){
        total_quantity_order = calculate_total_quantity_order(recipe_book, (*prepared_orders_head)->recipe, (*prepared_orders_head)->quantity);
        if(total_quantity_order <= capacity){
            capacity -= total_quantity_order;
            load_order_truck(&truck_head, &truck_tail, (*prepared_orders_head)->arrival_time, (*prepared_orders_head)->recipe, (*prepared_orders_head)->quantity, total_quantity_order, recipe_book);
            delete_order(prepared_orders_head, *prepared_orders_head);
        }else{
            break;
        }
    }
    if(truck_head == NULL){
        printf("camioncino vuoto\n");
    }else{
        while(truck_head != NULL){
            printf("%d %s %d\n", truck_head->arrival_time, truck_head->recipe, truck_head->quantity);
            delete_order(&truck_head, truck_head);
        }
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

void print_recipe_book(struct Recipe** recipe_book){
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

void print_store(struct Goods** store){
    printf("-------------------\nTABELLA MAGAZZINO:\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (store[i]){
            printf("Name: %s, Quantity: %d\n", store[i]->name, store[i]->total_quantity);
            print_batch(store[i]);
        }
    }
    printf("-------------------\n");
}

void print_order(struct Order* prepared_orders_head, struct Order* pending_orders_head){
    struct Order* current = prepared_orders_head;
    printf("-------------------\nTABELLA ORDINI:\nOrdini pronti: ");
    while(current != NULL){
        printf("Name: %s, Quantity: %d, Time: %d -> ", current->recipe, current->quantity, current->arrival_time);
        current = current->next;
    }
    current = pending_orders_head;
    printf("\nOrdini in attesa: ");
    while(current != NULL){
        printf("Name: %s, Quantity: %d, Time: %d -> ", current->recipe, current->quantity, current->arrival_time);
        current = current->next;
    }
    printf("\n-------------------\n");
}

void print_data(struct Recipe** recipe_book, struct Goods** store, struct Order* prepared_orders_head, struct Order* pending_orders_head){
    print_recipe_book(recipe_book);
    print_store(store);
    print_order(prepared_orders_head, pending_orders_head);
}

int main(){
    //* Dichiarazione strutture dati:
    struct Recipe** recipe_book = NULL; // Tabella hash per la raccolta delle ricette
    struct Goods** store = NULL; // Tabella hash che rappresenta il magazzino: array di puntatori che punta alla merce (ingredienti) presente in magazzino
    struct Order* prepared_orders_head = NULL; // Ordini pronti ad essere spediti
    struct Order* prepared_orders_tail = NULL;
    struct Order* pending_orders_head = NULL; // Ordini in attesa di ricevere gli ingredienti
    struct Order* pending_orders_tail = NULL;

    //* Dichiarazione veriabili:
    int time = 0;
    int courier_period = 0; // Periodicità corriere
    int courier_capacity = 0; // Capienza corriere
    char comando[MAX_LEN];


    //* Lettura dati corriere: periodo e capienza
    if(scanf("%i", &courier_period) < 1){
        printf("Errore nella lettura di courier_period");
        return 0;
    }
    if(scanf("%i", &courier_capacity) < 1){
        printf("Errore nella lettura di courier_capacity");
        return 0;
    }
    
    //* Creazione strutture dati:
    recipe_book = create_recipe_book();
    store = create_store();

    
    //* Lettura comandi
    while(scanf("%s", comando) > 0){
        if(strcmp(comando, "aggiungi_ricetta") == 0){
            read_recipe(recipe_book);
            time++;
        }else{
            if(strcmp(comando, "rimuovi_ricetta") == 0){
                remove_recipe(recipe_book, prepared_orders_head, pending_orders_head);
                time++;
            }else{
                if(strcmp(comando, "rifornimento") == 0){
                    rifornimento(store, time);
                    prepare_pending_order(recipe_book, store, &prepared_orders_head, &prepared_orders_tail, &pending_orders_head, &pending_orders_tail, time); // dopo ogni rifornimento devo verificare se gli ordini in attesa possono essere evasi
                    time++;
                }else{
                    if(strcmp(comando, "ordine") == 0){
                        ordine(recipe_book, store, &prepared_orders_head, &prepared_orders_tail, &pending_orders_head, &pending_orders_tail, time);
                        time++;
                    }
                }
            }
        }

        // Stampa per debugging
        print_data(recipe_book, store, prepared_orders_head, pending_orders_head);

        // Spedizione ordini
        if(time % courier_period == 0){ // verifico se devo caricare il camion con gli ordini terminati
            load_truck(recipe_book, &prepared_orders_head, &prepared_orders_tail, courier_capacity);
        }
    }
    return 0;
}