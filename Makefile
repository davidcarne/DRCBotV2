

BUILD := build

CPPOBJS := gcode_interp.o   groupize.o net_group.o\
		 plot_vector.o GerbObj_line.o GerbObj_poly.o \
		macro_vm.o macro_parser.o partitioning.o\
		drc.o gerber_parse.o fileio.o inpoly.o util.o util_type.o\
		polygonize.o polymath.o

CPPOBJSS :=  main.o
CPPOBJSP :=  plot_main.o

ALL_OBJS := $(CPPOBJS) $(CPPOBJSS) $(CPPOBJSP)

GLOBAL_HEADERS := partitioning.h gerber_parse.h gcode_interp.h
CPPFLAGS := -g -fPIC
LIBS := -L/usr/X11R6/lib -lgd -lpng -lz -ljpeg -lfreetype -lm
CPP := g++

all: gerber_drc gerber_plot wrapper


.PHONY: todo
todo:
	grep -i todo src/* tests/*
gerber_drc: $(foreach obj,$(CPPOBJSS),$(BUILD)/$(obj)) $(foreach obj,$(CPPOBJS),$(BUILD)/$(obj))
	$(CPP) $(LIBS) $^ -o $@

gerber_plot: $(foreach obj,$(CPPOBJSP),$(BUILD)/$(obj)) $(foreach obj,$(CPPOBJS),$(BUILD)/$(obj))
	$(CPP) $(LIBS) $^ -o $@

$(foreach obj, $(ALL_OBJS), $(BUILD)/$(obj)): build/%.o: src/%.cpp  $(foreach hdr, $(GLOBAL_HEADERS), src/$(hdr))
	$(CPP) $(CPPFLAGS) -c $< -o $@

.PHONY: wrapper
wrapper: _gerb_script.so

_gerb_script.so: $(foreach obj,$(CPPOBJS),$(BUILD)/$(obj)) src/gerb_script.i src/gerb_script_util.cpp
	swig -c++ -python -o $(BUILD)/gerb_script_wrap.cxx -outdir . src/gerb_script.i
	g++ -I. -I/usr/include/python2.4 -c -fPIC src/gerb_script_util.cpp -o $(BUILD)/gerb_script_util.o
	g++ -I. -I/usr/include/python2.4 -c -fPIC -Isrc $(BUILD)/gerb_script_wrap.cxx -o $(BUILD)/gerb_script_wrap.o
	g++ -lgd -ljpeg -lpng -lz -lm -shared $(foreach obj,$(CPPOBJS),$(BUILD)/$(obj)) $(BUILD)/gerb_script_wrap.o $(BUILD)/gerb_script_util.o -o _gerb_script.so
	
.PHONY: dep
dep:
	gccmakedep *.cpp

.PHONY: clean
clean:
	rm -f gerber_drc gerber_plot gerb_script.py *.o *.so *.pyc build/*

.PHONY: distclean
distclean: clean
	rm -f *~ *.png *.bak *.so gerb_script_wrap.* src/*~ src/*.bak

# DO NOT DELETE

