import re

class DrillRack(object):
	def __init__(self):
		self.rack = {}
	
	def addDrill(self, index, size):
		self.rack[index] = size
		
		
class ExcellonRep(object):
	def __init__(self):
		self.hits = []
		self.rack = DrillRack()
		self.__lastX = 0
		self.__lastY = 0
	
		self.__maxx = None
		self.__maxy = None
		
	def addHit(self, tool, x,y):
		if x == None:
			x = self.__lastX
			
		if y == None:
			y = self.__lastY
		
		self.hits += [(tool, x,y)]
		self.__lastX = x
		self.__lastY = y
		self.__maxx = max(self.__maxx, x)
		self.__maxy = max(self.__maxy, y)
		
	def scaleToFit(self, size):
		
		scale = 1.0
		while self.__maxx / scale > size[0] or self.__maxy/scale > size[1]:
			scale *= 10
		
		newhits = []
		for tool,x,y in self.hits:
			newhits += [(tool, x/scale, y/scale)]
		self.hits = newhits
			
def calcCoord(coord, unit, mode, maxwidth):
	mult = 1000
	if (unit == "INCH"): mult = 25400
	
	sign = '+'
	if coord[0] in '+-':
		sign = coord[0]
		coord = coord[1:]
	if mode == "FIXED" or mode == "TRAILING":
		return int(sign + coord) * mult
	if mode == "LEADING":
		return int(sign + ("%-*s" % (maxwidth,coord)).replace(" ","0")) * mult

def unit_convert(mode, value):
	if mode == "INCH": return value * 25400
	elif mode == "METRIC": return value * 1000
	
def parseExcellon(file):
	
	rep = ExcellonRep()
	unit_mode = "INCH"
	lines = open(file,"r").readlines()
	
	content_lines = []
	in_header = False
	for num,i in enumerate(lines):
		i = i.strip().upper()
		if i == "%":
			in_header = False
		elif i.startswith("M48"):
			in_header = True			
		elif in_header:
		
			
			if i.startswith("M72") or i.startswith("METRIC"):
				unit_mode = "INCH"
				
			if i.startswith("M71") or i.startswith("INCH"):
				unit_mode = "METRIC"
				
			if (i.startswith("T")):
				m = re.match(r"^T(?P<id>\d+)C(?P<size>[0-9.]+)$", i)
				if not m:
					print "Error, could not parse T line %d: %s" % (num, i)
					return
				id = int(m.group("id"))
				size = unit_convert(unit_mode, float(m.group("size")))
				rep.rack.addDrill(id, size)
	
		else:
			content_lines += [i]
	
	max_coord_digits = 0
	min_coord_digits = 1000
	has_leading_zeros = False
	has_trailing_zeros = False
	
	for i in content_lines:
		if i.startswith("X") or i.startswith("Y"):
				m = re.match(r"^([XY][+-]?[0-9.]+)*$", i)
				if not m:
					print "Error - could not match coordinate line %s!" % i
					return
					
				m = re.findall(r"([XY][+-]?[0-9.]+)", i)
				
				for i in m:
					c = i[1:]
					if c[0] in '+-':
						c = c[1:]
						
					max_coord_digits = max(len(c), max_coord_digits)
					min_coord_digits = min(len(c), min_coord_digits)
					if c[0] == '0':
						has_leading_zeros = True
					if c[-1] == '0':
						has_trailing_zeros = True
	
	parsemode = None
	if (max_coord_digits == min_coord_digits):
		parsemode = "FIXED"
		
	elif has_trailing_zeros == has_leading_zeros:
		print "Can't determine parse settings"
		return
	elif has_trailing_zeros:
		parsemode = "TRAILING"
	else:
		parsemode = "LEADING"
	
	current_tool = None
	
	for i in content_lines:
		if i.startswith("M30"):
			break;
				
		if i.startswith("M72"):
			unit_mode = "INCH"
			
		if i.startswith("M71"):
			unit_mode = "METRIC"
			
		if i.startswith("T"):
				m = re.match(r"^T(?P<id>\d+)*$", i)
				if not m:
					print "Error - could not parse tool change line!"
					return
				current_tool = int(m.group("id"))
				
		elif i.startswith("X") or i.startswith("Y"):
				
				m = re.findall(r"([XY][+-]?[0-9.]+)", i)
				
				x = None
				y = None
				for i in m:
					coord = i[0]
					value = calcCoord(i[1:], unit_mode, parsemode, max_coord_digits)
					if (coord == 'X'):
						x = value
					if (coord == 'Y'):
						y = value
				rep.addHit(current_tool, x,y)
	return rep