#!/usr/bin/python

import os
from optparse import OptionParser

import cairo._cairo as cairo

import gerberDRC
import math
import networkx

o = OptionParser()

o.add_option("-d", "--debug-level", 
	type="int", dest="debuglevel", default=1,
	help="Debug Level; 0-4, 4 being most verbose")

o.add_option("-m", "--render-mode", 
	type="str", dest="rendermode", default="REALISTIC_TOP",
	help="one of REALISTIC_TOP, REALISTIC_BOT, EAGLE")
	
(options, args) = o.parse_args()

calculate_sizing_using_only_zero_width_lines = False
mode = options.rendermode

gerberDRC.setDebugLevel(gerberDRC.debug_level_t(options.debuglevel))

def identifyLayer(name, convention="PROTEL"):
	if name.find('.') == -1: return None
	
	extension = name[1+name.rfind("."):].upper()

	if convention == "PROTEL":
		protel_layers = {
			"GML" : ("RS274X","OUTLINE"),
			"GTP" : ("RS274X","PASTE_TOP"),
			"GTO" : ("RS274X","SILKSCREEN_TOP"),
			"GTS" : ("RS274X","SOLDERMASK_TOP"),
			"GTL" : ("RS274X","COPPER_TOP"),
			"GBL" : ("RS274X","COPPER_BOTTOM"),
			"GBS" : ("RS274X","SOLDERMASK_BOTTOM"),
			"GBO" : ("RS274X","SILKSCREEN_BOTTOM"),
			"GBP" : ("RS274X","PASTE_BOTTOM"),
			"GML" : ("RS274X","MILLING"),
			"TXT" : ("EXCELLON","DRILL")
			};
		if extension in protel_layers:
			return protel_layers[extension]
	

class PlotSet(object):
	def __init__(self):
		self.alpha = 1
		self.ovr = 0
		self.ovg = 0
		self.ovb = 0
		self.drawfilled = True
		self.drawinverted = False
		self.strokeZeroWidthLines = False
		self.clipToZeroWidthLines = False
		self.renderOperator = cairo.OPERATOR_OVER
		
def creatCairoContourPath(obj, cr):
	segs = obj.getPolyData().segs
	
	cr.move_to (segs[0].x, segs[0].y);
	for j in xrange(len(segs[1:])+1):
			a = segs[j]
			b = segs[(j+1)% len(segs)]
			if (a.lt == gerberDRC.point_line.line_render_type_t.LR_ARC):
				#print a.x, a.y
				angle1 = math.atan2(a.y - a.cy, a.x - a.cx)
				angle2 = math.atan2(b.y - a.cy, b.x - a.cx)
				radius1 = math.sqrt((a.y - a.cy) ** 2 + (a.x - a.cx) ** 2)
				cr.arc(a.cx, a.cy, radius1, angle1, angle2)
			else:
				cr.line_to (b.x, b.y);

def createCairoLineCenterLinePath(obj, cr):
	cr.move_to (obj.sx, obj.sy);
	cr.line_to (obj.ex, obj.ey);

def setBackground(cr, outlines):
	global mode
	
	if mode == "REALISTIC_TOP":
		cr.set_operator(cairo.OPERATOR_SOURCE)
		cr.set_source_rgba(0,0,0, 1)
		cr.paint()
		
		cr.set_source_rgba(0.9140625*.9, 0.9140625*.9, 0.6796875*.9, 1)
		
		if outlines:
			for i in outlines:
				cr.move_to(i[0][0], i[0][1])
				for x,y in i[1:]:
					cr.line_to(x,y)
				cr.close_path()
			cr.fill()
		else:
			cr.paint()
			
		
	
