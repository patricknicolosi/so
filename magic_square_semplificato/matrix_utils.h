
#include "stdlib.h"
#include "stdio.h"

int is_magic_square(int square[9])
{
    int result = square[0] + square[1] + square[2];
    return square[0] + square[1] + square[2] == result ||
           square[3] + square[4] + square[5] == result ||
           square[6] + square[7] + square[8] == result ||
           square[0] + square[4] + square[8] == result;
}

void print_square(int square[9])
{
    for (int i = 0; i < 9; i++)
        printf("%d ", square[i]);
    printf("\n");
}
