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


#ifndef _GERBOBJ_POLY_H_
#define _GERBOBJ_POLY_H_

#include "gerbobj.h"
#include <list>

class GerbObj_Poly : public GerbObj {
public:
	
	GerbObj_Poly() : GerbObj() {cached = false;};
	
	typedef std::list<Point> point_list_t;
	typedef point_list_t::iterator i_point_list_t;
	
	void addPoint(Point p)
	{
		points.push_back(p);
	}
	
	bool is_ccw(void);
	Rect getBounds()
	{
		if (!cached)
		{
			cached_rect = Rect();
			i_point_list_t i = points.begin();
			
			for (;i!=points.end();i++)
			{
				cached_rect.mergePoint(*i);
			}
		}	
		
		cached = true;
		return cached_rect;
	}
	
	point_list_t points;

	
private:	
	Rect cached_rect;
	bool cached;

protected:
	RenderPoly * createPolyData();
};

#endif
