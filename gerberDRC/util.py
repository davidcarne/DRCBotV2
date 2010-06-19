import networkx
import itertools
import _gerber_utils as GD

def point_round(x,y):
	return (int(round(x/0.0001)), int(round(y/0.0001)))

# This function is intended for use in finding the outside
# perimeter of the board
# No winding is guaranteed
def buildCyclePathsForLineSegments(segments):
	g = networkx.Graph()
	
	# First, add the nodes and construct edges between them.
	for i in segments:
		start_node = point_round(i.sx, i.sy)
		end_node = point_round(i.ex, i.ey)
		
		g.add_node(start_node, startpoint=(i.sx, i.sy, i.width))
		g.add_node(end_node,   startpoint=(i.ex, i.ey, i.width))
		g.add_edge(start_node, end_node)
	
	# separate into disjoint subcomponents 
	component_nodes = networkx.connected_components(g)
	
	# create graphs from node ids for all subcomponents
	component_graphs = [g.subgraph(i).copy() for i in component_nodes]
	
	del g
	
	outline_paths = []
	for subgraph in component_graphs:
		# If the graph is not of degree 2, then it can't be a pure cycle
		# since we've already established it is a completely connected loop
		if all([degree == 2 for degree in subgraph.degree().itervalues()]):
			# Transversal of the graph starting from a random node
			ordered = networkx.dfs_preorder(subgraph)
			
			# Path = (x,y) coordinates for each node in the ordered transversal
			path = [subgraph.node[j]['startpoint'] for j in ordered]
			
			outline_paths.append(path)
			
	return outline_paths


def calculateBoundingRectFromObjects(layers, useOnlyZeroWidth=True):
	r = GD.Rect()
	for i in layers:
		for gerb_obj in i.all:
			if not useOnlyZeroWidth or isinstance(gerb_obj, GD.GerbObj_Line) and (gerb_obj.width == 0):
					r.mergeBounds(gerb_obj.getBounds())
	return r
	
def calculateBoundingRectFromVisibleObjects(layers):
	r = GD.Rect()
	for i in layers:
		for gerb_obj in i.all:
			if not isinstance(gerb_obj, GD.GerbObj_Line) or (gerb_obj.width != 0):
					r.mergeBounds(gerb_obj.getBounds())
	return r


def calculateBoundingRectFromOutlines(outlines):
	r = GD.Rect()
	points = itertools.chain(*outlines)
	for i in points:
		r.mergePoint(GD.Point(i[0], i[1]),i[2])
	return r
	
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