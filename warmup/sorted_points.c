/*
ECE344 lab1 - Basic Data Structures: Linked List   Author: Mingqi Hou 999767676
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "point.h"
#include "sorted_points.h"
#include <math.h>
#include <stdbool.h>


// Struct sorted_points to be the node of list and hold the list
// Initialized sp is a header node hold only the location of the read head
// Location of the read head is held in sp->next node
struct sorted_points
{
	double x;
	double y;
	double length;
	bool header_node;
	struct sorted_points *next_node;
};
typedef struct sorted_points sorted_points;

// This function initialize a new list, header node that hold the location
// of the read head is initialized
// next node = NULL --> empty list
// To use this fucntion to initailze a node, manually set header_node to false!
struct sorted_points *sp_init()
{
	struct sorted_points *sp;

	sp = (struct sorted_points *)malloc(sizeof(sorted_points));
	assert(sp);

    	sp->x = 0;
   	sp->y = 0;
	sp->length = 0;
	sp->header_node = true;
	sp->next_node = NULL;

	return sp;
}

void sp_destroy(struct sorted_points *sp)
{
	while(sp!= NULL)
	{
        sorted_points *to_be_destroied = sp;
        sp = sp->next_node;
        free(to_be_destroied);
	}
}

int sp_add_point(struct sorted_points *sp, double x, double y)
{
    // Allocate the new node to be inserted
    sorted_points *new_node = sp_init();
    if(!new_node)
        return 0;
    new_node->x = x;
    new_node->y = y;
    new_node->header_node = false;
    new_node->next_node = NULL;
    // Get the lenght of new pint
    int point_length = sqrt(pow(x, 2)+pow(y, 2));
	new_node->length = point_length;

    // Insert at empty list
    if(sp->header_node == true && sp->next_node == NULL)
    {
        sp->next_node = new_node;
        return 1;
    }
    else
    {
        sorted_points * current_node = sp->next_node; //sp->next_node is the real head of the list
        sorted_points * previous_node = sp; // sp is the one before read head
        while(current_node != NULL)
        {
            if (current_node->length < point_length) // go next
            {
                previous_node = current_node;
                current_node = current_node->next_node;
            }
            // Same length, need further discussion
            else if (current_node->length == point_length)
            {
                // New node is smaller in x, insert before node
                if(x < current_node->x)
                {
                    new_node->next_node = current_node;
                    previous_node->next_node = new_node;
                    return 1;
                }
                // Same x, need further discussion
                else if(x == current_node->x)
                {
                    // New node is smaller in y, insert before node
                    // Same nodes, locaiton doens't matter, insert before node
                    if (y <= current_node->y)
                    {
                        new_node->next_node = current_node;
                        previous_node->next_node = new_node;
                        return 1;
                    }
                    // New node is larger in y, go next node
                    else
                    {
                        previous_node = current_node;
                        current_node = current_node->next_node;
                    }
                }
                // Larger x, go next node
                else
                {
                    previous_node = current_node;
                    current_node = current_node->next_node;
                }
            }
            // new node is shorted than node, insert before node
            else
            {
                new_node->next_node = current_node;
                previous_node->next_node = new_node;
                return 1;
            }
        }
        //Insert at the end of list
        new_node->next_node = NULL;
        previous_node->next_node = new_node;
        return 1;
    }
    free(new_node);
	return 0;
}

int sp_remove_first(struct sorted_points *sp, struct point *ret)
{
    if(sp->header_node == true && sp->next_node == NULL)
        return 0;
    else
    {
        sorted_points * delete_node = sp->next_node; //real head of the list
        sp->next_node = delete_node->next_node;
        ret = point_set(ret, delete_node->x, delete_node->y);
        free(delete_node);
        return 1;
    }
    return 0;
}

int sp_remove_last(struct sorted_points *sp, struct point *ret)
{
    if(sp->header_node == true && sp->next_node == NULL)
        return 0;
    else
    {
        sorted_points * current_node = sp->next_node; //real head of the list
        sorted_points * previous_node = sp;

        while(current_node->next_node != NULL)
        {
            previous_node = current_node;
            current_node = current_node->next_node;
        }
        ret = point_set(ret, current_node->x, current_node->y);
        previous_node->next_node = NULL;
        free(current_node);
        return 1;
    }
}

int sp_remove_by_index(struct sorted_points *sp, int index, struct point *ret)
{
	if(sp->header_node == true && sp->next_node == NULL)
        return 0;
    else
    {
        sorted_points * current_node = sp->next_node;
        sorted_points * previous_node = sp;
        int count = 0;

        //counting..
        while(current_node->next_node != NULL)
        {
            if (index == count)
            {
                previous_node->next_node = current_node->next_node;
                ret = point_set(ret, current_node->x, current_node->y);
                free(current_node);
                return 1;
            }
            else
            {
                previous_node = current_node;
                current_node = current_node->next_node;
                count++;
            }
        }
        if (index == count && current_node->next_node == NULL)
        {
            previous_node->next_node = NULL;
            ret = point_set(ret, current_node->x, current_node->y);
            free(current_node);
            return 1;
        }
    }
    return 0;
}

int sp_delete_duplicates(struct sorted_points *sp)
{
	int count =0;
	if(sp->header_node == true && sp->next_node == NULL)
        return 0;
    else
    {
        sorted_points * current_node = sp->next_node;
        sorted_points * latter_node = sp->next_node->next_node;
        while(latter_node != NULL)
        {
            if(current_node->length == latter_node->length && current_node->x == latter_node->x && current_node->y == latter_node->y)
            {
                count++;
                current_node->next_node = latter_node->next_node;
                free(latter_node);
                latter_node = current_node->next_node;
            }
            else
            {
                current_node = latter_node;
                latter_node = latter_node->next_node;
            }
        }
        return count;
    }
	return -1;
}


