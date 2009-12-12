/*
 *  Portions Copyright 2006,2009 David Carne and 2007,2008 Spark Fun Electronics
 *
 *
 *  This file is part of gerberDRC.
 *
 *  gerberDRC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  gerberDRC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util_type.h"


Vector2D::Vector2D(double x, double y)
{
	m_start = Point(0,0);
	m_end = Point(x,y);
}

Vector2D::Vector2D(Point start, Point end)
{
	m_start = start;
	m_end = end;
}

Point Vector2D::getStart(void)
{
	return m_start;
}
Point Vector2D::getEnd(void)
{
	return m_end;
}
double Vector2D::getDeltaX(void)
{
	return m_end.x - m_start.x;
}
double Vector2D::getDeltaY(void)
{
	return m_end.y - m_start.y;
}

double cross(Vector2D & a, Vector2D & b)
{
	return a.getDeltaX()*b.getDeltaY() - a.getDeltaY() * b.getDeltaX();
}

Point::Point (double x, double y)
{
    this->x = x;
    this->y = y;
};

Point::Point ()
{
    this->x = 0;
    this->y = 0;
};

Rect::Rect ()
{
    set = false;
}


Rect::Rect (double x1, double y1, double x2, double y2)
{
    a.x = fmin (x1, x2);
    b.x = fmax (x1, x2);
    a.y = fmin (y1, y2);
    b.y = fmax (y1, y2);
    set = true;
}


Point Rect::getStartPoint () const
{
    return a;
};

Point Rect::getCWP1 () const
{
    return Point (b.x, a.y);
};

Point Rect::getEndPoint () const
{
    return b;
};

Point Rect::getCWP2 () const
{
    return Point (a.x, b.y);
};

double Rect::getWidth () const
{
    return b.x - a.x;
};

double Rect::getHeight () const
{
    return b.y - a.y;
};

void Rect::printRect () const
{
    printf ("Rect: [%f,%f] -> [%f,%f], w: %f, h: %f\n", a.x, a.y, b.x, b.y,
        getWidth (), getHeight ());
}


bool Rect::pointInRectOpen (double x, double y) const
{
    return x > a.x && x < b.x && y > a.y && y < b.y;
}


bool Rect::pointInRectClosed (Point a) const
{
    return pointInRectClosed (a.x, a.y);
}


bool Rect::pointInRectClosed (double x, double y) const
{
    return x >= a.x && x <= b.x && y >= a.y && y <= b.y;
}


void Rect::mergeBounds (const Rect & r)
{
    if (set)
    {
        a.x = fmin (a.x, r.a.x);
        b.x = fmax (b.x, r.b.x);
        a.y = fmin (a.y, r.a.y);
        b.y = fmax (b.y, r.b.y);
    }
    else
    {
        a = r.a;
        b = r.b;
        set = true;
    }
}


void Rect::mergePoint (const Point & r, double radius)
{
    if (set)
    {
        a.x = fmin (a.x, r.x - radius);
        b.x = fmax (b.x, r.x + radius);
        a.y = fmin (a.y, r.y - radius);
        b.y = fmax (b.y, r.y + radius);
    }
    else
    {
        a.x = r.x - radius;
        b.x = r.x + radius;
        a.y = r.y - radius;
        b.y = r.y + radius;
        set = true;
    }
}


void Rect::feather (double s)
{
    feather (s, s);
}


void Rect::feather (double x, double y)
{
    a.x -= x;
    a.y -= y;
    b.x += x;
    b.y += y;
}


bool Rect::intersectsWith (const Rect & r)
{
	return !(a.x > r.b.x || b.x < r.a.x ||
		a.y > r.b.y || b.y < r.b.y);
}
