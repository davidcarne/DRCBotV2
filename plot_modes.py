import cairo._cairo as cairo

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


class PlotModeEagle:
	pass
	
	def getRenderOrder(self):
		return ["MILLING", "PASTE_TOP","SILKSCREEN_TOP","SOLDERMASK_TOP", "COPPER_TOP","COPPER_1",
			"COPPER_2","COPPER_BOTTOM", "SOLDERMASK_BOTTOM", "SILKSCREEN_BOTTOM", "PASTE_BOTTOM"]

	def getPlotSettings(self, layer):
		ps = PlotSet()
		
		ps.drawfilled = True
		ps.drawinverted = False
		ps.strokeZeroWidthLines = True
		
		if (layer.startswith("SILKSCREEN")):
			ps.ovr = 0.9
			ps.ovg = 0.9
			ps.ovb = 0.9
		elif (layer == "COPPER_TOP"):
			ps.ovr = 0.5
			ps.ovg = 0
			ps.ovb = 0
		elif (layer == "COPPER_BOTTOM"):
			ps.ovr = 0
			ps.ovg = 0
			ps.ovb = 0.5
		elif (layer.startswith("SOLDERMASK")):
			ps.alpha = 0.3
			ps.drawfilled = True
			ps.strokeZeroWidthLines = False
			ps.ovr = 0.8
			ps.ovg = 0.8
			ps.ovb = 0.0
		else:
			ps.ovr = 1
			ps.ovg = 0
			ps.ovb = 1
			
		ps.renderOperator = cairo.OPERATOR_ADD
			
		return ps
		
	def renderBackground(self, context, outlines):
		cr = context
		cr.set_operator(cairo.OPERATOR_OVER)
		cr.set_source_rgba(0,0,0, 1)
		cr.paint()

	def getTransformArgs(self):
		return {}
		
class PlotModeRealistic:
	def __init__(self, view):
		self.view = view
	
	def getRenderOrder(self):
		if self.view == "TOP":
			return ["SILKSCREEN_TOP","SOLDERMASK_TOP", "COPPER_TOP"];
	
		if self.view == "BOTTOM":
			return ["SILKSCREEN_BOTTOM","SOLDERMASK_BOTTOM", "COPPER_BOTTOM"]
	
	def getPlotSettings(self, layer):
		ps = PlotSet()
		
		ps.drawfilled = True
		ps.drawinverted = False
		ps.strokeZeroWidthLines = True
		
		if (layer.startswith("SILKSCREEN")):
			ps.ovr = 0.95
			ps.ovg = 0.95
			ps.ovb = 0.95
		elif (layer.startswith("COPPER")):
			ps.ovr = 0.84765625
			ps.ovg = 0.52734375
			ps.ovb = 0.09765625
			ps.strokeZeroWidthLines = False

		elif (layer.startswith("SOLDERMASK")):
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
	
	def renderBackground(self, cr, outlines):
		cr.set_operator(cairo.OPERATOR_OVER)
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

	
	def getTransformArgs(self):
		if self.view == "BOT":
			return {"mirror_x": True}
			
		return {}


def getPlotSettings(name):
	if name.startswith("REALISTIC"): return PlotModeRealistic(name.split("_")[1])
	elif name.startswith("EAGLE"): return PlotModeEagle()
	return None