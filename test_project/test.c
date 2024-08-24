#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 50
#define RECIPE_BOOK_SIZE 499979
#define STORE_SIZE 503
#define STORE_PRIME 499 // numero primo per la funzione hash 2

//! DEFINIZIONE STRUTTURE DATI
struct Ingredient{
    struct Goods* goods_ptr;
    int quantity;
    struct Ingredient* next;
};

struct Recipe{
    char name[MAX_LEN];
    struct Ingredient* ingredients_head;
    struct Recipe* next_recipe;
};

struct Batch{ // lotti
    int quantity;
    int expiration;
    struct Batch* next;
};

struct Goods{ // merce presente nel magazzino -> ingredienti
    char name[MAX_LEN];
    int total_quantity;
    struct Batch* batches_head;
};

struct Order{
    struct Recipe* recipe_ptr;
    int arrival_time;
    int quantity;
    struct Order* next;
    struct Order* prev;
};


//! DEFINIZIONE FUNZIONI PER LA GESTIONE DELL'HASHING
unsigned int key_function(char *name){ // Algoritmo DJB2
    unsigned int k = 5381;
    int c;
    while ((c = *name++))
        k = ((k << 5) + k) + c;
    return k;
}

unsigned int hash_function_RB(char *name){return key_function(name) % RECIPE_BOOK_SIZE;}

unsigned int hash_function_S(char *name){return key_function(name) % STORE_SIZE;}
unsigned int hash_function2_S(char *name){return STORE_PRIME - (key_function(name) % STORE_PRIME);}
// Open addressing -> double hashing
unsigned int double_hashing_S(char* name, int i){return (hash_function_S(name) + i*hash_function2_S(name)) % STORE_SIZE;}



//! FUNZIONI DI GESTIONE DEI LOTTI
struct Batch* create_batch(int quantity, int expiration){
    struct Batch* batch = (struct Batch*)malloc(sizeof(struct Batch));

    batch->quantity = quantity;
    batch->expiration = expiration;
    batch->next = NULL;

    return batch;
}

void free_batch_head(struct Goods* goods){
    if(goods == NULL){
        return;
    }

    struct Batch* temp = goods->batches_head;
    goods->batches_head = goods->batches_head->next;

    free(temp);
}

void delete_expired_batch(struct Goods* goods, int current_time){
    while(goods->batches_head != NULL){
        if(goods->batches_head->expiration <= current_time){ // un lotto scade a partire dalla data di scadenza
            // tolgo l'ingrediente dalla quantità totale
            goods->total_quantity -= goods->batches_head->quantity;

            // elimino il batch in testa
            free_batch_head(goods);
        }else{
            return;
        }
    }
}

//* Inserisce i lotti in lista a partire dalla testa, ma in modo che la lista risulti sempre in ordine crescente di expiration
void insert_batch(struct Goods* goods, char* name, int quantity, int expiration, int current_time){
    struct Batch* new_batch = create_batch(quantity, expiration);

    delete_expired_batch(goods, current_time);

    if(goods->batches_head == NULL){ // lista vuota
        goods->batches_head = new_batch;
    }else{
        if(new_batch->expiration < goods->batches_head->expiration){ // new_batch va aggiunto in testa
            new_batch->next = goods->batches_head;
            goods->batches_head = new_batch;
        }else{
            struct Batch* current = goods->batches_head;

            while(current->next != NULL && new_batch->expiration > current->next->expiration){
                current = current->next;
            }
            new_batch->next = current->next;
            current->next = new_batch;
        }
    }
}

void free_batches(struct Batch* batches_head){
    if(batches_head == NULL){
        return;
    }

    struct Batch* temp = NULL;

    while(batches_head != NULL){
        temp = batches_head;
        batches_head = batches_head->next;
        free(temp);
    }

    batches_head = NULL;
}


//! FUNZIONI DI GESTIONE DEL MAGAZZINO
struct Goods* create_goods(char* name){
    struct Goods* goods = (struct Goods*)malloc(sizeof(struct Goods));

