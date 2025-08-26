/*
Scrivere un programma in C che utilizzi i thread per realizzare una pipeline a 3 stadi:
Un thread reader genera una sequenza di numeri interi (per esempio da 1 a 10) e li inserisce in una coda intermedia con capienza limitata.
Un thread verifier estrae i numeri dalla coda intermedia e seleziona solo quelli che rispettano una certa condizione (ad esempio i numeri pari). Questi numeri vengono inseriti in una seconda coda con capienza limitata.
Il thread principale (main) svolge il ruolo di consumatore finale: legge i numeri dalla seconda coda e li stampa a video.
Requisiti:
Nessuna variabile globale: le strutture dati devono essere passate ai thread tramite puntatori a strutture condivise.
*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "semaphore.h"

int prima_posizione_vuota(int *array, int len)
{
    for (int i = 0; i < len; i++)
        if (array[i] == 0)
            return i;
    return -1;
}

int prima_posizione_piena(int *array, int len)
{
    for (int i = 0; i < len; i++)
        if (array[i] != 0)
            return i;
    return -1;
}

int genera_int()
{
    return (rand() % 10) + 1;
}

void stampa_array(int *array, int len)
{
    for (int i = 0; i < len; i++)
        printf("%d ", array[i]);
    printf("\n");
}

typedef struct
{
    int coda_iniziale[10];

    int coda_finale[5];
    int cursore_coda_finale;

    pthread_mutex_t accedendo_alla_coda_iniziale;
    sem_t posizioni_libere_coda_iniziale;
    sem_t posizioni_occupate_coda_iniziale;

    pthread_mutex_t accedendo_alla_coda_finale;
    sem_t posizioni_libere_coda_finale;
    sem_t posizioni_occupate_coda_finale;

} parametri_thread;

void *thread_generatore(void *arg)
{
    parametri_thread *params;
    params = (parametri_thread *)arg;

    while (1)
    {
        sem_wait(&(params->posizioni_libere_coda_iniziale));

        pthread_mutex_lock(&(params->accedendo_alla_coda_iniziale));
        int res = prima_posizione_vuota(params->coda_iniziale, 10);
        params->coda_iniziale[res] = genera_int();
        pthread_mutex_unlock(&(params->accedendo_alla_coda_iniziale));

        sem_post(&(params->posizioni_occupate_coda_iniziale));

        sleep(rand() % 2);
    }

    pthread_exit(0);
}
void *thread_verificatore(void *arg)
{
    parametri_thread *params = (parametri_thread *)arg;

    while (1)
    {
        sem_wait(&(params->posizioni_occupate_coda_iniziale));

        pthread_mutex_lock(&(params->accedendo_alla_coda_iniziale));
        int res = prima_posizione_piena(params->coda_iniziale, 10);
        int val = params->coda_iniziale[res];
        params->coda_iniziale[res] = 0;
        pthread_mutex_unlock(&(params->accedendo_alla_coda_iniziale));

        sem_post(&(params->posizioni_libere_coda_iniziale));

        if (val % 2 == 0)
        {
            sem_wait(&(params->posizioni_libere_coda_finale));
            pthread_mutex_lock(&(params->accedendo_alla_coda_finale));
            int res2 = prima_posizione_vuota(params->coda_finale, 5);
            params->coda_finale[res2] = val;
            pthread_mutex_unlock(&(params->accedendo_alla_coda_finale));
            sem_post(&(params->posizioni_occupate_coda_finale));
        }

        sleep(rand() % 2);
    }

    pthread_exit(0);
}

int main(void)
{
    parametri_thread *params;
    params = malloc(sizeof(parametri_thread));

    pthread_mutex_init(&(params->accedendo_alla_coda_iniziale), NULL);
    pthread_mutex_init(&(params->accedendo_alla_coda_finale), NULL);

    int coda_iniziale_temp[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    memcpy(params->coda_iniziale, coda_iniziale_temp, sizeof(int) * 10);

    int coda_finale_temp[5] = {0, 0, 0, 0, 0};
    memcpy(params->coda_finale, coda_finale_temp, sizeof(int) * 5);

    params->cursore_coda_finale = 0;

    sem_init(&(params->posizioni_libere_coda_iniziale), 0, 10);
    sem_init(&(params->posizioni_occupate_coda_iniziale), 0, 0);

    sem_init(&(params->posizioni_libere_coda_finale), 0, 10);
    sem_init(&(params->posizioni_occupate_coda_finale), 0, 0);

    stampa_array(params->coda_iniziale, 10);
    stampa_array(params->coda_finale, 5);

    pthread_t generatori[8];
    for (int i = 0; i < 8; i++)
        pthread_create(&generatori[i], NULL, thread_generatore, params);

    pthread_t verificatori[7];
    for (int i = 0; i < 7; i++)
        pthread_create(&verificatori[i], NULL, thread_verificatore, params);

    while (1)
    {
        sem_wait(&(params->posizioni_occupate_coda_finale));

        pthread_mutex_lock(&(params->accedendo_alla_coda_finale));
        int res = prima_posizione_piena(params->coda_finale, 5);
        params->coda_finale[res] = 0;
        pthread_mutex_unlock(&(params->accedendo_alla_coda_finale));

        sem_post(&(params->posizioni_libere_coda_finale));

        sleep(rand() % 3);
    }

    for (int i = 0; i < 8; i++)
        pthread_join(generatori[i], NULL);
    for (int i = 0; i < 7; i++)
        pthread_join(verificatori[i], NULL);

    exit(EXIT_SUCCESS);
}