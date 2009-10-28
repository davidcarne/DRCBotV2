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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "gcode_interp.h"
#include "gerber_parse.h"
#include "main.h"

enum light_mode_t {
	L_OFF,
	L_ON,
	L_FLASH
};


struct GCODE_state {
	enum unit_mode um;

	enum light_mode_t lm;
	int last_ap;
	
	int G_op;

	bool interp_360;
	bool poly_fill;

	bool coord_accum;
	
	bool done;
	
	GerbObj_Poly * cpoly;
	
	// Naming is bad here - last is the last set value
	// aka WHERE WE'RE GOING WHEN WE EXECUTE
	double last_x;
	double last_y;
	double last_i;
	double last_j;
	
	// Current is the current location of the print head
	// aka WHERE WE ARE BEFORE WE EXECUTE
	double current_x;
	double current_y;
	
};

bool can_trace_aperture(struct aperture * ap)
{
	if (ap->type == AP_MACRO)
		return false;
	
	// Don't know what this is, so return false
	if (ap->type == AP_T)
		return false;
}

bool handle_G_op(struct GCODE_state * s, struct gcode_block * b, Vector_Outp * v)
{
	switch (b->int_data)
	{
		case 74:
			s->interp_360 = false;
			break;
		case 75:
			s->interp_360 = true;
			break;
		case 36:
			s->cpoly = new GerbObj_Poly();
			s->poly_fill = true;
			break;
		case 37:
			v->all.insert(s->cpoly);
			s->poly_fill = false;
			break;
		case 54:
			break;
		case 70:
			s->um = UNITMODE_IN;
			break;
		case 71:
			s->um = UNITMODE_MM;
			break;

		default:
			s->G_op = b->int_data;	
	}
	return true;
}

bool handle_D_op(struct GCODE_state * s, struct gcode_block * b)
{
	if (b->int_data >= 10)
	{
		s->last_ap = b->int_data;
	} else {
		switch(b->int_data)
		{
			// Draw line
			case 1:
				s->lm = L_ON;
				break;
			case 2:
				s->lm = L_OFF;
				break;
			case 3:
				
				s->coord_accum = true;
				s->lm = L_FLASH;
				break;
			default:
				DBG_ERR_PF("Invalid DCODE %d", b->int_data);
				return false;
		}
	}
	
	return true;
}

/*
 * All these are various "Stop"s - Until further notice, I'll consider then
 * end of programs
 *
 */
bool handle_M_op(struct GCODE_state * s, struct gcode_block * b)
{
	s->done = true;
	return true;
}

bool handle_coord(struct GCODE_state * s, struct gcode_block * b)
{
	switch (b->op)
	{
		case GCO_X:
			s->last_x = b->dbl_data;
			break;
		case GCO_Y:
			s->last_y = b->dbl_data;
			break;
		case GCO_I:
			s->last_i = b->dbl_data;
			break;
		case GCO_J:
			s->last_j = b->dbl_data;
			break;
	}
	
	s->coord_accum = true;
}

GerbObj * aperture_flash_create_poly(struct GCODE_state *s,struct aperture * ap)
{
	assert(ap != NULL);

	switch(ap->type)
	{
		case AP_CIRCLE:
			{ 
				GerbObj_Line * l = new GerbObj_Line();
				l->sx = s->last_x;
				l->sy = s->last_y;
				l->ex = s->last_x;
				l->ey = s->last_y;
				l->width = ap->circle_p.OD;
				l->lt = LT_STRAIGHT;
				l->lc = LC_ROUND;
				return l;
			}
			break;
		case AP_RECT:
			{
			
				GerbObj_Poly * p = new GerbObj_Poly();
				
				
				p->addPoint(Point(s->last_x + ap->rect_p.XAD/2, s->last_y - ap->rect_p.YAD/2));
				p->addPoint(Point(s->last_x + ap->rect_p.XAD/2, s->last_y + ap->rect_p.YAD/2));
				p->addPoint(Point(s->last_x - ap->rect_p.XAD/2, s->last_y + ap->rect_p.YAD/2));		
				p->addPoint(Point(s->last_x - ap->rect_p.XAD/2, s->last_y - ap->rect_p.YAD/2));	
				return p;
			}
			
			break;
		case AP_OVAL:
			{
			
				
				GerbObj_Line * l = new GerbObj_Line();
				
				double w = fmin(ap->rect_p.YAD,ap->rect_p.XAD);
				double xs = ap->rect_p.XAD-w;
				double ys = ap->rect_p.YAD-w;
				
				l->sx = s->last_x - xs/2;
				l->sy = s->last_y - ys/2;
				l->ex = s->last_x + xs/2;
				l->ey = s->last_y + ys/2;
				
				l->width = w;
				l->lt = LT_STRAIGHT;
				l->lc = LC_ROUND;
				return l;
			}
			
			break;


			
		case AP_POLY:
                {
                    
                        double r = ap->poly_p.OD / 2;
			int si = (int)ap->poly_p.NS;
                        float t = ap->poly_p.DR / 180.0f * M_PI;
                        
                        GerbObj_Poly * p = new GerbObj_Poly();
                        float step = 2.0 * M_PI / si;
			
			double xc = s->last_x;
			double yc = s->last_y;
                        for (int i=0; i < si; i++)
                        {
				float ts = step * i + t;
				p->addPoint(Point(cos(ts) * r + xc, sin(ts) * r + yc));
                        }
                        return p;
                }
                case AP_T:
			return NULL;
			
		case AP_MACRO:
			return ap->macro_p.compiled_macro->execute(ap->macro_p.params,s->last_x, s->last_y);
	}
	
	return NULL;
}

