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

#include <vector>
#include <list>

#include <stdlib.h>
#include <stdio.h>

#include "drill_parse.h"

struct drill_parse_mode {
	bool mode_cfg;
	bool parse_more;
	bool units_metric;
	
	enum {
		P_LZ,
		P_TZ, 
		P_FLOAT
	} coord_mode;
	
	int tool
};
static bool parse_m_cmd(struct drill_file * rep, struct drill_parse_mode *m, char * l)
{
	if (strlen(l) < 3)
		return false;
	
	int mcmd = atoi(l+m);
	switch (mcmd)
	{
		case 72:
		case 48:
		case 30:
			
		default:
			printf("Unsupported drill command, ignoring\n");
			return false;
	}

}
static bool parse_drill_line(struct drill_file * rep, struct drill_parse_mode *m, char * l)
{
	// Check if we're going from config to mode
	if (strcmp(l,"%") == 0)
	{
		m->mode_cfg = false;
		return true;
	}
	
	switch (l[0])
	{
		case 'M':
			return parse_m_cmd(rep,m,l);
		case 'T':
			if (
		case 'X':
	}

}

struct drill_file * create_drill_file_rep_from_filename(char * filename)
{
	struct drill_parse_mode m;
	m.mode_cfg = true;
	m.parse_more = true;
	FILE * f = fopen(filename,"r");
	if (!f)
	{
		perror("File");
		return NULL;
	}
	
	
	struct drill_file * rep = new drill_file;

	// initialize
	rep->drill_aps = std::vector<double>();
	rep->list = std::list<struct drill_hit>();

	
	while (!feof(f) && m.parse_more)
	{	
		char buf[1024];
		fgets(f,buf,1023);
		parse_drill_line(rep,m,buf);
	}
	fclose(f);
	return rep;
}
void free_drill_file_rep(struct drill_file * rep)
{
	delete rep;
}