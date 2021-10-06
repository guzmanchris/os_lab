#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 20

/* Initial configuration */
int producer_pointer = 0;
int consumer_pointer = 0;
int full = 0;
int empty = 1;
int buffer [BUFFER_SIZE];
pthread_t tid;
pthread_mutex_t mutex;

/* Insert an item into the buffer, while protecting shared memory. */
void insert(int item){
    while (full);  /* Can't insert into a full buffer. Wait for the buffer to have available spaces */

    /* ---- Ensure critical section is safe using a mutex lock ---- */
    pthread_mutex_lock(&mutex);
    buffer[producer_pointer] = item;
    producer_pointer = (producer_pointer + 1) % BUFFER_SIZE;
    empty = 0;
    if (producer_pointer == consumer_pointer) full = 1;
    pthread_mutex_unlock(&mutex);
    /*-------------------------------------------------------------*/

    sleep(1);  /* Sleep helps program run more smoothly */
}

/* Remove an item from the buffer, while protecting shared memory. */
int remove_item(){
    while (empty); /* Can't remove from empty buffer. Wait for elements to be inserted. */

    /* ---- Ensure critical section is safe using a mutex lock ---- */
    pthread_mutex_lock(&mutex);
    int item;
    item = buffer[consumer_pointer];
    consumer_pointer = (consumer_pointer + 1) % BUFFER_SIZE;
    full = 0;
    if (consumer_pointer == producer_pointer) empty = 1;
    pthread_mutex_unlock(&mutex);
    /*-------------------------------------------------------------*/

    sleep(1);  /* Sleep helps program run more smoothly */
    return item;
}

void * producer(void * param){
    int item;
    while(1){
        item = rand() % BUFFER_SIZE ;
        insert(item);
        printf("inserted: %d\n", item);
    }
}

void * consumer(void * param){
    int item;
    while(1){
        item = remove_item();
        printf("removed: %d\n", item);
    }
}

int main(int argc, char * argv[])
{
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int i;

    pthread_mutex_init(&mutex, NULL);

    for (i = 0; i < producers; i++)
        pthread_create(&tid, NULL, producer,NULL);

    for (i = 0; i < consumers; i++)
        pthread_create(&tid, NULL, consumer, NULL);

    pthread_join(tid,NULL);
}

