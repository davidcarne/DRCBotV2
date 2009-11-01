import math

try:
	import cairo
	cairo.LinearGradient
except:
	import cairo._cairo as cairo


def magic():
	WIDTH, HEIGHT  = 256, 256

	surface = cairo.ImageSurface(cairo.FORMAT_ARGB32, WIDTH, HEIGHT)
	cr = cairo.Context(surface)

	cr.scale(WIDTH/1.0, HEIGHT/1.0)

	cr.set_line_width(1.0/WIDTH)
	cr.set_source_rgb(0, 0, 0)
	cr.rectangle(0.25, 0.25, 0.5, 0.5)
	cr.stroke()

	fo = file('render/f.png', 'w')

	surface.write_to_png(fo)