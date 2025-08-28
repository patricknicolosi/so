/*
-Il thread principale si occupa di leggere ogni riga del file.
-Per ogni riga del file verrC  creato un thread per decifare
-Il thread principale scriverC  su una struttura dati che condividerC  con
il rispettivo thread che decifra.
-Ogni volta che il thread decifratore avrC  fatto scriverC  nel buffer di prima
 la frase in chiaro
 - Il thread principale scriverC  la frase in chiaro e passera a creare il nuovo thread per la nuova frase

*/

#include "misc.h"

typedef struct
{
    Sentence sentence;
    sem_t presente_frase_da_decifrare;
    sem_t presente_frase_decifrata;
} SharedParams;

void *Descryptor(void *arg)
{
    SharedParams *params = (SharedParams *)arg;

    FILE *file = fopen("keys.txt", "r");

    // Memorizza gli alfabeti
    char line[200];
    char encrypted_alphabets[3][27];
    int encrypted_alphabets_index = 0;
    while (fgets(line, 200, file))
    {
        remove_spaces(line);
        strcpy(encrypted_alphabets[encrypted_alphabets_index], line);
        encrypted_alphabets_index++;
    }

    printf("DECRYPTOR[%d]: resto in attesa della frase da decifrare\n", params->sentence.type);
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

    printf("DECRYPTOR[%d]: Dico a MAIN che c'C( una frase decifrata\n", params->sentence.type);
    sem_post(&(params->presente_frase_decifrata));

    pthread_exit(0);
}

int main(void)
{
    FILE *file;
    file = fopen("ciphertext.txt", "r");

    if (!file)
    {
        perror("Errore durante apertura ciphertext.txt !\n");
        return -1;
    }

    Sentence sentences[57];

    // Legge tutte le frasi criptate da file
    char line[100];
    int sentences_index = 0;
    while (fgets(line, 100, file) && sentences_index < 57)
    {
        remove_spaces(line);
        char *string_sub = substring(line, 2, (int)strlen(line));
        char *type_sub = substring(line, 0, 1);

        // Carica sul tipo Sentence
        strcpy(sentences[sentences_index].value, string_sub);
        sentences[sentences_index].type = atoi(type_sub);
        sentences_index++;
    }

    fclose(file);

    // Definisco i parametri
    SharedParams *params = malloc(sizeof(SharedParams));

    if (!params)
    {
        perror("malloc err\n");
        return -1;
    }

    sem_init(&(params->presente_frase_da_decifrare), 0, 0);
    sem_init(&(params->presente_frase_decifrata), 0, 0);

    // Devo passare le varie frasi ai thread
    for (int i = 0; i < 57; i++)
    {
        // Passo la frase al decryptor
        params->sentence.type = sentences[i].type;
        strcpy(params->sentence.value, sentences[i].value);

        // Dichiaro il thread
        pthread_t t;

        // Creo i thread
        pthread_create(&t, NULL, Descryptor, params);

        // Passo al thread la frase da decifrase
        sem_post(&(params->presente_frase_da_decifrare));
        printf("MAIN[%d]: dico che C( presente una frase da decifrare\n", sentences[i].type);

        // Aspetto la risposta
        sem_wait(&(params->presente_frase_decifrata));
        printf("MAIN[%d]: frase decifrata = %s\n", sentences[i].type, params->sentence.value);

        printf("MAIN: Sono arrivato a servire la frase numero: %d\n", i);

        pthread_join(t, NULL);
    }

    return 0;
}