def choosePlotSettings(rentype):
	global mode
	
	
	ps = PlotSet()
	
	ps.drawfilled = True
	ps.drawinverted = False
	ps.strokeZeroWidthLines = True
	
	if mode == "EAGLE":
		if (rentype.startswith("SILKSCREEN")):
			ps.ovr = 0.9
			ps.ovg = 0.9
			ps.ovb = 0.9
		elif (rentype == "COPPER_TOP"):
			ps.ovr = 0.5
			ps.ovg = 0
			ps.ovb = 0
		elif (rentype == "COPPER_BOTTOM"):
			ps.ovr = 0
			ps.ovg = 0
			ps.ovb = 0.5
		elif (rentype.startswith("SOLDERMASK")):
			ps.alpha = 0.3
			ps.drawfilled = True
			ps.strokeZeroWidthLines = False
			ps.drawinverted = True
			ps.ovr = 0.0
			ps.ovg = 0.8
			ps.ovb = 0.0
		else:
			ps.ovr = 1
			ps.ovg = 0
			ps.ovb = 1
			
		ps.renderOperator = cairo.OPERATOR_ADD
		
	elif mode == "REALISTIC_TOP":
		if (rentype.startswith("SILKSCREEN_TOP")):
			ps.ovr = 0.95
			ps.ovg = 0.95
			ps.ovb = 0.95
		elif (rentype == "COPPER_TOP"):
			ps.ovr = 0.84765625
			ps.ovg = 0.52734375
			ps.ovb = 0.09765625
		elif (rentype.startswith("SOLDERMASK_TOP")):
			ps.alpha = 0.8
			ps.drawfilled = True
			ps.strokeZeroWidthLines = False
			ps.drawinverted = True
			ps.ovr = 0.0
			ps.ovg = 0.4
			ps.ovb = 0.0
		else:
			ps.alpha = 0
	
	return ps
		
def renderGerberFile(rep, cr, rentype, outlines):

	ps = choosePlotSettings(rentype)
	
	cr.push_group()
	if not ps.drawinverted:
		cr.set_operator(cairo.OPERATOR_OVER)
	else:
		cr.set_operator(cairo.OPERATOR_OVER)
		cr.set_source_rgba(ps.ovr, ps.ovg, ps.ovb, 1)
		
		if outlines:
			for i in outlines:
				cr.move_to(i[0][0], i[0][1])
				for x,y in i[1:]:
					cr.line_to(x,y)
				cr.close_path()
			cr.fill()
		else:
			cr.paint()
			
		cr.set_operator(cairo.OPERATOR_DEST_OUT)
		
	cr.set_source_rgba(ps.ovr, ps.ovg, ps.ovb, 1)

	for k in rep.all:
		if isinstance(k, gerberDRC.GerbObj_Line) and (k.width == 0) and ps.strokeZeroWidthLines:
				createCairoLineCenterLinePath(k,cr)
				cr.stroke()
		else:
			creatCairoContourPath(k, cr)
			
			if (ps.drawfilled):
				cr.fill()
			else:
				cr.stroke()		
			
	cr.pop_group_to_source()
	cr.paint_with_alpha(ps.alpha)
	cr.set_operator(ps.renderOperator)


layers = {}
c=0
path = "examples/gerbers/"
for i in os.listdir(path):
	print "Parsing: %s" % i
	identification = identifyLayer(i)
	
	if not identification:
		print "Could not identify type of %s - skipping." % i
		continue
	
	fmt, layer = identification


	if (fmt == 'RS274X'):
		f = gerberDRC.parseFile(path+i)
		if (not f):
			print "Could not parse %s" % src
			continue 
		
		p = gerberDRC.runRS274XProgram(f)

		layers[layer] = p

	elif (fmt == 'EXCELLON'):
		f = gerberDRC.parseExcellon(path + i)
		layers[layer] = f
		
	else:
		print "Can't handle file type %s" % fmt
		continue


render_order = ["MILLING", "PASTE_TOP","SILKSCREEN_TOP","SOLDERMASK_TOP",
	"COPPER_TOP","COPPER_1","COPPER_2","COPPER_BOTTOM",
	"SOLDERMASK_BOTTOM", "SILKSCREEN_BOTTOM", "PASTE_BOTTOM"]

