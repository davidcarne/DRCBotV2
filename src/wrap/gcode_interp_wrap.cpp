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
#include "gcode_interp.h"
#include "gerbobj.h"
#include "gerbobj_poly.h"
#include "gerbobj_line.h"
#include "render.h"


#include "boost/python.hpp"

#include "boost/python/suite/indexing/vector_indexing_suite.hpp"

#include "gerbobj_poly.h"

namespace bp = boost::python;

struct GerbObj_wrapper : GerbObj, bp::wrapper< GerbObj > {
	
    GerbObj_wrapper( )
    : GerbObj( )
	, bp::wrapper< GerbObj >(){
        // null constructor
		
    }
	
    virtual ::RenderPoly * createPolyData(  ){
        bp::override func_createPolyData = this->get_override( "createPolyData" );
        return func_createPolyData(  );
    }
	
    virtual ::Rect getBounds(  ){
        bp::override func_getBounds = this->get_override( "getBounds" );
        return func_getBounds(  );
    }
	
};


typedef std::list<sp_gerber_object_layer > gll_t;
typedef gll_t::const_iterator (gll_t::*goll_rci)(void) const;
typedef gll_t::const_reverse_iterator (gll_t::*goll_rrci)(void) const;


typedef std::list<sp_GerbObj> gerbobjlist_t;
typedef gerbobjlist_t::const_iterator (gerbobjlist_t::*gol_rci)(void) const;
typedef gerbobjlist_t::const_reverse_iterator (gerbobjlist_t::*gol_rrci)(void) const;

namespace bp = boost::python;


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(mergePointOverloads, mergePoint, 1, 2)


