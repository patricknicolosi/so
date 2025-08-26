/*
Pipeline a 2 stadi
Scrivi un programma con due thread:
Thread A genera numeri casuali da 1 a 100 e li inserisce in una coda di capienza 5.
Thread B legge i numeri dalla coda e li stampa.
Usa mutex e semafori per proteggere l’accesso alla coda.
Obiettivo: capire la gestione di coda e sincronizzazione.
*/

#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"
#include "unistd.h"
#include "string.h"

#define BUFFER_SIZE 5

int random_int()
{
    return (rand() % 100) + 1;
}

int first_empty_position(int *array, int len)
{
    for (int i = 0; i < len; i++)
        if (array[i] == 0)
            return i;
    return -1;
}
int first_full_position(int *array, int len)
{
    for (int i = 0; i < len; i++)
        if (array[i] != 0)
            return i;
    return -1;
}

typedef struct
{
    int buffer[BUFFER_SIZE];
    sem_t empty_positions_on_buffer;
    sem_t full_positions_on_buffer;
    pthread_mutex_t working_on_buffer;
} shared_params;

void *A(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    while (1)
    {
        sem_wait(&(params->empty_positions_on_buffer));

        pthread_mutex_lock(&(params->working_on_buffer));
        int res = first_empty_position(params->buffer, BUFFER_SIZE);
        params->buffer[res] = random_int();
        pthread_mutex_unlock(&(params->working_on_buffer));

        sem_post(&(params->full_positions_on_buffer));

        sleep(rand() % 2);
    }

    pthread_exit(0);
}
void *B(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    while (1)
    {
        sem_wait(&(params->full_positions_on_buffer));

        pthread_mutex_lock(&(params->working_on_buffer));
        int res = first_full_position(params->buffer, BUFFER_SIZE);
        printf("%d \n", params->buffer[res]);
        params->buffer[res] = 0;
        pthread_mutex_unlock(&(params->working_on_buffer));

        sem_post(&(params->empty_positions_on_buffer));

        sleep(rand() % 2);
    }

    pthread_exit(0);
}

int main(void)
{
    shared_params *params;
    params = malloc(sizeof(shared_params));

    int temp_buffer[BUFFER_SIZE] = {0, 0, 0, 0, 0};
    memcpy(params->buffer, temp_buffer, sizeof(int) * BUFFER_SIZE);
    pthread_mutex_init(&(params->working_on_buffer), NULL);
    sem_init(&(params->empty_positions_on_buffer), 0, BUFFER_SIZE);
    sem_init(&(params->full_positions_on_buffer), 0, 0);

    pthread_t tA, tB;
    pthread_create(&tA, NULL, A, params);
    pthread_create(&tB, NULL, B, params);
    pthread_join(tA, NULL);
    pthread_join(tB, NULL);

    return 0;
}