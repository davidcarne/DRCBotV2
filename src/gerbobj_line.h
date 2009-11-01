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


#ifndef _GERBOBJ_LINE_H_
#define _GERBOBJ_LINE_H_

#include "gerbobj.h"

class GerbObj_Line : public GerbObj {
public:
	
	GerbObj_Line() : GerbObj() {};
	
	// Start coordinate, [optional center point for arc]
	double sx,sy,ex,ey,cx,cy;
	
	double width;
	
	Rect getBounds()
	{
		Rect r = Rect(sx,sy,ex,ey);
		// TODO: Feather needs to take into account trace spacing if it is to handle drc right
		r.feather(width);
		return r;
	}
	

	
	// This enumeration seems unused. Not sure if functionality will be needed later, so keep.
	enum line_cap_type_t 
	{
		//LC_NONE,
		LC_ROUND,
		//LC_RECT,
	};
	
	
	// and how todraw the line
	enum line_trace_type_t lt;
	enum line_cap_type_t lc;
	
protected:
	RenderPoly * createPolyData();
};

#endif

