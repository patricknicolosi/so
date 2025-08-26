/*

Ho una banca con 3 conti correnti. Ogni conto è cosi strutturato:
- saldo
- ID

Ho 5 clienti. Ogni cliente è così strutturato:
- ID
- ID_conto

Ogni cliente può fare delle operazioni di versamento o prelievo solo se
l'ID_conto corrisponde.

Ogni cliente fa 20 operazioni random tra versamenti o prelievi di una cifra random.
Ogni prelievo può essere possibile solo se si ha disponibilità nel conto.

*/

#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"
#include "string.h"
#include "fcntl.h"

#define users_n 5
#define bank_accounts_n 3
#define action_per_user 20

typedef struct
{
    int ID, bank_account_ID;
} User;

typedef struct
{
    int ID;
    double balance;
} BankAccount;

typedef struct
{
    BankAccount accounts[bank_accounts_n];
    User users[users_n];
    pthread_mutex_t mutex;
} thread_params;

thread_params params = {
    .users = {
        {0, 0},
        {1, 1},
        {2, 1},
        {3, 0},
        {4, 2},
    },
    .accounts = {
        {0, 20.0},
        {1, 1000.0},
        {2, 0.0},
    },
    .mutex = PTHREAD_MUTEX_INITIALIZER,
};

void *thread(void *arg)
{
    thread_params *params;
    params = (thread_params *)arg;

    int account_founded = -1;
    int user_ID = rand() % 5;

    for (int i = 0; i < bank_accounts_n; i++)
        if (params->accounts[i].ID == params->users[user_ID].bank_account_ID)
            account_founded = i;

    if (account_founded == -1)
        pthread_exit(0);

    pthread_mutex_lock(&(params->mutex));
    printf("Utente %d su conto %d saldo iniziale: %.2f\n",
           user_ID, params->accounts[account_founded].ID,
           params->accounts[account_founded].balance);

    for (int i = 0; i < action_per_user; i++)
    {
        int amount = rand() % 100;
        int action = rand() % 3;
        if (action > 1)
        {
            params->accounts[account_founded].balance += amount;
            printf("Utente %d -> VERSAMENTO: +%d (saldo: %.2f)\n",
                   user_ID, amount, params->accounts[account_founded].balance);
        }
        else
        {
            if (params->accounts[account_founded].balance - amount < 0)
            {
                printf("Utente %d -> ERRORE prelievo -%d (saldo: %.2f)\n",
                       user_ID, amount, params->accounts[account_founded].balance);
            }
            else
            {
                params->accounts[account_founded].balance -= amount;
                printf("Utente %d -> PRELIEVO: -%d (saldo: %.2f)\n",
                       user_ID, amount, params->accounts[account_founded].balance);
            }
        }
        pthread_mutex_unlock(&(params->mutex));
    }
    pthread_exit(0);
}

int main(void)
{

    pthread_t users[users_n];
    for (int i = 0; i < users_n; i++)
        pthread_create(&users[i], NULL, thread, &params);
    for (int i = 0; i < users_n; i++)
        pthread_join(users[i], NULL);

    return 0;
}