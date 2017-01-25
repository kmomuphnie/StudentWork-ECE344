/*
ECE344 lab1 - Basic Data Structures: Hash Table   Author: Mingqi Hou 999767676
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "wc.h"
#include <string.h>
#include <stdbool.h>
// use wc as hash element and hash table, use bool hash_element to identtify if wc is element or table.
// wc_init initialze hash table, element variable is set to -1 or NULL.
// set the initial size for hash table to be 2^9 = 512
// real hash table element hash_table != NULL;
// fack header hash_table == NULL
struct wc
{
    int hash_value;
    char *key_word;
    int word_counter;
    struct wc *next_conflict_element;
    bool hash_element;

    int table_size;
    int hash_element_counter;
    struct wc **hash_table;
    struct wc *real_table_element;
};

struct wc * wc_init()
{
	struct wc *wc;
	wc = (struct wc *)malloc(sizeof(struct wc));
	assert(wc);

    wc->hash_element = false;
    wc->word_counter= -1;
    wc->key_word = NULL;
    wc->hash_value = -1;
    wc->next_conflict_element = NULL;

    wc->table_size =-1;
    wc->hash_element_counter=-1;
    wc->hash_table = NULL;
    wc->real_table_element = (struct wc*)malloc(sizeof(struct wc));

    wc->real_table_element->hash_element = false;
    wc->real_table_element->word_counter = -1;
    wc->real_table_element->key_word = NULL;
    wc->real_table_element->hash_value = -1;
    wc->real_table_element->next_conflict_element = NULL;

    wc->real_table_element->table_size = 512;
    wc->real_table_element->hash_element_counter =0;
    wc->real_table_element->hash_table = (struct wc**)malloc(sizeof(struct wc*)*(wc->real_table_element->table_size));
    wc->real_table_element->real_table_element = NULL;

    int i;
    for(i=0; i < wc->real_table_element->table_size; i++)
        wc->real_table_element->hash_table[i] =NULL;
    return wc;
}

int
wc_insert_word(struct wc *wc, char *word)
{
    //Disable rehashing for this lab
    //Rehashing needs further coding work to be functional!
    /*
    int total_element = wc->real_table_element->hash_element_counter;
    int total_size = wc->real_table_element->table_size;
    if (total_element > (0.85*total_size))
    {
        //Need rehash!
        wc->real_table_element->table_size = 2*total_size;
        struct wc ** old_table = wc->real_table_element->hash_table;
        wc->real_table_element->hash_table=(struct wc**)malloc(sizeof(struct wc*)*(wc->real_table_element->table_size));
        if(!wc->real_table_element->hash_table)
            return 0;

        int i;
        for(i=0; i < total_size; i++)
            wc->real_table_element->hash_table[i] = old_table[i];
        for(i=total_size; i< (2*total_size); i++)
            wc->real_table_element->hash_table[i] = NULL;
    }
    */

	// calculate hash value
	// Original Hash Function: djb2 Original Author: dan bernstein
	int hash_value = 2*strlen(word)+1;
	int char_int;
	int char_count;
	for (char_count=0; char_count<strlen(word); char_count++)
	{
        char_int = (int)word[char_count];
        hash_value = hash_value * 33 + char_int;
	}
    hash_value = abs(hash_value % (wc->real_table_element->table_size));
    // allocate and set up new element
    struct wc * new_element = (struct wc*)malloc(sizeof(struct wc));
    if(!new_element)
        return 0;
    new_element->hash_element = true;
    new_element->word_counter = 1;
    new_element->hash_value = hash_value;

    new_element->key_word=(char*) malloc(strlen(word) * sizeof(char)); /* allocate room for the string */
    if(!new_element->key_word)
        return 0;
    strncpy(new_element->key_word, word, strlen(word));

    new_element->next_conflict_element = NULL;

    new_element->hash_element_counter=-1;
    new_element->hash_table=NULL;
    new_element->table_size=-1;
    new_element->real_table_element=NULL;

    // insert
    if(wc->real_table_element->hash_table[hash_value]==NULL) //NO element at this location, direct insert
    {
        wc->real_table_element->hash_table[hash_value] = new_element;
        wc->real_table_element->hash_element_counter++;
        return 1;
    }
    else
    {
        struct wc* current_element = wc->real_table_element->hash_table[hash_value];
        struct wc* previous_element = NULL;
        while (current_element!=NULL)  // go throught linked list at the same hash value
        {
            if(current_element->hash_value == new_element->hash_value && !strcmp(current_element->key_word, new_element->key_word))    //if same word, increment counter and stop
            {
                current_element->word_counter = current_element->word_counter +1;
                free(new_element);
                return 1;
            }
            else                                    // different word keep going
            {
                previous_element = current_element;
                current_element = current_element->next_conflict_element;
            }
        }
        //no same word, add new conflict element to end of the list
        previous_element->next_conflict_element = new_element;
        wc->real_table_element->hash_element_counter++;
        return 1;
    }
	return 0;
}

void
wc_output(struct wc *wc)
{
	int total_size = wc->real_table_element->table_size;
	int i;
	for(i=0; i<total_size; i++)
	{
        if(wc->real_table_element->hash_table[i]!=NULL)
        {
            struct wc* current_element = wc->real_table_element->hash_table[i];
            while(current_element != NULL)
            {
                printf("%s:%d\n", current_element->key_word, current_element->word_counter);
                current_element= current_element->next_conflict_element;
            }
        }
        // else go next hash value
	}
}
