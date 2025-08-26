#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"

typedef struct
{
    long long op1;
    long long op2;
    long long minor_res_operation;
    char operation;
    sem_t dati_disponibili;
    sem_t CALC_disponibile;
    pthread_mutex_t accesso_struttura_condivisa;
    int terminazione;
} SharedParams;

void *OP1(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    FILE *file;
    file = fopen("A.op1", "r");

    long long first_operands[100];

    char line[BUFSIZ];
    int first_operands_index = 0;
    while (fgets(line, BUFSIZ, file))
    {
        first_operands[first_operands_index] = atoll(line);
        first_operands_index++;
    }

    printf("OP1: Ho letto tutti i primi operandi\n");

    for (int i = 0; i < 100; i++)
    {
        // ASpetta che CALC torni a essere disponibile
        printf("OP1: Aspetto che CALC sia disponibile\n");
        sem_wait(&params->CALC_disponibile);
        pthread_mutex_lock(&(params->accesso_struttura_condivisa));
        params->op1 = first_operands[i];
        pthread_mutex_unlock(&(params->accesso_struttura_condivisa));
        printf("OP1: Dice a CALC che C( disponibile il primo operando\n");
        // Dice a CALC che il primo operando C( a disposizione
        sem_post(&(params->dati_disponibili));
    }

    pthread_exit(0);
}

void *OP2(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    FILE *file;
    file = fopen("A.op2", "r");

    long long second_operands[100];

    char line[BUFSIZ];
    int second_operands_index = 0;
    while (fgets(line, BUFSIZ, file))
    {
        second_operands[second_operands_index] = atoll(line);
        second_operands_index++;
    }

    printf("OP2: Ho letto tutti i secondi operandi\n");

    for (int i = 0; i < 100; i++)
    {
        // ASpetta che CALC torni a essere disponibile
        printf("OP2: Aspetto che CALC sia disponibile\n");
        sem_wait(&params->CALC_disponibile);
        pthread_mutex_lock(&(params->accesso_struttura_condivisa));
        params->op2 = second_operands[i];
        pthread_mutex_unlock(&(params->accesso_struttura_condivisa));
        // Dice a CALC che il primo operando C( a disposizione
        printf("OP2: Dice a CALC che C( disponibile il secondo operando\n");
        sem_post(&(params->dati_disponibili));
    }

    pthread_exit(0);
}

void *OPs(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    FILE *file;
    file = fopen("A.ops", "r");

    char operations[100];
    long long file_res = 0;

    char line[BUFSIZ];
    int operations_index = 0;
    while (fgets(line, BUFSIZ, file))
    {

        operations[operations_index] = line[0];
        operations_index++;
        if (operations_index == 101)
        {
            file_res = atoll(line);
            break;
        }
    }

    printf("OPS: Ho letto tutte le operazioni e risultati\n");

    long long my_res = 0;

    for (int i = 0; i < 100; i++)
    {

        // Aspetto che CALC sia disponibile
        printf("OPS[%d]: Aspetto che CALC sia disponibile\n", i);
        sem_wait(&params->CALC_disponibile);
        // Imposto l'operando
        pthread_mutex_lock(&(params->accesso_struttura_condivisa));
        params->operation = operations[i];
        pthread_mutex_unlock(&(params->accesso_struttura_condivisa));
        // Avviso CALC
        printf("OPS[%d]: Avviso CALC\n", i);
        sem_post(&params->dati_disponibili);
        my_res += params->minor_res_operation;
    }

    printf("OPS: Il risultato che hai calcolato tu è %lld invece quello del file è %lld\n", my_res, file_res);

    params->terminazione = 1;
    sem_post(&params->dati_disponibili);
    sem_post(&params->dati_disponibili);
    sem_post(&params->dati_disponibili);

    pthread_exit(0);
}

void *CALC(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    while (1)
    {
        // Aspetta che siano presenti primo operando, secondo e operazione
        printf("CALC: Aspetto primo operando\n");
        sem_wait(&(params->dati_disponibili));
        printf("CALC: Aspetto secondo operando\n");
        sem_wait(&(params->dati_disponibili));
        printf("CALC: Aspetto operazione\n");
        sem_wait(&(params->dati_disponibili));

        if (params->terminazione == 1)
            break;

        printf("CALC: Faccio operazione \n");
        pthread_mutex_lock(&(params->accesso_struttura_condivisa));
        if (params->operation == '+')
        {
            params->minor_res_operation = (params->op1 + params->op2);
        }
        else if (params->operation == '-')
        {
            params->minor_res_operation = (params->op1 - params->op2);
        }
        else if (params->operation == 'x')
        {
            params->minor_res_operation = (params->op1 * params->op2);
        }
        pthread_mutex_unlock(&(params->accesso_struttura_condivisa));

        printf("CALC: Avviso OP1,OP2,OPs\n");
        sem_post(&(params->CALC_disponibile));
        sem_post(&(params->CALC_disponibile));
        sem_post(&(params->CALC_disponibile));
    }

    pthread_exit(0);
}

int main(void)
{
    SharedParams *params;
    params = malloc(sizeof(SharedParams));

    params->terminazione = 0;
    sem_init(&(params->dati_disponibili), 0, 0);
    sem_init(&(params->CALC_disponibile), 0, 3);
    pthread_mutex_init(&(params->accesso_struttura_condivisa), NULL);

    pthread_t op1_t,
        op2_t, ops_t, calc_t;

    pthread_create(&op1_t, NULL, OP1, params);
    pthread_create(&op2_t, NULL, OP2, params);
    pthread_create(&ops_t, NULL, OPs, params);
    pthread_create(&calc_t, NULL, CALC, params);

    pthread_join(op1_t, NULL);
    pthread_join(op2_t, NULL);
    pthread_join(ops_t, NULL);
    pthread_join(calc_t, NULL);

    return 0;
}