extern double angle_from_dxdy(double dx, double dy);

GerbObj * aperture_slide_create_poly_straight(struct GCODE_state *s,struct aperture * ap)
{
	if (ap->type != AP_CIRCLE)
	{
		
		// create the aperture
		GerbObj_Poly * p = dynamic_cast<GerbObj_Poly *>(aperture_flash_create_poly(s,ap));

		if (!p)
		{
			DBG_ERR_PF("Cannot slide non-circ, non-poly AP\n");
			return NULL;
		}
		// Now we rotate the aperture so the slide line is on the x;
		double sl_vec[2] = {s->current_x - s->last_x, s->current_y - s->last_y};

		double theta = -angle_from_dxdy(sl_vec[0], sl_vec[1]);

		double maxY = -1000000;
		double minY =  1000000;
		int maxYi = 0;
		int minYi = 0;
		int i=0;
		//printf("%f %f %f\n", sl_vec[0], sl_vec[1], sin(theta) * sl_vec[0] + cos(theta) * sl_vec[1]);
		GerbObj_Poly::i_point_list_t it = p->points.begin();
		for (; it != p->points.end(); it++,i++)
		{
			
			Point & p = *it;
			 
			double y_ = sin(theta) * (p.x - s->last_x) + cos(theta) * (p.y-s->last_y);
			//printf("[%f] %f %f %f\n",theta,  (p.x-s->last_x), (p.y-s->last_y), y_);
			
			if (y_ < minY)
			{
				minY = y_;
				minYi = i;
			}
			
			if (y_ > maxY)
			{
				maxY = y_;
				maxYi = i;
			}
		}
		
		//printf("%f %d %f %d\n", minY, minYi, maxY, maxYi); 

		GerbObj_Poly * n = new GerbObj_Poly();
		
		i=0;
		it = p->points.begin();
		bool mode = false;
		for (; it != p->points.end(); it++,i++)
		{	
			if (!mode)
				n->addPoint(*it);
			else	
				n->addPoint(Point((*it).x - s->last_x + s->current_x, (*it).y - s->last_y + s->current_y));
				
			if (i == maxYi || i== minYi)
			{
				mode = !mode;
			
				if (!mode)
					n->addPoint(*it);
				else	
					n->addPoint(Point((*it).x - s->last_x + s->current_x, (*it).y - s->last_y + s->current_y));
			}
		}
		
		delete p;
		return n;


	} else {
		
		GerbObj_Line * obj = new GerbObj_Line();
		
		obj->lc = LC_ROUND;
		obj->lt = LT_STRAIGHT;
		obj->width = ap->circle_p.OD;
		obj->sx = s->last_x;
		obj->sy = s->last_y;
		obj->ex = s->current_x;
		obj->ey = s->current_y;
		return obj;
	}	

}

GerbObj * aperture_slide_create_poly(struct GCODE_state *s,struct aperture * ap)
{
	switch(s->G_op){
		case 1: 
		return aperture_slide_create_poly_straight(s,ap);
	
		default:
		return NULL;
	}
}

	
GerbObj * create_poly(struct GCODE_state * s, struct gerber_file * gerb)
{
	assert(s->lm != L_OFF);

	
	if (s->lm == L_ON)
	{
		return aperture_slide_create_poly(s,gerb->ap_list[s->last_ap]);
	}

	if (s->lm == L_FLASH)
	{
		return aperture_flash_create_poly(s,gerb->ap_list[s->last_ap]);
	}
	
	return NULL;
}

