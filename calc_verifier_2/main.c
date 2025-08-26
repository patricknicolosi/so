#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "string.h"
#include "semaphore.h"

int string_contains(char *string, char character)
{
    for (int i = 0; i < strlen(string); i++)
    {
        if (string[i] == character)
            return i;
    }
    return -1;
}

char *string_sub(char *string, int start, int end)
{
    int len = end - start + 1;
    char *substring = malloc(len + 1);
    int j = 0;
    for (int i = start; i <= end; i++)
    {
        substring[j] = string[i];
        j++;
    }
    substring[j] = '\0';
    return substring;
}

typedef struct
{
    long long operand_1;
    long long operand_2;

    char operation;
    long long temp_result;

    sem_t add_operation;
    sem_t sub_operation;
    sem_t mul_operation;
    sem_t operation_completed;

    pthread_mutex_t computing;

    long long candidate_result;
} shared_params;

void *ADD(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    while (1)
    {
        sem_wait(&(params->add_operation));

        printf("Sto facendo la somma tra %lld e %lld \n", params->operand_1, params->operand_2);

        pthread_mutex_lock(&(params->computing));
        params->temp_result = params->operand_1 + params->operand_2;
        pthread_mutex_unlock(&(params->computing));

        sem_post(&(params->operation_completed));
    }

    pthread_exit(0);
}

void *SUB(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    while (1)
    {
        sem_wait(&(params->sub_operation));

        printf("Sto facendo la sottrazione tra %lld e %lld \n", params->operand_1, params->operand_2);

        pthread_mutex_lock(&(params->computing));
        params->temp_result = params->operand_1 - params->operand_2;
        pthread_mutex_unlock(&(params->computing));

        sem_post(&(params->operation_completed));
    }

    pthread_exit(0);
}

void *MUL(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    while (1)
    {
        sem_wait(&(params->mul_operation));
        printf("Sto facendo il prodotto tra %lld e %lld \n", params->operand_1, params->operand_2);

        pthread_mutex_lock(&(params->computing));
        params->temp_result = params->operand_1 * params->operand_2;
        pthread_mutex_unlock(&(params->computing));

        sem_post(&(params->operation_completed));
    }

    pthread_exit(0);
}

void *CALC(void *arg)
{
    FILE *file;

    file = fopen("source.txt", "r");

    char line[BUFSIZ];

    int numbers[BUFSIZ];
    int numbers_index = 0;

    char operations[BUFSIZ];
    int operations_index = 0;

    int operand_1 = 0;
    long candidate_result = 0;

    int total_operations = 0;

    while (fgets(line, BUFSIZ, file))
    {

        if ((string_contains(line, '+') != -1 ||
             string_contains(line, '-') != -1 ||
             string_contains(line, 'x') != -1) &&
            line[1] == ' ')
        {
            operations[operations_index] = line[0];
            operations_index++;
            total_operations++;

            char *temp;
            temp = string_sub(line, 2, strlen(line) - 2);
            numbers[numbers_index] = atoll(temp);
            numbers_index++;
        }
        else
        {
            if (operand_1 == 0)
                operand_1 = atoll(line);
            else
                candidate_result = atoll(line);
        }
    }

    operations_index = 0;
    numbers_index = 0;

    shared_params *params;
    params = (shared_params *)arg;

    params->operand_1 = operand_1;
    params->operand_2 = numbers[numbers_index];
    params->temp_result = operand_1;

    params->candidate_result = candidate_result;
    params->operation = operations[operations_index];

    while (operations_index != total_operations)
    {
        if (operations[operations_index] == '+')
            sem_post(&(params->add_operation));
        else if (operations[operations_index] == '-')
            sem_post(&(params->sub_operation));
        else if (operations[operations_index] == 'x')
            sem_post(&(params->mul_operation));

        sem_wait(&(params->operation_completed));

        numbers_index++;
        operations_index++;

        params->operand_1 = params->temp_result;
        params->operand_2 = numbers[numbers_index];
    }

    printf("Candidato: %lld, Temporaneo: %lld \n", params->candidate_result, params->temp_result);
    if (params->candidate_result == params->temp_result)
    {
        printf("Risultato corretto \n");
    }
    else
    {
        printf("Risultato non corretto \n");
    }

    pthread_exit(0);
}

int main(void)
{
    shared_params *params;
    params = malloc(sizeof(shared_params));

    pthread_mutex_init(&(params->computing), NULL);

    sem_init(&(params->add_operation), 0, 0);
    sem_init(&(params->sub_operation), 0, 0);
    sem_init(&(params->mul_operation), 0, 0);
    sem_init(&(params->operation_completed), 0, 0);

    pthread_t CALC_t, ADD_t, SUB_t, MUL_t;
    pthread_create(&CALC_t, NULL, CALC, params);
    pthread_create(&ADD_t, NULL, ADD, params);
    pthread_create(&SUB_t, NULL, SUB, params);
    pthread_create(&MUL_t, NULL, MUL, params);

    pthread_join(CALC_t, NULL);
    pthread_join(ADD_t, NULL);
    pthread_join(SUB_t, NULL);
    pthread_join(MUL_t, NULL);

    return 0;
}