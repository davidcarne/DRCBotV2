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
#include "drc.h"
#include "groupize.h"
#include "polymath.h"

#include <math.h>

void initDRC(struct drcSettings * s)
{
	s->minTraceWidth = 0.0069;
	s->minTraceSpace = 0.0069;
}

bool doDRC(Vector_Outp * v, struct drcSettings * s, bool drawErrors)
{
	bool success = true;
	
	//printf("lengthdb size %d\n", v->obj_dist_lut.size());
	long hits =0, misses = 0;
	std::set<GerbObj *>::iterator it = v->all.begin();
	for (;it!=v->all.end();it++)
	{
		double width;
		GerbObj * g = *it;
		
		GerbObj_Line * l_i;
		GerbObj_Poly * l_p;
		if (l_i = dynamic_cast<GerbObj_Line*>(g))
		{
			width = l_i->width;
		} else if (l_p = dynamic_cast<GerbObj_Poly*>(g)) {
			width = INFINITY;
		}
		
		if (width < s->minTraceWidth)
		{
			printf("DRC Error - trace to narrow\n");
			g->flag = FLG_WIDTH;
			success = false;
		}
	}
	
	// now check for distance between groups
	it = v->all.begin();
	for (;it!=v->all.end();it++)
	{
		
		GerbObj_Line * g = (GerbObj_Line*)*it;
		
		std::set<GerbObj*> gs= v->lines.retrieve((g->sx+g->ex)/2,(g->sy+g->ey)/2,(g->sx-g->ex)/2 + g->width,(g->sy-g->ey)/2 + g->width);
		std::set<GerbObj*>::iterator j = gs.begin();
		for (;j!=gs.end();j++)	
		{
			GerbObj_Line * h = (GerbObj_Line*)*j;
			if (g->getOwner() == h->getOwner())
				continue;
			double d;
			
			// TODO: brittle hash fn! changeme
			
			long ka = g < h ?  (long)g : (long)h;
			long kb = g < h ?  (long)h : (long)g;
			
			long key = ((long)ka << 16) | ((long)ka >> 16) | (long)kb;
			//d = v->obj_dist_lut[key];
			
			//d -= (g->width + h->width)/2;
			d = distanceBetween(g,h);
			if (d < s->minTraceSpace)
			{
				printf("DRC space fail: [%p %p] %lf - %lf\n", g,h, d, s->minTraceSpace);
				
				/*GErr * q = new GErr();
					v->errors.insert(q);
					q->x = g->sx;
					q->y = g->sy;
					q->nx = h->sx;
					q->ny = h->sy;
					q->t = ERR_Line;
					*/
				g->flag = FLG_SPACE;
				h->flag = FLG_SPACE;
				success = false;
			}
		}
			
	}
	return success;
}

