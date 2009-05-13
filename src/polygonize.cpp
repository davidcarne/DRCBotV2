#include "polygonize.h"
#define maxsteps 64
GerbObj_Poly * createPolyForLine(GerbObj_Line * l)
{
	double dx = l->sx - l->ex;
	double dy = l->sy - l->ey;
	double angle;

	// Angle  = angle of line
	angle = atan2(dy, dx);
	// rotate 90
	angle += M_PI/2;

	double radius = l->width / 2;
		
	//RenderPoly * obj = new RenderPoly();

	int nsteps = l->width * 1000;
	if (nsteps > maxsteps)
		nsteps = maxsteps;
	if (nsteps < 2)
		nsteps = 2;

	// No polygon data for something with 0 width
	if (l->width == 0)
		return NULL;

	GerbObj_Poly * p = new GerbObj_Poly();

	float theta = -M_PI/2;
	float thetastep = M_PI / nsteps;
	for (int i=0; i<=nsteps; i++)
	{
		double pdx = cos(angle + 3.0 * M_PI / 2.0+theta) * radius;
		double pdy = sin(angle + 3.0 * M_PI / 2.0+theta) * radius;
		p->addPoint(Point(l->sx + pdx, l->sy + pdy));
		theta += thetastep;
	}
	theta = -M_PI/2;
	for (int i=0; i<=nsteps; i++)
	{
		double pdx = cos(angle + 3.0 * M_PI / 2.0+theta) * radius;
		double pdy = sin(angle + 3.0 * M_PI / 2.0+theta) * radius;
		p->addPoint(Point(l->ex - pdx, l->ey - pdy));
		theta += thetastep;

	}




	return p;
}
void polygonize_vector_outp(Vector_Outp * v)
{
	std::set <GerbObj*>::iterator i = v->all.begin();
	int c = 0;
	for (;i!=v->all.end();)
	{
		GerbObj_Line * line = dynamic_cast<GerbObj_Line*>(*i);
		if (line)
		{	
			
			// First, remove the line from the map
			v->all.erase(i++);

			// Create a polygon for the line [no end caps for now]
			GerbObj_Poly * p = createPolyForLine(line);
			if (p)
				v->all.insert(p);
			delete line;

			c++;
		} else {
			++i;
		}
	}
	printf("Removed %d lines from map\n", c);
}
