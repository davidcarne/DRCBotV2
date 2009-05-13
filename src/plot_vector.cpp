#include <stdio.h>
#include <gd.h>
#include <math.h>
#include <assert.h>
#include "main.h"
#include "gcode_interp.h"
#include "plot_vector.h"
#include "util_type.h"


void _cgdImageCopy (gdImagePtr dst, gdImagePtr src, int dstX, int dstY, int srcX,
             int srcY, int w, int h)
{
	int c;
	int x, y;
	int tox, toy;
	int i;
	int colorMap[gdMaxColors];

	if (dst->trueColor) {
		/* 2.0: much easier when the destination is truecolor. */
          /* 2.0.10: needs a transparent-index check that is still valid if
		*          * the source is not truecolor. Thanks to Frank Warmerdam.
	  *                   */

		if (src->trueColor) {
			for (y = 0; (y < h); y++) {
				for (x = 0; (x < w); x++) {
					int c = gdImageGetTrueColorPixel (src, srcX + x, srcY + y);
					gdImageSetPixel (dst, dstX + x, dstY + y, c);
				}
			}
		} else {
			/* source is palette based */
			for (y = 0; (y < h); y++) {
				for (x = 0; (x < w); x++) {
					int c = gdImageGetPixel (src, srcX + x, srcY + y);
					if (c != src->transparent) {
						gdImageSetPixel(dst, dstX + x, dstY + y, gdTrueColorAlpha(src->red[c], src->green[c], src->blue[c], src->alpha[c]));
					}
				}
			}
		}
		return;
	}

	for (i = 0; (i < gdMaxColors); i++)
	{
		colorMap[i] = (-1);
	}
	toy = dstY;
	for (y = srcY; (y < (srcY + h)); y++)
	{
		tox = dstX;
		for (x = srcX; (x < (srcX + w)); x++)
		{
			int nc;
			int mapTo;
			c = gdImageGetPixel (src, x, y);
			/* Added 7/24/95: support transparent copies */
			if (gdImageGetTransparent (src) == c)
			{
				tox++;
				continue;
			}
			/* Have we established a mapping for this color? */
			if (src->trueColor)
			{
                          /* 2.05: remap to the palette available in the
				destination image. This is slow and
				works badly, but it beats crashing! Thanks 
				to Padhrig McCarthy. */
				mapTo = gdImageColorResolveAlpha (dst,
						gdTrueColorGetRed (c),
						gdTrueColorGetGreen (c),
						gdTrueColorGetBlue (c),
						gdTrueColorGetAlpha (c));
			}
			else if (colorMap[c] == (-1))
			{
				/* If it's the same image, mapping is trivial */
				if (dst == src)
				{
					nc = c;
				}
				else
				{
                  /* Get best match possible. This
					function never returns error. */
					nc = gdImageColorResolveAlpha (dst,
							src->red[c], src->green[c],
							src->blue[c], src->alpha[c]);
				}
				colorMap[c] = nc;
				mapTo = colorMap[c];
			}
			else
			{
				mapTo = colorMap[c];
			}
			gdImageSetPixel (dst, tox, toy, mapTo);
			tox++;
		}
		toy++;
	}
}



double angle_from_dxdy(double dx, double dy)
{
	double angle;
	
	if (dx == 0)
	{
		angle = dy > 0 ? M_PI / 2.0 : 3.0 * M_PI / 2.0;
	} else {
		angle = atan(dy/dx);

		if (dx < 0)
			angle += M_PI;
	}

	return angle;
}


void drawTransformedLine(gdImagePtr im, double x1, double y1, double x2, double y2, int color, const plotOptions & p)
{
		gdImageLine(im,  (int)((x1+p.offX) * p.scale), 
						p.rh - (int)((y1+p.offY) * p.scale),
						(int)((x2+p.offX) * p.scale), 
						p.rh - (int)((y2+p.offY) * p.scale), color);
}

void drawTransformedLine(gdImagePtr im, Point a, Point b, int color, const plotOptions & p)
{
	drawTransformedLine(im, a.x, a.y, b.x, b.y, color, p);
}

