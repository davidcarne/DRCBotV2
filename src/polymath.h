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



#ifndef _POLYMATH_H_
#define _POLYMATH_H_

enum GH_op_t {
	GH_op_subtract,
	GH_op_intersect,
	GH_op_union
	};

struct GH_vertex;

// Pass NULL to start a new polygon
GH_vertex * GH_polyPoint(GH_vertex * v, double x, double y);
GH_vertex * GH_getPolyPoint(GH_vertex *v, double * x, double * y);

GH_vertex * GH_polygon_boolean(GH_vertex * p1, GH_vertex * p2, enum GH_op_t op);
void GH_free_polygons(GH_vertex * polys);

#endif