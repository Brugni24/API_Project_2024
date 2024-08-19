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

void remove_recipe(struct Recipe** recipe_book){
    char name[MAX_LEN] = "";
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta
        if(search_recipe(recipe_book, name) == -1){ // la ricetta non è presente nel ricettario
            printf("non presente\n");
        }else{ // la ricetta è presente
            
                int index = search_recipe(recipe_book, name);
                free_recipe(recipe_book[index]);
                recipe_book[index] = NULL;
                printf("rimossa\n");
            
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



int main(){
    //* Dichiarazione strutture dati:
    struct Recipe** recipe_book = NULL; // Tabella hash per la raccolta delle ricette

    //* Dichiarazione veriabili:
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
    
    //* Lettura comandi
    while(scanf("%s", comando) > 0){
        if(recipe_counter >= TABLE_SIZE*0.75){
            fprintf(stderr, "Errore: tabella hash sovraccaricata\n");
            exit(EXIT_FAILURE);
        }
        // printf("%d) Comando: %s  --> ", time, comando);
        if(strcmp(comando, "aggiungi_ricetta") == 0){
            read_recipe(recipe_book);
        }
        print_recipe_book(recipe_book);
    }
    return 0;
}