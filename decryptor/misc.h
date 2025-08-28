#include "stdio.h"
#include "stdlib.h"
#include "semaphore.h"
#include "pthread.h"
#include "string.h"
#include "ctype.h"

char *substring(char *string, int start, int end)
{
    char *substring;
    substring = malloc((sizeof(char) * 100));
    int j = 0;
    for (int i = start; i < end; i++)
    {
        substring[j] = string[i];
        j++;
    }
    return substring;
}

typedef struct
{
    int type;
    char value[100];
} Sentence;

int index_in_alphabet(char *alphabet, char character)
{
    for (int i = 0; i < strlen(alphabet); i++)
    {
        if (tolower(alphabet[i]) == tolower(character))
            return i;
    }
    return -1;
}

char *decrypt(char *encrypted_string, char *crypted_alphabet)
{
    char alphabet[27] = "abcdefghijklmnopqrstuvwxyz";
    size_t len = strlen(encrypted_string);

    char *decrypted_string = malloc(len + 1);
    if (!decrypted_string)
    {
        exit(1);
    }

    for (size_t i = 0; i < len; i++)
    {
        int pos = index_in_alphabet(crypted_alphabet, encrypted_string[i]);
        if (pos == -1)
        {
            decrypted_string[i] = encrypted_string[i];
        }
        else
        {
            decrypted_string[i] = alphabet[pos];
        }
    }
    decrypted_string[len] = '\0';
    return decrypted_string;
}