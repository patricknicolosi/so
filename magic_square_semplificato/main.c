#include "matrix_utils.h"
#include "string.h"
#include "pthread.h"
#include "semaphore.h"

typedef struct
{
    int empty_index_process_queue;
    int process_queue[10][9];

    int empty_index_final_queue;
    int final_queue[3][9];

    FILE *file;

    pthread_mutex_t reader_mutex;
    pthread_mutex_t verifier_mutex;

} shared_params;

void *reader(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    char line[BUFSIZ];

    pthread_mutex_lock(&(params->reader_mutex));

    fgets(line, BUFSIZ, params->file);

    char *token = strtok(line, ",");
    int square[9];
    int i = 0;

    while (token != NULL && i < 9)
    {
        square[i] = atoi(token);
        token = strtok(NULL, ",");
        i++;
    }

    memcpy(params->process_queue[params->empty_index_process_queue], square, sizeof(int) * 9);
    print_square(params->process_queue[params->empty_index_process_queue]);
    params->empty_index_process_queue++;

    pthread_mutex_unlock(&(params->reader_mutex));
    pthread_exit(0);
}

void *verifier(void *arg)
{
    shared_params *params;
    params = (shared_params *)arg;

    pthread_mutex_lock(&(params->verifier_mutex));
    int res = is_magic_square(params->process_queue[params->empty_index_process_queue - 1]);
    if (res == 1)
    {
        memcpy(params->final_queue[params->empty_index_final_queue], params->process_queue[params->empty_index_final_queue], sizeof(int) * 9);
        params->empty_index_final_queue++;
    }
    pthread_mutex_unlock(&(params->verifier_mutex));

    pthread_exit(0);
}

int main(void)
{

    shared_params *params;
    params = malloc(sizeof(shared_params));

    params->empty_index_process_queue = 0;
    params->empty_index_final_queue = 0;
    params->file = fopen("source.txt", "r");
    pthread_mutex_init(&(params->reader_mutex), NULL);
    pthread_mutex_init(&(params->verifier_mutex), NULL);

    pthread_t readers[10];
    for (int i = 0; i < 10; i++)
        pthread_create(&(readers[i]), NULL, reader, params);
    for (int i = 0; i < 10; i++)
        pthread_join(readers[i], NULL);

    pthread_t verifiers[3];
    for (int i = 0; i < 3; i++)
        pthread_create(&(verifiers[i]), NULL, verifier, params);
    for (int i = 0; i < 3; i++)
        pthread_join(verifiers[i], NULL);

    fclose(params->file);

    printf("--------\nI quadrati finali trovati sono:\n");

    print_square(params->final_queue[0]);
    print_square(params->final_queue[1]);
    print_square(params->final_queue[2]);

    return 0;
}