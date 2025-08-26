#include "stdio.h"
#include "pthread.h"
#include "semaphore.h"
#include "string.h"
#include "unistd.h"
#include "stdlib.h"

#define BUFFER_SIZE 10

void produce(int *array, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (array[i] == 0)
        {
            array[i] = rand() % 1000;
            return;
        }
    }
}

void consume(int *array, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (array[i] != 0)
        {
            array[i] = 0;
            return;
        }
    }
}

void print_array(int *array, int len, char *s)
{
    printf("%s: ", s);
    for (int i = 0; i < len; i++)
        printf("%d | ", array[i]);
    printf("\n");
}

typedef struct
{
    int buffer[BUFFER_SIZE];
    sem_t full_positions, empty_positions;
} shared_params;

void *producer(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    for (int i = 0; i < 100; i++)
    {
        sem_wait(&(params->empty_positions));
        produce(params->buffer, BUFFER_SIZE);
        char s[100] = "PRODUCER";
        print_array(params->buffer, BUFFER_SIZE, s);
        sem_post(&(params->full_positions));
    }

    pthread_exit(0);
}

void *consumer(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    for (int i = 0; i < 100; i++)
    {
        sem_wait(&(params->full_positions));
        consume((params->buffer), BUFFER_SIZE);
        char s[100] = "CONSUMER";
        print_array(params->buffer, BUFFER_SIZE, s);
        sem_post(&(params->empty_positions));
    }

    pthread_exit(0);
}

int main(void)
{

    shared_params *params;
    params = malloc(sizeof(shared_params));

    sem_init(&(params->empty_positions), 0, BUFFER_SIZE);
    sem_init(&(params->full_positions), 0, 0);

    int temp_array[BUFFER_SIZE] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };
    memcpy(params->buffer, temp_array, sizeof(int) * BUFFER_SIZE);

    pthread_t consumer_t;
    pthread_t producer_t;

    pthread_create(&consumer_t, NULL, consumer, params);
    pthread_create(&(producer_t), NULL, producer, params);

    pthread_join(consumer_t, NULL);
    pthread_join(producer_t, NULL);

    return 0;
}