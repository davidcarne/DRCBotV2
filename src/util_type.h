#include "math.h"
#ifndef util_type
#define util_type
#include <stdio.h>


class Point {
	public:
		Point(double x, double y);
		Point();
		double x, y;
		
};

class Vector2D {
	public:
		Vector2D(double x, double y);
		Vector2D(Point start, Point end);
		
		Point getStart(void);
		Point getEnd(void);
		double getDeltaX(void);
		double getDeltaY(void);

	private:
		Point m_start, m_end;

};

double cross(Vector2D & a, Vector2D & b);

class Rect {

	public:
		Rect();
		Rect(double x1, double y1, double x2, double y2);
		
		Point getStartPoint() const;
		Point getCWP1() const;
		Point getEndPoint() const;
		Point getCWP2() const;
		
		double getWidth() const;
		
		double getHeight() const;
		void printRect()  const;
		bool pointInRectOpen(double x, double y) const;
		bool pointInRectClosed(Point a) const;
		
		bool pointInRectClosed(double x, double y) const;
		
		void mergeBounds(const Rect &r);
		
		void mergePoint(const Point &r);
		
		void feather(double s);
		void feather(double x, double y);
		bool intersectsWith(const Rect & r);
	private:
		Point a,b;
		bool set;
		
};

#endif


