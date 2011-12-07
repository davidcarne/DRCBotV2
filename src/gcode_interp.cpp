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

#include "gerbobj_line.h"
#include "gerbobj_poly.h"

#include "gcode_interp.h"
#include "gerber_parse.h"
#include "main.h"
#include "types.h"

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
	
	// Destination when we execute
	double destination_x;
	double destination_y;
	double destination_i;
	double destination_j;
	
	// Location before executing
	double current_x;
	double current_y;
	
};

static double unit_convert(struct GCODE_state * s, double data)
{
	if (s->um == UNITMODE_IN)
		return data * 25400;
	return data * 1000;
}

bool can_trace_aperture(const struct RS274X_Program::aperture * ap)
{
	if (ap->type == RS274X_Program::AP_MACRO)
		return false;
	
	// Don't know what this is, so return false
	if (ap->type == RS274X_Program::AP_T)
		return false;
	
	
	return true;
}

bool handle_G_op(struct GCODE_state * s, const struct RS274X_Program::gcode_block & b, sp_gerber_object_layer v)
{
	switch (b.int_data)
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
			v->draws.push_back(sp_GerbObj(s->cpoly));
			s->cpoly = NULL;
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
			s->G_op = b.int_data;	
	}
	return true;
}

bool handle_D_op(struct GCODE_state * s, const struct RS274X_Program::gcode_block & b)
{
	if (b.int_data >= 10)
	{
		s->last_ap = b.int_data;
	} else {
		switch(b.int_data)
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
				DBG_ERR_PF("Invalid DCODE %d", b.int_data);
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
bool handle_M_op(struct GCODE_state * s, const struct RS274X_Program::gcode_block & b)
{
	s->done = true;
	return true;
}

bool handle_coord(struct GCODE_state * s, const struct RS274X_Program::gcode_block & b)
{
	// TODO: handle abs / inc
	switch (b.op)
	{
		case RS274X_Program::GCO_X:
			s->destination_x = unit_convert(s, b.dbl_data);
			break;
		case RS274X_Program::GCO_Y:
			s->destination_y = unit_convert(s, b.dbl_data);
			break;
		case RS274X_Program::GCO_I:
			s->destination_i = unit_convert(s, b.dbl_data);
			break;
		case RS274X_Program::GCO_J:
			s->destination_j = unit_convert(s, b.dbl_data);
			break;
		default:
			DBG_ERR_PF("Cannot handle coord for op: %d", b.op);
			return false;
	}
	
	s->coord_accum = true;
	return true;
}

GerbObj * aperture_flash_create_poly(struct GCODE_state *s, const struct RS274X_Program::aperture * ap)
{
	assert(ap != NULL);

	switch(ap->type)
	{
		case RS274X_Program::AP_CIRCLE:
			{ 
				GerbObj_Line * l = new GerbObj_Line();
				l->sx = s->destination_x;
				l->sy = s->destination_y;
				l->ex = s->destination_x;
				l->ey = s->destination_y;
				l->width = ap->circle_p.OD;
				l->lt = LT_STRAIGHT;
				l->lc = GerbObj_Line::LC_ROUND;
				return l;
			}
			break;
		case RS274X_Program::AP_RECT:
			{
			
				GerbObj_Poly * p = new GerbObj_Poly();
				
				
				p->addPoint(Point(s->destination_x + ap->rect_p.XAD/2, s->destination_y - ap->rect_p.YAD/2));
				p->addPoint(Point(s->destination_x + ap->rect_p.XAD/2, s->destination_y + ap->rect_p.YAD/2));
				p->addPoint(Point(s->destination_x - ap->rect_p.XAD/2, s->destination_y + ap->rect_p.YAD/2));		
				p->addPoint(Point(s->destination_x - ap->rect_p.XAD/2, s->destination_y - ap->rect_p.YAD/2));	
				return p;
			}
			
			break;
		case RS274X_Program::AP_OVAL:
			{
			
				
				GerbObj_Line * l = new GerbObj_Line();
				
				double w = fmin(ap->rect_p.YAD,ap->rect_p.XAD);
				double xs = ap->rect_p.XAD-w;
				double ys = ap->rect_p.YAD-w;
				
				l->sx = s->destination_x - xs/2;
				l->sy = s->destination_y - ys/2;
				l->ex = s->destination_x + xs/2;
				l->ey = s->destination_y + ys/2;
				
				l->width = w;
				l->lt = LT_STRAIGHT;
				l->lc = GerbObj_Line::LC_ROUND;
				return l;
			}
			
			break;


			
		case RS274X_Program::AP_POLY:
                {
                    
                        double r = ap->poly_p.OD / 2;
			int si = (int)ap->poly_p.NS;
                        float t = ap->poly_p.DR / 180.0f * M_PI;
                        
                        GerbObj_Poly * p = new GerbObj_Poly();
                        float step = 2.0 * M_PI / si;
			
			double xc = s->destination_x;
			double yc = s->destination_y;
                        for (int i=0; i < si; i++)
                        {
				float ts = step * i + t;
				p->addPoint(Point(cos(ts) * r + xc, sin(ts) * r + yc));
                        }
                        return p;
                }
                case RS274X_Program::AP_T:
			return NULL;
			
		case RS274X_Program::AP_MACRO:
			return ap->macro_p.compiled_macro->execute(ap->macro_p.params,s->destination_x, s->destination_y);
	}
	
	return NULL;
}


GerbObj * aperture_slide_create_poly_straight(struct GCODE_state *s, const struct RS274X_Program::aperture * ap)
{
	if (ap->type != RS274X_Program::AP_CIRCLE)
	{
		
		// create the aperture
		GerbObj_Poly * p = dynamic_cast<GerbObj_Poly *>(aperture_flash_create_poly(s,ap));

		if (!p)
		{
			DBG_ERR_PF("Cannot slide non-circ, non-poly AP");
			return NULL;
		}
		// Now we rotate the aperture so the slide line is on the x;
		double sl_vec[2] = {s->current_x - s->destination_x, s->current_y - s->destination_y};

		double theta = -atan2(sl_vec[1], sl_vec[0]);

		double maxY = -1000000;
		double minY =  1000000;
		int maxYi = 0;
		int minYi = 0;
		int i=0;

		GerbObj_Poly::i_point_list_t it = p->points.begin();
		for (; it != p->points.end(); it++,i++)
		{
			
			Point & p = *it;
			 
			double y_ = sin(theta) * (p.x - s->destination_x) + cos(theta) * (p.y-s->destination_y);
			
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
		
		GerbObj_Poly * n = new GerbObj_Poly();
		
		i=0;
		it = p->points.begin();
		bool mode = false;
		for (; it != p->points.end(); it++,i++)
		{	
			if (!mode)
				n->addPoint(*it);
			else	
				n->addPoint(Point((*it).x - s->destination_x + s->current_x, (*it).y - s->destination_y + s->current_y));
				
			if (i == maxYi || i== minYi)
			{
				mode = !mode;
			
				if (!mode)
					n->addPoint(*it);
				else	
					n->addPoint(Point((*it).x - s->destination_x + s->current_x, (*it).y - s->destination_y + s->current_y));
			}
		}
		
		delete p;
		return n;


	} else {
		
		GerbObj_Line * obj = new GerbObj_Line();
		
		obj->lc = GerbObj_Line::LC_ROUND;
		obj->lt = LT_STRAIGHT;
		obj->width = ap->circle_p.OD;
		obj->sx = s->destination_x;
		obj->sy = s->destination_y;
		obj->ex = s->current_x;
		obj->ey = s->current_y;
		return obj;
	}	

}

GerbObj * aperture_slide_create_poly(struct GCODE_state *s, const struct RS274X_Program::aperture * ap)
{
	switch(s->G_op){
		case 1: 
		return aperture_slide_create_poly_straight(s,ap);
	
		default:
		return NULL;
	}
}

	
GerbObj * create_poly(struct GCODE_state * s, sp_RS274X_Program gerb)
{
	assert(s->lm != L_OFF);

	
	if (s->lm == L_ON)
	{
		return aperture_slide_create_poly(s, gerb->getAperture(s->last_ap));
	}

	if (s->lm == L_FLASH)
	{
		return aperture_flash_create_poly(s, gerb->getAperture(s->last_ap));
	}
	
	return NULL;
}

void createPolysForCurve(struct GCODE_state * s, sp_RS274X_Program gerb, sp_gerber_object_layer vect, bool poly_point) {
	
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
		cx = s->current_x + s->destination_i;
		cy = s->current_y + s->destination_j;
	} else {
		// TODO: implement quad testing
		return;
	}
	
	double r = sqrt(s->destination_i*s->destination_i + s->destination_j*s->destination_j);
	
	// hack for poorly spec'ed I/J coords
	double r2 = sqrt((s->current_x-cx)*(s->current_x-cx) + (s->current_y-cy)*(s->current_y-cy));
	if (r2 > r)
		r = r2;
		
	double st_theta_sin = asin(fmin((s->current_y - cy)/r,1.0));
	double st_theta_cos = acos(fmin((s->current_x - cx)/r,1.0));
	
	double en_theta_sin = asin(fmin((s->destination_y - cy)/r,1.0));
	double en_theta_cos = acos(fmin((s->destination_x - cx)/r,1.0));

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

	double destination_x = s->current_x;
	double destination_y = s->current_y;

	if (!poly_point)
		s->cpoly->addPoint(Point(destination_x, destination_y));



	GErr * e;
		

	for (int i=1; i<= steps; i++)
	{
		double theta = st_theta + theta_step * i;
		double x = cos(theta) * r + cx;
		double y = sin(theta) * r + cy;
		if (poly_point)
		{
			GerbObj_Line * obj = new GerbObj_Line();
		
			obj->lc = GerbObj_Line::LC_ROUND;
			obj->lt = LT_STRAIGHT;
			obj->width = gerb->getAperture(s->last_ap)->circle_p.OD;
			obj->sx = destination_x;
			obj->sy = destination_y;
			obj->ex = x;
			obj->ey = y;
		
			
	
			vect->draws.push_back(sp_GerbObj(obj));
		} else {
			s->cpoly->addPoint(Point(x, y));
		}
		
		destination_x = x;
		destination_y = y;
	}


}

bool handle_exec(struct GCODE_state * s, sp_RS274X_Program gerb, sp_gerber_object_layer vect)
{
	
	// Check if we've accumulated any coords since the last exec
	if (s->coord_accum)
	{
		// Make sure that the aperture is ok.
		
		// argh - some programs zero with an invalid ap
		if (gerb->getAperture(s->last_ap) == NULL && !s->poly_fill && s->lm != L_OFF)
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
							vect->draws.push_back(sp_GerbObj(output_poly));
						} else {
					
							DBG_ERR_PF("Unhandled Move from (%lf,%lf) to (%lf,%lf) ap %d light %s", 
							 s->current_x, s->current_y, s->destination_x, s->destination_y, s->last_ap,
							 s->lm == L_OFF ? "off" : s->lm == L_ON ? "on" : "flash");

							return false;
						}
					} else {
						if (s->lm == L_FLASH)
						{
							DBG_ERR_PF("Cannot flash on poly fill!");
							return false;
						}
						s->cpoly->addPoint(Point(s->destination_x, s->destination_y));
					}
					

				}
				
				
				s->current_x = s->destination_x;
				s->current_y = s->destination_y;
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
								vect->draws.push_back(sp_GerbObj(output_poly));
						} else {

								DBG_ERR_PF("Unhandled Move from (%lf,%lf) to (%lf,%lf) ap %d light %s",
								 s->current_x, s->current_y, s->destination_x, s->destination_y, s->last_ap,
								 s->lm == L_OFF ? "off" : s->lm == L_ON ? "on" : "flash");
								 GErr * e = new GErr();

								 return false;
						}
					}
				} else {
					createPolysForCurve(s,gerb,vect, false);
				}
				
				s->current_x = s->destination_x;
				s->current_y = s->destination_y;
				break;

			case 10:
			case 11:
			case 12:
			default:
				DBG_ERR_PF("Cannot handle Gcode %d", s->G_op);
				return false;
		}
		
		
	}
	
	// per page 45 of the RS274X specification, rev E
	// L_FLASH stays in effect until a new layer is encountered.
	// That implies that L_ON does not, so reset after execution
	if (s->lm == L_ON)
		s->lm = L_OFF;
	
	// Reset the coordinate accumulator
	s->coord_accum = false;
	return true;
}

