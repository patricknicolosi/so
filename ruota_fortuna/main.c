/*
- Ci sono n giocatori rappresentati rispettivamente da n thread
- Che fanno m partite
- Il gioco sarà gestito dal thread principale M (Mike)
- Tutti i thread giocatore e Mike condivideranno la struct Tabellone che contiene:
  1. Frase da scoprire
  2. Lettere già chiamate
  3. I punteggi attuali dei giocatori
  4. La lettera scelta dal giocatore attuale
*/

#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "pthread.h"
#include "string.h"
#include "semaphore.h"
#include "ctype.h"

#define NUMERO_GIOCATORI 5

// Ritorna il numero di occorrenze di un carattere e in una stringa string
int string_contains(char *string, char e)
{
    int total = 0;
    for (int i = 0; i < strlen(string); i++)
        if (toupper(string[i]) == toupper(e))
            total++;
    if (total == 0)
        return -1;
    return total;
}

char random_char()
{
    int random_index = rand() % 26;
    char alphabet[25] = "abcdefghijlmnopqrstuwxyz";
    return alphabet[random_index];
}

int max(int *array, int len)
{
    int max = 0;
    for (int i = 0; i < len; i++)
    {
        if (array[i] > max)
            max = array[i];
    }
    return max;
}

typedef struct
{
    char frase_da_scoprire[BUFSIZ];
    int lettere_chiamate[BUFSIZ];
    int punteggi[NUMERO_GIOCATORI];
    char ultima_lettera_chiamata;
} Tabellone;

typedef struct
{
    sem_t aspetto_giocatore;
    sem_t aspetto_mike;
    Tabellone *tabellone;
    int terminazione;
} ParametriCondivisi;

void *Giocatore(void *arg)
{
    ParametriCondivisi *params;
    params = (ParametriCondivisi *)arg;

    while (1)
    {
        sem_wait(&(params->aspetto_mike));
        if (params->terminazione == 1)
            break;
        params->tabellone->ultima_lettera_chiamata = random_char();
        printf("GIOCATORE: Il giocatore sceglie %c\n", params->tabellone->ultima_lettera_chiamata);
        sem_post(&(params->aspetto_giocatore));
    }

    pthread_exit(0);
}

void *Mike(void *arg)
{
    ParametriCondivisi *params;
    params = (ParametriCondivisi *)arg;

    sem_init(&(params->aspetto_giocatore), 0, 0);
    sem_init(&(params->aspetto_mike), 0, 0);

    FILE *file;
    file = fopen("source.txt", "r");

    if (file == NULL)
    {
        printf("Errore: impossibile aprire il file source.txt\n");
        pthread_exit(0);
    }

    char line[BUFSIZ];
    char frasi[BUFSIZ][300];

    // Sposta tutte le frasi del file in un array
    int num_frasi = 0;
    while (fgets(line, BUFSIZ, file))
    {
        line[strcspn(line, "\n")] = '\0';
        strcpy(frasi[num_frasi], line);
        num_frasi++;
    }

    // Vengono scelti i giocatori e comunicato il tabellone
    pthread_t giocatori_t[NUMERO_GIOCATORI];
    for (int i = 0; i < NUMERO_GIOCATORI; i++)
        pthread_create(&giocatori_t[i], NULL, Giocatore, params);

    // Conta il numero di lettere indovinate
    int lettere_indovinate = 0;

    // Sorteggia una frase e la fa "vedere nel tabellone"
    strcpy(params->tabellone->frase_da_scoprire, frasi[rand() % num_frasi]);

    while (1)
    {
        // Mike è pronto
        sem_post(&(params->aspetto_mike));

        // Si interrogano i giocatori uno alla volta
        for (int i = 0; i < NUMERO_GIOCATORI; i++)
        {
            // Verifico se il giocatore ha fatto una proposta
            sem_wait(&(params->aspetto_giocatore));

            // Se la frase contiene la lettera chiamata dal giocatore i
            int check_res = string_contains(params->tabellone->frase_da_scoprire, params->tabellone->ultima_lettera_chiamata);
            if (check_res != -1)
            {
                // Il giocatore i ottiene un punteggio casuale
                params->tabellone->punteggi[i] = params->tabellone->punteggi[i] + (rand() % 300);
                lettere_indovinate += check_res;
                printf("MIKE: La lettera %c chiamata dal giocatore %d FA PARTE della frase %s \n", params->tabellone->ultima_lettera_chiamata, i, params->tabellone->frase_da_scoprire);
                printf("MIKE: Sono state indovinate %d\n lettere ", lettere_indovinate);
            }
            else
            {
                printf("MIKE: La lettera %c chiamata dal giocatore %d NON FA PARTE della frase %s \n", params->tabellone->ultima_lettera_chiamata, i, params->tabellone->frase_da_scoprire);
            }

            // Controlla se ci sono abbastanza lettere indovinate
            if (lettere_indovinate >= strlen(params->tabellone->frase_da_scoprire))
            {
                params->terminazione = 1;

                for (int i = 0; i < NUMERO_GIOCATORI; i++)
                    sem_post(&(params->aspetto_mike));

                for (int i = 0; i < NUMERO_GIOCATORI; i++)
                    pthread_join(giocatori_t[i], NULL);

                printf("Vince il giocatore con punteggio %d", max(params->tabellone->punteggi, NUMERO_GIOCATORI));
                pthread_exit(0);
            }

            sem_post(&(params->aspetto_mike));
        }
    }
}

int main(void)
{

    ParametriCondivisi *params;
    params = malloc(sizeof(ParametriCondivisi));

    params->tabellone = malloc(sizeof(Tabellone));
    memset(params->tabellone, 0, sizeof(Tabellone));

    params->terminazione = 0;

    pthread_t mike_t;
    pthread_create(&mike_t, NULL, Mike, params);
    pthread_join(mike_t, NULL);

    return 0;
}