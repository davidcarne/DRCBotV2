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
#include <boost/shared_ptr.hpp>
#include <string>

#include "partitioning.h"
#include "util_type.h"


#include "gerber_parse.h"
#include "gerbobj.h"

class net_group;
class Vector_Outp;
class GerbObj;
class RenderPoly;

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


struct gerber_object_layer {
	gerber_object_layer()
	{
		polarity = RS274X_Program::LP_D;
		name = "";
		step_repeat.X = 0;
		step_repeat.Y = 0;
		step_repeat.x_step = 0;
		step_repeat.y_step = 0;
	};
	
	RS274X_Program::layer_polar_t polarity;
	std::string name;
	struct {
		int X,Y;
		double x_step, y_step;
	} step_repeat;
	
	std::list <sp_GerbObj> draws;
};

typedef boost::shared_ptr<gerber_object_layer> sp_gerber_object_layer;

class Vector_Outp {
public:
	std::list <sp_gerber_object_layer> layers;
	sp_gerber_object_layer current_layer;
	
	//Part2D<GerbObj*> lines;
};


typedef boost::shared_ptr<Vector_Outp> sp_Vector_Outp;
sp_Vector_Outp gcode_run(sp_RS274X_Program gerb);
#endif

