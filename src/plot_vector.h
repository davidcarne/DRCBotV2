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

#ifndef _PLOT_VECTOR_H_
#define _PLOT_VECTOR_H_

#include <gd.h>

class Vector_Outp;

class plotOptions {
	public:
	plotOptions(){showBoundingBoxes = false; colorByGroups = false;};
	
	bool showBoundingBoxes;
	bool colorByGroups;
	bool colorByRandom;
	bool colorByOVR;

	unsigned char ovr,ovg,ovb;
	
	bool drawfilled;
	bool drawinverted;
	int alpha;

	double scale;
	double offX;
	double offY;
	int rw,rh;

};
gdImagePtr plot_vector(Vector_Outp * vf,  plotOptions & p);
void plot_vector_onto(Vector_Outp * vf,  plotOptions & p, gdImagePtr );


#endif