void plot_rect(gdImagePtr im, const Rect & r, int color, const plotOptions & p)
{
	drawTransformedLine(im, r.getStartPoint(), r.getCWP1(),color,p);
	drawTransformedLine(im, r.getCWP1(), r.getEndPoint(),color,p);
	drawTransformedLine(im, r.getEndPoint(), r.getCWP2(),color,p);
	drawTransformedLine(im, r.getCWP2(), r.getStartPoint(),color,p);
}

void plot_arc(gdImagePtr im, double xs, double ys, double xe, double ye, double xc, double yc, int color, const plotOptions & p)
{

        double dxs = xs-xc;
        double dys = ys-yc;


        double dxe = xe-xc;
        double dye = ye-yc;

        double s_angle = angle_from_dxdy(dxs, dys);
        double e_angle = angle_from_dxdy(dxe, dye);

        double r = sqrt(dxs*dxs+dys*dys);

        double s_n_deg = -s_angle * 360.0 /(2.0*M_PI);
        double e_n_deg = -e_angle * 360.0 /(2.0*M_PI);


        while (s_n_deg < 0 || e_n_deg < 0)
        {
                s_n_deg += 360.0;
                e_n_deg += 360.0;
        }
	if (!p.drawfilled)
	        gdImageArc(im,  (int)((xc+p.offX) * p.scale), p.rh - (int)((yc+p.offY) * p.scale), (int)(2*r * p.scale), 
                        (int)(2*r * p.scale), (int)e_n_deg, (int)s_n_deg, color);
	else
		gdImageFilledArc(im,  (int)((xc+p.offX) * p.scale), p.rh - (int)((yc+p.offY) * p.scale), (int)(2*r * p.scale), 
                        (int)(2*r * p.scale), (int)e_n_deg, (int)s_n_deg, color,gdPie);
}

int tX(float x, const plotOptions & p)
{
	return (int)((x+p.offX) * p.scale);
}
int tY(float y, const plotOptions & p)
{
	return p.rh - (int)((y+p.offY) * p.scale);
}

