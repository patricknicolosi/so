#include "string.h"
#include "stdio.h"
#include "stdlib.h"

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