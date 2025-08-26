#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"

typedef struct
{
    sem_t sem1, sem2, sem3;
} thread_params;

void *thread1(void *argc)
{
    thread_params *params;
    params = (thread_params *)params;

    for (int i = 0; i < 100; i++)
    {
        sem_wait(&(params->sem1));
        printf("1");
        sem_post(&(params->sem2));
    }
}
void *thread2(void *argc)
{
    thread_params *params;
    params = (thread_params *)params;

    for (int i = 0; i < 100; i++)
    {
        sem_wait(&(params->sem2));
        printf("2");
        sem_post(&(params->sem3));
    }
}
void *thread3(void *argc)
{
    thread_params *params;
    params = (thread_params *)params;

    for (int i = 0; i < 100; i++)
    {
        sem_wait(&(params->sem3));
        printf("3\n");
        sem_post(&(params->sem1));
    }
}

int main()
{
    pthread_t t1, t2, t3;

    thread_params *params;
    params = malloc(sizeof(thread_params));

    sem_init(&(params->sem1), 0, 1);
    sem_init(&(params->sem2), 0, 0);
    sem_init(&(params->sem3), 0, 0);

    pthread_create(&t1, NULL, thread1, params);
    pthread_create(&t2, NULL, thread2, params);
    pthread_create(&t3, NULL, thread3, params);

    return 0;
}