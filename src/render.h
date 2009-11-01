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

#ifndef _RENDER_H_
#define _RENDER_H_

#include <vector>


enum flagerr_t
{
	FLG_NONE=0,
	FLG_WIDTH=1,
	FLG_SPACE=2,
	FLG_NO_OWNER=3,
};

struct point_line {
	// Start coordinate, [optional center point for arc]
	double x,y,cx,cy;
	
	enum line_render_type_t
	{
		LR_STRAIGHT,
		LR_ARC	
	};
	
	// and how to draw the line
	enum line_render_type_t lt;
};

class RenderPoly {
public:
	
	std::vector<struct point_line*> segs;
	float r,g,b;
	enum flagerr_t flag;
	float fillptx, fillpty;
	int oid;
	
};

#endif

