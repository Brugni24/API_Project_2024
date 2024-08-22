#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 256
#define RECIPE_BOOK_SIZE 5000
#define RECIPE_BOOK_PRIME 4999 // numero primo per la funzione hash 2

int recipes_counter = 0;

//! DEFINIZIONE STRUTTURE DATI
struct Ingredient{
    char name[MAX_LEN];
    int quantity;
    struct Ingredient* next;
};

struct Recipe{
    int key;
    char name[MAX_LEN];
    struct Ingredient* ingredients_head;
    struct Recipe* next_recipe;
};

unsigned int key_function(char *name){ // Algoritmo DJB2
    unsigned int k = 5381;
    int c;
    while ((c = *name++))
        k = ((k << 5) + k) + c;
    return k;
}

unsigned int hash_function_RB(char *name){return key_function(name) % RECIPE_BOOK_SIZE;}


struct Recipe** create_recipe_book(){
    struct Recipe** recipe_book = NULL;
    recipe_book = (struct Recipe**)calloc(RECIPE_BOOK_SIZE, sizeof(struct Recipe*)); // alloca spazio per le celle della tabella

    // Controllo se l'allocazione di memoria è andata a buon fine
    if (recipe_book == NULL) {
        // Gestione dell'errore di allocazione della memoria
        printf("Errore: allocazione della memoria fallita\nFunzione: create_recipe_book()\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < RECIPE_BOOK_SIZE; i++){
        recipe_book[i] = NULL;
    }

    return recipe_book;
}

struct Recipe* create_recipe(char* name, struct Ingredient* ingredients_head){
    struct Recipe* recipe = (struct Recipe*)malloc(sizeof(struct Recipe));

    // Controllo se l'allocazione di memoria è andata a buon fine
    if (recipe == NULL) {
        // Gestione dell'errore di allocazione della memoria
        printf("Errore: allocazione della memoria fallita\nFunzione: create_recipe()\n");
        exit(EXIT_FAILURE);
    }
    
    recipe->key = key_function(name);
    strcpy(recipe->name, name);
    recipe->ingredients_head = ingredients_head;
    recipe->next_recipe = NULL;

    return recipe;
}

void insert_recipe(struct Recipe** recipe_book, char* name, struct Ingredient* ingredients_head){
    struct Recipe* new_recipe = create_recipe(name, ingredients_head);
    
    // Calcolo l'indice della ricetta
    int index = hash_function_RB(name);

    recipes_counter++;

    if(recipe_book[index] == NULL){ // Cella vuota
        recipe_book[index] = new_recipe;
    }else{ // Cella occupata
        new_recipe->next_recipe = recipe_book[index];
        recipe_book[index] = new_recipe;
    }
}

int main(){
    struct Recipe** recipe_book = create_recipe_book();

    char comando[MAX_LEN] = "";

    while(scanf("%s", comando) > 0){
        insert_recipe(recipe_book, comando, NULL);
    }

    printf("Numero ricette inserite: %d\n", recipes_counter);

    return 0;
}