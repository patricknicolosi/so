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

int random_int()
{
    return (rand() % 10) + 1;
}

int first_empty_position(int *array, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (array[i] == -1)
            return i;
    }
    return -1;
}

int first_full_position(int *array, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (array[i] != -1)
            return i;
    }
    return -1;
}

typedef struct
{
    int coda_iniziale[10];
    int coda_finale[5];
    sem_t posizioni_vuote_coda_iniziale;
    sem_t posizioni_piene_coda_iniziale;
    sem_t posizioni_vuote_coda_finale;
    sem_t posizioni_piene_coda_finale;
    pthread_mutex_t mutex_coda_iniziale;
    pthread_mutex_t mutex_coda_finale;
} SharedParams;

void *Producer(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    while (1)
    {
        // Produce solo se c'è almeno una posizione libera nella coda iniziale
        sem_wait(&(params->posizioni_vuote_coda_iniziale));

        pthread_mutex_lock(&(params->mutex_coda_iniziale));

        // Riempie la prima posizione vuota della coda che trova
        int search_res = first_empty_position(params->coda_iniziale, 10);
        params->coda_iniziale[search_res] = random_int();

        pthread_mutex_unlock(&(params->mutex_coda_iniziale));
        sleep(rand() % 2);

        // Avverte che nella coda iniziale c'è una posizione in più
        sem_post(&(params->posizioni_piene_coda_iniziale));
    }

    pthread_exit(0);
}

void *Verifier(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    while (1)
    {
        // Aspetta se c'è almeno una posizione vuota nella coda finale
        sem_wait(&(params->posizioni_vuote_coda_finale));

        pthread_mutex_lock(&(params->mutex_coda_finale));
        pthread_mutex_lock(&(params->mutex_coda_iniziale));

        // Prende la prima posiizone piene della coda iniziale
        int search_res_coda_iniziale = first_full_position(params->coda_iniziale, 10);
        // Assegna la prima posizione libera della coda finale
        int search_res_coda_finale = first_empty_position(params->coda_finale, 5);
        params->coda_finale[search_res_coda_finale] = params->coda_iniziale[search_res_coda_iniziale];
        // Svuota la posizione di coda iniziale
        params->coda_iniziale[search_res_coda_iniziale] = -1;

        pthread_mutex_unlock(&(params->mutex_coda_finale));
        pthread_mutex_unlock(&(params->mutex_coda_iniziale));

        sleep(rand() % 2);

        // Ci dice che c'è una posizione vuota in piu nella coda iniziale
        sem_post(&(params->posizioni_vuote_coda_iniziale));
        sem_post(&(params->posizioni_piene_coda_finale));
    }

    pthread_exit(0);
}

int main(void)
{

    srand(time(NULL));

    SharedParams *params;
    params = malloc(sizeof(SharedParams));

    for (int i = 0; i < 10; i++)
    {
        params->coda_iniziale[i] = -1;
    }
    for (int i = 0; i < 5; i++)
    {
        params->coda_finale[i] = -1;
    }

    sem_init(&(params->posizioni_vuote_coda_iniziale), 0, 10);
    sem_init(&(params->posizioni_piene_coda_iniziale), 0, 0);
    sem_init(&(params->posizioni_vuote_coda_finale), 0, 5);
    sem_init(&(params->posizioni_piene_coda_finale), 0, 0);

    pthread_mutex_init(&(params->mutex_coda_iniziale), NULL);
    pthread_mutex_init(&(params->mutex_coda_finale), NULL);

    pthread_t producer_t, verifier_t;

    pthread_create(&(producer_t), NULL, Producer, params);
    pthread_create(&(verifier_t), NULL, Verifier, params);

    while (1)
    {
        // Aspetta se ci sono posizioni piene nella coda finale
        sem_wait(&(params->posizioni_piene_coda_finale));

        pthread_mutex_lock(&(params->mutex_coda_finale));

        // Prende la prima cosa che trova in questa coda la stampa e libera
        int search_res = first_full_position(params->coda_finale, 5);
        printf("%d \n", params->coda_finale[search_res]);
        params->coda_finale[search_res] = -1;

        pthread_mutex_unlock(&(params->mutex_coda_finale));

        sleep(rand() % 2);

        // Avverte che c'è nuovo spazio nella coda finale
        sem_post(&(params->posizioni_vuote_coda_finale));
    }

    pthread_join(&producer_t, NULL);
    pthread_join(&Verifier, NULL);

    return 0;
}
