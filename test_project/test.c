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
};

//* ISTANZE GLOBALI
struct Recipe** recipe_book = NULL; // Tabella hash per la raccolta delle ricette
struct Goods** store = NULL; // Tabella hash che rappresenta il magazzino: array di puntatori che punta alla merce (ingredienti) presente in magazzino
struct Order* prepared_orders = NULL; // Ordini pronti ad essere spediti
struct Order* pending_orders = NULL; // Ordini in attesa di ricevere gli ingredienti

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

void remove_recipe(){
    char name[MAX_LEN];
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta
        if(search_recipe(name) == -1){ // la ricetta non è presente
            printf("non presente\n");
        }else{ // la ricetta è presente
            if(1 < 0){  // è presente un ordine con la ricetta in questione

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



//* FUNZIONI PER DEBUGGING
void print_ingredients_table(struct Ingredient** tb_ingredients){
    printf("\nTabella Ingredienti\n-------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (tb_ingredients[i]){
            printf("Index: %d, Key: %d, Value: %s, Quantity: %d\n", i, tb_ingredients[i]->key, tb_ingredients[i]->name, tb_ingredients[i]->quantity);
        }
    }
    printf("-------------------\n\n");
}

void print_table_recipe(){
    printf("\nTabella Ricette\n-------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (recipe_book[i]){
            printf("Index: %d, Key: %d, Value: %s\n", i, recipe_book[i]->key, recipe_book[i]->name);
        }
    }
    printf("-------------------\n\n");
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

void print_store(){
    printf("\nTabella Store\n-------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (store[i]){
            printf("Index: %d, Key: %d, Name: %s, Quantity:%d\n", i, store[i]->key, store[i]->name, store[i]->total_quantity);
        }
    }
    printf("-------------------\n\n");
}

int main(){
    int t_corriere = 0;
    int q_corriere = 0;

    //* Lettura dati corriere: periodo e quantità
    if(scanf("%i", &t_corriere)<1){
        printf("Errore nella lettura di t_corriere");
        return 0;
    }else{
        printf("Periodo corriere: %i\n", t_corriere);
    }
    if(scanf("%i", &q_corriere)<1){
        printf("Errore nella lettura di q_corriere");
        return 0;
    }else{
        printf("Quantità corriere: %i\n", q_corriere);
    }

    //* Lettura comandi
    char comando[MAX_LEN];
    create_recipe_book();
    create_store();

    while(scanf("%s", comando) > 0){
        // Determino il comando
        if(strcmp(comando, "aggiungi_ricetta") == 0){
            read_recipe(recipe_book);
        }else{
            if(strcmp(comando, "rimuovi_ricetta") == 0){
                remove_recipe(recipe_book);
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
    print_table_recipe(recipe_book);
    print_store();
    print_batch();
    return 0;
}