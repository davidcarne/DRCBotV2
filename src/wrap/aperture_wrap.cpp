
#include <boost/python.hpp>
#include "wrap_fns.h"
#include "main.h"
#include "gerber_parse.h"

static boost::python::object aperatureValueHelper(RS274X_Program::aperture & blk)
{
	using namespace boost::python;
	
	switch (blk.type)
	{
		case RS274X_Program::AP_CIRCLE:
			return object(blk.circle_p);
		case RS274X_Program::AP_RECT:
			return object(blk.rect_p);
		case RS274X_Program::AP_OVAL:
			return object(blk.oval_p);
		case RS274X_Program::AP_POLY:
			return object(blk.poly_p);
		case RS274X_Program::AP_T:
			return object();
		case RS274X_Program::AP_MACRO:
			return object(blk.macro_p);
	}
	
	return object();
}


typedef const RS274X_Program::aperture_map_t r274apm_t;
typedef r274apm_t::const_iterator (r274apm_t::*rci)(void) const;
typedef r274apm_t::const_reverse_iterator (r274apm_t::*rrci)(void) const;

void aperture_wrap(void)
{
	using namespace boost::python;
	
	enum_<RS274X_Program::aperture_type>("aperture_type")
	.value("AP_CIRCLE", RS274X_Program::AP_CIRCLE)
	.value("AP_RECT", RS274X_Program::AP_RECT)
	.value("AP_OVAL", RS274X_Program::AP_OVAL)
	.value("AP_POLY", RS274X_Program::AP_POLY)
	.value("AP_T", RS274X_Program::AP_T)
	.value("AP_MACRO", RS274X_Program::AP_MACRO)
	;
	
	class_<RS274X_Program::aperture::circle_ap_t>("circle_ap_t", init<>())
	.def_readwrite("OD", &RS274X_Program::aperture::circle_ap_t::OD)
	.def_readwrite("XAHD", &RS274X_Program::aperture::circle_ap_t::XAHD)
	.def_readwrite("YAHD", &RS274X_Program::aperture::circle_ap_t::YAHD)
	;
	
	
	class_<RS274X_Program::aperture::rect_ap_t>("rect_ap_t", init<>())
	.def_readwrite("XAD", &RS274X_Program::aperture::rect_ap_t::XAD)
	.def_readwrite("YAD", &RS274X_Program::aperture::rect_ap_t::YAD)
	.def_readwrite("XAHD", &RS274X_Program::aperture::rect_ap_t::XAHD)
	.def_readwrite("YAHD", &RS274X_Program::aperture::rect_ap_t::YAHD)
	;
	
	
	class_<RS274X_Program::aperture::oval_ap_t>("oval_ap_t", init<>())
	.def_readwrite("XAD", &RS274X_Program::aperture::oval_ap_t::XAD)
	.def_readwrite("YAD", &RS274X_Program::aperture::oval_ap_t::YAD)
	.def_readwrite("XAHD", &RS274X_Program::aperture::oval_ap_t::XAHD)
	.def_readwrite("YAHD", &RS274X_Program::aperture::oval_ap_t::YAHD)
	;
	
	class_<RS274X_Program::aperture::poly_ap_t>("poly_ap_t", init<>())
	.def_readwrite("OD", &RS274X_Program::aperture::poly_ap_t::OD)
	.def_readwrite("NS", &RS274X_Program::aperture::poly_ap_t::NS)
	.def_readwrite("DR", &RS274X_Program::aperture::poly_ap_t::DR)
	.def_readwrite("XAHD", &RS274X_Program::aperture::poly_ap_t::XAHD)
	.def_readwrite("YAHD", &RS274X_Program::aperture::poly_ap_t::YAHD)
	;

	class_<RS274X_Program::aperture::macro_ap_t>("macro_ap_t", init<>())
	.def_readwrite("name", &RS274X_Program::aperture::macro_ap_t::macro_name)
	;
	
	class_<RS274X_Program::aperture>("aperature", init<>())
	.def_readonly("ap_type", &RS274X_Program::aperture::type)
	.add_property("value", aperatureValueHelper);
	
	class_<RS274X_Program::aperture_map_t>("aperture_map_t", init<>())
	.def("__iteritems__", range(static_cast<rci>(&r274apm_t::begin), 
						   static_cast<rci>(&r274apm_t::end)))
	/*.def("__reversed__", range(static_cast<rrci>(&r274apm_t::rbegin), 
							   static_cast<rrci>(&r274apm_t::rend)))*/
	.def("__len__", &r274apm_t::size);

}

