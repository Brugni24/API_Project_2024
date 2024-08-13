#ifndef ORDER_H
#define ORDER_H

#define MAX_LEN 256

struct Order{
    int arrival_time;
    int quantity;
    char recipe[MAX_LEN];
    struct Order* next;
};

void ordine(struct Recipes* tb_recipe);

#endif