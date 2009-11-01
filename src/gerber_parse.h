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

#ifndef _GERBER_PARSE_H_
#define _GERBER_PARSE_H_

#include <boost/shared_ptr.hpp>

#include <map>
#include <string>
#include <vector>
#include "macro_vm.h"
#include "types.h"

/* 
 * RS274X description
 *
 * RS274X is composed of two types of data:
 * 	RS274X Parameters
 * 	RS274D Codes
 *
 * Parameters are divided into four categories:
 *	Directive
 *	Image
 *	Aperture
 *	Layer
 *
 * Image + Aperture parameters can be handled once per file
 * Aperture params are not changed after setup and image params
 * are only set one per file [Used for entire image]
 *
 * Layer parameters need to be passed to the interpreter "as they happen"
 * This is because they define a new layer with new settings
 *
 * Directives are a special case. Some are used during the parse stage to
 * normalize numbers, but all are passed through to the interpreter as they
 * happen. Most are ignored, but they may be utilized by the interpreter if
 * it deems it necessary.
 *
 * Gerber file parsing inputs:
 * 	Gerber file
 * Output:
 * 	List of apertures /w compiled macros, image params
 */


#define MAX_APERTURES 1000

class RS274X_Program {
public:
	enum image_param_polarity { IP_POS, IP_NEG };
	enum image_param_justify { IJ_LEFT, IJ_CENTER };
	enum image_param_rotate { IR_0, IR_90, IR_180, IR_270 };
	
	struct rs274x_image_param {
		// IJ - Image justify
		// This probably won't be used in the DRC or anything
		bool IJ_set;
		enum image_param_justify IJ_justify_A;
		enum image_param_justify IJ_justify_B;
		double IJ_offset_A;
		double IJ_offset_B;
		
		// IN - Image name
		bool IN_set;
		char * IN_value;
		
		// IO - Image offset
		// // How is this used in comparison to the IJ offset?
		bool IO_set;
		double IO_offset_A;
		double IO_offset_B;
		
		// IP - image polarity
		bool IP_set;
		enum image_param_polarity IP_polarity;
		
		// IR - Image rotation
		bool IR_set;
		enum image_param_rotate IR_rotate;
		
		// PF - Plotter film
		bool PF_set;
		char * PF_value;
	};
	
	enum aperture_type {
		AP_CIRCLE,
		AP_RECT,
		AP_OVAL,
		AP_POLY,
		AP_T, // NO DOCS ON THIS ONE! Thermal?
		AP_MACRO
	};
	
	struct aperture {
		enum aperture_type type;
		
		// Standard Circle aperture
		typedef struct {
			double OD;
			double XAHD;
			double YAHD;
		} circle_ap_t;
		
		// Standard Rect aperture
		typedef struct {
			
			double XAD;
			double YAD;
			double XAHD;
			double YAHD;
			
		} rect_ap_t;
		
		// Standard Oval aperture
		typedef struct {
			double XAD;
			double YAD;
			double XAHD;
			double YAHD;
		} oval_ap_t;
		
		// Standard Poly aperture
		typedef struct {
			double OD;
			double NS;
			double DR;
			double XAHD;
			double YAHD;
			
		} poly_ap_t;
		
		typedef struct {
			double * params;
			int num_params;
			char * macro_name;
			Macro_VM * compiled_macro;
		} macro_ap_t;
		
		union {
			circle_ap_t circle_p;
			rect_ap_t rect_p;
			oval_ap_t oval_p;
			poly_ap_t poly_p;
			macro_ap_t macro_p;
		};
		
	};
	
	
	
	enum coord_abs_inc {
		COORD_ABS,
		COORD_INC
	};
	
	enum omit_lead_trail {
		OMIT_LEADING,
		OMIT_TRAILING
	};
	
	struct parse_info {
		bool parse_set;
		
		enum omit_lead_trail lt;
		
		int N_width;
		int G_width;
		int D_width;
		int M_width;
		
		int X_lead;
		int X_trail;
		int Y_lead;
		int Y_trail;
	};
	

	
	enum layer_polar_t {
		LP_C,
		LP_D
	};
	
	
	enum gcode_directive_type_t {
		DIR_AS, // Axis Select
		DIR_FS,	// Format Select - Emitted
		DIR_MI, // Mirror Image
		DIR_MO,	// Mode of Units - Emitted
		DIR_OF, // Offset
		DIR_SF, // Scale Factor
		LY_KO,
		LY_LN,
		LY_LP,
		LY_SR
	};
	enum gcode_op_type {
		GCO_G,
		GCO_M,
		GCO_D,
		GCO_X,
		GCO_Y,
		GCO_I,
		GCO_J,
		GCO_DIR, // Special GCODE op for directive
		GCO_END
	};
	struct gcode_directive_data_t {
		enum gcode_directive_type_t dir;
		union {
			struct {
				char A, B;
			} AS_P;
			
			struct {
				enum coord_abs_inc ai;
			} FS_P;
			
			struct {
			} KO_P;
			
			struct {
				char * name;
			} LN_P;
			
			struct {
				enum layer_polar_t lp;
			} LP_P;
			
			struct {
				bool mir_A;
				bool mir_B;
			} MI_P;
			
			struct {
				enum unit_mode um;
			} MO_P;
			
			struct {
				double A;
				double B;
			} OF_P;
			
			struct {
				double A;
				double B;
			} SF_P;
			
		};
	};
	
	struct gcode_block {
		enum gcode_op_type op;
		
		// No need to store all at once -
		union {
			int int_data;
			double dbl_data;
			struct gcode_directive_data_t gdd_data;
		};
	};
	
	struct parse_info					m_parse_settings;
	struct rs274x_image_param			m_image_params;
	
	
	// Maps aperture IDs to aperture objects
	typedef std::map<int, struct aperture *> aperture_map_t;
	aperture_map_t		m_ap_map;
	
	
	const struct aperture * getAperture(int index) const
	{
		aperture_map_t::const_iterator ci = m_ap_map.find(index);
		if (ci == m_ap_map.end())
			return NULL;
		return (*ci).second;
	}
	
	// Sequential list of operations to be performed by the virtual machine
	typedef std::list<struct gcode_block> operations_list_t;
	operations_list_t		m_operations;
	
	
	std::map<std::string, Macro_VM *>	m_macro_name_to_aperture;
private:
};

typedef boost::shared_ptr<RS274X_Program> sp_RS274X_Program;

sp_RS274X_Program parseRS274X(char * filename);

#endif
