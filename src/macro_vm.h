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

#ifndef MACRO_VM
#define MACRO_VM

#include <list>
#include <stack>
#include <vector>

/* This VM draws from some of the ideas used in gerbv's macro parser,
 * but no code was copied, or used as a direct reference */

enum MACRO_OP_TYPE
{
	OP_NOP,	 // Never actually emitted. Used only during debugging
	OP_ADD,  // Add the top two numbers on the stack, leaving only the sum on the stack
	OP_SUB,  // Subtract the number on the top of the stack, from the number second from top
	OP_MUL, 
	OP_DIV,  // Divide the 2nd number on stack, by the number on top
	OP_PUSH, // Push a constant on the stack
	OP_FETCH,// Push a numbered variable
	OP_PRIM, // Emit a primitive
	OP_STORE // Top of stack into numbered variable
};

struct Macro_OP {	
	MACRO_OP_TYPE op;	// Operand
	int ival;			// Integer value of operand [if any]
	float fval;			// Floating point value of operand [if any]
};

class GerbObj_Poly;

class Macro_VM {

	public:
	GerbObj_Poly * execute(double * params, float x, float y);
	
	void addInstr(MACRO_OP_TYPE t, float fval, int ival)
	{
		struct Macro_OP o;
		o.op = t;
		o.fval = fval;
		o.ival = ival;
		code.push_back(o);
	}
	
	void print();
	private:
		bool renderPrim5(GerbObj_Poly * p, float x, float y);
		bool renderPrim4(GerbObj_Poly * p, float x, float y);
		bool renderPrim21(GerbObj_Poly * p, float x, float y);
		typedef std::vector<Macro_OP> code_type_t;
		typedef code_type_t::iterator i_code_type_t;
		
		code_type_t code;
		std::stack<float> mem_stack;


};

bool parse_macro(Macro_VM * vm, const char * str);

#endif

