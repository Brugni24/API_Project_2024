#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "order.h"
#include "recipe.h"
#include "utils.h"

struct Order* prepared_orders = NULL;
struct Order* pending_orders = NULL;



void ordine(struct Recipes* tb_recipe){
    char name[MAX_LEN];
    int quantity = 0;
    if(scanf("%s", name) > 0 ){ // leggo il nome della ricetta dell'ordine
        if(search_recipe(tb_recipe, name) == -1){ // la ricetta non è presente
            printf("rifiutato\n");
        }else{ // la ricetta è presente
            if(scanf("%d", &quantity) > 0){
                printf("accettato\n");
                insert_order(name, quantity);
            }
        }
    }
}