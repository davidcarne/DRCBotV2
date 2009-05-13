%module gerb_script
%{ 
#include "main.h"
#include "gerber_parse.h"
#include "gcode_interp.h"
#include "plot_vector.h"
#include "groupize.h"
#include "partitioning.h"
#include "drc.h"
#include "util_type.h"
#include "polygonize.h"

GerbObj_Line * cast_GerbObj_ToLine(GerbObj * v);
GerbObj_Poly * cast_GerbObj_ToPoly(GerbObj * v);
int writePNGFile(char * file, gdImagePtr im);

%}


%include "std_vector.i"
%include "std_set.i"
namespace std {
        %template(set_net_group) set <net_group*>;
        %template(set_net_group_v) vector <net_group*>;
        %template(set_GerbObj) set <GerbObj*>;
        %template(set_GerbObj_v) vector <GerbObj*>;
}

%include "main.h"
%include "gerber_parse.h"
%include "gcode_interp.h"
%include "plot_vector.h"
%include "groupize.h"
%include "partitioning.h"
%include "drc.h"
%include "util_type.h"
%include "gdwrap.h"
%include "polygonize.h"

GerbObj_Line * cast_GerbObj_ToLine(GerbObj * v);
GerbObj_Poly * cast_GerbObj_ToPoly(GerbObj * v);
int writePNGFile(char * file, gdImagePtr im);
