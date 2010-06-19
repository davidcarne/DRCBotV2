#!/usr/bin/python

import gerberDRC as GD

import gerberDRC.util as GU
import sys
path = sys.argv[1]

GD.setDebugLevel(GD.debug_level_t(0))

f = GD.parseFile(path)
if not f:
	print "Could not parse file %s" % path
	exit(1)
	
p = GD.runRS274XProgram(f)

if not p:
	print "Could not run program"
	exit(1)
for n, i in enumerate(p.layers):
	print "Layer %d: '%s'" % (n,i.name)
	print "\tpolarity: %s" %(i.polarity)
	
	
	
import cairo._cairo as cairo

	
def createCairoLineCenterLinePath(obj, cr):
	cr.move_to (obj.sx, obj.sy);
	cr.line_to (obj.ex, obj.ey);
		
def renderGerberFile(rep, cr):
	
	cr.push_group()
	cr.set_operator(cairo.OPERATOR_OVER)
	

	for j in rep.layers:
		if (j.polarity == "LP_C"):
				cr.set_source_rgba(1,0,0, 1)
		else:
				cr.set_source_rgba(0,0,1, 1)

		for k in j.draws:
			GD.emitGerbObjectCairoPath(cr, k)
			
			cr.stroke()		
			
	cr.pop_group_to_source()
	cr.paint_with_alpha(1)

	
srcrect = GU.calculateBoundingRectFromPCBLayers([p], False)

print srcrect

# Calculate the image size and transform
(width, height), transform = GD.prepareCairoTransform(1024, srcrect, pad = 50, trim_to_ratio = True)

# Prepare a surface to render onto
surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, width, height)
cr = cairo.Context(surface)

# Apply the transform to the context
transform(cr)

renderGerberFile(p, cr)

surface.write_to_png(open("out.png", 'w'))
