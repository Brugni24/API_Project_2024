#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 256
#define TABLE_SIZE 10000
#define PRIME 9973 // numero primo per la funzione hash 2

int recipe_counter = 0;

//* DEFINIZIONE STRUTTURE DATI
struct Ingredient{
    char name[MAX_LEN];
    int quantity;
    struct Ingredient* next;
};

struct Recipe{
    int key;
    char name[MAX_LEN];
    struct Ingredient* ingredients_head;
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
    
    // controllo se l'allocazione di memoria è andata a buon fine
    if (ingredient == NULL) {
        // Gestione dell'errore di allocazione della memoria
        printf("Errore: allocazione della memoria fallita\n");
        exit(EXIT_FAILURE);
    }

    strcpy(ingredient->name, name);
    ingredient->quantity = quantity;
    ingredient->next = NULL;
    
    return ingredient;
}

struct Ingredient* search_ingredient(struct Ingredient* ingredients_head, char* name){
    struct Ingredient* current = ingredients_head;

    while(current != NULL){
        if(strcmp(current->name, name) == 0){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void insert_ingredient(struct Ingredient** ingredients_head, char* name, int quantity){
    // cerco se l'ingrediente è già presente
    struct Ingredient* temp = search_ingredient(*ingredients_head, name);

    if(temp == NULL){ // ingrediente non presente
        // creo il nodo
        struct Ingredient* new_ingredient = create_ingredient(name, quantity);
        
        if(*ingredients_head == NULL){ // lista vuota
            *ingredients_head = new_ingredient;
        }else{ // aggiungo ingrediente in testa
            new_ingredient->next = *ingredients_head;
            *ingredients_head = new_ingredient;
        }
    }else{ // ingrediente già presente -> aumento la quantità senza aggiungere un nuovo nodo
        temp->quantity += quantity;
    }
}

void free_ingredients(struct Ingredient* ingredients_head){
    struct Ingredient* temp = NULL;

    while(ingredients_head != NULL){
        temp = ingredients_head;
        ingredients_head = ingredients_head->next;
        free(temp);
    }
}


//* FUNZIONI DI GESTIONE DELLE RICETTE
struct Recipe* create_recipe(char* name, struct Ingredient* ingredients_head){
    struct Recipe* recipe = (struct Recipe*)malloc(sizeof(struct Recipe));
    
    recipe->key = key_function(name);
    strcpy(recipe->name, name);
    recipe->ingredients_head = ingredients_head;

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

void insert_recipe(struct Recipe** recipe_book, char* name, struct Ingredient* ingredients_head){
    struct Recipe* recipe = create_recipe(name, ingredients_head);

    int index = hash_function(recipe->name);
    struct Recipe* current = recipe_book[index];

    recipe_counter++;

    if (current == NULL){ // Cella vuota
        recipe_book[index] = recipe;
    }else{ // Cella occupata --> collisione
        handle_collision_recipe(recipe_book, recipe, (index+1));
        return;
    }
}

int search_recipe(struct Recipe** recipe_book, char* name){
    int index = hash_function(name); // calcolo l'indice della ricetta
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
    free_ingredients(recipe->ingredients_head);
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
        int i = 0;

        while(current != NULL && current->arrival_time < arrival_time){
            current = current->next;
            i++;
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

void delete_order(struct Order** head, struct Order** tail, struct Order* order_to_delete){
    if(*head == NULL || order_to_delete == NULL || *tail == NULL){ // se la testa o l'ordine da eliminare sono NULL
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

    if(*tail == order_to_delete){
        *tail = order_to_delete->prev;
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
    int goods_index;
    int total_quantity_needed;
    struct Ingredient* current_ingredient = recipe_book[search_recipe(recipe_book, recipe_name)]->ingredients_head; // testa della lista di ingredienti della ricetta in oggetto
    
    while(current_ingredient != NULL){
        goods_index = search_goods(store, current_ingredient->name);
        if(goods_index == -1){ // ingrediente non trovato in magazzino -> non posso procedere con l'ordine e deve essere messo in attesa
            return 0;
        }

        // elimino i lotti scaduti
        delete_expired_batch(store, goods_index, time);

        // calcolo la quantità totale necessaria
        total_quantity_needed = current_ingredient->quantity * quantity;

        if(store[goods_index]->total_quantity < total_quantity_needed){
            return 0; // scorte insufficienti -> non posso procedere con l'ordine e deve essere messo in attesa
        }
        current_ingredient = current_ingredient->next;
    }

    return 1; // scorte sufficienti -> ordine viene mandato in preparazione
}

void order_preparation(struct Recipe** recipe_book, struct Goods** store, struct Order** prepared_orders_head, struct Order** prepared_orders_tail, struct Order** pending_orders_head, struct Order** pending_orders_tail, char* name, int quantity, int time, int arrival_time){
    if(check_ingredients_availability(recipe_book, store, name, quantity, time)){ // ci sono ingredienti a sufficienza per preparare l'ordine
        insert_order(prepared_orders_head, prepared_orders_tail, arrival_time, name, quantity);

        // eliminare ingredienti usati
        int recipe_index = search_recipe(recipe_book, name);
        int goods_index = 0;
        int total_quantity_needed = 0;
        struct Ingredient* current_ingredient = recipe_book[recipe_index]->ingredients_head;

        while(current_ingredient != NULL){
            goods_index = search_goods(store, current_ingredient->name);
            total_quantity_needed = current_ingredient->quantity * quantity;
            remove_batch(store, goods_index, total_quantity_needed);
            store[goods_index]->total_quantity -= total_quantity_needed; // aggiorno indice total_quantity
            current_ingredient = current_ingredient->next;
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
            delete_order(pending_orders_head, pending_orders_tail, current);
        }
        current = current->next;
    }
}

int calculate_total_quantity_order(struct Recipe** recipe_book, char* recipe_name, int order_quantity){
    int total_quantity_order = 0;
    int recipe_index = search_recipe(recipe_book, recipe_name);
    struct Ingredient* current_ingredient = recipe_book[recipe_index]->ingredients_head;

    while(current_ingredient != NULL){
        total_quantity_order += order_quantity * current_ingredient->quantity;
        current_ingredient = current_ingredient->next;
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
    struct Ingredient* ingredients_head = NULL;
    char recipe_name[MAX_LEN] = "";
    char name[MAX_LEN] = "";
    int quantity = 0;
    if(scanf("%s", recipe_name) > 0){ // nome ricetta
        if(search_recipe(recipe_book, recipe_name) == -1){
            while(1){ // creo la lista degli ingredienti
                if (scanf("%s", name) > 0) {
                    if (scanf("%d", &quantity) > 0) {
                        insert_ingredient(&ingredients_head, name, quantity);
                    }
                } 
                // Controlla se il prossimo carattere è un newline o EOF
                int next_char = getchar();
                if (next_char == '\n' || next_char == EOF) {
                    break;
                }
            }
            insert_recipe(recipe_book, recipe_name, ingredients_head);
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
    char name[MAX_LEN] = "";
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
    char name [MAX_LEN] = "";
    int quantity = 0;
    int expiration = 0;
    while(1){
        if(scanf("%s", name) > 0){ // nome goods
            if (scanf("%d", &quantity) > 0) { // quantità goods
                if (scanf("%d", &expiration) > 0 && expiration > time) { // data di scadenza goods
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
    char name[MAX_LEN] = "";
    int quantity = 0;
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta dell'ordine
        if(scanf("%d", &quantity) > 0){
            if(search_recipe(recipe_book, name) == -1){ // la ricetta non è presente
                printf("rifiutato\n");
            }else{ // la ricetta è presente
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
            delete_order(prepared_orders_head, prepared_orders_tail, *prepared_orders_head);
        }else{
            break;
        }
    }
    if(truck_head == NULL){
        printf("camioncino vuoto\n");
    }else{
        // struct Order* current = truck_head;
        // printf("LISTA TRUCK:");
        // while(current != NULL){
        //     printf("%d %s %d -> ", current->arrival_time, current->recipe, current->quantity);
        //     current = current->next;
        // }
        // printf("\n");

        while(truck_head != NULL){
            printf("%d %s %d\n", truck_head->arrival_time, truck_head->recipe, truck_head->quantity);
            delete_order(&truck_head, &truck_tail, truck_head);
        }
    }
}

//* FUNZIONI PER DEBUGGING
void print_ingredients_list(struct Ingredient* head){
    struct Ingredient* current = head;
    printf("Ingredienti: ");
    while(current != NULL){
        printf("%s, %d  -  ", current->name, current->quantity);
        current = current->next;
    }
    printf("\n");
}

void print_recipe_book(struct Recipe** recipe_book){
    printf("-------------------\nTABELLA RICETTE:\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (recipe_book[i]){
            printf("Index: %d, Key: %d, Value: %s\n", i, recipe_book[i]->key, recipe_book[i]->name);
            print_ingredients_list(recipe_book[i]->ingredients_head);
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
            printf("Index: %d, Key: %d, Name: %s, Quantity:%d\n", i, store[i]->key, store[i]->name, store[i]->total_quantity);
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
    char comando[MAX_LEN] = "";


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
        if(recipe_counter >= TABLE_SIZE*0.75){
            printf("Errore: tabella hash sovraccaricata\n");
            exit(EXIT_FAILURE);
        }
        // printf("%d) Comando: %s  --> ", time, comando);
        if(strcmp(comando, "aggiungi_ricetta") == 0){
            read_recipe(recipe_book);
        }else{
            if(strcmp(comando, "rimuovi_ricetta") == 0){
                remove_recipe(recipe_book, prepared_orders_head, pending_orders_head);
            }else{
                if(strcmp(comando, "rifornimento") == 0){
                    rifornimento(store, time);
                    prepare_pending_order(recipe_book, store, &prepared_orders_head, &prepared_orders_tail, &pending_orders_head, &pending_orders_tail, time); // dopo ogni rifornimento devo verificare se gli ordini in attesa possono essere evasi
                }else{
                    if(strcmp(comando, "ordine") == 0){
                        ordine(recipe_book, store, &prepared_orders_head, &prepared_orders_tail, &pending_orders_head, &pending_orders_tail, time);
                    }
                }
            }
        }

        // Stampa per debugging
        // print_data(recipe_book, store, prepared_orders_head, pending_orders_head);
        // print_order(prepared_orders_head, pending_orders_head);
        // Spedizione ordini
        time++;
        if(time % courier_period == 0){ // verifico se devo caricare il camion con gli ordini terminati
            // printf("SPEDIZIONE ALL'ISTANTE DI TEMPO: %d\n", time);
            load_truck(recipe_book, &prepared_orders_head, &prepared_orders_tail, courier_capacity);
        }
    }
    return 0;
}