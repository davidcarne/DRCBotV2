/*
 *  Portions Copyright 2006,2009 David Carne and 2007,2008 Spark Fun Electronics
 *
 *
 *  This file is part of gerberDRC.
 *
 *  gerberDRC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  gerberDRC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <math.h>
#include <assert.h>

#include "gcode_interp.h"
#include "polymath.h"

#include "groupize.h"
#include "partitioning.h"

#include <deque>

// between closed
bool btw_c(double a, double b, double c)
{
	return (a <= b && b <= c) || (c <= b  && b <= a);
}

double xint, yint;

double pointPointDistance(double x, double y, double u, double v)
{
	double a = u-x;
	double b = v-y;
	return sqrt(a*a+b*b);
}

bool point_in_poly(const Point & a, GerbObj_Poly * p);

float naive_poly_comparison(GerbObj_Poly * a, GerbObj_Poly * b)
{
	if (!a->getBounds().intersectsWith(b->getBounds()))
		return INFINITY;
	
	GerbObj_Poly::i_point_list_t it = a->points.begin();
	for (; it!=a->points.end();it++)
		if (point_in_poly(*it, b))
			return 0;
		
	GerbObj_Poly::i_point_list_t itb = b->points.begin();
	for (; itb!=b->points.end();itb++)
		if (point_in_poly(*itb, a))
			return 0;

	return INFINITY;
}


#define DIM     2               /* Dimension of points */
typedef double  tPointd[DIM];   /* type double point */
typedef tPointd * tPolygond;/* type double polygon */
char	InPoly( tPointd q, tPolygond P, int n );

Vector_Outp * tmp;
bool point_in_poly(const Point & a, GerbObj_Poly * p)
{
	if (!p->getBounds().pointInRectClosed(a))
		return false;

	tPolygond p_n = new tPointd[p->points.size()];
	
	GerbObj_Poly::i_point_list_t it = p->points.begin();
	int i=0;
	float lx;
	float ly;
	for (; it!=p->points.end(); it++)
	{
		p_n[i][0] = (*it).x;
		p_n[i][1] = (*it).y;
		i++;
		lx = (*it).x;
		ly = (*it).y;
	}
	
	tPointd c;
	c[0] = a.x;
	c[1] = a.y;
	
	char pointstat = InPoly(c, p_n, p->points.size());
	delete p_n;

	if (pointstat != 'o')
		return true;

	return false;
}

float distanceBetween(GerbObj * a, GerbObj * b)
{
	GerbObj_Poly * p_a = dynamic_cast<GerbObj_Poly*>(a);
	GerbObj_Poly * p_b = dynamic_cast<GerbObj_Poly*>(b);
	
	
	if (p_a && p_b)
		return naive_poly_comparison(p_a , p_b);

	printf("Cannot handle Line comparisons anymore - convert to polys first!\n");
	return INFINITY;
}

GH_vertex * createGHForGerbObj_Poly(GerbObj_Poly * p)
{
	assert(p);

	GH_vertex * poly = NULL, *pp;
	GerbObj_Poly::i_point_list_t it = p->points.begin();

	while (it != p->points.end())
	{	
		if (poly == NULL)
			pp = poly = GH_polyPoint(NULL, (*it).x, (*it).y);
		else
			pp = GH_polyPoint(pp, (*it).x, (*it).y);

		it++;
	}
	return poly;
}

GerbObj_Poly * tryMerge(GerbObj_Poly * a, GerbObj_Poly * b)
{
	GH_vertex * p1 = createGHForGerbObj_Poly(a);
	GH_vertex * p2 = createGHForGerbObj_Poly(b);
	//printf("Finished creating poly objects\n");
	GH_vertex * output = GH_polygon_boolean(p1, p2, GH_op_union);
	//printf("Boolean merge done\n");
	GH_free_polygons(p1);
	GH_free_polygons(p2);
	if (!output)
		return NULL;

	GerbObj_Poly * p = new GerbObj_Poly();
		

	// output iterator
	GH_vertex * oit = output;
	double x,y;
	while (oit = GH_getPolyPoint(oit, &x, &y))
		p->addPoint(Point(x, y));
	p->addPoint(Point(x, y));

	GH_free_polygons(output);
	return p;
}

void groupize(Vector_Outp * data)
{


	/* Algo is this:
	 * merge queue = all
	 * done queue = empty
	 * do
	 * 	current = mergequeue.pop
	 * 	while merge queue not empty
	 * 		item = merge queue.pop
	 * 		m = tryMerge(item, current)
	 * 		if (m)
	 * 			delete item, current
	 * 			move done queue to bottom of merge queue [done queue empty]
	 *			add m to bottom of merge queue
	 *			break
	 *		else
	 *			add item to done queue
	 *	
	 *	if (merge queue is empty)
	 *		add current to all
	 *	merge queue = done queue
	 * while merge queue is not empty
	 */
	std::deque<GerbObj *> merge_queue(data->all.begin(), data->all.end());
	std::deque<GerbObj *> done_queue;

	// Erase the original dataset
	data->all.clear();

	while (merge_queue.size() > 0)
	{
		GerbObj_Poly * current = dynamic_cast<GerbObj_Poly *>(merge_queue.front());
		merge_queue.pop_front();

		while (merge_queue.size() > 0)
		{
			GerbObj_Poly * item = dynamic_cast<GerbObj_Poly *>(merge_queue.front());
			merge_queue.pop_front();

		
			GerbObj_Poly * m = NULL;
			// Only try to merge if it makes sense
			if (item->getBounds().intersectsWith(current->getBounds()))
				m = tryMerge(item, current);
			if (m)
			{
				printf("Merge suceeded\n");
				delete item;
				delete current;
				merge_queue.insert(merge_queue.end(), done_queue.begin(), done_queue.end());
				done_queue.clear();
				merge_queue.push_back(m);
				printf("queue size after merge = %d\n", merge_queue.size());
				break;
			} else {
				//printf("Merge failed\n");
				done_queue.push_back(item);
			}

		}
		if (merge_queue.size() == 0)
		{
			data->all.insert(current);
			merge_queue = done_queue;
			done_queue.clear();
		}
	}















	/*
			
			if (a == b || a->getOwner() != NULL && a->getOwner() == b->getOwner())
			{
				j++;
				continue;
			}

				//printf("Merging 1\n");
				if (a->getOwner() != NULL && a->getOwner() != b->getOwner())
				{
					mergeCount++;
				}

				GerbObj_Poly * p = tryMerge(dynamic_cast<GerbObj_Poly*>(a),
					dynamic_cast<GerbObj_Poly*>(b));*/

}

