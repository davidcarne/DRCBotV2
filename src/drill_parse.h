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

#ifndef _DRILL_PARSE_H_
#define _DRILL_PARSE_H_

struct drill_hit {
	int tool;
	double x;
	double y;
};

struct drill_file {
	std::vector<double> drill_aps;
	std::list<struct drill_hit> hits;
};
struct drill_file * create_drill_file_rep_from_filename(char * filename);
void free_drill_file_rep(struct drill_file * rep);

#endif

