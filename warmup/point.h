#ifndef _POINT_H_
#define _POINT_H_

/* DO NOT CHANGE THIS FILE */

/* simplistic definition of point and operations on a point for 2D
 * double-precision space. */
struct point {
	double x;
	double y;
};

/* See the lab documentation for details regarding these functions. */
void point_translate(struct point *p, double x, double y);
double point_distance(const struct point *p1, const struct point *p2);
/* this function should return -1, 0, or 1, depending on whether p1 is smaller
 * than, equal to, or larger than p2. */
int point_compare(const struct point *p1, const struct point *p2);

static inline double
point_X(const struct point *p)
{
	return p->x;
}

static inline double
point_Y(const struct point *p)
{
	return p->y;
}

static inline struct point *
point_set(struct point *p, double x, double y)
{
	p->x = x;
	p->y = y;
	return p;
}

#endif /* _POINT_H_ */
