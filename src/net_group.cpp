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

#include "gcode_interp.h"
#include <math.h>
static int current_set_id=0;
net_group * new_net_group(Vector_Outp * f)
{
	net_group * p= new net_group(current_set_id++);
	f->groups.insert(p);
	
	p->r = 0.0;// rand() / (float)RAND_MAX/2 +0.5;
	p->g = 1.0;//rand() / (float)RAND_MAX/2 +0.5;
	p->b = 0.0;//rand() / (float)RAND_MAX/2 +0.5;
		
	return p;
}

net_group::net_group(int id)
{
	set_id = id;
}

void group_together(Vector_Outp * f, GerbObj * a, GerbObj * b)
{
	if (a->getOwner() && !b->getOwner() || a->getOwner() && a->getOwner()->getSize() < b->getOwner()->getSize())
	{
		add_to_group(f,a->getOwner(),b);
		return;
	} else if (b->getOwner())
	{
		add_to_group(f,b->getOwner(),a);
		return;
	}
	
	net_group * n = new_net_group(f);
	add_to_group(f,n,a);
	add_to_group(f,n,b);
}
static int set_merges = 0;
void add_to_group(Vector_Outp * f, net_group * n, GerbObj * o)
{
	if (o->owner == n)
		return;
		
	// If it is already a member of a group, we need to remove the group from the output
	// merge all group members into the target group and delete the original group
	if (o->owner)
	{
		// Iterate through the objects
		net_group * s = o->owner;
		net_group::i_gObjSet_t start = s->start();
		net_group::i_gObjSet_t end = s->end();
		net_group::i_gObjSet_t i;
		
		// point them to the target group
		for (i=start; i!= end; i++)
		{
			// Point it to the new group
			GerbObj * cObj = *i;
			cObj->owner = n;
			
			// Add it to the new group
			//n->members.insert(cObj);
			
			// Remove it from the original group
			//s->members.erase(i);
		}
		
		n->members.insert(s->members.begin(), s->members.end());
		
		// we no longer need the group, delete it
		delete s;
		f->groups.erase(s);
	} else {
		// Otherwise, just point both ways
		n->members.insert(o);
		o->owner = n;
	}
}


	
net_group::i_gObjSet_t net_group::start(void)
{
	return members.begin();
}

net_group::i_gObjSet_t net_group::end(void)
{
	return members.end();
}