void createPolysForCurve(struct GCODE_state * s, struct gerber_file * gerb, Vector_Outp * vect, bool poly_point) {
	
	double cx;
	double cy;
	
	enum {
		S_CCW,
		S_CW
		} stepdir;
		
	if (s->G_op == 2)
	{
		stepdir = S_CW;
	} else {
		stepdir = S_CCW;
	}
	
	if (s->interp_360)
	{
		cx = s->current_x + s->last_i;
		cy = s->current_y + s->last_j;
	} else {
		// TODO: implement quad testing
		return;
	}
	
	double r = sqrt(s->last_i*s->last_i + s->last_j*s->last_j);
	
	// hack for poorly spec'ed I/J coords
	double r2 = sqrt((s->current_x-cx)*(s->current_x-cx) + (s->current_y-cy)*(s->current_y-cy));
	if (r2 > r)
		r = r2;
		
	double st_theta_sin = asin(fmin((s->current_y - cy)/r,1.0));
	double st_theta_cos = acos(fmin((s->current_x - cx)/r,1.0));
	
	double en_theta_sin = asin(fmin((s->last_y - cy)/r,1.0));
	double en_theta_cos = acos(fmin((s->last_x - cx)/r,1.0));

	double st_theta = (st_theta_sin >= 0 ? st_theta_cos : 2*M_PI - st_theta_cos );
	double en_theta = (en_theta_sin >= 0 ? en_theta_cos : 2*M_PI - en_theta_cos );;
	
	double theta_D;
	if (stepdir == S_CCW)
	{
		theta_D = en_theta - st_theta;
		if (theta_D <= 0)
			theta_D += 2 * M_PI;
	} else {
		theta_D = en_theta - st_theta;
		if (theta_D >= 0)
			theta_D -= 2 * M_PI;
	
	}
	

	int steps = (int)( 20 * theta_D);
	if (steps < 0)
		steps = -steps;

	double theta_step = theta_D / steps;
//	if (stepdir == S_CW)
//		theta_step *= -1;
		
	double last_x = s->current_x;
	double last_y = s->current_y;

	if (!poly_point)
		s->cpoly->addPoint(Point(last_x, last_y));

	//printf("CURVE: cx=%f cy=%f theta_D=%f st=%f,%f en=%f,%f step=%f[%d]\n",cx,cy,theta_D,s->last_x,s->last_y,s->current_x, s->current_y,theta_step,steps);
	//printf("CURVE2: s/s=%f s/c=%f, e/s=%f, e/c=%f\n", st_theta_sin, st_theta_cos, en_theta_sin, en_theta_cos);
	//printf("CURVE3: %f\n",(s->last_x - cx)/r);


GErr * e;
		

	for (int i=1; i<= steps; i++)
	{
		double theta = st_theta + theta_step * i;
		double x = cos(theta) * r + cx;
		double y = sin(theta) * r + cy;
		if (poly_point)
		{
			//printf("\tlastx %f, lasty %f, x %f, y %f\n",last_x, last_y, x,y);
			GerbObj_Line * obj = new GerbObj_Line();
		
			obj->lc = LC_ROUND;
			obj->lt = LT_STRAIGHT;
			obj->width = gerb->ap_list[s->last_ap]->circle_p.OD;
			obj->sx = last_x;
			obj->sy = last_y;
			obj->ex = x;
			obj->ey = y;
		
			
	
			vect->all.insert(obj);
		} else {
			s->cpoly->addPoint(Point(x, y));
		}
		
		last_x = x;
		last_y = y;
	}


}

