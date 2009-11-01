
SRCS=src/gerber_parse.cpp src/wrap/gerber_parse_wrap.cpp src/wrap/aperture_wrap.cpp \
	src/util.cpp src/fileio.cpp src/macro_parser.cpp src/macro_vm.cpp \
	src/gerb_script_util.cpp src/util_type.cpp src/gerbobj_line.cpp src/gerbobj_poly.cpp \
	src/gcode_interp.cpp src/wrap/gerber_utils_wrap.cpp src/wrap/gcode_interp_wrap.cpp \
	src/wrap/plot_wrap.cpp src/plot_vector.cpp
	
OBJS=$(patsubst %.cpp,build/%.o, $(SRCS) )

CPPFLAGS = `python-config --includes` -g -Isrc/ -Wall -Werror -Wno-unused -Wnewline-eof
LDFLAGS = -lboost_python `python-config --ldflags` -lgd 

_gerber_utils.so: $(OBJS)
	@echo "LD   $@"
	@g++ $(LDFLAGS) $^ --shared -o $@
	
build/%.o: %.cpp
	@echo "CXX	$<"
	@mkdir -p $(@D)
	@g++ $(CPPFLAGS) -c -o $@ $<
	
.PHONY: clean
clean:
	rm -rf build _gerber_utils.so
