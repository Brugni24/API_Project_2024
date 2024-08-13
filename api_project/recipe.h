#ifndef RECIPE_H
#define RECIPE_H

#define MAX_LEN 256

//* Dichiarazione strutture dati
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


//* Dichiarazione funzioni
struct Ingredient* create_ingredient(char* name, int quantity);
struct Ingredients* create_table_ingredients();
struct Recipes* create_table_recipes();
struct Recipe* create_recipe(char* name, struct Ingredients* tb_ingredient);
void print_table(struct Ingredients* table);
void print_table_recipe(struct Recipes* table);
void handle_collision_ingredient(struct Ingredients* table, struct Ingredient* ingredient, int index);
void handle_collision_recipe(struct Recipes* table, struct Recipe* recipe, int index);
void insert_ingredient(struct Ingredients* table, char* name, int quantity);
void insert_recipe(struct Recipes* table, struct Recipe* recipe);
struct Ingredient* search_ingredient(struct Ingredients* table, char* name);
int search_recipe(struct Recipes* table, char* name);
void print_search_ingredient(struct Ingredients* table, char* name);
void read_recipe(struct Recipes* table);
void free_ingredients(struct Ingredients* table);
void free_recipe(struct Recipe* recipe);
void remove_recipe(struct Recipes* table);

#endif