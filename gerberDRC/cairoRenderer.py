import math

try:
	import cairo
	cairo.LinearGradient
except:
	import cairo._cairo as cairo

import _gerber_utils

def emitGerbObjectCairoPath(context, gerbobj):
	cr = context
	
	segs = gerbobj.getPolyData().segs
	
	cr.move_to (segs[0].x, segs[0].y);
	
	for j in xrange(len(segs[1:])+1):
			a = segs[j]
			b = segs[(j+1)% len(segs)]
			
			if (a.lt == _gerber_utils.point_line.line_render_type_t.LR_ARC):
				angle1 = math.atan2(a.y - a.cy, a.x - a.cx)
				angle2 = math.atan2(b.y - a.cy, b.x - a.cx)
				radius1 = math.sqrt((a.y - a.cy) ** 2 + (a.x - a.cx) ** 2)
				cr.arc(a.cx, a.cy, radius1, angle1, angle2)
			else:
				cr.line_to (b.x, b.y);
	
	cr.close_path()

# Note - trim-to-units implies centered
def prepareCairoTransform(imsize, srcrect, pad = 0, mirror_x = False, mirror_y = False, trim_to_ratio = False, pad_in_unit_coords=False, centered=True):
	r = srcrect
	
	try:
		px_width, px_height = imsize
	except TypeError:
		px_width = px_height = imsize
	
	scale_x = (px_width - pad * 2) / r.getWidth()
	scale_y = (px_height - pad * 2) / r.getHeight()
	
	scale = min(scale_x, scale_y)

	if (trim_to_ratio):
		px_width = int(scale * r.getWidth() + pad * 2)
		px_height = int(scale * r.getHeight() + pad * 2)
		
	unit_pad = pad
	
	if not pad_in_unit_coords:
		unit_pad = unit_pad = pad / scale
	
	gerber_lower_left_x = r.getStartPoint().x
	gerber_lower_left_y = r.getStartPoint().y
	gerber_width = r.getWidth()
	gerber_height = r.getHeight()
	
	if (mirror_x): 
		scale_x = -scale
		offset_x = -gerber_width - gerber_lower_left_x - unit_pad
	else:
		scale_x = scale
		offset_x = -gerber_lower_left_x + unit_pad
	
	
	# Geber is mirrored vertically compared to device coords
	if (mirror_y): 
		scale_y = scale
		offset_y = -gerber_lower_left_y + unit_pad
	else:	
		scale_y = -scale
		offset_y = -gerber_height - gerber_lower_left_y - unit_pad
	
	
	def applyTransform(cr):
		cr.scale(scale_x, scale_y)
		cr.translate(offset_x, offset_y);
		cr.set_line_width(1 / scale)
	
	return ( (px_width, px_height) , applyTransform )
