#include "stdio.h"
#include "stdlib.h"
#include "pthread.h"
#include "semaphore.h"
#include "string.h"

#define BAMBINO_FIELD_LEN 100

typedef struct
{
    char name[BAMBINO_FIELD_LEN];
    char type[BAMBINO_FIELD_LEN];
    char gift[BAMBINO_FIELD_LEN];
    double price;
} Child;

void read_from_file(char *filename, Child *child_list, int child_field_index)
{
    FILE *file;
    file = fopen(filename, "r");
    char line[BUFSIZ];
    int i = 0;
    while (fgets(line, BUFSIZ, file))
    {
        int token_index = 0;
        char *token = strtok(line, ";");
        char splitted_string[2][BAMBINO_FIELD_LEN];
        int splitted_index = 0;
        do
        {
            strcpy(splitted_string[splitted_index], token);
            token = strtok(NULL, ";");
            splitted_index++;
        } while (token != NULL);

        splitted_string[1][strcspn(splitted_string[1], "\n")] = '\0';

        if (child_field_index == 0)
            strcpy(child_list[i].name, splitted_string[0]);
        else if (child_field_index == 1)
            strcpy(child_list[i].type, splitted_string[1]);
        else if (child_field_index == 2)
            strcpy(child_list[i].gift, splitted_string[1]);
        else if (child_field_index == 3)
        {
            strcpy(child_list[i].gift, splitted_string[0]);
            child_list[i].price = atof(splitted_string[1]);
        }
        i++;
    }
    fclose(file);
}
