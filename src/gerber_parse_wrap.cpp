
#include <boost/python.hpp>
#include "gerber_parse.h"
#include "main.h"
#include "gerb_script_util.h"

boost::python::object gcodeBlockValueHelper(RS274X_Program::gcode_block blk)
{
	using namespace boost::python;

	switch (blk.op)
	{
		case RS274X_Program::GCO_G:
		case RS274X_Program::GCO_M:
		case RS274X_Program::GCO_D:
			return object(blk.int_data);
			
		case RS274X_Program::GCO_X:
		case RS274X_Program::GCO_Y:
		case RS274X_Program::GCO_I:
		case RS274X_Program::GCO_J:
			return object(blk.dbl_data);
		
		case RS274X_Program::GCO_DIR:
		case RS274X_Program::GCO_END:
			return object();
	}
}

typedef const RS274X_Program::operations_list_t r274opl_t;
typedef r274opl_t::const_iterator (r274opl_t::*rci)(void) const;
typedef r274opl_t::const_reverse_iterator (r274opl_t::*rrci)(void) const;

BOOST_PYTHON_MODULE(_gerber_utils)
{
	using namespace boost::python;

	//def("vc", gcodeBlockValueHelper);
	
	enum_<debug_level_t>("debug_level_t")
    .value("DEBUG_NONE", DEBUG_NONE)
    .value("DEBUG_ERROR", DEBUG_NONE)
    .value("DEBUG_WARN", DEBUG_WARN)
    .value("DEBUG_MSG", DEBUG_MSG)
    .value("DEBUG_VERBOSE", DEBUG_VERBOSE)
    ;
	
	enum_<RS274X_Program::gcode_op_type>("gcode_op_type")
	.value("GCO_G",RS274X_Program::GCO_G)
	.value("GCO_M",RS274X_Program::GCO_M)
	.value("GCO_D",RS274X_Program::GCO_D)
	.value("GCO_X",RS274X_Program::GCO_X)
	.value("GCO_Y",RS274X_Program::GCO_Y)
	.value("GCO_I",RS274X_Program::GCO_I)
	.value("GCO_J",RS274X_Program::GCO_J)
	.value("GCO_DIR",RS274X_Program::GCO_DIR)
	.value("GCO_END",RS274X_Program::GCO_END)
	;
	
	class_<RS274X_Program::gcode_block>("gcode_block", init<>())
	.def_readonly("optype",&RS274X_Program::gcode_block::op)
	.add_property("value", gcodeBlockValueHelper)
	;
	
	class_<RS274X_Program::operations_list_t>("operations_list_t", init<>())
	.def("__iter__", range(static_cast<rci>(&r274opl_t::begin), 
						   static_cast<rci>(&r274opl_t::end)))
	.def("__reversed__", range(static_cast<rrci>(&r274opl_t::rbegin), 
							   static_cast<rrci>(&r274opl_t::rend)))
	.def("__len__", &r274opl_t::size);
	
	
	class_<RS274X_Program::aperture_map_t>("aperture_map_t", init<>());
	
	class_<RS274X_Program, boost::shared_ptr<RS274X_Program> >("RS274X_Program",init<>())
		.def_readonly("operations", &RS274X_Program::m_operations)
		.def_readonly("apertures", &RS274X_Program::m_ap_map);
		

    def("parseFile", parseRS274X);
	def("setDebugLevel",setDebugLevel);
}