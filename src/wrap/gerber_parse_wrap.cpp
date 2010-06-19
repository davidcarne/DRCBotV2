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


#include <boost/python.hpp>
#include "wrap_fns.h"
#include "main.h"
#include "gerb_script_util.h"
#include "gerber_parse.h"

static boost::python::object gcodeBlockValueHelper(RS274X_Program::gcode_block blk)
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
	
	return object();
}

typedef const RS274X_Program::operations_list_t r274opl_t;
typedef r274opl_t::const_iterator (r274opl_t::*rci)(void) const;
typedef r274opl_t::const_reverse_iterator (r274opl_t::*rrci)(void) const;

void gerberParserWrap()
{
	using namespace boost::python;

	
	enum_<RS274X_Program::image_param_polarity>("image_param_polarity")
	.value("IP_POS", RS274X_Program::IP_NEG)
	.value("IP_NEG", RS274X_Program::IP_NEG)
	;
	
	enum_<RS274X_Program::image_param_justify>("image_param_justify")
	.value("IJ_LEFT", RS274X_Program::IJ_LEFT)
	.value("IJ_CENTER", RS274X_Program::IJ_CENTER)
	;
	
	enum_<RS274X_Program::image_param_rotate>("image_param_rotate")
	.value("IR_0", RS274X_Program::IR_0)
	.value("IR_90", RS274X_Program::IR_90)
	.value("IR_180", RS274X_Program::IR_180)
	.value("IR_270", RS274X_Program::IR_270)
	;
	
	class_<RS274X_Program::rs274x_image_param>("rs274x_image_param", init<>())
	.def_readonly("IJ_set", &RS274X_Program::rs274x_image_param::IJ_set)
	.def_readonly("IJ_justify_A", &RS274X_Program::rs274x_image_param::IJ_justify_A)
	.def_readonly("IJ_justify_B", &RS274X_Program::rs274x_image_param::IJ_justify_B)
	.def_readonly("IJ_offset_A", &RS274X_Program::rs274x_image_param::IJ_offset_A)
	.def_readonly("IJ_offset_B", &RS274X_Program::rs274x_image_param::IJ_offset_B)
	.def_readonly("IN_set", &RS274X_Program::rs274x_image_param::IN_set)
	.def_readonly("IN_value", &RS274X_Program::rs274x_image_param::IN_value)
	.def_readonly("IO_set", &RS274X_Program::rs274x_image_param::IO_set)
	.def_readonly("IO_offset_A", &RS274X_Program::rs274x_image_param::IO_offset_A)
	.def_readonly("IO_offset_B", &RS274X_Program::rs274x_image_param::IO_offset_B)
	.def_readonly("IP_set", &RS274X_Program::rs274x_image_param::IP_set)
	.def_readonly("IP_polarity", &RS274X_Program::rs274x_image_param::IP_polarity)
	.def_readonly("IR_set", &RS274X_Program::rs274x_image_param::IR_set)
	.def_readonly("IR_rotate", &RS274X_Program::rs274x_image_param::IR_rotate)
	.def_readonly("PF_set", &RS274X_Program::rs274x_image_param::PF_set)
	.def_readonly("PF_value", &RS274X_Program::rs274x_image_param::PF_value)
	;
	
	
	enum_<RS274X_Program::layer_polar_t>("layer_polar_t")
    .value("LP_C", RS274X_Program::LP_C)
    .value("LP_D", RS274X_Program::LP_D)
    ;
	
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
	
	
	class_<RS274X_Program, boost::shared_ptr<RS274X_Program> >("RS274X_Program",init<>())
		.def_readonly("operations", &RS274X_Program::m_operations)
		.def_readonly("apertures", &RS274X_Program::m_ap_map);
		

    def("parseFile", parseRS274X);
	def("setDebugLevel",setDebugLevel);
}