void gcodeInterpWrap(void)
{
	using namespace boost::python;
	
	def("runRS274XProgram", gcode_run);
	
	
    bp::class_< GerbObj_wrapper, boost::noncopyable >( "GerbObj", bp::no_init ).def( bp::init< >() )
	.def( 
		 "getPolyData"
		 , (::RenderPoly * ( ::GerbObj::* )(  ) )( &::GerbObj::getPolyData ), return_value_policy<reference_existing_object>())
	.def("getBounds",&GerbObj::getBounds);
    
	
    bp::class_< GerbObj_Poly, bp::bases< GerbObj > >( "GerbObj_Poly", bp::init< >() );
	
	bp::class_< GerbObj_Line, bp::bases< GerbObj > >( "GerbObj_Line", bp::init< >() )
	.def_readwrite("sx",&GerbObj_Line::sx)
	.def_readwrite("sy",&GerbObj_Line::sy)
	.def_readwrite("ex",&GerbObj_Line::ex)
	.def_readwrite("ey",&GerbObj_Line::ey)
	.def_readwrite("width",&GerbObj_Line::width)
	.def_readwrite("cx",&GerbObj_Line::cx)
	.def_readwrite("cy",&GerbObj_Line::cy);

	
	bp::register_ptr_to_python< boost::shared_ptr< GerbObj > >();
	
	
	
	bp::class_< gerber_object_layer, boost::noncopyable >( "gerber_object_layer", bp::no_init ).def( bp::init< >() )
	.def_readwrite("name", &gerber_object_layer::name)
	.def_readwrite("polarity", &gerber_object_layer::polarity)
	.def_readwrite("draws", &gerber_object_layer::draws);
	
	bp::register_ptr_to_python< boost::shared_ptr< gerber_object_layer > >();
	
	class_<gll_t>("gerb_object_layer_list", init<>())
	.def("__iter__", range(static_cast<goll_rci>(&gll_t::begin), 
						   static_cast<goll_rci>(&gll_t::end)))
	.def("__reversed__", range(static_cast<goll_rrci>(&gll_t::rbegin), 
							   static_cast<goll_rrci>(&gll_t::rend)))
	.def("__len__", &gll_t::size);
	
	
	
	
	class_<gerbobjlist_t>("gerbObjList", init<>())
		.def("__iter__", range(static_cast<gol_rci>(&gerbobjlist_t::begin), 
						   static_cast<gol_rci>(&gerbobjlist_t::end)))
		.def("__reversed__", range(static_cast<gol_rrci>(&gerbobjlist_t::rbegin), 
							static_cast<gol_rrci>(&gerbobjlist_t::rend)))
		.def("__len__", &gerbobjlist_t::size);
	
	
	class_<Vector_Outp, boost::shared_ptr<Vector_Outp> >("PolygonLayers", init<>())
	.def_readonly("layers",&Vector_Outp::layers)
	;
	
	bp::class_< RenderPoly >( "RenderPoly" )    
	.def_readwrite( "b", &RenderPoly::b )    
	.def_readwrite( "fillptx", &RenderPoly::fillptx )    
	.def_readwrite( "fillpty", &RenderPoly::fillpty )    
	.def_readwrite( "flag", &RenderPoly::flag )    
	.def_readwrite( "g", &RenderPoly::g )    
	.def_readwrite( "oid", &RenderPoly::oid )    
	.def_readwrite( "r", &RenderPoly::r )    
	.def_readwrite( "segs", &RenderPoly::segs );
	
	{ //::std::vector< point_line* >
        typedef bp::class_< std::vector< point_line* > > vector_less__point_line_ptr___greater__exposer_t;
        vector_less__point_line_ptr___greater__exposer_t vector_less__point_line_ptr___greater__exposer = vector_less__point_line_ptr___greater__exposer_t( "vector_less__point_line_ptr___greater_" );
        bp::scope vector_less__point_line_ptr___greater__scope( vector_less__point_line_ptr___greater__exposer );
        vector_less__point_line_ptr___greater__exposer.def( bp::vector_indexing_suite< ::std::vector< point_line* > >() );
    }
	
    { //::point_line
        typedef bp::class_< point_line > point_line_exposer_t;
        point_line_exposer_t point_line_exposer = point_line_exposer_t( "point_line" );
        bp::scope point_line_scope( point_line_exposer );
        bp::enum_< point_line::line_render_type_t>("line_render_type_t")
		.value("LR_STRAIGHT", point_line::LR_STRAIGHT)
		.value("LR_ARC", point_line::LR_ARC)
		.export_values()
		;
        point_line_exposer.def_readwrite( "cx", &point_line::cx );
        point_line_exposer.def_readwrite( "cy", &point_line::cy );
        point_line_exposer.def_readwrite( "lt", &point_line::lt );
        point_line_exposer.def_readwrite( "x", &point_line::x );
        point_line_exposer.def_readwrite( "y", &point_line::y );
    }
	
	bp::register_ptr_to_python< point_line * >();

	bp::class_< Point >( "Point", bp::init< double, double >(( bp::arg("x"), bp::arg("y") )) )    
	.def( bp::init< >() )    
	.def_readwrite( "x", &Point::x )    
	.def_readwrite( "y", &Point::y );
	
	// Auto-generated
    bp::class_< Rect >( "Rect", bp::init< >() )    
	.def( bp::init< double, double, double, double >(( bp::arg("x1"), bp::arg("y1"), bp::arg("x2"), bp::arg("y2") )) )    
	.def( 
		 "feather"
		 , (void ( ::Rect::* )( double ) )( &::Rect::feather )
		 , ( bp::arg("s") ) )    
	.def( 
		 "feather"
		 , (void ( ::Rect::* )( double,double ) )( &::Rect::feather )
		 , ( bp::arg("x"), bp::arg("y") ) )    
	.def( 
		 "getCWP1"
		 , (::Point ( ::Rect::* )(  ) const)( &::Rect::getCWP1 ) )    
	.def( 
		 "getCWP2"
		 , (::Point ( ::Rect::* )(  ) const)( &::Rect::getCWP2 ) )    
	.def( 
		 "getEndPoint"
		 , (::Point ( ::Rect::* )(  ) const)( &::Rect::getEndPoint ) )    
	.def( 
		 "getHeight"
		 , (double ( ::Rect::* )(  ) const)( &::Rect::getHeight ) )    
	.def( 
		 "getStartPoint"
		 , (::Point ( ::Rect::* )(  ) const)( &::Rect::getStartPoint ) )    
	.def( 
		 "getWidth"
		 , (double ( ::Rect::* )(  ) const)( &::Rect::getWidth ) )    
	.def( 
		 "intersectsWith"
		 , (bool ( ::Rect::* )( ::Rect const & ) )( &::Rect::intersectsWith )
		 , ( bp::arg("r") ) )    
	.def( 
		 "mergeBounds"
		 , (void ( ::Rect::* )( ::Rect const & ) )( &::Rect::mergeBounds )
		 , ( bp::arg("r") ) )    
	.def( 
		 "mergePoint"
		 , &::Rect::mergePoint, mergePointOverloads())    
	.def( 
		 "pointInRectClosed"
		 , (bool ( ::Rect::* )( ::Point ) const)( &::Rect::pointInRectClosed )
		 , ( bp::arg("a") ) )    
	.def( 
		 "pointInRectClosed"
		 , (bool ( ::Rect::* )( double,double ) const)( &::Rect::pointInRectClosed )
		 , ( bp::arg("x"), bp::arg("y") ) )    
	.def( 
		 "pointInRectOpen"
		 , (bool ( ::Rect::* )( double,double ) const)( &::Rect::pointInRectOpen )
		 , ( bp::arg("x"), bp::arg("y") ) )    
	.def( 
		 "printRect"
		 , (void ( ::Rect::* )(  ) const)( &::Rect::printRect ) );
}

