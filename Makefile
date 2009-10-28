
SRCS=src/gerber_parse.cpp src/wrap/gerber_parse_wrap.cpp src/wrap/aperture_wrap.cpp \
	src/util.cpp src/fileio.cpp src/macro_parser.cpp src/macro_vm.cpp \
	src/gerb_script_util.cpp src/util_type.cpp src/GerbObj_line.cpp src/GerbObj_poly.cpp \
	
	
OBJS=$(patsubst %.cpp,build/%.o, $(SRCS) )

CPPFLAGS = `python-config --includes` -g -Isrc/
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