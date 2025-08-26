/*

Cameriere ->(aspetta fattorino) Fattorino
*/

#include "misc.h"
typedef struct
{
    sem_t aspetto_cameriere;
    sem_t aspetto_pizzaiolo;
    sem_t aspetto_fattorino;
    int terminazione;
    Ordine ordine_attuale;
} SharedParams;
void *Fattorino(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;
    // Il pizzaiolo conosce i tempi di preparazione
    Tempo *tempi;
    tempi = malloc(sizeof(Tempo) * 20);
    read_tempi_from_file(tempi);
    while (1)
    {
        // Fattorino aspetta pizzaiolo
        printf("FATTORINO: Aspetto che il pizzaiolo sia prtono!\n");
        sem_wait(&(params->aspetto_pizzaiolo));
        if (params->terminazione == 1)
            break;
        // Simula la consegna
        sleep(rand() % 3);
        printf("FATTORINO: Avviso cameriere che mi sono liberato!\n");
        sem_post(&(params->aspetto_fattorino));
    }
    pthread_exit(0);
}
void *Pizzaiolo(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;
    // Il pizzaiolo conosce i tempi di preparazione
    Tempo *tempi;
    tempi = malloc(sizeof(Tempo) * 20);
    read_tempi_from_file(tempi);

    pthread_t fattorino_t;
    pthread_create(&fattorino_t, NULL, Fattorino, params);

    while (1)
    {
        // Pizzaiolo aspetta Cameriere
        printf("PIZZAIOLO: Aspetto il cameriere!\n");
        sem_wait(&(params->aspetto_cameriere));
        if (params->terminazione == 1)
            break;
        // Simula preparazione
        sleep((tempo_corrispondente(params->ordine_attuale.pizza, tempi)) / 5);
        // Pizzaiolo avvisa Fattorino
        printf("PIZZAIOLO: Dico al fattorino che puo partire\n");
        sem_post(&(params->aspetto_pizzaiolo));
    }

    pthread_join(fattorino_t, NULL);
    pthread_exit(0);
}
void *Cameriere(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;
    // Legge gli ordini e li mette in memoria
    Ordine *ordini;
    ordini = malloc(sizeof(Ordine) * 20);
    read_ordini_from_file(ordini);

    pthread_t pizzaiolo_t;
    pthread_create(&pizzaiolo_t, NULL, Pizzaiolo, params);

    printf("CAMERIERE:Ho letto il file ordini\n");

    // Tiene in considerazione un turno alla volta
    for (int i = 0; i < 20; i++)
    {
        // Aspetta il fattorino
        printf("CAMERIERE: Aspetto che il fattorino si liberi!\n");
        sem_wait(&(params->aspetto_fattorino));
        params->ordine_attuale = ordini[i];
        // Avvisa Pizzaiolo che può preparare la pizza
        printf("CAMERIERE: Aspetto che il fattorino si liberi!\n");
        sem_post(&(params->aspetto_cameriere));

        printf("SIAMO ARRIVATI A SERVIRE %d/20 ordini\n", i + 1);
    }
    params->terminazione = 1;
    sem_post(&(params->aspetto_cameriere));
    sem_post(&(params->aspetto_pizzaiolo));
    pthread_join(pizzaiolo_t, NULL);
    pthread_exit(0);
}

int main(void)
{
    SharedParams *params;
    params = malloc(sizeof(SharedParams));
    sem_init(&(params->aspetto_fattorino), 0, 1);
    sem_init(&(params->aspetto_cameriere), 0, 0);
    sem_init(&(params->aspetto_pizzaiolo), 0, 0);
    params->terminazione = 0;
    pthread_t cameriere_t;
    pthread_create(&cameriere_t, NULL, Cameriere, params);
    pthread_join(cameriere_t, NULL);
    return 0;
}