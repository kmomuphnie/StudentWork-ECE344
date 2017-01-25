/*
ECE344 lab1 - Loops		Author: Mingqi Hou 999767676
*/
#include "common.h"
#include <stdio.h>


int
main(int argc, char *argv[])
{
    // argc is numbe of elements incluing ./words
    // char*argv[] is a double pointer recoding all elements as strings.
    // char*argv[0] is ./words
	int i;
	for(i=1; i<argc;  i++)
	{
        printf("%s\n", argv[i]);
	}
	return 0;
}