    strcpy(goods->name, name);
    goods->total_quantity = 0;
    goods->batches_head = NULL;

    return goods;
}

struct Goods** create_store(){
    struct Goods** store = (struct Goods**)calloc(STORE_SIZE, sizeof(struct Goods*));

    for (int i = 0; i < STORE_SIZE; i++){
        store[i] = NULL;
    }

    return store;
}

void handle_collision_goods(struct Goods** store, struct Goods* goods, int index){
    int i = 1;
    while(store[index] != NULL){
        index = double_hashing_S(goods->name, i);
        i++;
    }
    store[index] = goods;
}

int search_goods(struct Goods** store, char* name){
    int index = hash_function_S(name);
    int i = 0;

    while(store[index] != NULL){
        if(strcmp(store[index]->name, name) == 0){
            return index;
        }
        i++;
        index = double_hashing_S(name, i);
    }

    return -1;
}

void insert_goods(struct Goods** store, char* name, int quantity, int expiration, int time){
    int goods_index = search_goods(store, name); // indice merce

    // Controllo se la merce è già presente
    if(goods_index == -1){ // merce non presente -> creo un nuovo elemento
        struct Goods* goods = create_goods(name);

        int i = hash_function_S(name);

        struct Goods* current_item = store[i];

        if(current_item == NULL){ // cella vuota
            store[i] = goods;
            goods_index = i; // aggiorno l'indice di goods
        }else{ // cella occupata -> collisione
            handle_collision_goods(store, goods, i);
            goods_index = search_goods(store, name); // aggiorno l'indice di goods
        }
    }

    insert_batch(store[goods_index], name, quantity, expiration, time);
    store[goods_index]->total_quantity += quantity;
}

void free_goods(struct Goods* goods){
    if(goods == NULL){
        return;
    }

    free_batches(goods->batches_head);

    free(goods);

    goods = NULL;
}

void free_store(struct Goods** store){
    if(store == NULL){
        return;
    }

    for(int i = 0; i < STORE_SIZE; i++){
        if(store[i] != NULL){
            free_goods(store[i]);
            store[i] = NULL;
        }
    }

    free(store);
    store = NULL;
}




//! FUNZIONI DI GESTIONE DEGLI INGREDIENTI
struct Ingredient* create_ingredient(struct Goods* goods_ptr, int quantity){
    struct Ingredient* ingredient = (struct Ingredient*)malloc(sizeof(struct Ingredient)); // Alloca la memoria per il puntatore ad un nuovo ingrediente

    ingredient->goods_ptr = goods_ptr;
    ingredient->quantity = quantity;
    ingredient->next = NULL;
    
    return ingredient;
}

void insert_ingredient(struct Goods** store, struct Ingredient** ingredients_head, char* name, int quantity){
    // Cerco l'ingrediente nel magazzino
    int goods_index = search_goods(store, name);

    if(goods_index == -1){
        // Ingrediente non presente nel magazzino -> devo aggiungerlo
       struct Goods* goods = create_goods(name);

        int i = hash_function_S(name);

        struct Goods* current_item = store[i];

        if(current_item == NULL){ // cella vuota
            store[i] = goods;
            goods_index = i; // aggiorno l'indice di goods
        }else{ // cella occupata -> collisione
            handle_collision_goods(store, goods, i);
            goods_index = search_goods(store, name); // aggiorno l'indice di goods
        }
    }

    struct Ingredient* new_ingredient = create_ingredient(store[goods_index], quantity);
        
    if(*ingredients_head == NULL){ // lista vuota
        *ingredients_head = new_ingredient;
    }else{ // aggiungo ingrediente in testa
        new_ingredient->next = *ingredients_head;
        *ingredients_head = new_ingredient;
    }
}