#undef DBG_ERR_PF
#define DBG_ERR_PF printf
bool handle_exec(struct GCODE_state * s, struct gerber_file * gerb, Vector_Outp * vect)
{
	
	// Check if we've accumulated any coords since the last exec
	if (s->coord_accum)
	{
		// Make sure that the aperture is ok.
		
		// argh - some programs zero with an invalid ap
		if (gerb->ap_list[s->last_ap] == NULL && !s->poly_fill)
		{
			DBG_ERR_PF("BAD AP %d", s->last_ap);
			return true;
		}
		

		switch (s->G_op)
		{
			case  0:
				DBG_ERR_PF("Cannot handle Gcode 00");
				return false;
			case  1:
			{
				if (s->lm != L_OFF)
				{
					if (!s->poly_fill)
					{
						GerbObj * output_poly = create_poly(s,gerb);
					
						if (output_poly != NULL)
						{
							vect->all.insert(output_poly);
						} else {
					
							printf("Unhandled Move from (%lf,%lf) to (%lf,%lf) ap %d light %s\n", 
							 s->current_x, s->current_y, s->last_x, s->last_y, s->last_ap,
							 s->lm == L_OFF ? "off" : s->lm == L_ON ? "on" : "flash");
							 GErr * e = new GErr();
							 
							 return false;
							 
							 // Flash occurs @ print head start! not print head end
							 e->x = s->last_x;
							 e->y = s->last_y;
							 e->nx = s->current_x;
							 e->ny = s->current_y;
							 e->t = s->lm == L_FLASH ? ERR_Unhand : ERR_Line;
							 vect->errors.insert(e);
						
						}
					} else {
						if (s->lm == L_FLASH)
						{
							DBG_ERR_PF("Cannot flash on poly fill!\n");
							return false;
						}
							s->cpoly->addPoint(Point(s->last_x, s->last_y));
					}
				}
				
				
				s->current_x = s->last_x;
				s->current_y = s->last_y;
			}
			break;
				
			case  2:
			case  3:
				if (!s->poly_fill)
				{	if (s->lm == L_ON)
						createPolysForCurve(s,gerb,vect, true);
					else if (s->lm == L_FLASH)
					{	
						GerbObj * output_poly = create_poly(s,gerb);

                                                if (output_poly != NULL)
                                                {
                                                        vect->all.insert(output_poly);
                                                } else {

                                                        printf("Unhandled Move from (%lf,%lf) to (%lf,%lf) ap %d light %s\n",
                                                         s->current_x, s->current_y, s->last_x, s->last_y, s->last_ap,
                                                         s->lm == L_OFF ? "off" : s->lm == L_ON ? "on" : "flash");
                                                         GErr * e = new GErr();

                                                         return false;

                                                         // Flash occurs @ print head start! not print head end
                                                         e->x = s->last_x;
                                                         e->y = s->last_y;
                                                         e->nx = s->current_x;
                                                         e->ny = s->current_y;
                                                         e->t = s->lm == L_FLASH ? ERR_Unhand : ERR_Line;
                                                         vect->errors.insert(e);

                                                }
					}
				} else {
					createPolysForCurve(s,gerb,vect, false);
					//printf("Don't even think about curved polys\n");
					//return false;
				}
				
				// Aha - curve issues caused by not moving print head
				s->current_x = s->last_x;
				s->current_y = s->last_y;
				break;

			case 10:
			case 11:
			case 12:
			default:
				DBG_ERR_PF("Cannot handle Gcode %d", s->G_op);
				return false;
		}
		
		
	}
	
	
	// Reset the coordinate accumulator
	s->coord_accum = false;
	return true;
}

Vector_Outp * gcode_run(struct gerber_file * gerb)
{

	Vector_Outp * pt = new Vector_Outp();
		
	struct GCODE_state plot_state;
	bzero(&plot_state, sizeof(GCODE_state));

	plot_state.um = UNITMODE_IN;
	// HACK: some gerbers assume we start with AP 10.
	//plot_state.last_ap = 10;
	// Eagle doesn't seem to include this :/
	plot_state.G_op = 1;
	
	printf("Starting GCODE Virtual Machine\n");

	struct gcode_block * cur_op = gerb->first_gcode;

	while ((cur_op != NULL) && (!plot_state.done))
	{
		switch (cur_op->op)
		{
			case GCO_G:
				handle_G_op(&plot_state, cur_op, pt);
				break;
				
			case GCO_M:
				if (!handle_M_op(&plot_state, cur_op))
					return NULL;
				break;
				
			case GCO_D:
				if (!handle_D_op(&plot_state, cur_op))
					return NULL;
					
				break;
				
			case GCO_X:
			case GCO_Y:
			case GCO_I:
			case GCO_J:
				handle_coord(&plot_state, cur_op);
				break;

			case GCO_END:
				if (!handle_exec(&plot_state, gerb, pt))
				{
					printf("Could not execute gcode block!\n");
					return NULL;
				}
				break;

			case GCO_DIR:;
				//printf("Directive matching unhandled!\n");
				//return NULL;
		}
		cur_op = cur_op->next;
	}
	printf("GCODE Virtual Machine Finished\n");

	return pt;
}


