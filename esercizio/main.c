#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "semaphore.h"
#include "pthread.h"
#include "stdbool.h"
/*
- Player1, PLayer2 che aspettano di esser autorizzati da Giudice
- Giudice che autorizza i Player e aspetta la mossa, poi giudica quella vincente e aggiorna il Tabellone
- Tabellone tiene traccia delle vittorie, stampa la classifica dopo ogni turno e dice ai giocatori di giocatore

Struttura condivisa:
1. Mosse correnti player
2. flag terminazione
3. ultimo vincitore

*/

typedef enum
{
    SASSO,
    CARTA,
    FORBICE
} Mosse;

int morra(Mosse m1, Mosse m2)
{
    if (m1 == m2)
        return 0;
    else if (m1 == SASSO && m2 == CARTA)
        return 2;
    else if (m2 == SASSO && m1 == CARTA)
        return 1;
    else if (m1 == SASSO && m2 == FORBICE)
        return 1;
    else if (m2 == SASSO && m1 == FORBICE)
        return 2;
    else if (m1 == CARTA && m2 == FORBICE)
        return 2;
    else if (m2 == CARTA && m1 == FORBICE)
        return 1;
}

typedef enum
{
    PLAYER_1,
    PLAYER_2,
    GIUDICE,
    TABELLONE
} Ruoli;

typedef struct
{
    Mosse m1, m2;
    int ultimo_vincitore;
    bool done;
    sem_t sem[4];
    int numero_partite;
} SharedParams;

void init_shared_params(SharedParams *params, int numero_partite)
{
    int error;

    params->done = false;
    params->numero_partite = numero_partite;

    if ((error = sem_init(&(params->sem[PLAYER_1]), 0, 1)) != 0)
        exit(EXIT_FAILURE);
    if ((error = sem_init(&(params->sem[PLAYER_2]), 0, 1)) != 0)
        exit(EXIT_FAILURE);
    if ((error = sem_init(&(params->sem[GIUDICE]), 0, 0)) != 0)
        exit(EXIT_FAILURE);
    if ((error = sem_init(&(params->sem[TABELLONE]), 0, 0)) != 0)
        exit(EXIT_FAILURE);
}

typedef struct
{
    SharedParams *shared_params;
    int player_type;
} PlayerData;

void Player(void *arg)
{
    char *mosse_text[] = {"SASSO", "CARTA", "FORBICE"};

    PlayerData *params;
    params = (PlayerData *)arg;

    int error;
    while (params->shared_params->done == false)
    {
        if ((error = sem_wait(&(params->shared_params->sem[params->player_type]))) != 0)
            exit(EXIT_FAILURE);

        if (params->player_type == PLAYER_1)
        {
            params->shared_params->m1 = (rand() % 3) + 1;
            printf("%d ha lanciato %s", params->player_type, mosse_text[params->shared_params->m1]);
        }
        if (params->player_type == PLAYER_2)
        {
            params->shared_params->m2 = (rand() % 3) + 1;
            printf("%d ha lanciato %s", params->player_type, mosse_text[params->shared_params->m1]);
        }
        if ((error = sem_post(&(params->shared_params->sem[GIUDICE]))) != 0)
            exit(EXIT_FAILURE);
    }

    pthread_exit(NULL);
}

void Giudice(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    int error;
    while (params->done == false)
    {
        if ((error = sem_wait(&(params->sem[GIUDICE]))) != 0)
            exit(EXIT_FAILURE);
        if ((error = sem_wait(&(params->sem[GIUDICE]))) != 0)
            exit(EXIT_FAILURE);

        int morra_res = morra(params->m1, params->m2);
        if (morra_res == 0)
        {
            printf("Pareggio \n");

            if ((error = sem_post(&(params->sem[PLAYER_1]))) != 0)
                exit(EXIT_FAILURE);
            if ((error = sem_post(&(params->sem[PLAYER_2]))) != 0)
                exit(EXIT_FAILURE);
        }
        else
        {
            printf("vince il giocatore %d \n", morra_res);
            params->ultimo_vincitore = morra_res;

            if ((error = sem_post(&(params->sem[TABELLONE]))) != 0)
                exit(EXIT_FAILURE);
        }
    }

    pthread_exit(NULL);
}

void Tabellone(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    int classifica[2] = {0, 0};

    int error;
    for (int i = 0; i < params->numero_partite; i++)
    {
        if ((error = sem_wait(&(params->sem[TABELLONE]))) != 0)
            exit(EXIT_FAILURE);

        classifica[params->ultimo_vincitore - 1]++;

        if (i < params->numero_partite - 1)
        {
            printf("Player1:%d, Player2:%d \n", classifica[0], classifica[1]);
            if ((error = sem_post(&(params->sem[PLAYER_1]))) != 0)
                exit(EXIT_FAILURE);
            if ((error = sem_post(&(params->sem[PLAYER_2]))) != 0)
                exit(EXIT_FAILURE);
        }
    }

    params->done = true;
    if ((error = sem_post(&(params->sem[PLAYER_1]))) != 0)
        exit(EXIT_FAILURE);
    if ((error = sem_post(&(params->sem[PLAYER_2]))) != 0)
        exit(EXIT_FAILURE);
    if ((error = sem_post(&(params->sem[GIUDICE]))) != 0)
        exit(EXIT_FAILURE);
    if ((error = sem_post(&(params->sem[GIUDICE]))) != 0)
        exit(EXIT_FAILURE);

    printf("CLASSIFICA FINALE: Player1:%d, Player2:%d \n", classifica[0], classifica[1]);

    if (classifica[0] > classifica[1])
        printf("Vincitore PLayer1\n");
    else if (classifica[0] == classifica[1])
        printf("Pareggio\n");
    else
        printf("Vincitore PLayer2\n");

    pthread_exit(NULL);
}

int main(int arg, char **argv)
{
    srand(time(NULL));

    if (arg != 2 && atoi(argv[1]) == 0)
        exit(1); // e me ne esco

    SharedParams *params = malloc(sizeof(SharedParams));
    init_shared_params(params, atoi(argv[1]));

    pthread_t threads[4];

    PlayerData *d1 = malloc(sizeof(PlayerData));
    d1->player_type = PLAYER_1;
    d1->shared_params = params;

    PlayerData *d2 = malloc(sizeof(PlayerData));
    d2->player_type = PLAYER_2;
    d2->shared_params = params;

    // TODO: Aggiungere gestione errori
    pthread_create(&(threads[PLAYER_1]), NULL, (void *)Player, d1);
    pthread_create(&(threads[PLAYER_2]), NULL, (void *)Player, d2);
    pthread_create(&(threads[GIUDICE]), NULL, (void *)Giudice, params);
    pthread_create(&(threads[TABELLONE]), NULL, (void *)Tabellone, params);

    for (int i = 0; i < 4; i++)
        pthread_join(threads[i], NULL);

    return 0;
}