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
#include <string.h>

/** One standard library used during the development of this
 *  program had a bug in the following two functions
 *  Created our own as a workaround
 */
int  strnlen(char * str, size_t n)
{
	size_t i = 0;
	while (*str++ && i++ < n);
	return i;
}

char * FIX_strndup(char * str, size_t n)
{
  size_t len = strnlen (str, n);
  char *copy = (char*)malloc (len + 1);
  if (copy)
    {
      memcpy (copy, str, len);
      copy[len] = '\0';
    }
  return copy;
}
