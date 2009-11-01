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
#include <gd.h>

#include "main.h"
#include "gerber_parse.h"
#include "gcode_interp.h"
#include "plot_vector.h"
#include "groupize.h"
#include "partitioning.h"
#include "drc.h"
#include "util.h"
// usage ./gerber_drc [-d DEBUG_LEVEL] -o [FILE_NAME] filename

enum debug_level_t debug_level = DEBUG_ERROR;



Vector_Outp *  load_file(char * filename)
{
	struct gerber_file * gerb = create_gerber_file_rep_from_filename(filename);
		
	if (!gerb)
	{
		printf("Parse Failed!");
		return NULL;
	}
	
	Vector_Outp * vf = gcode_run(gerb);
	if (!vf)
	{
		printf("GCode execution failed!\n");
		return NULL;
	}
	
	printf("Created %d polygons\n", vf->all.size());
	printf("Found %d errors\n", vf->errors.size());

	free_gerber_file_rep(gerb);
	return vf;
}


int main(int argc, char ** argv)
{
	bool small = false;
	// for color match
	srandom(time(NULL));
	
	plotOptions p;
	p.colorByRandom = false;
	
	char * outfilename = "test.png";
	while (true) {
		int opt = getopt(argc, argv,  "d:o:bcrs");
		
		if (opt == -1)
			break;

		switch (opt)
		{
			case 's':
				small = true;
				break;
	
			case 'b':
				p.showBoundingBoxes = true;
				break;
				
			case 'c':
				p.colorByGroups = true;
				break;
				
			
			case 'r':
				p.colorByRandom = true;
				break;

			case 'o':
			{
				outfilename = optarg;
			}
			break;
			case 'd':
			{
					
				char * debugparse = optarg;
				if (strcasecmp(debugparse, "none") == 0)
				{
					debug_level = DEBUG_NONE;
				} else if(strcasecmp(debugparse, "ERROR") == 0)
				{
					debug_level = DEBUG_ERROR;
				}else if(strcasecmp(debugparse, "WARN") == 0)
				{
					debug_level = DEBUG_WARN;
				}else if(strcasecmp(debugparse, "MSG") == 0)
				{
					debug_level = DEBUG_MSG;
				}else if(strcasecmp(debugparse, "VERBOSE") == 0)
				{
					debug_level = DEBUG_VERBOSE;
				} else {
					printf("Invalid value %s for -d\n", debugparse);
					return -1;
				}
			}
		}
		
	}

	if (optind == argc)
	{
		printf("Error - no files specified to parse!\n");
		return -1;
	}
	
	int nFiles = argc-optind;
	Vector_Outp ** v_dat = new Vector_Outp*[nFiles];
	printf("To load %d gerber files\n", nFiles);

	for (int i=0; i<nFiles; i++)
	{
		v_dat[i] = load_file(argv[optind + i]);
	}

	// Now calculate the global bounding rect
	Rect bounds = Rect();
	for (int i=0; i<nFiles; i++)
	{
		if (!v_dat[i])
			continue;

		std::set <GerbObj*> :: iterator it = v_dat[i]->all.begin();;
		
		for (;it!=v_dat[i]->all.end();it++)
			bounds.mergeBounds((*it)->getBounds());
	}
	printf("Board boundaries: ");
	bounds.printRect();	
	
	/************** Calculate a good bounds ************************/
	p.offX = - bounds.getStartPoint().x;
	p.offY = - bounds.getStartPoint().y;
	
	double maxScale = fmin(2000/bounds.getWidth(), 2000/bounds.getHeight());

	if (small)
		maxScale = fmin(238/bounds.getWidth(), 137/bounds.getHeight());
	
	p.scale =fmin(500,maxScale);
	
	
	p.rw = (int)(p.scale * bounds.getWidth());
	p.rh = (int)(p.scale * bounds.getHeight());
	
	p.colorByOVR = true;
	p.drawinverted = false;
	p.ovr = 0xB8;
	p.ovg = 0x73;
	p.ovb = 0x33;
	p.alpha = 0xFF;
	p.drawfilled = true;
	
	/************* Plot the vector image data *******************/
	gdImagePtr im = NULL;

	for (int i=0; i<nFiles; i++)
	{
		if (!v_dat[i])
			continue;

		switch (i)
		{
			// setup default colorings
			case 1:
				// Soldermask - need to hack this
				p.drawfilled = true;
				p.drawinverted = true;
				p.alpha = 128;
				p.ovr = 0;
				p.ovg = 128;
				p.ovb = 0;
				break;
				
			case 2:
				// Silkscreen
				
				p.alpha = 0xFF;
				p.drawfilled = true;
				p.drawinverted = false;
				p.ovr = 255;
				p.ovg = 255;
				p.ovb = 255;
				break;
		}
		if (im)
			plot_vector_onto(v_dat[i],p,im);
		else
			im = plot_vector(v_dat[i],p);
		if (im == NULL)
		{
			printf("Error plotting image!");
			return false;
		}
	}
	
	FILE * pngout = fopen(outfilename, "wb");
	if (!pngout)
	{
		perror("");
		return -1;
	}
  	gdImagePng(im, pngout);
 	fclose(pngout);
	gdImageDestroy(im);
	
	return 0;
}
