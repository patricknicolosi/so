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
    char decrypted_string[100];

    for (int i = 0; i < strlen(encrypted_string); i++)
    {
        if (index_in_alphabet(crypted_alphabet, encrypted_string[i]) == -1)
        {
            decrypted_string[i] = encrypted_string[i];
        }
        else
        {
            int pos = index_in_alphabet(crypted_alphabet, encrypted_string[i]);
            decrypted_string[i] = alphabet[pos];
        }
    }
    return decrypted_string;
}