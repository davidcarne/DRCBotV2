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

#ifndef _GCODE_INTERP_H_
#define _GCODE_INTERP_H_


#include <set>
#include <list>
#include <vector>
#include <map>
#include <utility>
#include <tr1/unordered_map>
#include <boost/functional/hash.hpp>
#include "partitioning.h"
#include "util_type.h"


enum line_trace_type_t
{
	LT_STRAIGHT,
	LT_ARC	
};

enum line_cap_type_t 
{

	LC_NONE,
	LC_ROUND,
	LC_RECT,
};

class net_group;
class Vector_Outp;
class GerbObj;

enum flagerr_t
{
	FLG_NONE=0,
	FLG_WIDTH=1,
	FLG_SPACE=2,
	FLG_NO_OWNER=3,
};

/********************* polygon stuff for rendering ****************/
/*** Not used for GCode Parsing - used only for rendering ********/
struct point_line {
        // Start coordinate, [optional center point for arc]
        double x,y,cx,cy;
        
        // and how to draw the line
        enum line_trace_type_t lt;
};

class RenderPoly {
        public:

        std::vector<struct point_line*> segs;
        float r,g,b;
		enum flagerr_t flag;
		float fillptx, fillpty;
		int oid;
		
};

/********************* net_group *********************/
net_group * new_net_group(Vector_Outp * f);
void group_together(Vector_Outp * f, GerbObj * a, GerbObj * b);

class net_group {
	friend void add_to_group(Vector_Outp * f, net_group * n, GerbObj * o);
	friend net_group * new_net_group(Vector_Outp * f);
	
public:
	
	typedef std::set<GerbObj*> gObjSet_t;
	typedef gObjSet_t::iterator i_gObjSet_t;
	
	i_gObjSet_t start(void);
	i_gObjSet_t end(void);

	float r,g,b;
	int getSize() {return members.size();};
	int getID() {return set_id;};
protected:
	net_group(int id);
private:
	int set_id;
	std::set <GerbObj*> members;
};


/*********************** gerber objects *****************/
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
	virtual RenderPoly * createPolyData()
	{
		printf("Warning - creating unknown renderPoly\n");
		return NULL;
	}
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

/***** Line */
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
	// and how to draw the line
	enum line_trace_type_t lt;
	enum line_cap_type_t lc;
	
	protected:
		RenderPoly * createPolyData();
};

/***** Poly */
class GerbObj_Poly : public GerbObj {
	public:
	
	GerbObj_Poly() : GerbObj() {cached = false;};
	
	typedef std::list<Point> point_list_t;
	typedef point_list_t::iterator i_point_list_t;
	point_list_t points;
	
	void addPoint(Point p)
	{
		points.push_back(p);
	}
	Rect cached_rect;
	bool cached;
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
	protected:
		RenderPoly * createPolyData();
};

enum G_ERR_type 
{
	ERR_Unhand,
	ERR_Line,
};

class GErr {
	public:
	double x;
	double y;
	
	double nx;
	double ny;
	
	enum G_ERR_type t;
};

class Vector_Outp {
public:
	int last_set;
	std::set <GErr*> errors;
	std::set <net_group*>  groups;
	std::set <GerbObj*> all;
	Part2D<GerbObj*> lines;
};


void makeEndPartitions(Vector_Outp * outp);

Vector_Outp *  gcode_run(struct gerber_file * gerb);
#endif

