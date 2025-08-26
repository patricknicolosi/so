#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main(void)
{

    FILE *file;
    file = fopen("source.txt", "r");

    char line[BUFSIZ];
    char file_content[100][BUFSIZ];
    int i = 0;
    while (fgets(line, BUFSIZ, file))
    {
        char *token = strtok(line, ",");
        while (token != NULL)
        {
            strcpy(file_content[i], token);
            if (file_content[i][strlen(file_content[i]) - 1] == '\n')
                file_content[i][strlen(file_content[i]) - 1] = '\0';
            i++;
            token = strtok(NULL, ",");
        }
    }

    for (int j = 0; j < i; j++)
    {
        printf("%s \n", file_content[j]);
    }

    return 0;
}