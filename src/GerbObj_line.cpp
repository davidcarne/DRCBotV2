#include <math.h>

#include "gcode_interp.h"


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
	pt->lt = LT_STRAIGHT;
	pt->x = r->ex + pdx;
	pt->y = r->ey + pdy;
	obj->segs.push_back(pt);
	
	
	// Below_end
	pt = aPL();
	pt->lt = LT_ARC;
	pt->x = r->sx + pdx;
	pt->y = r->sy + pdy;
	pt->cx = r->sx;
	pt->cy = r->sy;
	obj->segs.push_back(pt);

	// Above_end
	pt = aPL();
	pt->lt = LT_STRAIGHT;
	pt->x = r->sx - pdx;
	pt->y = r->sy - pdy;
	obj->segs.push_back(pt);	

	// Above_start
	pt = aPL();
	pt->lt = LT_ARC;
	pt->x = r->ex - pdx;
	pt->y = r->ey - pdy;
	pt->cx = r->ex;
	pt->cy = r->ey;
	obj->segs.push_back(pt);
	
	obj->flag = r->flag;
	
	if (r->getOwner())
	{
		obj->oid = r->getOwner()->getID();
		obj->r = r->getOwner()->r;
		obj->g = r->getOwner()->g;
		obj->b = r->getOwner()->b;
	} else {
		obj->flag = FLG_NO_OWNER;
	}
	return obj;
}
RenderPoly * GerbObj_Line::createPolyData()
{
	
	
	
	return createRoundCapPoly(this);
}

