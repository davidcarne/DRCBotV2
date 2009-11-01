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

#ifndef _NET_GROUP_H_
#define _NET_GROUP_H_

/********************* net_group *********************/
net_group * new_net_group(Vector_Outp * f);
void group_together(Vector_Outp * f, GerbObj * a, GerbObj * b);

class net_group {
	friend void add_to_group(Vector_Outp * f, net_group * n, GerbObj * o);
	friend net_group * new_net_group(Vector_Outp * f);
	
public:
	
	typedef std::set<GerbObj*> gObjSet_t;
	typedef gObjSet_t::iterator i_gObjSet_t;
	
	i_gObjSet_t start(void);
	i_gObjSet_t end(void);
	
	float r,g,b;
	int getSize() {return members.size();};
	int getID() {return set_id;};
protected:
	net_group(int id);
private:
	int set_id;
	std::set <GerbObj*> members;
};

#endif

