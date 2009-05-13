#!/usr/bin/python
import pygame
from pygame.locals import *
import math
import sys

def main():
	file = open(sys.argv[1])
	lines = file.readlines();
	poly = 0;
	points = [];
	for i in lines:
		if i[0:4] == '$$ N':
			poly+=1;
			continue
		args = i.split()
		point = {}
		for j in args:
			
			if '=' in j:
				(name, val) = j.split("=")
				point[name] = val
				point["poly"] = poly
		points.append(point)
	
	for i in points:
		i["X"] = float(i["X"]);
		i["Y"] = float(i["Y"]);
		i["I"] = int(i["I"]);
		i["FLG"] = int(i["FLG"]);

	Infinity = float("infinity")
	xmin = Infinity
	xmax = -Infinity
	ymin = Infinity
	ymax = -Infinity

	for i in points:
		if i["X"] < xmin:
			xmin = i["X"]
		if i["X"] > xmax:
			xmax = i["X"]
		if i["Y"] < ymin:
			ymin = i["Y"]
		if i["Y"] > ymax:
			ymax = i["Y"]

	wx = (xmax - xmin) / 2 + xmin
	wy = (ymax - ymin) / 2 + ymin
	ww = xmax - xmin
	wh = ymax - ymin

	pygame.init()
        screen = pygame.display.set_mode((640, 480))

	done = False
	while not done:
		render(screen, wx,wy,ww,wh, points)
                for event in pygame.event.get():
                        if event.type in (QUIT, KEYDOWN):
                                done = True

def transform(x,y,sx,sy,cx,cy):
	dx = cx - x
	dy = cy - y
	px = dx * sx
	py = dy * sy
	px += 320
	py += 240
	return (px, py)

colortab = [ 
		[0, 255, 0],
		[0, 0, 255]
	   ];

def render(screen, x,y,w,h, points):
	screen.fill([0,0,0])
	sx = 600 / w
	sy = 440 / h
	if (sx > sy):
		sx = sy;
	else:
		sy = sx;

	n = len(points)
	for i in range(0, n):
		indexa = i % n
		indexb = (i + 1) % n 
		a = points[indexa]
		b = points[indexb]

		if (a["poly"] != b["poly"]):
			continue

		ap = transform(x,y,sx,sy,a["X"],a["Y"])
		bp = transform(x,y,sx,sy,b["X"],b["Y"])
		
		pygame.draw.line(screen, colortab[a["poly"]-1], ap, bp, 2)
		dcol = [255,255,0]
		if a["I"]:
			dcol = [255,0,0];
		pygame.draw.line(screen, dcol, ap, ap, 4)

	pygame.display.flip()

if  __name__ == "__main__":
    main()
	
