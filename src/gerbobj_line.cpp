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

#include "gcode_interp.h"
#include "render.h"
#include "gerbobj_line.h"

struct point_line * alloc_point_line()
{
	return (struct point_line *)malloc(sizeof(struct point_line));
}


#define aPL alloc_point_line
#define sPLp struct point_line *

RenderPoly * createRoundCapPoly(GerbObj_Line * r)
{
	double dx = r->sx - r->ex;
	double dy = r->sy - r->ey;
	double angle;

	angle = atan2(dy, dx);

	double radius = r->width / 2;
	
	double pdx = cos(angle + 3.0 * M_PI / 2.0) * radius;
	double pdy = sin(angle + 3.0 * M_PI / 2.0) * radius;
	
	// 4 Segments:
	// [terms for a right pointing line]
	// 
	// Below_start -> Below_end
	// Arc Below_end -> Above_end
	// Above_end -> Above_start
	// Arc Above_start -> Below_start
	
	RenderPoly * obj = new RenderPoly();
	
	obj->fillptx = r->sx;
	obj->fillpty = r->sy;
	
	struct point_line * pt;

	// Below_start
	pt = aPL();
	pt->lt = point_line::LR_STRAIGHT;
	pt->x = r->ex + pdx;
	pt->y = r->ey + pdy;
	obj->segs.push_back(pt);
	
	
	// Below_end
	pt = aPL();
	pt->lt = point_line::LR_ARC;
	pt->x = r->sx + pdx;
	pt->y = r->sy + pdy;
	pt->cx = r->sx;
	pt->cy = r->sy;
	obj->segs.push_back(pt);

	// Above_end
	pt = aPL();
	pt->lt = point_line::LR_STRAIGHT;
	pt->x = r->sx - pdx;
	pt->y = r->sy - pdy;
	obj->segs.push_back(pt);	

	// Above_start
	pt = aPL();
	pt->lt = point_line::LR_ARC;
	pt->x = r->ex - pdx;
	pt->y = r->ey - pdy;
	pt->cx = r->ex;
	pt->cy = r->ey;
	obj->segs.push_back(pt);
	
	obj->flag = r->flag;
	/*
	if (r->getOwner())
	{
		obj->oid = r->getOwner()->getID();
		obj->r = r->getOwner()->r;
		obj->g = r->getOwner()->g;
		obj->b = r->getOwner()->b;
	} else {
		obj->flag = FLG_NO_OWNER;
	}*/
	
	return obj;
}
RenderPoly * GerbObj_Line::createPolyData()
{
	
	
	
	return createRoundCapPoly(this);
}