void free_ingredients(struct Ingredient* ingredients_head){
    if(ingredients_head == NULL){
        return;
    }

    struct Ingredient* temp = NULL;

    while(ingredients_head != NULL){
        temp = ingredients_head;
        ingredients_head = ingredients_head->next;
        free(temp);
    }

    ingredients_head = NULL;
}



//! FUNZIONI DI GESTIONE DELLE RICETTE
struct Recipe* create_recipe(char* name, struct Ingredient* ingredients_head){
    struct Recipe* recipe = (struct Recipe*)malloc(sizeof(struct Recipe));

    strcpy(recipe->name, name);
    recipe->ingredients_head = ingredients_head;
    recipe->next_recipe = NULL;

    return recipe;
}

struct Recipe** create_recipe_book(){
    struct Recipe** recipe_book = NULL;
    recipe_book = (struct Recipe**)calloc(RECIPE_BOOK_SIZE, sizeof(struct Recipe*)); // alloca spazio per le celle della tabella

    for (int i = 0; i < RECIPE_BOOK_SIZE; i++){
        recipe_book[i] = NULL;
    }

    return recipe_book;
}

void insert_recipe(struct Recipe** recipe_book, char* name, struct Ingredient* ingredients_head){
    struct Recipe* new_recipe = create_recipe(name, ingredients_head);
    
    // Calcolo l'indice della ricetta
    int index = hash_function_RB(name);

    if(recipe_book[index] == NULL){ // Cella vuota
        recipe_book[index] = new_recipe;
    }else{ // Cella occupata
        new_recipe->next_recipe = recipe_book[index];
        recipe_book[index] = new_recipe;
    }
}

struct Recipe* search_recipe(struct Recipe** recipe_book, char* name){
    // Calcolo l'indice della ricetta
    int index = hash_function_RB(name);

    if(recipe_book[index] != NULL){
        struct Recipe* current = recipe_book[index];

        while(current != NULL){
            if(strcmp(current->name, name) == 0){
                return current;
            }else{
                current = current->next_recipe;
            }
        }
    }

    return NULL;
}

void free_recipe(struct Recipe** recipe_book, char* name){
    int index = hash_function_RB(name);
    struct Recipe* current = recipe_book[index];
    struct Recipe* prev = NULL;

    while(current != NULL){
        if(strcmp(current->name, name) == 0){
            if(prev == NULL){
                recipe_book[index] = current->next_recipe;
            }else{
                prev->next_recipe = current->next_recipe;
            }
            free_ingredients(current->ingredients_head);
            free(current);
            return;
        }
        prev = current;
        current = current->next_recipe;
    }
}

void free_recipe_list(struct Recipe* recipe_head){
    if(recipe_head == NULL){
        return;
    }

    struct Recipe* temp = NULL;

    while(recipe_head != NULL){
        temp = recipe_head;
        recipe_head = recipe_head->next_recipe;
        free_ingredients(temp->ingredients_head);
        free(temp);
    }

    recipe_head = NULL;
}

void free_recipe_book(struct Recipe** recipe_book){
    if(recipe_book == NULL){
        return;
    }

    for(int i = 0; i < RECIPE_BOOK_SIZE; i++){
        if(recipe_book[i] != NULL){
            free_recipe_list(recipe_book[i]);
            recipe_book[i] = NULL;
        }
    }

    free(recipe_book);
    
    recipe_book = NULL;
}



//! FUNZIONI DI GESTIONE DEGLI ORDINI
struct Order* create_order(struct Recipe* recipe_ptr, int arrival_time, int quantity){
    struct Order* new_order = (struct Order*)malloc(sizeof(struct Order));

    new_order->recipe_ptr = recipe_ptr;
    new_order->arrival_time = arrival_time;
    new_order->quantity = quantity;
    new_order->next = NULL;
    new_order->prev = NULL;
    
    return new_order;
}

