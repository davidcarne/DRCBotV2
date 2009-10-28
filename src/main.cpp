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

#include "util.h"

#include "main.h"
#include "gerber_parse.h"
#include "gcode_interp.h"
#include "plot_vector.h"
#include "groupize.h"
#include "partitioning.h"
#include "drc.h"
// usage ./gerber_drc [-d DEBUG_LEVEL] -o [FILE_NAME] filename

enum debug_level_t debug_level = DEBUG_ERROR;

#if 0
int main(int argc, char ** argv)
{
	// for color match
	srandom(time(NULL));
	
	plotOptions p;
	p.colorByRandom = false;
	p.drawinverted = false;
	p.drawfilled = true;
	p.colorByOVR = false;
	char * outfilename = "test.png";
	while (true) {
		int opt = getopt(argc, argv,  "d:o:bcr");
		
		if (opt == -1)
			break;

		switch (opt)
		{
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
	struct gerber_file * gerb = create_gerber_file_rep_from_filename(argv[optind]);
		
	if (!gerb)
	{
		printf("Parse Failed!");
		return -1;
	}
	
	Vector_Outp * vf = gcode_run(gerb);
	if (!vf)
	{
		printf("GCode execution failed!\n");
		return -1;
	}
	
	printf("Created %d polygons\n", vf->all.size());
	printf("Found %d errors\n", vf->errors.size());
	
	

	free_gerber_file_rep(gerb);
	
	
	
	printf("Partitioning....\n");
	makeEndPartitions(vf);
	
	printf("Grouping...\n");
	groupize(vf);

	
	Rect bounds = Rect();
	std::set <GerbObj*> :: iterator it = vf->all.begin();;
	
	for (;it!=vf->all.end();it++)
		bounds.mergeBounds((*it)->getBounds());
	
	
	printf("Board boundaries: ");
	bounds.printRect();
	
	
	printf("Found %d groups\n",  vf->groups.size());
	
	struct drcSettings s;
	initDRC(&s);
	bool drc_ok = doDRC(vf,&s,true);
	
	
	
	
	/************* Plot the vector *******************/
	
	p.offX = - bounds.getStartPoint().x;
	p.offY = - bounds.getStartPoint().y;
	
	double maxScale = fmin(2000/bounds.getWidth(), 2000/bounds.getHeight());
	
	p.scale =fmin(500,maxScale);
	
	
	p.rw = (int)(p.scale * bounds.getWidth());
	p.rh = (int)(p.scale * bounds.getHeight());
	
	gdImagePtr im = plot_vector(vf,p);
	if (im == NULL)
	{
		printf("Error plotting image!");
		return false;
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
	
	if (!drc_ok)
		return 255;
	return 0;
}
#endif