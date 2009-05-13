
//#define BOOST_SPIRIT_DEBUG

#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/spirit/core.hpp>
#include <boost/spirit/tree/ast.hpp>

#include <boost/spirit/tree/parse_tree.hpp>
#include <boost/spirit/tree/ast.hpp>
#include <boost/spirit/tree/tree_to_xml.hpp>
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
            //  Start grammar definition
            value     =   leaf_node_d[ real_p ];
			
			preset    =   no_node_d[ch_p('$')] >> int_p ;
			
            negation      =   (value | preset)
                        |   (root_node_d[ch_p('-')] >> negation);

            term        =   negation >>
                            *(  (root_node_d[as_lower_d[ch_p('x')]] >> negation)
                              | (root_node_d[ch_p('/')] >> negation)
                            );

            expression  =   term >>
                            *(  (root_node_d[ch_p('+')] >> term)
                              | (root_node_d[ch_p('-')] >> term)
                            );
							
			prim = int_p >> !(no_node_d[ch_p(',')] >> infix_node_d[expression >> *(ch_p(',') >> expression)]) >> no_node_d[*ch_p(',')];
			
			assign = preset >> no_node_d[ch_p('=')] >> expression;
			
			block = prim | assign;
            //  End grammar definition

            // turn on the debugging info.
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

int strtol_b(std::vector<char>::iterator start, std::vector<char>::iterator end)
{
		return strtol(std::string(start,end).c_str(),NULL,10);
}

float strtof_b(std::vector<char>::iterator start, std::vector<char>::iterator end)
{
		return strtof(std::string(start,end).c_str(),NULL);
}

void eval_expression(iter_t const& i, Macro_VM * vm)
{
	if (i->value.id() == macroblock::assignID)
	{
		// That will generate code to calculate value
		eval_expression(i->children.begin()+1,vm);
		iter_t const& pres = i->children.begin();
		//printf("STORE %s\n", std::string(pres->value.begin(), pres->value.end()).c_str());
		
		vm->addInstr(OP_STORE,0,strtol_b(pres->value.begin(), pres->value.end()));
		
	} else if (i->value.id() == macroblock::primID)
	{
	
		iter_t t = i->children.begin()+1;
		for (; t != i->children.end(); t++)
			eval_expression(t,vm);
			
		iter_t const& pres = i->children.begin();
		//printf("PRIM %s\n", std::string(pres->value.begin(), pres->value.end()).c_str());
		vm->addInstr(OP_PRIM,0,strtol_b(pres->value.begin(), pres->value.end()));
		
	} else if (i->value.id() == macroblock::expressionID) {
		iter_t t = i->children.begin();
		for (; t != i->children.end(); t++)
			eval_expression(t,vm);
			
		if (*(i->value.begin()) == '+')
		{
			vm->addInstr(OP_ADD,0,0);
			//printf("ADD\n");
		}
		else
		{
			
			vm->addInstr(OP_SUB,0,0);
			//printf("SUB\n");
		}
	} else if (i->value.id() == macroblock::negationID) {
	
		// we generate two instructions to push a 0 before a negation clause then sub after the contents
		//printf("PUSH 0");
		vm->addInstr(OP_PUSH,0,0);
		eval_expression(i->children.begin(),vm);
		//printf("SUB");
		vm->addInstr(OP_SUB,0,0);
			
	} else if (i->value.id() == macroblock::termID) {
		iter_t t = i->children.begin();
		for (; t != i->children.end(); t++)
			eval_expression(t,vm);
			
		if (tolower(*(i->value.begin())) == 'x')
		{
			//printf("MUL\n");
			vm->addInstr(OP_MUL,0,0);
		}else{
			//printf("DIV\n");
			vm->addInstr(OP_DIV,0,0);
		
		}
	} else if (i->value.id() == macroblock::presetID) {
		//printf("FETCH %s\n", std::string(i->value.begin(), i->value.end()).c_str());
			vm->addInstr(OP_FETCH,0,strtol_b(i->value.begin(), i->value.end()));
	} else if (i->value.id() == macroblock::valueID) {
		//printf("PUSH %s\n", std::string(i->value.begin(), i->value.end()).c_str());
			vm->addInstr(OP_PUSH,strtof_b(i->value.begin(), i->value.end()),0);
	} else {
		//printf("PARSE ERROR\n");
	}
}

////////////////////////////////////////////////////////////////////////////
bool parse_macro(Macro_VM * vm, const char * str)
{
	if (!strlen(str))
		return true;
	
	
    macroblock mb_parser;
	
	tree_parse_info<> info = ast_parse(str, mb_parser, boost::spirit::space_p );
	if (info.full)
	{
		evaluate(info,vm);
		vm->print();
		return true;
	}
	
	return false;
}
/*
int
main()
{

	Macro_VM * vm = new Macro_VM();
    string str;
    while (getline(cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        if (parse_macro(vm, str.c_str()))
        {
			printf("Parse suceeded\n");
		}
        else
        {
            cout << "parsing failed\n";
        }
    }

    cout << "Bye... :-) \n\n";
    return 0;
}*/



