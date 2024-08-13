#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "recipe.h"
#include "utils.h"

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

void print_table(struct Ingredients* table){
    printf("\nTabella Ingredienti\n-------------------\n");
    for (int i = 0; i < TABLE_SIZE; i++){
        if (table->ingredients[i]){
            printf("Index: %d, Key: %d, Value: %s, Quantity: %d\n", i, table->ingredients[i]->key, table->ingredients[i]->name, table->ingredients[i]->quantity);
        }
    }
    printf("-------------------\n\n");
}

void print_table_recipe(struct Recipes* table){
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
    table->ingredients[index] = ingredient;
    table->count++;
}

void handle_collision_recipe(struct Recipes* table, struct Recipe* recipe, int index){
    int i = 1;
    while(table->recipes[index] != NULL){
        index = double_hashing(recipe->name, i);
        i++;
    }
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