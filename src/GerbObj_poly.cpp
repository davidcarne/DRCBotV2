#include "gcode_interp.h"

#include <math.h>
#include <assert.h>
#include "gcode_interp.h"


static struct point_line * alloc_point_line()
{
	return (struct point_line *)malloc(sizeof(struct point_line));
}

	
bool GerbObj_Poly::is_ccw()
{
	if (points.size() == 0)
		return false;
	if (points.size() == 1)
		return true;

	i_point_list_t i = points.begin();
	
	bool debug = false;

	const Point & first_point = *i;
	Point last_point = first_point;
	i++;
	
	double a = 0;
	for (;i!=points.end();i++)
	{
		const Point & cur_point = *i;
		double area = last_point.x*cur_point.y-cur_point.x*last_point.y;
		a += area;
		last_point = cur_point;
	}
	a += last_point.x*first_point.y-first_point.x*last_point.y;
	return a >= 0;
}

#define aPL alloc_point_line
#define sPLp struct point_line *

RenderPoly * GerbObj_Poly::createPolyData()
{
	RenderPoly * rp = new RenderPoly();
	sPLp pt;
	
	i_point_list_t it = points.begin();
	
	for (;it != points.end();it++)
	{
		pt = aPL();
		pt->lt = LT_STRAIGHT;
		pt->x = (*it).x;
		pt->y = (*it).y;
		rp->segs.push_back(pt);
	}
	
	rp->flag = flag;
	
	if (getOwner())
	{
		rp->oid = getOwner()->getID();
		rp->r = getOwner()->r;
		rp->g = getOwner()->g;
		rp->b = getOwner()->b;
	} else {
		rp->flag = FLG_NO_OWNER;
	}
	
	return rp;
}

