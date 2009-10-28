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

/* This VM draws from some of the ideas used in gerbv's macro parser,
 * but no code was copied, or used as a direct reference */

#define BOOST_SPIRIT_USE_OLD_NAMESPACE

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_push_back_actor.hpp>
#include <boost/spirit/include/classic_ast.hpp>

#include <boost/spirit/include/classic_parse_tree.hpp>
#include <boost/spirit/include/classic_tree_to_xml.hpp>

#include <iostream>
#include <vector>
#include <string>
#include <iostream>
#include <stack>
#include <functional>
#include <string>
#include <cassert>
#include <ctype.h>
#include "macro_vm.h"
//#define BOOST_SPIRIT_DUMP_PARSETREE_AS_XML

#if defined(BOOST_SPIRIT_DUMP_PARSETREE_AS_XML)
#include <map>
#endif

using namespace boost::spirit;

using namespace std;

/* Boost Spirit grammar for parsing of macros
 *
 */
struct macroblock : public grammar<macroblock>
{
    static const int valueID = 1;
    static const int negationID = 2;
    static const int termID = 3;
    static const int expressionID = 4;
    static const int presetID = 5;
	static const int primID = 6;
	static const int assignID = 7;
	static const int blockID = 8;
	
    template <typename ScannerT>
    struct definition
    {
        definition(macroblock const& /*self*/)
        {
            // Any real number. ex: 3.1415
            value     =   leaf_node_d[ real_p ];
			
			// A Preset. ex: $3
			preset    =   no_node_d[ch_p('$')] >> int_p ;
			
			// Negation ex: -$3
            negation      =   (value | preset)
                        |   (root_node_d[ch_p('-')] >> negation);
			
			// ex: $3 * 2 or 3 / -$2
            term        =   negation >>
                            *(  (root_node_d[as_lower_d[ch_p('x')]] >> negation)
                              | (root_node_d[ch_p('/')] >> negation)
                            );
			
			// ex: $3 * 2 - 1
            expression  =   term >>
                            *(  (root_node_d[ch_p('+')] >> term)
                              | (root_node_d[ch_p('-')] >> term)
                            );
			
			prim = int_p >> !(no_node_d[ch_p(',')] >> infix_node_d[expression >> *(ch_p(',') >> expression)]) >> no_node_d[*ch_p(',')];
			
			assign = preset >> no_node_d[ch_p('=')] >> expression;
			
			block = prim | assign;
			
            BOOST_SPIRIT_DEBUG_RULE(value);
            BOOST_SPIRIT_DEBUG_RULE(negation);
            BOOST_SPIRIT_DEBUG_RULE(term);
            BOOST_SPIRIT_DEBUG_RULE(expression);
		    BOOST_SPIRIT_DEBUG_RULE(preset);
        	
            BOOST_SPIRIT_DEBUG_RULE(prim);
            BOOST_SPIRIT_DEBUG_RULE(block);
            BOOST_SPIRIT_DEBUG_RULE(assign);
        }

        rule<ScannerT, parser_context<>, parser_tag<expressionID> >   expression;
        rule<ScannerT, parser_context<>, parser_tag<termID> >         term;
        rule<ScannerT, parser_context<>, parser_tag<negationID> >       negation;
        rule<ScannerT, parser_context<>, parser_tag<valueID> >      value;
        rule<ScannerT, parser_context<>, parser_tag<presetID> >      preset;
        rule<ScannerT, parser_context<>, parser_tag<primID> >      prim;
        rule<ScannerT, parser_context<>, parser_tag<assignID> >      assign;

        rule<ScannerT, parser_context<>, parser_tag<blockID> >      block;
		
        rule<ScannerT, parser_context<>, parser_tag<blockID> > const&
        start() const { return block; }
    };
};

typedef char const*         iterator_t;
typedef tree_match<iterator_t> parse_tree_match_t;
typedef parse_tree_match_t::tree_iterator iter_t;

////////////////////////////////////////////////////////////////////////////

void eval_expression(iter_t const& i, Macro_VM * vm);

void evaluate(tree_parse_info<> & info, Macro_VM *vm)
{
	eval_expression(info.trees.begin(),vm);
}

/*
 * Convert a string to a long given a start and end iterator
 */
int strtol_b(std::vector<char>::iterator start, std::vector<char>::iterator end)
{
		return strtol(std::string(start,end).c_str(),NULL,10);
}

/*
 * Convert a string to a float given a start and end iterator
 */
float strtof_b(std::vector<char>::iterator start, std::vector<char>::iterator end)
{
		return strtof(std::string(start,end).c_str(),NULL);
}

/* 
 * Recursive bytecode generator. This should be commented more
 */
void eval_expression(iter_t const& i, Macro_VM * vm)
{
	// $1 = [..expr..]
	if (i->value.id() == macroblock::assignID)
	{
		// Generate code to calculate the value of the expression
		eval_expression(i->children.begin()+1,vm);
		
		iter_t const& pres = i->children.begin();
		
		// And an op to store it in the preset
		vm->addInstr(OP_STORE,0,strtol_b(pres->value.begin(), pres->value.end()));
		
	} 
	else if (i->value.id() == macroblock::primID) {
	
		iter_t t = i->children.begin()+1;
		for (; t != i->children.end(); t++)
			eval_expression(t,vm);
			
		iter_t const& pres = i->children.begin();
		vm->addInstr(OP_PRIM,0,strtol_b(pres->value.begin(), pres->value.end()));
		
	}
	else if (i->value.id() == macroblock::expressionID) {
		// Evaluate all children
		iter_t t = i->children.begin();
		for (; t != i->children.end(); t++)
			eval_expression(t,vm);
		
		// Then, ops to combine the children
		if (*(i->value.begin()) == '+')
			vm->addInstr(OP_ADD,0,0);
		else
			vm->addInstr(OP_SUB,0,0);
		
	} else if (i->value.id() == macroblock::negationID) {
		// Implement negation by PUSH 0; [clause]; SUB
		vm->addInstr(OP_PUSH,0,0);
		eval_expression(i->children.begin(),vm);
		vm->addInstr(OP_SUB,0,0);
			
	} else if (i->value.id() == macroblock::termID) {
		// Evaluate all children
		iter_t t = i->children.begin();
		for (; t != i->children.end(); t++)
			eval_expression(t,vm);
		
		// Then, ops to combine the children
		if (tolower(*(i->value.begin())) == 'x')
			vm->addInstr(OP_MUL,0,0);
		else
			vm->addInstr(OP_DIV,0,0);
		
	} else if (i->value.id() == macroblock::presetID) {
		// Presets are just a FETCH, integer value being preset #
		vm->addInstr(OP_FETCH,0,strtol_b(i->value.begin(), i->value.end()));
	} else if (i->value.id() == macroblock::valueID) {
		// constants are just a PUSH, float value being the value of the constant
		vm->addInstr(OP_PUSH,strtof_b(i->value.begin(), i->value.end()),0);
	} else {
		// No code required for any other tokens
	}
}

////////////////////////////////////////////////////////////////////////////
bool parse_macro(Macro_VM * vm, const char * str)
{
	if (!strlen(str))
		return true;
	
    macroblock mb_parser;
	
	// Create an abstract syntax tree that we later use for our recursive descent parser
	tree_parse_info<> info = ast_parse(str, mb_parser, boost::spirit::space_p );
	
	// The parse info tree being "full" indicates that the macro was fully parsed sucessfully
	if (info.full)
	{
		// Generate bytecode
		evaluate(info,vm);
		return true;
	}
	
	return false;
}
