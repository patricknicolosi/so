#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include "pthread.h"
#include "string.h"
#include "semaphore.h"

#define player_number 5
#define card_number 3

int cinquino(int card[3][5])
{
    int first_row = card[0][0] == -2 &&
                    card[0][1] == -2 &&
                    card[0][2] == -2 &&
                    card[0][3] == -2 &&
                    card[0][4] == -2;
    int second_row = card[1][0] == -2 &&
                     card[1][1] == -2 &&
                     card[1][2] == -2 &&
                     card[1][3] == -2 &&
                     card[1][4] == -2;
    int third_row = card[2][0] == -2 &&
                    card[2][1] == -2 &&
                    card[2][2] == -2 &&
                    card[2][3] == -2 &&
                    card[2][4] == -2;
    return first_row == 1 || second_row == 1 || third_row == 1;
}

int bingo(int card[3][5])
{
    int total = 0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (card[i][j] == -2)
                total++;
        }
    }
    return total == 15;
}

void print_card(int card[3][5])
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            if (card[i][j] == -2)
                printf(" X ");
            else
                printf("%2d ", card[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

typedef struct
{
    int last_called_number;
    int is_dealer_end;
    sem_t players_ready;
    sem_t dealer_ready;
} shared_params;

typedef struct
{
    int id;
    int cards[card_number][3][5];
    shared_params *shared_params;
} player_params;

void *Player(void *arg)
{
    player_params *params;
    params = (player_params *)arg;

    while (1)
    {
        sem_wait(&(params->shared_params->dealer_ready));

        if (params->shared_params->is_dealer_end == 1)
            break;

        // Chiudo le caselle ai valori corrispondenti
        for (int i = 0; i < card_number; i++)
            for (int j = 0; j < 3; j++)
                for (int t = 0; t < 5; t++)
                    if ((params->cards)[i][j][t] == params->shared_params->last_called_number)
                        (params->cards)[i][j][t] = -2;

        sem_post(&(params->shared_params->players_ready));
    }

    for (int i = 0; i < card_number; i++)
    {
        if (cinquino(params->cards[i]))
            printf("Il giocatore %d ha fatto CINQUINO sulla scheda %d\n", params->id, i + 1);
        if (bingo(params->cards[i]))
            printf("Il giocatore %d ha fatto BINGO sulla scheda %d\n", params->id, i + 1);
    }

    pthread_exit(0);
}

void *Dealer(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    pthread_t players[player_number];

    // Creo i player
    for (int i = 0; i < player_number; i++)
    {
        player_params *p = malloc(sizeof(player_params));
        p->id = i + 1;

        for (int c = 0; c < card_number; c++)
            for (int r = 0; r < 3; r++)
                for (int t = 0; t < 5; t++)
                    p->cards[c][r][t] = (rand() % 75) + 1;

        p->shared_params = params;
        pthread_create(&players[i], NULL, Player, p);
    }

    // Si fanno 75 pescate
    for (int i = 0; i < 75; i++)
    {
        // Pesco un numero
        params->last_called_number = (rand() % 75) + 1;

        printf("%d) è uscito %d\n", i + 1, params->last_called_number);

        // Dico a tutti i player che il dealer è pronto a chiamare un nuovo numero
        for (int i = 0; i < player_number; i++)
            sem_post(&(params->dealer_ready));

        // Aspetto che tutti sono pronti
        for (int i = 0; i < player_number; i++)
            sem_wait(&(params->players_ready));
    }

    params->is_dealer_end = 1;

    for (int i = 0; i < player_number; i++)
        pthread_join(players[i], NULL);

    pthread_exit(0);
}

int main(void)
{
    srand(time(NULL));

    pthread_t dealer_t;

    shared_params *params;
    params = malloc(sizeof(shared_params));

    params->is_dealer_end = 0;

    sem_init(&(params->players_ready), 0, 0);
    sem_init(&(params->dealer_ready), 0, 0);

    pthread_create(&dealer_t, NULL, Dealer, params);
    pthread_join(dealer_t, NULL);

    return 0;
}