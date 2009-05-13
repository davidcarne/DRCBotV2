#include "gcode_interp.h"
#include "macro_vm.h"
#include "math.h"
bool Macro_VM::renderPrim5(GerbObj_Poly * p, float x, float y)
{
	//printf("Stack depth: %d\n", mem_stack.size());
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
	//printf("Rendering poly: [%d %d %f,%f %f %f]\n",exposure,verts,xc,yc,diam,rot);
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


bool Macro_VM::renderPrim4(GerbObj_Poly * p, float x, float y)
{
	//printf("Stack depth: %d\n", mem_stack.size());
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

bool Macro_VM::renderPrim21(GerbObj_Poly * p, float x, float y)
{
	//printf("Stack depth: %d\n", mem_stack.size());
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
//loosely based off gerbv's macro parser
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

