/*  
    COMANDI PER ESECUZIONE E COMPILAZIONE:
    Compilazione: gcc -Wall -Werror -std=gnu11 -O2 -lm main.c recipe.c store.c utils.c  -o program
    Esecuzione: ./program < file_input.txt > file_output.txt
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "recipe.h"
#include "store.h"
#include "utils.h"

int main(){
    //* Lettura comandi
    char comando[MAX_LEN];
    struct Recipes* tb_recipe = create_table_recipes();
    create_store(); // alloca memoria per il vettore di puntatori

    while(scanf("%s", comando) > 0){
        // Determino il comando
        if(strcmp(comando, "aggiungi_ricetta") == 0){
            read_recipe(tb_recipe);
        }else{
            if(strcmp(comando, "rimuovi_ricetta") == 0){
                remove_recipe(tb_recipe);
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
    print_table_recipe(tb_recipe);
    print_store();
    print_batch();

    return 0;
}