//* Inserimento fatto a partire dalla testa, ma seguendo un ordine descrescente di arrival_time
void insert_order(struct Order** head, struct Order** tail, struct Order* new_order){

    if(*head == NULL || *tail == NULL){ // lista vuota
        *head = new_order;
        *tail = new_order;
    }else{ // aggiungi in ordine di arrivo
        struct Order* current = *head;

        while(current != NULL && current->arrival_time >= new_order->arrival_time){
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

int search_recipe_orders(struct Order* head, struct Recipe* recipe){
    struct Order* current = head;

    while(current != NULL){
        if(current->recipe_ptr == recipe){
            return 1;
        }
        current = current->next;
    }

    return 0;
}

struct Order* remove_order(struct Order** head, struct Order** tail, struct Order* order_to_remove){

    if(*head == order_to_remove){ // se devo eliminare la testa
        *head = order_to_remove->next;
    }

    if(order_to_remove->prev != NULL){ // se il nodo da eliminare ha un predecessore
        order_to_remove->prev->next = order_to_remove->next;
    }

    if(order_to_remove->next != NULL){  // se il nodo da eliminare ha un successore
        order_to_remove->next->prev = order_to_remove->prev;
    }

    if(*tail == order_to_remove){
        *tail = order_to_remove->prev;
    }

    order_to_remove->next = NULL;
    order_to_remove->prev = NULL;

    return order_to_remove;
}

void delete_order(struct Order** head, struct Order** tail, struct Order* order_to_delete){
    order_to_delete = remove_order(head, tail, order_to_delete);

    free(order_to_delete);
}

void free_orders(struct Order** head, struct Order** tail){
    if(*head == NULL || *tail == NULL){
        return;
    }

    struct Order* current = *head;
    struct Order* next_node;
    
    while(current != NULL){
        next_node = current->next;
        free(current);
        current = next_node;
    }

    *head = NULL;
    *tail = NULL;
}

int check_ingredients_availability(struct Recipe* recipe, int quantity, int current_time){
    int total_quantity_needed = 0;
    struct Ingredient* current_ingredient = recipe->ingredients_head; // testa della lista di ingredienti della ricetta in oggetto
    
    while(current_ingredient != NULL){
        // elimino i lotti scaduti
        delete_expired_batch(current_ingredient->goods_ptr, current_time);

        // calcolo la quantità totale necessaria
        total_quantity_needed = current_ingredient->quantity * quantity;

        if(current_ingredient->goods_ptr->total_quantity < total_quantity_needed){
            return 0; // scorte insufficienti -> non posso procedere con l'ordine e deve essere messo in attesa
        }
        current_ingredient = current_ingredient->next;
    }

    return 1; // scorte sufficienti -> ordine viene mandato in preparazione
}

void remove_consumed_batch(struct Recipe* recipe, int order_quantity){
    int total_quantity_needed = 0; // quantità totale dell'ingrediente richiesto dalla ricetta dell'ordine in processo
    struct Ingredient* current_ingredient = recipe->ingredients_head;

    while(current_ingredient != NULL){
        // calcolo la quantità totale di ingrediente che mi serve
        total_quantity_needed = current_ingredient->quantity * order_quantity;

        while(total_quantity_needed > 0 && current_ingredient->goods_ptr->batches_head != NULL){
            // verifico se mi basta un lotto o me ne servono di più
            if(total_quantity_needed - current_ingredient->goods_ptr->batches_head->quantity >= 0){
                // mi servono più lotti
                total_quantity_needed -= current_ingredient->goods_ptr->batches_head->quantity;
                free_batch_head(current_ingredient->goods_ptr);
            }else{ 
                // devo sottrare la quantità dell'ingrediente usato dalla batch in testa
                current_ingredient->goods_ptr->batches_head->quantity -= total_quantity_needed;
                total_quantity_needed = 0;
            }
        }

        current_ingredient->goods_ptr->total_quantity -= current_ingredient->quantity * order_quantity; // aggiorno indice total_quantity

        current_ingredient = current_ingredient->next;
    }
}

//* Arrivano gli ordini nuovi e vengono smistati a seconda che si possano preparare oppure no
void order_preparation(struct Recipe* recipe, struct Order** prepared_orders_head, struct Order** prepared_orders_tail, struct Order** pending_orders_head, struct Order** pending_orders_tail, struct Order* new_order, int arrival_time){
    // Controllo se gli ingredienti sono sufficienti
    if(check_ingredients_availability(recipe, new_order->quantity, new_order->arrival_time)){ // ci sono ingredienti a sufficienza per preparare l'ordine
        insert_order(prepared_orders_head, prepared_orders_tail, new_order);

        // eliminare ingredienti usati
        remove_consumed_batch(recipe, new_order->quantity);
    }else{
        insert_order(pending_orders_head, pending_orders_tail, new_order);
    }
}

//* Viene chiamata al momento di un rifornimento e verifica se ci sono ordini in attesa che possono essere evasi
void prepare_pending_order(struct Order** prepared_orders_head, struct Order** prepared_orders_tail, struct Order** pending_orders_head, struct Order** pending_orders_tail, int current_time){
    struct Order* current = *pending_orders_tail;
    struct Order* temp = NULL;

    while(current != NULL){ // scorro la lista dalla coda per trovare il più vecchio ordine che posso evadere
        // Controllo se l'ordine corrente può essere evaso
        if(check_ingredients_availability(current->recipe_ptr, current->quantity, current_time)){
            // elimino gli ingredienti usati
            remove_consumed_batch(current->recipe_ptr, current->quantity);

            // valorizzo temp e incremento current
            temp = current;
            current = current->prev;

            // rimuovo l'ordine dalla lista di attesa e lo inserisco nella lista di quelli pronti
            insert_order(prepared_orders_head, prepared_orders_tail, remove_order(pending_orders_head, pending_orders_tail, temp));
        }
        else{
            current = current->prev;
        }
    }
}



//! FUNZIONI DI GESTIONE DELLA SPEDIZIONE
int calculate_order_weight(struct Order* order){
    int order_weight = 0;
    struct Ingredient* current_ingredient = order->recipe_ptr->ingredients_head;

    while(current_ingredient != NULL){
        order_weight += order->quantity * current_ingredient->quantity;
        current_ingredient = current_ingredient->next;
    }

    return order_weight;
}

void insert_truck(struct Recipe** recipe_book, struct Order** head, struct Order** tail, struct Order* new_order, int order_weight){

    if(*head == NULL || *tail == NULL){ // lista vuota
        *head = new_order;
        *tail = new_order;
    }else{ // aggiungi in ordine di peso dell'ordine
        struct Order* current = *head;

        while(current != NULL && order_weight <= calculate_order_weight(current)){
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



//! FUNZIONI STRUTTURALI
void read_recipe(struct Recipe** recipe_book, struct Goods** store){
    struct Ingredient* ingredients_head = NULL;
    char recipe_name[MAX_LEN] = "";
    char name[MAX_LEN] = "";
    int quantity = 0;

    if(scanf("%s", recipe_name) > 0){ // nome ricetta
        // Controllo se la ricetta è già presente
        if(search_recipe(recipe_book, recipe_name) == NULL){
            // La ricetta non è presente, quindi procedo con la lettura
            while(1){ // creo la lista degli ingredienti
                if (scanf("%s", name) > 0) {
                    if (scanf("%d", &quantity) > 0) {
                        insert_ingredient(store, &ingredients_head, name, quantity);
                    }
                } 
                // Controlla se il prossimo carattere è un newline o EOF
                int next_char = getchar();
                if (next_char == '\n' || next_char == EOF) {
                    break;
                }
            }
            // Inserisco la ricetta una volta aver letto e salvato in una lista tutti gli ingredienti
            insert_recipe(recipe_book, recipe_name, ingredients_head);
            printf("aggiunta\n");
        }else{
            // La ricetta è già presente
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
    struct Recipe* recipe = NULL;

    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta
        recipe = search_recipe(recipe_book, name);
        if(recipe == NULL){ // la ricetta non è presente nel ricettario
            printf("non presente\n");
        }else{ // la ricetta è presente
            if(search_recipe_orders(prepared_orders_head, recipe) || search_recipe_orders(pending_orders_head, recipe)){  // è presente un ordine con la ricetta in questione
                printf("ordini in sospeso\n");
                return;
            }else{
                free_recipe(recipe_book, name);
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

void order(struct Recipe** recipe_book, struct Goods** store, struct Order** prepared_orders_head, struct Order** prepared_orders_tail, struct Order** pending_orders_head, struct Order** pending_orders_tail, int time){ // legge nome della ricetta e la quantità da preparare
    char name[MAX_LEN] = "";
    int quantity = 0;
    struct Recipe* recipe = NULL;

    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta dell'ordine
        if(scanf("%d", &quantity) > 0){
            recipe = search_recipe(recipe_book, name);

            if(recipe == NULL){ // la ricetta non è presente
                printf("rifiutato\n");
            }else{ // la ricetta è presente
                struct Order* new_order = create_order(recipe, time, quantity);

                order_preparation(recipe, prepared_orders_head, prepared_orders_tail, pending_orders_head, pending_orders_tail, new_order, time);

                printf("accettato\n");
            }
        }
    }
}

void truck(struct Recipe** recipe_book, struct Order** prepared_orders_head, struct Order** prepared_orders_tail, int capacity){
    struct Order* truck_head = NULL;
    struct Order* truck_tail = NULL;
    int order_weight = 0;
    struct Order* temp = NULL;

    while(*prepared_orders_tail != NULL && *prepared_orders_head != NULL && capacity > 0){
        // calcolo il peso dell'ordine
        order_weight = calculate_order_weight(*prepared_orders_tail);

        if(order_weight <= capacity){
            capacity -= order_weight;

            temp = remove_order(prepared_orders_head, prepared_orders_tail, *prepared_orders_tail);

            insert_truck(recipe_book, &truck_head, &truck_tail, temp, order_weight);
        }else{
            break;
        }
    }
    if(truck_head == NULL){
        printf("camioncino vuoto\n");
    }else{
        while(truck_head != NULL){
            printf("%d %s %d\n", truck_head->arrival_time, truck_head->recipe_ptr->name, truck_head->quantity);
            delete_order(&truck_head, &truck_tail, truck_head);
        }
    }

    free_orders(&truck_head, &truck_tail);
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
        // printf("%d) Comando: %s  --> ", time, comando);
        if(strcmp(comando, "aggiungi_ricetta") == 0){
            read_recipe(recipe_book, store);
        }else{
            if(strcmp(comando, "rimuovi_ricetta") == 0){
                remove_recipe(recipe_book, prepared_orders_head, pending_orders_head);
            }else{
                if(strcmp(comando, "rifornimento") == 0){
                    rifornimento(store, time);
                    prepare_pending_order(&prepared_orders_head, &prepared_orders_tail, &pending_orders_head, &pending_orders_tail, time); // dopo ogni rifornimento devo verificare se gli ordini in attesa possono essere evasi
                }
                else{
                    if(strcmp(comando, "ordine") == 0){
                        order(recipe_book, store, &prepared_orders_head, &prepared_orders_tail, &pending_orders_head, &pending_orders_tail, time);
                    }
                }
            }
        }

        time++;
        if(time % courier_period == 0){ // verifico se devo caricare il camion con gli ordini terminati
            truck(recipe_book, &prepared_orders_head, &prepared_orders_tail, courier_capacity);
        }
    }

    free_recipe_book(recipe_book);
    free_store(store);
    free_orders(&prepared_orders_head, &prepared_orders_tail);
    free_orders(&pending_orders_head, &pending_orders_tail);
    
    return 0;
}