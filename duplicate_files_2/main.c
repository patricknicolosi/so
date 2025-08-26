// compito_2025-04-17.pdf
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "pthread.h"
#include "semaphore.h"

/*
- n thread READER che leggeranno il file e lo passeranno all'unico thread WRITER
- Ogni READER invierà blocchi del file grandi 1KB (1024)
- Tutti i thread avranno uno stack condiviso fatto da 10 elementi. Ogni elemento sarà cosi formato:
  1. il blocco da 1024 byte
  2. il nome del file
  .. altri flag eventuali

*/

typedef struct
{
    char e[BUFSIZ];
    char filename[500];
    int occupato;
    sem_t aspetto_writer;
    sem_t aspetto_reader;
} StackElement;

int find_first_empty_position(StackElement *stack)
{
    for (int i = 0; i < 10; i++)
    {
        if (stack[i].occupato == 0)
            return i;
    }
    return -1;
}

int find_first_full_position(StackElement *stack)
{
    for (int i = 0; i < 10; i++)
    {
        if (stack[i].occupato == 1)
            return i;
    }
    return -1;
}

typedef struct
{
    sem_t posizioni_libere, posizioni_occupate;
    StackElement buffer[10];
    char filename[500];
} SharedParams;

void *READER(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    // Prende la prima posizione vuota del buffer
    int index_res = find_first_empty_position(params->buffer);

    strcpy(params->buffer[index_res].filename, params->filename);
    sem_init(&(params->buffer[index_res].aspetto_reader), 0, 0);
    sem_init(&(params->buffer[index_res].aspetto_writer), 0, 1);

    // Apre il file
    FILE *file;
    file = fopen(params->buffer[index_res].filename, "r");

    char line[BUFSIZ];

    // Dice che questa cella è occupata già da un Reader
    params->buffer[index_res].occupato = 1;

    while (fgets(line, BUFSIZ, file))
    {
        // Aspetta se il writer sta leggendo proprio questo pezzo
        sem_wait(&(params->buffer[index_res]).aspetto_writer);
        // Copia il pezzo di testo dal sorgente al buffer
        strcpy(params->buffer[index_res].e, line);
        // Segnala al Writer che è presente un pezzo da copiare
        sem_post(&(params->buffer[index_res]).aspetto_reader);
    }
    params->buffer[index_res].occupato = 0;

    pthread_exit(0);
}

void *WRITER(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    while (1)
    {
        // Trova la prima posizione che è occupata
        int search_index = find_first_full_position(params->buffer);
        if (search_index == -1)
            continue;

        // Attende il reader
        sem_wait(&(params->buffer[search_index]).aspetto_reader);

        // Scrive il file e lo chiude
        FILE *file;
        file = fopen(params->buffer[search_index].filename, "a");
        fwrite(params->buffer[search_index].e, sizeof(char), strlen(params->buffer[search_index].e), file);
        fclose(file);

        // Svuota il buffer intermedio
        params->buffer[search_index].e[0] = '\0';
        params->buffer[search_index].occupato = 0;

        // In qualche modo avvisa il READER corrispondente che può continuare
        sem_post(&(params->buffer[search_index]).aspetto_writer);
    }

    pthread_exit(0);
}

int main(void)
{

    SharedParams *params;
    params = malloc(sizeof(SharedParams));

    strcpy(params->filename, "1.txt");

    pthread_t reader, writer;
    pthread_create(&reader, NULL, READER, params);
    pthread_create(&writer, NULL, WRITER, params);

    pthread_join(reader, NULL);
    pthread_join(writer, NULL);

    exit(EXIT_SUCCESS);
}