const char * decode_gcode_directive_type(enum RS274X_Program::gcode_directive_type_t dt)
{
	switch (dt) {
		case RS274X_Program::DIR_AS: return "DIR_AS";
		case RS274X_Program::DIR_FS: return "DIR_FS";
		case RS274X_Program::DIR_MI: return "DIR_MI";
		case RS274X_Program::DIR_MO: return "DIR_MO";
		case RS274X_Program::DIR_OF: return "DIR_OF";
		case RS274X_Program::DIR_SF: return "DIR_SF";
		case RS274X_Program::LY_KO: return "LY_KO";
		case RS274X_Program::LY_LN: return "LY_LN";
		case RS274X_Program::LY_LP: return "LY_LP";
		case RS274X_Program::LY_SR: return "LY_SR";
	}
	assert(false); // INVALID directive, so trap out
}



void setupLayerIfNone(Vector_Outp * pt)
{
	assert((pt->layers.size() && pt->current_layer) ||
		   (!pt->layers.size() && !pt->current_layer));
	
	if (!pt->current_layer)
	{
		sp_gerber_object_layer s = sp_gerber_object_layer(new gerber_object_layer);
		pt->layers.push_back(s);
		pt->current_layer = s;
	}
}

void cloneToNewLayerIfDrawn(Vector_Outp * pt)
{
	setupLayerIfNone(pt);
	if (pt->current_layer->draws.size())
	{
		sp_gerber_object_layer olds = pt->current_layer;
		sp_gerber_object_layer s = sp_gerber_object_layer(new gerber_object_layer);
		pt->layers.push_back(s);
		pt->current_layer = s;
		s->name = olds->name;
		s->polarity = olds->polarity;
		s->step_repeat = olds->step_repeat;
		
	}
}

