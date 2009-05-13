// loosely based off gerbv's macro parser
#include <list>
#include <stack>
#include <vector>
#ifndef MACRO_VM
#define MACRO_VM
enum MACRO_OP_TYPE
{
	OP_NOP,
	OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_PUSH,
	OP_FETCH,
	OP_PRIM,
	OP_STORE
};

struct Macro_OP {
	
	MACRO_OP_TYPE op;
	int ival;
	float fval;	
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
		//printf("Add %d %f %d\n",o.op, o.fval, o.ival);
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