void plot_vector_onto(Vector_Outp * vf, plotOptions &ps, gdImagePtr im)
{
	gdImagePtr im_old = im;
	gdImageAlphaBlending(im, 1);
	if (ps.drawinverted)
	{
		// create a temporary image
		
		im = gdImageCreateTrueColor(ps.rw, ps.rh);
		gdImageAlphaBlending(im, 0); 
		
		// Fill the image with this color - then we draw transparent areas on it
		int fcol = gdTrueColorAlpha(ps.ovr, ps.ovg, ps.ovb, 128 -(ps.alpha / 2));
		gdImageFilledRectangle(im, 0,0,ps.rw, ps.rh, fcol);
	}
	
	 
	// Setup the background
  	int black = gdImageColorAllocate(im, 0, 0, 0);
	
	// Our plot color
	int white = gdImageColorAllocate(im, 255, 255, 255);
 	int green = gdImageColorAllocate(im, 0, 255, 0);
	
 	int green2 = gdImageColorAllocate(im, 0, 230, 0);
 	int red = gdImageColorAllocate(im, 255, 0, 0);
	
	
 	int orange = gdImageColorAllocate(im, 255, 128, 0);
	int linecol = gdImageColorAllocate(im,128,128,128);
	int col[7] = {	gdImageColorAllocate(im, 255,0,0),
					gdImageColorAllocate(im, 0,255,0),
					gdImageColorAllocate(im, 0,0,255),
					gdImageColorAllocate(im, 255,255,0),
					gdImageColorAllocate(im, 0,255,255),
					gdImageColorAllocate(im, 255,0,255),
					gdImageColorAllocate(im, 255,255,255)};
					
	
 	int badowner = gdImageColorAllocate(im, 128, 0, 128);
	int badwidth = orange;
	int badspace = gdImageColorAllocate(im, 255, 0, 128);
	
	std::set<GerbObj*>::iterator it = vf->all.begin();
	int lcoldef=green;

	if (ps.colorByOVR)
		if (ps.drawinverted)
			lcoldef = gdTransparent;
		else
			lcoldef = gdTrueColorAlpha(ps.ovr, ps.ovg, ps.ovb, 128 -(ps.alpha / 2));
		
	for (; it != vf->all.end(); it++)
	{
		GerbObj * p = (*it);
	
			
		RenderPoly * rp = p->getPolyData();
		if (rp == NULL)
			continue;
		
		//lcoldef = green
		if (ps.colorByOVR)
		{
			// do nothing - default color already set
		}
		else if (ps.colorByGroups)
		{
			lcoldef = col[((int)rp->oid) % 7];
		}
		else
		{
			switch(p->flag)
			{
				case FLG_NONE:
					lcoldef = green;
					break;
				case FLG_WIDTH:
					lcoldef = badwidth;
					break;
				case FLG_SPACE:
					lcoldef = badspace;
					break;
				case FLG_NO_OWNER:
					lcoldef = badowner;
					break;
			}
		}
		
		//(int)(rp->r*255),(int)(rp->g*255),(int)(rp->b*255)
		std::vector<struct point_line*>::iterator v_it = rp->segs.begin();

		if (v_it == rp->segs.end())
			continue;

		assert (v_it != rp->segs.end());
		struct point_line * first_seg = (*v_it);
		v_it++;
		assert (v_it != rp->segs.end());
		struct point_line * last_seg = first_seg;
		
		for (;v_it != rp->segs.end(); v_it++)
		{
			if (last_seg->lt == LT_STRAIGHT)
				drawTransformedLine(im, last_seg->x, last_seg->y, (*v_it)->x, (*v_it)->y, lcoldef, ps);
			else
				plot_arc(im, last_seg->x,last_seg->y, (*v_it)->x, (*v_it)->y, last_seg->cx, last_seg->cy, lcoldef, ps);
				
			
			last_seg = *v_it;
		}
/*
			int l = 0.06f;
			gdImageSetThickness(im, 1);
			drawTransformedLine(im, first_seg->x - l, first_seg->y - l, first_seg->x +l, first_seg->y + l, orange, ps);
			drawTransformedLine(im, first_seg->x + l, first_seg->y - l, first_seg->x -l, first_seg->y + l, orange, ps);
			
			gdImageSetThickness(im, 1);
*/
		if (last_seg->lt == LT_STRAIGHT)
			drawTransformedLine(im, last_seg->x, last_seg->y, first_seg->x, first_seg->y, lcoldef, ps);
		else
			plot_arc(im, last_seg->x,last_seg->y, first_seg->x, first_seg->y, last_seg->cx, last_seg->cy, lcoldef, ps);
		
	
		if (ps.drawfilled)
		{
			int nPoints = rp->segs.size();
			gdPoint * polyPoints = new gdPoint[nPoints];
			for (int i=0; i< nPoints; i++)
			{
				polyPoints[i].x = tX(rp->segs[i]->x,ps);
				polyPoints[i].y = tY(rp->segs[i]->y,ps);
			}
			gdImageFilledPolygon(im, polyPoints, nPoints, lcoldef);
			delete polyPoints;
		}
		if (ps.showBoundingBoxes)
			plot_rect(im, p->getBounds(),col[4],ps);
		
	}
	
	std::set<GErr*>::iterator eit = vf->errors.begin();
	for (; eit != vf->errors.end(); eit++)
	{
		GErr * e = *eit;
		
		printf("Plotting error %lf %lf\n", e->x, e->y);
		
		double l = 0.02f;
		
		if (e->t == ERR_Unhand)
		{
			gdImageSetThickness(im, 1);
			drawTransformedLine(im, e->x - l, e->y - l, e->x +l, e->y + l, orange, ps);
			drawTransformedLine(im, e->x + l, e->y - l, e->x -l, e->y + l, orange, ps);
			
			gdImageSetThickness(im, 1);
		} else {
			drawTransformedLine(im, e->x, e->y, e->nx, e->ny, linecol, ps);
		}
	}
	if (ps.drawinverted)
	{
		gdImageAlphaBlending(im_old, 1);
		gdImageAlphaBlending(im, 1); 
		_cgdImageCopy(im_old,im,0,0,0,0,ps.rw, ps.rh);
	}
}

gdImagePtr plot_vector(Vector_Outp * vf, plotOptions &ps)
{
	gdImagePtr im = gdImageCreateTrueColor(ps.rw, ps.rh);
	plot_vector_onto(vf,ps,im);
	return im;
}

