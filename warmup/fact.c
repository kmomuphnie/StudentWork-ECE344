/*
ECE344 lab1 - Procedure Calls 	Author: Mingqi Hou 999767676
*/
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool is_digit (char* argv);
int factorial (int factor);

int main(int argc, char *argv[])
{
	if (argc!=2)
        printf("Huh?\n");
    else
    {
        if (!is_digit(argv[1]))
            printf("Huh?\n");
        else
        {
            int input_int = atoi(argv[1]);
            if (input_int>12)
                printf("Overflow\n");
	else if (input_int <1)
		printf("Huh?\n");
            else
            {
                int result = factorial (input_int);
                printf("%d\n", result);
            }
        }
    }
	return 0;
}


bool is_digit (char* input)
{
    int i=0;
    while(input[i] != '\0')
    {
        if(input[i]<48 || input[i]>57)
            return false;
        i++;
    }
    return true;
}

int factorial (int factor)
{
    if(factor<=1)
        return factor;
    else
        return factor*factorial(factor-1);
}
