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

#ifndef _GERBOBJ_H_
#define _GERBOBJ_H_

#include "render.h"

class RenderPoly;
class Vector_Outp;
class net_group;


#include "util_type.h"
#include <boost/shared_ptr.hpp>


enum line_trace_type_t
{
	LT_STRAIGHT,
	LT_ARC	
};

class GerbObj {
public:
	
	RenderPoly* getPolyData()
	{
		if (!cached)
			cached = createPolyData();
		return cached;
	}
	
	friend void add_to_group(Vector_Outp * f, net_group * n, GerbObj * o);
	
	net_group* getOwner() {return owner;};
	

	
	
	enum flagerr_t flag;
	
	virtual Rect getBounds()=0;
	~GerbObj()
	{
	}
	
protected:
	virtual RenderPoly * createPolyData() = 0;
	net_group * owner;
	
	
	GerbObj()
	{
		cached = NULL;
		owner = NULL;
		flag = FLG_NONE;
	}
	
	
	
private:
	RenderPoly * cached;
};


typedef boost::shared_ptr<GerbObj> sp_GerbObj;
#endif

