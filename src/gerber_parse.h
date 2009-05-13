#include <map>
#include <string>
#include <vector>
#include "macro_vm.h"

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
	union {
		// Standard Circle aperture
		struct {
			double OD;
			double XAHD;
			double YAHD;
		} circle_p;
		
		// Standard Rect aperture
		struct {

			double XAD;
			double YAD;
			double XAHD;
			double YAHD;
			
		} rect_p;

		// Standard Oval aperture
		struct {
			double XAD;
			double YAD;
			double XAHD;
			double YAHD;
		} oval_p;

		// Standard Poly aperture
		struct {
			double OD;
			double NS;
			double DR;
			double XAHD;
			double YAHD;

		} poly_p;

		struct {
			double * params;
			int num_params;
			char * macro_name;
			Macro_VM * compiled_macro;
		} macro_p;
	};

};

enum omit_lead_trail {
	OMIT_LEADING,
	OMIT_TRAILING
};

enum coord_abs_inc {
	COORD_ABS,
	COORD_INC
};

enum unit_mode {
	UNITMODE_IN,
	UNITMODE_MM
};

struct parse_info {
	bool parse_set;

	enum omit_lead_trail lt;
	enum coord_abs_inc ai;

	int N_width;
	int G_width;
	int D_width;
	int M_width;

	int X_lead;
	int X_trail;
	int Y_lead;
	int Y_trail;

	// Now encoded as directives
	//enum unit_mode um;
};

enum layer_polar_t {
	LP_C,
	LP_D
};

enum gcode_directive_type_t {
	DIR_AS,
	DIR_FS,
	DIR_MI,
	DIR_MO,
	DIR_OF,
	DIR_SF,
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
	//union {
		int int_data;
		double dbl_data;
		struct gcode_directive_data_t gdd_data;
	//};
	struct gcode_block * next;
};

#define MAX_APERTURES 1000

struct gerber_file {
	struct parse_info parse_settings;

	struct rs274x_image_param image_params;

	struct aperture * ap_list[MAX_APERTURES];

	// Map to convert names to compiled macros
	// Not needed after initial parse
	std::map<std::string, Macro_VM *> ap_map;
	
	// Pointers to head and tail of the instruction list
	struct gcode_block * first_gcode;
	struct gcode_block * last_gcode;
};

struct gerber_file * create_gerber_file_rep_from_filename(char * filename);
void free_gerber_file_rep(struct gerber_file * rep);

