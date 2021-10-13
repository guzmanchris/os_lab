#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 20

/* Define Monitor Struct */
struct monitor {
    /* Shared Variable Declarations */
    int buffer [BUFFER_SIZE];
    int producer_pointer;
    int consumer_pointer;

    /* Conditional Variables */
    pthread_cond_t full;
    pthread_cond_t empty;

    /* Mutex lock */
    pthread_mutex_t mutex;
};

/* Global Variables */
struct monitor monitor;
pthread_t tid;

/* Insert an item into the buffer, while protecting shared memory. */
void insert(int item){
    /* ---- Ensure critical section is safe using a mutex lock and conditional variable ---- */
    pthread_cond_wait(&monitor.empty, &monitor.mutex);  /* Can't insert into a full buffer. Wait for the buffer to have available spaces */

    monitor.buffer[monitor.producer_pointer] = item;
    monitor.producer_pointer = (monitor.producer_pointer + 1) % BUFFER_SIZE;

    pthread_cond_signal(&monitor.full);
    /*--------------------------------------------------------------------------------------*/

    sleep(1);  /* Sleep helps program run more smoothly */
}

/* Remove an item from the buffer, while protecting shared memory. */
int remove_item(){
    /* ---- Ensure critical section is safe using a mutex lock and conditional variable ---- */
    pthread_cond_wait(&monitor.full, &monitor.mutex); /* Can't remove from empty buffer. Wait for elements to be inserted. */

    int item;
    item = monitor.buffer[monitor.consumer_pointer];
    monitor.consumer_pointer = (monitor.consumer_pointer + 1) % BUFFER_SIZE;

    pthread_cond_signal(&monitor.empty);
    /*----------------------------------------------------------------------------------------*/
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

/* Initialize all monitor variables */
void init_monitor(struct monitor monitor) {
    monitor.producer_pointer = 0;
    monitor.consumer_pointer = 0;
    pthread_cond_init(&monitor.full, NULL);
    pthread_cond_init(&monitor.empty, NULL);
    pthread_mutex_init(&monitor.mutex, NULL);
}

int main(int argc, char * argv[])
{
    int producers = atoi(argv[1]);
    int consumers = atoi(argv[2]);
    int i;

    init_monitor(monitor);

    for (i = 0; i < producers; i++)
        pthread_create(&tid, NULL, producer,NULL);

    for (i = 0; i < consumers; i++)
        pthread_create(&tid, NULL, consumer, NULL);

    pthread_join(tid,NULL);
}