bool handle_directive(struct GCODE_state * plot_state, const struct RS274X_Program::gcode_block & op, Vector_Outp * pt)
{
	assert(plot_state);
	assert(op.op == RS274X_Program::GCO_DIR);
	switch (op.gdd_data.dir)
	{
		case RS274X_Program::DIR_AS:
			DBG_MSG_PF("Error - axis setup not supported");
			return false;
			
		case RS274X_Program::DIR_FS:
			if (op.gdd_data.FS_P.ai == RS274X_Program::COORD_INC)
			{
				DBG_MSG_PF("Error - switching to incremental mode [not yet supported]");
				return false;
			}
			break;
			
		case RS274X_Program::DIR_MI:
			DBG_MSG_PF("Error - mirror image not supported");
			return false;
			
		case RS274X_Program::DIR_MO:
			plot_state->um = op.gdd_data.MO_P.um;
			break;
			
		case RS274X_Program::DIR_OF:
			DBG_MSG_PF("Error - offset not supported");
			return false;
		case RS274X_Program::DIR_SF:
			DBG_MSG_PF("Error - scale factor not supported");
			return false;
		case RS274X_Program::LY_KO:
			cloneToNewLayerIfDrawn(pt);
			break;
			
		case RS274X_Program::LY_LN:
			cloneToNewLayerIfDrawn(pt);
			DBG_MSG_PF("Setting layer name to %s", op.gdd_data.LN_P.name);
			pt->current_layer->name = op.gdd_data.LN_P.name;
			break;
			
		case RS274X_Program::LY_LP:
			cloneToNewLayerIfDrawn(pt);
			pt->current_layer->polarity = op.gdd_data.LP_P.lp;
			break;
			
		case RS274X_Program::LY_SR:
			cloneToNewLayerIfDrawn(pt);
			pt->current_layer->step_repeat.X = op.gdd_data.SR_P.X;
			pt->current_layer->step_repeat.X = op.gdd_data.SR_P.Y;
			pt->current_layer->step_repeat.x_step = op.gdd_data.SR_P.x_step;
			pt->current_layer->step_repeat.y_step = op.gdd_data.SR_P.y_step;
			break;
	}
	
	return true;
	
}

