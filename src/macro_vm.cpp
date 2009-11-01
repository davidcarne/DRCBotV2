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

#include "gcode_interp.h"
#include "macro_vm.h"
#include "math.h"

#include "gerbobj_poly.h"

/* Render a n-gon primitive
 * p: destination polygon
 * x,y: head location at macro invocation
 *
 */
bool Macro_VM::renderPrim5(GerbObj_Poly * p, float x, float y)
{
	if (mem_stack.size() < 6)
	{
		return false;
	}
	float rot = mem_stack.top(); mem_stack.pop();
	float diam = mem_stack.top(); mem_stack.pop();
	float yc = mem_stack.top(); mem_stack.pop();
	float xc = mem_stack.top(); mem_stack.pop();
	int verts = (int)mem_stack.top(); mem_stack.pop();
	int exposure = (int)mem_stack.top(); mem_stack.pop();

	diam/=2;
	rot /= 180.0;
	rot *= M_PI;
	float thetaStep = 2*M_PI/verts;
	for (int i=0; i< verts; i++)
	{
		float theta = i*thetaStep+rot;
		p->addPoint(Point(cos(theta)*diam+xc+x, sin(theta)*diam+yc+y));
	}
	return true;
}

/* Render an outline primitive [aka, polygon defined by points] */
bool Macro_VM::renderPrim4(GerbObj_Poly * p, float x, float y)
{
	if (mem_stack.size() < 2)
	{
		return false;
	}

	float rot = mem_stack.top(); mem_stack.pop();

	while (mem_stack.size() > 2)
	{
		float ay = mem_stack.top(); mem_stack.pop();
		float ax = mem_stack.top(); mem_stack.pop();

		p->addPoint(Point(x+ax, y+ay));
	}
	while (mem_stack.size() > 0)
	{
		mem_stack.pop();
	}
	// last two are exp + verts
	return true;
}

// Render a line defined by width, height and center point
bool Macro_VM::renderPrim21(GerbObj_Poly * p, float x, float y)
{
	if (mem_stack.size() < 6)
	{
		return false;
	}

	float rot = mem_stack.top(); mem_stack.pop();
	float yc = mem_stack.top(); mem_stack.pop();
	float xc = mem_stack.top(); mem_stack.pop();
	float yh = mem_stack.top(); mem_stack.pop();
	float xh = mem_stack.top(); mem_stack.pop();
	int exposure = (int)mem_stack.top(); mem_stack.pop();

	rot = rot/180.0 * M_PI;

	float t = atan(yh / xh);
	float r = sqrt(xh*xh / 4 + yh*yh / 4);

	p->addPoint(Point(cos(rot + t)*r+x, sin(rot + t)*r+y));
	p->addPoint(Point(cos(rot - t)*r+x, sin(rot - t)*r+y));

	p->addPoint(Point(cos(rot + t + M_PI)*r+x, sin(rot + t + M_PI)*r+y));
	p->addPoint(Point(cos(rot - t + M_PI)*r+x, sin(rot - t + M_PI)*r+y));
	// last two are exp + verts
	return true;
}

/*
 * Execute the macro, and return the generated polygon
 */
GerbObj_Poly * Macro_VM::execute(double * params, float x, float y)
{
	GerbObj_Poly * p = new GerbObj_Poly();
	i_code_type_t i = code.begin();
	
	for(;i!=code.end(); i++)
	{
		Macro_OP & m = *i;
		switch(m.op)
		{
			case OP_NOP:
			{
				break;
			}
			case OP_ADD:
			{
				float a = mem_stack.top(); mem_stack.pop();
				float b = mem_stack.top(); mem_stack.pop();
				mem_stack.push(a+b);
				break;
			}
			case OP_SUB:
			{
				float a = mem_stack.top(); mem_stack.pop();
				float b = mem_stack.top(); mem_stack.pop();
				mem_stack.push(b-a);
				break;
			}
			case OP_MUL:
			{
				float a = mem_stack.top(); mem_stack.pop();
				float b = mem_stack.top(); mem_stack.pop();
				mem_stack.push(a * b);
				break;
			}
			case OP_DIV:
			{
				float denom = mem_stack.top(); mem_stack.pop();
				float num = mem_stack.top(); mem_stack.pop();
				mem_stack.push(num/denom);
				break;
			}
			
			case OP_PUSH:
				mem_stack.push(m.fval);
				break;
				
			case OP_FETCH:
				mem_stack.push(params[m.ival-1]);
				break;
			
			case OP_STORE:
				params[m.ival] = mem_stack.top(); mem_stack.pop();
				break;
				
			case OP_PRIM:
				// No other primitives than the following have been seen in production
				// Adding them shouldn't be a big deal if they show up
				switch (m.ival){
					case 5:
						if (!renderPrim5(p,x,y))
						{
							delete p;
							return NULL;
						}
						break;
					case 4:
						if (!renderPrim4(p,x,y))
						{
							delete p;
							return NULL;
						}
						break;
					case 21:
						if (!renderPrim21(p,x,y))
						{
							delete p;
							return NULL;
						}
						break;
					
				}
				break;
				
				
		}
	}
	
	return p;
}

/*
 * Print the contents of the program
 */
void Macro_VM::print()
{
		i_code_type_t i = code.begin();
	
		for(;i!=code.end(); i++)
		{
			switch (i->op)
			{
				case OP_NOP:
					printf("NOP\n");
					break;
				case OP_ADD:
					printf("ADD\n");
					break;
				case OP_SUB:
					printf("SUB\n");
					break;
				case OP_MUL:
					printf("MUL\n");
					break;
				case OP_DIV:
					printf("DIV\n");
					break;
				case OP_PUSH:
					printf("PUSH %lf\n", i->fval);
					break;
				case OP_FETCH:
					printf("FETCH %d\n", i->ival);
					break;
				case OP_STORE:
					printf("STORE %d\n", i->ival);
					break;
				case OP_PRIM:
					printf("PRIM %d\n", i->ival);
					break;
			}
		}
}

