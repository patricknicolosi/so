
#include "stdio.h"
#include "stdlib.h"
#include "semaphore.h"
#include "pthread.h"
#include "string.h"
#include "unistd.h"

typedef struct
{
    char nome[100];
    char pizza[100];
    char indirizzo[300];
} Ordine;

typedef struct
{
    char pizza[100];
    int tempo;
} Tempo;

int tempo_corrispondente(char *pizza, Tempo *tempo)
{
    for (int i = 0; i < 20; i++)
    {
        if (strcmp(tempo[i].pizza, pizza))
            return tempo[i].tempo;
    }
    return -1;
}

void read_ordini_from_file(Ordine *ordini)
{
    FILE *file;
    file = fopen("ordini.txt", "r");

    char line[BUFSIZ];

    int ordini_index = 0;
    while (fgets(line, BUFSIZ, file))
    {
        char *token = strtok(line, ",");
        int token_index = 0;
        char splitted_string[3][300];
        while (token != NULL)
        {
            strcpy(splitted_string[token_index], token);
            token_index++;
            token = strtok(NULL, ",");
        }

        splitted_string[2][strlen(splitted_string[2]) - 1] = '\0';

        strcpy(ordini[ordini_index].nome, splitted_string[0]);
        strcpy(ordini[ordini_index].pizza, splitted_string[1]);
        strcpy(ordini[ordini_index].indirizzo, splitted_string[2]);

        ordini_index++;
    }

    return;
}

void read_tempi_from_file(Tempo *tempi)
{
    FILE *file;
    file = fopen("tempi_preparazione.txt", "r");

    char line[BUFSIZ];

    int tempi_index = 0;
    while (fgets(line, BUFSIZ, file))
    {
        char *token = strtok(line, ",");
        int token_index = 0;
        char splitted_string[3][300];
        while (token != NULL)
        {
            strcpy(splitted_string[token_index], token);
            token_index++;
            token = strtok(NULL, ",");
        }

        splitted_string[1][strlen(splitted_string[2]) - 1] = '\0';

        strcpy(tempi[tempi_index].pizza, splitted_string[0]);
        tempi[tempi_index].tempo = atoi(splitted_string[1]);

        tempi_index++;
    }

    return;
}
