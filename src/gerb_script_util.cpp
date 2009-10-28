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
#include <unistd.h>
#include <stdlib.h>
#include "gcode_interp.h"
#include "main.h"
#include <gd.h>

debug_level_t debug_level;

void setDebugLevel(debug_level_t new_level)
{
	debug_level = new_level;
}

GerbObj_Line * cast_GerbObj_ToLine(GerbObj * v)
{
	return dynamic_cast<GerbObj_Line*>(v);
}


GerbObj_Poly * cast_GerbObj_ToPoly(GerbObj * v)
{
	return dynamic_cast<GerbObj_Poly*>(v);
}

int writePNGFile(char * file, gdImagePtr im)
{
	FILE * pngout = fopen(file, "wb");
	if (!pngout)
	{
		perror("");
		return -1;
	}
	gdImagePng(im, pngout);
	fclose(pngout);
	return 0;
}

