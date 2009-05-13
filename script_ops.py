from gerb_script import *
gcode_decode = {GCO_G: "GCO_G",
		GCO_M: "GCO_M",
		GCO_D: "GCO_D",
		GCO_X: "GCO_X",
		GCO_Y: "GCO_Y",
		GCO_I: "GCO_I",
		GCO_J: "GCO_J",
		GCO_END: "GCO_END",
		GCO_DIR: "GCO_DIR",
	};

def print_gcode_chain(gcode):
	while gcode:
		print_gcode(gcode)
		gcode = gcode.next


def print_gcode(gcode):
	
	print gcode_decode[gcode.op],
	if gcode.op in [GCO_G, GCO_D]:
		print gcode.int_data,
	elif gcode.op == GCO_END:
		print

