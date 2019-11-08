#include "BENSCHILLIBOWL.h"

#include <assert.h>
#include <stdlib.h>
#include <time.h>

bool IsEmpty(BensChilliBowl* bcb);
bool IsFull(BensChilliBowl* bcb);
void AddOrderToBack(Order **orders, Order *order);

MenuItem BensChilliBowlMenu[] = { 
    "BensChilli", 
    "BensHalfSmoke", 
    "BensHotDog", 
    "BensChilliCheeseFries", 
    "BensShake",
    "BensHotCakes",
    "BensCake",
    "BensHamburger",
    "BensVeggieBurger",
    "BensOnionRings",
};
int BensChilliBowlMenuLength = 10;

MenuItem PickRandomMenuItem() {
    return BensChilliBowlMenu[rand() % BensChilliBowlMenuLength];
}

BensChilliBowl* OpenRestaurant(int max_size, int expected_num_orders) {
    
    BensChilliBowl *cb = (BensChilliBowl*) malloc(sizeof(BensChilliBowl));
    
    cb->orders = NULL;
    cb->current_size = 0;
    cb->max_size = max_size;
    cb->next_order_number = 1;
    cb->orders_handled = 0;
    cb->expected_num_orders = expected_num_orders;
    
    pthread_mutex_init(&(cb->mutex), NULL);
    pthread_cond_init(&(cb->can_add_orders), NULL);
    pthread_cond_init(&(cb->can_get_orders), NULL);
    
    printf("Restaurant is open!\n");
    return cb;
}

void CloseRestaurant(BensChilliBowl* bcb) {
    printf("Orders handled: %d\n", bcb->orders_handled);
    printf("Orders expected: %d\n", bcb->expected_num_orders);
    if(bcb->expected_num_orders == bcb->orders_handled){
       pthread_mutex_destroy(&bcb->mutex);
       pthread_cond_destroy(&bcb->can_add_orders);
       pthread_cond_destroy(&bcb->can_get_orders);
    }
    free(bcb);
    printf("Restaurant is closed!\n");
}

int AddOrder(BensChilliBowl* bcb, Order* order) {
    pthread_mutex_lock(&(bcb->mutex));
    while (IsFull(bcb))
        pthread_cond_wait(&(bcb->can_add_orders), &(bcb->mutex));
    
    bcb->current_size++;
    order->order_number = bcb->next_order_number++;
    AddOrderToBack(&(bcb->orders), order);
    
    pthread_cond_signal(&(bcb->can_get_orders));
    pthread_mutex_unlock(&(bcb->mutex));
    return order->order_number;
}

Order *GetOrder(BensChilliBowl* bcb) {
    pthread_mutex_lock(&(bcb->mutex));
    while (IsEmpty(bcb))
        pthread_cond_wait(&(bcb->can_get_orders), &(bcb->mutex));
    Order *o = bcb->orders;
    bcb->orders = bcb->orders->next;
    bcb->current_size--;
    bcb->orders_handled++;
    pthread_cond_signal(&(bcb->can_add_orders));
    if (bcb->orders_handled == bcb->expected_num_orders)
        pthread_cond_broadcast(&(bcb->can_add_orders));
    pthread_mutex_unlock(&(bcb->mutex));
    return o;
}

bool IsEmpty(BensChilliBowl* bcb) {
    return bcb->current_size == 0;
}

bool IsFull(BensChilliBowl* bcb) {
    return bcb->current_size == bcb->max_size;
}

void AddOrderToBack(Order **orders, Order *order) {

    if (!orders[0]) {
        orders[0] = order;
        order->next = NULL;
    }
    Order *temp = orders[0]; 
    while (temp->next != NULL)
        temp = temp->next;
    temp->next = order;
    order->next = NULL;
    return;
}

