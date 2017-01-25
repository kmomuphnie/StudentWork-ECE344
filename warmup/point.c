/*
ECE344 lab1 - Headers, Linking, Structs 	Author: Mingqi Hou 999767676
*/
#include <assert.h>
#include "common.h"
#include "point.h"
#include <math.h>
#include <stdio.h>
/* update *p by increasing p->x by x and p->y by y */
void
point_translate(struct point *p, double x, double y)
{
	p->x = p->x + x;
	p->y = p->y + y;
}

/* return the distance from p1 to p2 */
double
point_distance(const struct point *p1, const struct point *p2)
{
	double ax, by, distance;
	ax = p1->x - p2->x;
	by = p1->y - p2->y;
	distance = sqrt(pow(ax, 2)+pow(by, 2));
	return distance;
}

int
point_compare(const struct point *p1, const struct point *p2)
{
	double p1_length, p2_length;
	p1_length = sqrt(pow(p1->x, 2)+pow(p1->y, 2));
	p2_length = sqrt(pow(p2->x, 2)+pow(p2->y, 2));
	if ((p1_length - p2_length)== 0)
        return 0;
    else if ((p1_length - p2_length) < 0)
        return -1;
    else
    	return 1;
}

