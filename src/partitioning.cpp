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
#include <math.h>
void makeEndPartitions(Vector_Outp * outp)
{
	std::set<GerbObj*>::iterator i = outp->all.begin();
	
	/*for (;i!=outp->all.end();i++)
	{
		GerbObj_Line * e = (GerbObj_Line*)*i;
		
		outp->endpoints.insert(e->sx, e->sy, e);
		
		outp->endpoints.insert(e->ex, e->ey, e);
	}*/
	
	i = outp->all.begin();
	for (;i!=outp->all.end();i++)
	{
		GerbObj * e = *i;
	
		
		outp->lines.insertbounded(e->getBounds(),e);
	}
}

