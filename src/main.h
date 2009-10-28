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


#ifndef _MAIN_H_
#define _MAIN_H_

enum debug_level_t {

	DEBUG_NONE = 0,
	DEBUG_ERROR = 1,
	DEBUG_WARN = 2,
	DEBUG_MSG = 3,
	DEBUG_VERBOSE = 4

	};


extern enum debug_level_t debug_level;

#define _DBG_PF(lvl, ...) {if (debug_level >= lvl) { printf(__VA_ARGS__); printf("\n"); }}
#define DBG_ERR_PF(...) _DBG_PF(DEBUG_ERROR, __VA_ARGS__)
#define DBG_WARN_PF(...) _DBG_PF(DEBUG_WARN, __VA_ARGS__)
#define DBG_MSG_PF(...) _DBG_PF(DEBUG_MSG, __VA_ARGS__)
#define DBG_VERBOSE_PF(...) _DBG_PF(DEBUG_VERBOSE, __VA_ARGS__)

#define strndup(str,len) FIX_strndup(str,len)

char * FIX_strndup(char * str, size_t len);

#endif