sp_Vector_Outp gcode_run(sp_RS274X_Program gerb)
{

	Vector_Outp * pt = new Vector_Outp();
		
	struct GCODE_state plot_state;
	bzero(&plot_state, sizeof(GCODE_state));

	plot_state.um = UNITMODE_IN;
	
	// HACK: some gerbers assume we start with AP 10.
	//plot_state.last_ap = 10;
	// TODO: Determine if this needs to be re-enabled.
	
	
	// Eagle doesn't seem to include this :/
	plot_state.G_op = 1;
	
	DBG_MSG_PF("Starting GCODE Virtual Machine");

	RS274X_Program::operations_list_t::const_iterator ci = 
		gerb->m_operations.begin();
	
	RS274X_Program::operations_list_t::const_iterator end = 
		gerb->m_operations.end();
	
	while (ci != end && (!plot_state.done))
	{
		const struct RS274X_Program::gcode_block & cur_op = *ci;
		switch (cur_op.op)
		{
			case RS274X_Program::GCO_G:
				setupLayerIfNone(pt);
				handle_G_op(&plot_state, cur_op, pt->current_layer);
				break;
				
			case RS274X_Program::GCO_M:
				if (!handle_M_op(&plot_state, cur_op))
					return sp_Vector_Outp();
				break;
				
			case RS274X_Program::GCO_D:
				if (!handle_D_op(&plot_state, cur_op))
					return sp_Vector_Outp();
					
				break;
				
			case RS274X_Program::GCO_X:
			case RS274X_Program::GCO_Y:
			case RS274X_Program::GCO_I:
			case RS274X_Program::GCO_J:
				handle_coord(&plot_state, cur_op);
				break;

			case RS274X_Program::GCO_END:
				setupLayerIfNone(pt);
				if (!handle_exec(&plot_state, gerb, pt->current_layer))
				{
					DBG_ERR_PF("Could not execute gcode block!");
					return sp_Vector_Outp();
				}
				break;

			case RS274X_Program::GCO_DIR:;
				if (!handle_directive(&plot_state, cur_op, pt))
				{
					DBG_ERR_PF("Could not handle directive %s", decode_gcode_directive_type(cur_op.gdd_data.dir));
					return sp_Vector_Outp();
				}
		}
		ci++;
	}
	DBG_MSG_PF("GCODE Virtual Machine Finished");

	return sp_Vector_Outp(pt);
}


