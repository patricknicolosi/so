#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "pthread.h"
#include "semaphore.h"

#define BUFFER_SIZE 4096

char *substring(char *string, int start, int end)
{
    char *substring = malloc((strlen(string) - end + start) + 1);
    int substring_index = 0;
    for (int i = start; i < end + 1; i++)
    {
        substring[substring_index] = string[i];
        substring_index++;
    }
    substring[substring_index] = '\0'; // aggiunto terminatore
    return substring;
}

typedef enum
{
    ADD,
    SUB,
    MUL,
    CALC
} ThreadType;

typedef struct
{
    long long operand_one;
    long long operand_two;
    char operation;
    long long res;
    sem_t semaphores[4];
    bool done;
} SharedParams;

void init_shared_params(SharedParams *params)
{
    sem_init(&(params->semaphores[CALC]), 0, 1);
    sem_init(&(params->semaphores[ADD]), 0, 0);
    sem_init(&(params->semaphores[SUB]), 0, 0);
    sem_init(&(params->semaphores[MUL]), 0, 0);
    params->done = false;
}

void add(void *arg)
{
    SharedParams *params = (SharedParams *)arg;

    while (true)
    {
        sem_wait(&(params->semaphores[ADD]));
        if (params->done)
            break;
        params->res = params->operand_one + params->operand_two;
        printf("[ADD] %lld + %lld = %lld\n", params->operand_one, params->operand_two, params->res);
        params->operand_one = params->res;
        sem_post(&(params->semaphores[CALC]));
    }

    pthread_exit(0);
}

void sub(void *arg)
{
    SharedParams *params = (SharedParams *)arg;

    while (true)
    {
        sem_wait(&(params->semaphores[SUB]));
        if (params->done)
            break;
        params->res = params->operand_one - params->operand_two;
        printf("[SUB] %lld - %lld = %lld\n", params->operand_one, params->operand_two, params->res);
        params->operand_one = params->res;
        sem_post(&(params->semaphores[CALC]));
    }

    pthread_exit(0);
}

void mul(void *arg)
{
    SharedParams *params = (SharedParams *)arg;

    while (true)
    {
        sem_wait(&(params->semaphores[MUL]));
        if (params->done)
            break;
        params->res = params->operand_one * params->operand_two;
        printf("[MUL] %lld x %lld = %lld\n", params->operand_one, params->operand_two, params->res);
        params->operand_one = params->res;
        sem_post(&(params->semaphores[CALC]));
    }

    pthread_exit(0);
}

void calc(void *arg)
{
    SharedParams *params = (SharedParams *)arg;

    FILE *file = fopen("calc1.txt", "r");
    char line[BUFFER_SIZE];

    int lines_number = 0;
    while (fgets(line, BUFFER_SIZE, file))
        lines_number++;

    printf("[CALC] Sono presenti %d righe nel file\n", lines_number);

    fseek(file, 0, SEEK_SET);

    char lines[lines_number][BUFFER_SIZE / 2];
    for (int i = 0; i < lines_number; i++)
    {
        fgets(line, BUFFER_SIZE, file);
        line[strcspn(line, "\n")] = 0;
        strcpy(lines[i], line);
    }

    long long file_result = atoll(lines[lines_number - 1]);
    long long operands[lines_number - 2];
    char operations[lines_number - 2];

    int j = 1;
    for (int i = 0; i < lines_number - 2; i++)
    {
        operands[i] = atoll(substring(lines[j], 2, strlen(lines[j]) - 1));
        operations[i] = lines[j][0];
        printf("[CALC] Operazione letta: %c %lld\n", operations[i], operands[i]);
        j++;
    }

    params->operand_one = atoll(lines[0]);
    printf("[CALC] Primo operando: %lld\n", params->operand_one);

    for (int i = 0; i < lines_number - 2; i++)
    {
        sem_wait(&(params->semaphores[CALC]));

        params->operand_two = operands[i];
        params->operation = operations[i];

        if (operations[i] == '+')
        {
            sem_post(&(params->semaphores[ADD]));
        }
        else if (operations[i] == '-')
        {
            sem_post(&(params->semaphores[SUB]));
        }
        else if (operations[i] == 'x')
        {
            sem_post(&(params->semaphores[MUL]));
        }
    }

    params->done = true;
    sem_post(&(params->semaphores[ADD]));
    sem_post(&(params->semaphores[SUB]));
    sem_post(&(params->semaphores[MUL]));

    printf("[CALC] Risultato calcolato: %lld, Risultato file: %lld\n", params->res, file_result);
    if (params->res == file_result)
    {
        printf("[CALC] Il risultato corrisponde\n");
    }

    pthread_exit(0);
}

int main(int arg, char **argv)
{
    SharedParams *shared_params = malloc(sizeof(SharedParams));
    init_shared_params(shared_params);

    pthread_t threads[4];

    pthread_create(&threads[CALC], NULL, calc, shared_params);
    pthread_create(&threads[ADD], NULL, add, shared_params);
    pthread_create(&threads[SUB], NULL, sub, shared_params);
    pthread_create(&threads[MUL], NULL, mul, shared_params);

    for (int i = 0; i < 4; i++)
        pthread_join(threads[i], NULL);

    return 0;
}
