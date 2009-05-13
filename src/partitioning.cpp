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