#render_order = ["SOLDERMASK_TOP"]
imgfile = None


IMSIZE = 1024

r = gerberDRC.Rect()
for i in reversed(render_order):
	if i in layers.keys():
		t = layers[i];
		for k in t.all:
			if calculate_sizing_using_only_zero_width_lines and \
				(not isinstance(k, gerberDRC.GerbObj_Line) or (k.width != 0)):
				continue;
			r.mergeBounds(k.getBounds())
			
if "DRILL" in layers:
	layers["DRILL"].scaleToFit((r.getEndPoint().x, r.getEndPoint().y))


def point_hash(x,y):
	return (int(round(x/0.0001)), int(round(y/0.0001)))


outline_paths = []

if "MILLING" in layers:
	g = networkx.Graph()
	t = layers["MILLING"];
	for k in t.all:
		if isinstance(k, gerberDRC.GerbObj_Line):
			g.add_node(point_hash(k.sx, k.sy), startpoint=(k.sx, k.sy))
			g.add_node(point_hash(k.ex, k.ey), startpoint=(k.ex, k.ey))
			
			g.add_edge(point_hash(k.sx, k.sy), point_hash(k.ex, k.ey))
	c= networkx.connected_components(g)
	
	subg = [g.subgraph(i, copy=True) for i in c]
	for i in subg:
		if all([j == 2 for j in i.degree()]):
			
			path = []
			ordered = networkx.dfs_preorder(i)
			for j in ordered:
				path += [i.node[j]['startpoint']]
			outline_paths += [path]
			
elif "COPPER_TOP" in layers:
	g = networkx.Graph()
	t = layers["COPPER_TOP"];
	for k in t.all:
		if isinstance(k, gerberDRC.GerbObj_Line) and (k.width == 0):
			g.add_node(point_hash(k.sx, k.sy), startpoint=(k.sx, k.sy))
			g.add_node(point_hash(k.ex, k.ey), startpoint=(k.ex, k.ey))
			
			g.add_edge(point_hash(k.sx, k.sy), point_hash(k.ex, k.ey))
			
	c= networkx.connected_components(g)
	
	subg = [g.subgraph(i, copy=True) for i in c]
	for i in subg:
		if all([j == 2 for j in i.degree()]):
			
			path = []
			ordered = networkx.dfs_preorder(i)
			for j in ordered:
				path += [i.node[j]['startpoint']]
			outline_paths += [path]
			

maxdim = max(r.getWidth(), r.getHeight())
scale = IMSIZE / maxdim

WIDTH = scale * r.getWidth() 
HEIGHT = scale * r.getHeight()

PAD=100

r.printRect()
surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, int(WIDTH)+PAD, int(HEIGHT)+PAD)
#surface = cairo.PDFSurface("render/layers.pdf", int(WIDTH)+PAD, int(HEIGHT)+PAD)
cr = cairo.Context(surface)
cr.scale(scale, -scale)
cr.translate(-r.getStartPoint().x + PAD/2/scale,-r.getHeight()-r.getStartPoint().y - PAD/2/scale);
cr.set_line_width(5.0/WIDTH)

setBackground(cr, outline_paths)
		
for i in reversed(render_order):
	if i in layers.keys():

		t = layers[i];
		
		print "Rendering %s - " % i, t
		renderGerberFile(t, cr, i, outline_paths)

if "DRILL" in layers:
	print "Rendering Drills"
	cr.set_operator(cairo.OPERATOR_OVER)
	cr.set_source_rgba(0,0,0,1);
	for tool,x,y in layers["DRILL"].hits:
		radius = layers["DRILL"].rack.rack[tool]
		cr.arc(x,y,radius/2,0,math.pi * 2)
		cr.close_path()
		cr.fill()
		
fo = open('render/layers.png', 'w')
surface.write_to_png(fo)