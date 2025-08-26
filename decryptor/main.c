/*
-Il thread principale si occupa di leggere ogni riga del file.
-Per ogni riga del file verrà creato un thread per decifare
-Il thread principale scriverà su una struttura dati che condividerà con
il rispettivo thread che decifra.
-Ogni volta che il thread decifratore avrà fatto scriverà nel buffer di prima
 la frase in chiaro
 - Il thread principale scriverà la frase in chiaro e passera a creare il nuovo thread per la nuova frase

*/

#include "misc.h"

typedef struct
{
    int terminazione;
    Sentence sentence;
    sem_t presente_frase_da_decifrare;
    sem_t presente_frase_decifrata;
} SharedParams;

void *Descryptor(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    FILE *file;
    file = fopen("keys.txt", "r");

    // Memorizza gli alfabeti
    char line[200];
    char encrypted_alphabets[3][27];
    int encrypted_alphabets_index = 0;
    while (fgets(line, 200, file))
    {
        strcpy(encrypted_alphabets[encrypted_alphabets_index], line);
        encrypted_alphabets_index++;
    }

    while (1)
    {
        sem_wait(&(params->presente_frase_da_decifrare));

        char *encrypt_string;
        if (params->sentence.type == 1)
        {
            encrypt_string = decrypt(params->sentence.value, encrypted_alphabets[0]);
            strcpy(params->sentence.value, encrypt_string);
        }
        else if (params->sentence.type == 2)
        {
            encrypt_string = decrypt(params->sentence.value, encrypted_alphabets[1]);
            strcpy(params->sentence.value, encrypt_string);
        }
        else if (params->sentence.type == 3)
        {
            encrypt_string = decrypt(params->sentence.value, encrypted_alphabets[2]);
            strcpy(params->sentence.value, encrypt_string);
        }

        sem_post(&(params->presente_frase_decifrata));
    }

    pthread_exit(0);
}

int main(void)
{
    FILE *file;
    file = fopen("ciphertext.txt", "r");

    Sentence sentences[57];

    // Legge tutte le frasi criptate da file
    char line[100];
    int sentences_index = 0;
    while (fgets(line, 100, file))
    {
        strcpy(line, substring(line, 0, strlen(line) - 1));
        char *string_sub = substring(line, 2, strlen(line));
        char *type_sub = substring(line, 0, 1);

        // Carica sul tipo Sentence
        strcpy(sentences[sentences_index].value, string_sub);
        sentences[sentences_index].type = atoi(type_sub);
        sentences_index++;
    }

    // Definisco i parametri
    SharedParams *params;
    params = malloc(sizeof(SharedParams));

    params->terminazione = 0;
    sem_init(&(params->presente_frase_da_decifrare), 0, 0);
    sem_init(&(params->presente_frase_decifrata), 0, 1); // Il thread principale puo partire subito

    // Dichiaro i tre thread
    pthread_t t1, t2, t3;

    // Creo i thread
    pthread_create(&t1, NULL, Descryptor, params);
    pthread_create(&t2, NULL, Descryptor, params);
    pthread_create(&t3, NULL, Descryptor, params);

    // Devo passare le varie frasi ai thread
    for (int i = 0; i < 57; i++)
    {
        sem_wait(&(params->presente_frase_decifrata));
        printf("%s \n", params->sentence.value);
        params->sentence = sentences[i];
        sem_post(&(params->presente_frase_da_decifrare));
    }

    params->terminazione = 1;
    sem_post(&(params->presente_frase_da_decifrare));

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}