#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "pthread.h"
#include "semaphore.h"

#define BUFFER_SIZE 4096

char *substring(char *string, int start, int end)
{
    char *substring = malloc(end - start + 2);
    int substring_index = 0;
    for (int i = start; i < end + 1; i++)
    {
        substring[substring_index] = string[i];
        substring_index++;
    }
    substring[substring_index] = '\0';
    return substring;
}

typedef enum
{
    ADD = 0,
    SUB = 1,
    MUL = 2,
} OperationType;

typedef struct
{
    long long operand_one;
    long long operand_two;
    char operation;
    long long res;
    bool done;

    int pid;

    pthread_mutex_t shared_params_mutex;

    sem_t *calc_sem;
    sem_t operations_sem[3];
} SharedParams;

typedef struct
{
    SharedParams *shared_params;
    int pid;
    char *filename;
} CalcParams;

void init_shared_params(SharedParams *params, int calc_thread_n)
{
    sem_init(&(params->operations_sem[ADD]), 0, 0);
    sem_init(&(params->operations_sem[SUB]), 0, 0);
    sem_init(&(params->operations_sem[MUL]), 0, 0);

    params->calc_sem = malloc(sizeof(sem_t) * calc_thread_n);
    for (int i = 0; i < calc_thread_n; i++)
        sem_init(&(params->calc_sem[i]), 0, 0);

    pthread_mutex_init(&params->shared_params_mutex, 0);

    params->done = false;
    params->res = 0;
}

void *add(void *arg)
{
    SharedParams *params = (SharedParams *)arg;

    while (true)
    {
        sem_wait(&(params->operations_sem[ADD]));
        if (params->done)
            break;
        params->res = params->operand_one + params->operand_two;
        printf("[ADD %d] %lld + %lld = %lld\n", params->pid, params->operand_one, params->operand_two, params->res);
        sem_post(&(params->calc_sem[params->pid]));
    }

    pthread_exit(0);
}

void *sub(void *arg)
{
    SharedParams *params = (SharedParams *)arg;

    while (true)
    {
        sem_wait(&(params->operations_sem[SUB]));
        if (params->done)
            break;
        params->res = params->operand_one - params->operand_two;
        printf("[SUB %d] %lld + %lld = %lld\n", params->pid, params->operand_one, params->operand_two, params->res);
        sem_post(&(params->calc_sem[params->pid]));
    }

    pthread_exit(0);
}

void *mul(void *arg)
{
    SharedParams *params = (SharedParams *)arg;

    while (true)
    {
        sem_wait(&(params->operations_sem[MUL]));
        if (params->done)
            break;
        params->res = params->operand_one * params->operand_two;
        printf("[MUL %d] %lld + %lld = %lld\n", params->pid, params->operand_one, params->operand_two, params->res);
        sem_post(&(params->calc_sem[params->pid]));
    }

    pthread_exit(0);
}

void *calc(void *arg)
{
    CalcParams *params = (CalcParams *)arg;

    FILE *file = fopen(params->filename, "r");
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
        j++;
    }

    long long temp_result;
    for (int i = 0; i < lines_number - 2; i++)
    {
        pthread_mutex_lock(&(params->shared_params->shared_params_mutex));
        if (i == 0)
            params->shared_params->operand_one = atoll(lines[0]);
        else
            params->shared_params->operand_one = temp_result;
        params->shared_params->operand_two = operands[i];
        params->shared_params->operation = operations[i];
        params->shared_params->pid = params->pid;

        if (operations[i] == '+')
        {
            sem_post(&(params->shared_params->operations_sem[ADD]));
        }
        else if (operations[i] == '-')
        {
            sem_post(&(params->shared_params->operations_sem[SUB]));
        }
        else if (operations[i] == 'x')
        {
            sem_post(&(params->shared_params->operations_sem[MUL]));
        }
        sem_wait(&(params->shared_params->calc_sem[params->pid]));
        temp_result = params->shared_params->res;
        pthread_mutex_unlock(&(params->shared_params->shared_params_mutex));
    }

    if (temp_result == file_result)
        printf("[CALC %d] Il risultato calcolato %lld corrisponde a %lld \n", params->pid, temp_result, file_result);
    else
        printf("[CALC %d] Il risultato calcolato %lld NON corrisponde a %lld \n", params->pid, temp_result, file_result);

    pthread_exit(0);
}

int main(int arg, char **argv)
{

    int calc_thread_n = 3;

    char file_names[3][10] = {"calc1.txt", "calc2.txt", "calc3.txt"};

    SharedParams *shared_params = malloc(sizeof(SharedParams));
    init_shared_params(shared_params, calc_thread_n);

    pthread_t operations_thread[3];

    pthread_create(&operations_thread[ADD], NULL, add, shared_params);
    pthread_create(&operations_thread[SUB], NULL, sub, shared_params);
    pthread_create(&operations_thread[MUL], NULL, mul, shared_params);

    pthread_t calc_threads[calc_thread_n];

    for (int i = 0; i < calc_thread_n; i++)
    {
        CalcParams *calc_params = malloc(sizeof(CalcParams));
        calc_params->filename = file_names[i];
        calc_params->shared_params = shared_params;
        calc_params->pid = i;
        pthread_create(&calc_threads[i], NULL, calc, calc_params);
    }

    for (int i = 0; i < calc_thread_n; i++)
        pthread_join(calc_threads[i], NULL);

    shared_params->done = true;
    sem_post(&(shared_params->operations_sem[ADD]));
    sem_post(&(shared_params->operations_sem[SUB]));
    sem_post(&(shared_params->operations_sem[MUL]));

    for (int i = 0; i < 3; i++)
        pthread_join(operations_thread[i], NULL);

    return 0;
}
