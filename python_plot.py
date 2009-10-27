#!/usr/bin/python

from gerb_script import *
import sys
from optparse import OptionParser
from sets import Set
from script_ops import *

# Setup the options parser
parser = OptionParser()
parser.add_option("-s", "--silkscreen", dest="silkscreen",
	help="Silkscreen layer input", metavar="FILE")
parser.add_option("-c", "--copper", dest="copper",
	help="Copper layer input", metavar="FILE")
parser.add_option("-m", "--soldermask", dest="soldermask",
	help="Soldermask input", metavar="FILE")

parser.add_option("-o", "--output", dest="output",
	help="write output to FILE", metavar="FILE")

(options, args) = parser.parse_args()

if not options.copper:
	print "Copper layer is required for plotting"
	sys.exit();

if not options.output:
	print "Must specify an output filename"
	sys.exit();

copper_file_rep = create_gerber_file_rep_from_filename(options.copper)
#print copper_file_rep
#print_gcode_chain(copper_file_rep.first_gcode)

copper_gcode_vers = None

if copper_file_rep == None:
	print ("Did not get a file rep")
	sys.exit(-1)
else:
	copper_gcode_vers = gcode_run(copper_file_rep)

if not copper_gcode_vers:
	print "Did not sucessfully execute gerber!"
	sys.exit(-1);

# Turn generated GCODE_Lines into polygons

if len(copper_gcode_vers.all) == 0:
	print "GCODE run created no groups!"
	sys.exit(-1)

print "Created %d elements" % len(copper_gcode_vers.all)

polygonize_vector_outp(copper_gcode_vers)
makeEndPartitions(copper_gcode_vers);
groupize(copper_gcode_vers)
print "Found %d groups" % len(copper_gcode_vers.groups);

nlines = 0;
npolys = 0
nccw = 0;
print "Found %d elements" % len(copper_gcode_vers.all)

for x in copper_gcode_vers.all:
	line = cast_GerbObj_ToLine(x)
	if line:
		nlines+= 1
	elif cast_GerbObj_ToPoly(x):
		npolys+=1

	poly = cast_GerbObj_ToPoly(x)
	if poly:
		if poly.is_ccw():
			nccw += 1
		else:
			poly.flag = FLG_SPACE

print "%d are lines, %d polys, %d ccw" % (nlines, npolys, nccw)

#sys.exit()

bounds = Rect()

for x in copper_gcode_vers.all:
	bounds.mergeBounds(x.getBounds())

bounds.printRect()

ratio = bounds.getWidth() / bounds.getHeight()

size=1800
if (ratio < 1):
	x = size * ratio
	y = size
else:
	x = size
	y = size / ratio
	'''s=drcSettings();
initDRC(s);
doDRC(copper_gcode_vers,s,True);
'''
ps = plotOptions()
ps.showBoundingBoxes = False
ps.colorByGroups = True
ps.colorByRandom = False
ps.colorByOVR = False
ps.ovr = 0xFF
ps.ovg = 0x80
ps.ovb = 0x20

ps.scale = x / bounds.getWidth()
ps.offX = - bounds.getStartPoint().x
ps.offY = - bounds.getStartPoint().y
ps.rw = int(x)
ps.rh = int(y)

ps.drawfilled = False
ps.drawinverted = False

#img = gdImageCreateTrueColor(ps.rw, ps.rh);
img = plot_vector(copper_gcode_vers, ps);
print writePNGFile("test.png", img)
gdImageDestroy(img)
