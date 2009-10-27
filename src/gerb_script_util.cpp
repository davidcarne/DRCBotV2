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