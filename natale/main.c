#include "misc.h"

typedef struct
{
    Child selected_child;
    sem_t waiting_ElfoSmistatore;
    sem_t waiting_BabboNatale;
    sem_t waiting_ElfoProduttore;
    int exit_condition;
} SharedParams;

void *ElfoProduttore(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    // numero lettere ricevute, numero bambini buoni, numero bambini cattivi, costo totale di produzione
    int letters_number = 0;
    int good_children = 0;
    int bad_children = 0;
    double total_production_cost = 0;

    while (1)
    {
        // Aspetta di essere chiamata da Babbo Natale
        printf("Elfo Produttore: Aspetta Babbo Natale!\n");
        if (params->exit_condition == 1)
            break;
        sem_wait(&(params->waiting_BabboNatale));
        // Aggiorna le statistiche
        printf("Elfo Produttore: Sto producendo\n");
        letters_number++;
        total_production_cost += params->selected_child.price;
        if (strcmp(params->selected_child.type, "good"))
            good_children++;
        else
            bad_children++;
        // Avverte Elfo Smistatore che C( pronto
        printf("Elfo Produttore: Elfo Smistatore vai!\n");
        sem_post(&(params->waiting_ElfoProduttore));
    }

    printf("Numero lettere: %d, Bambini buoni: %d, Bambini cattivi: %d, Costo totale produzione: %f \n", letters_number, good_children, bad_children, total_production_cost);

    pthread_exit(0);
}

void *BabboNatale(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    // Crea elfo produttore
    pthread_t elfo_produttore_t;
    pthread_create(&(elfo_produttore_t), NULL, ElfoProduttore, params);

    while (1)
    {

        // Aspetta Elfo Smistatore
        printf("Babbo Natale: Aspetta Elfo Smistatore!\n");
        if (params->exit_condition == 1)
        {
            sem_post(&(params->waiting_BabboNatale));
            break;
        }
        sem_wait(&(params->waiting_ElfoSmistatore));
        // Avverte Elfo Produttore
        printf("Babbo Natale: Sveglia Elfo Produttore!\n");
        sem_post(&(params->waiting_BabboNatale));
    }

    pthread_join(elfo_produttore_t, NULL);
    pthread_exit(0);
}

void *ElfoSmistatore(void *arg)
{
    SharedParams *params;
    params = (SharedParams *)arg;

    // Si conserva in memoria la lista di tutti i bambini
    Child children_list[6];
    read_from_file("letters_file.txt", children_list, 0);
    read_from_file("good_bad_file.txt", children_list, 1);
    read_from_file("prices_file.txt", children_list, 2);
    read_from_file("prices_file.txt", children_list, 3);

    // Chiamo Babbo Natale
    pthread_t babbo_natale_t;
    pthread_create(&(babbo_natale_t), NULL, BabboNatale, params);

    printf("Elfo smistatore: Ho letto tutti i bimbi\n");

    for (int i = 0; i < 6; i++)
    {
        // Seleziona un bambino dalla lista
        params->selected_child = children_list[i];
        printf("Elfo smistatore: Ho selezionato %s\n", params->selected_child.name);
        printf("Elfo smistatore: Sveglia Babbo Natale!\n");
        // Sveglia Babbo Natale da parte di Elfo Smistatore
        sem_post(&(params->waiting_ElfoSmistatore));
        printf("Elfo smistatore: Aspetta Elfo Produttore!\n");
        // Aspetta che Elfo Produttore finisce
        sem_wait(&(params->waiting_ElfoProduttore));
    }

    params->exit_condition = 1;
    sem_post(&(params->waiting_ElfoSmistatore));

    pthread_join(babbo_natale_t, NULL);

    pthread_exit(0);
}

int main(void)
{

    SharedParams *params;
    params = malloc(sizeof(SharedParams));

    params->exit_condition = 0;

    sem_init(&(params->waiting_ElfoSmistatore), 0, 0);
    sem_init(&(params->waiting_ElfoProduttore), 0, 0);
    sem_init(&(params->waiting_BabboNatale), 0, 0);

    pthread_t elfo_smistatore_t;
    pthread_create(&(elfo_smistatore_t), NULL, ElfoSmistatore, params);
    pthread_join(elfo_smistatore_t, NULL);

    return 0;
}
