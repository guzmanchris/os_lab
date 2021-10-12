#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFER_SIZE 20

/* Initial configuration */
int producer_pointer = 0;
int consumer_pointer = 0;
sem_t mutex, empty, full;
int buffer [BUFFER_SIZE];
pthread_t tid;

/* Insert an item into the buffer, while protecting shared memory. */
void insert(int item){
    sem_wait(&empty);  /* Can't insert into a full buffer. Wait for the buffer to have available spaces */

    /* ---- Ensure critical section is safe using a binary semaphore ---- */
    sem_wait(&mutex);
    buffer[producer_pointer] = item;
    producer_pointer = (producer_pointer + 1) % BUFFER_SIZE;
    sem_post(&mutex);
    /*-------------------------------------------------------------*/

    sem_post(&full);  /* Let consumer know an item was inserted */
    sleep(1);  /* Sleep helps program run more smoothly */
}

/* Remove an item from the buffer, while protecting shared memory. */
int remove_item(){
    sem_wait(&full);  /* Can't remove from empty buffer. Wait for elements to be inserted. */

    /* ---- Ensure critical section is safe using a binary semaphore ---- */
    sem_wait(&mutex);
    int item;
    item = buffer[consumer_pointer];
    consumer_pointer = (consumer_pointer + 1) % BUFFER_SIZE;
    sem_post(&mutex);
    /*-------------------------------------------------------------*/

    sem_post(&empty);  /* Let producer know an item was consumed */
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

    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);

    for (i = 0; i < producers; i++)
        pthread_create(&tid, NULL, producer,NULL);

    for (i = 0; i < consumers; i++)
        pthread_create(&tid, NULL, consumer, NULL);

    pthread_join(tid,NULL);

    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);
}

