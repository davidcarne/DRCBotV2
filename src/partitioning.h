#include <map>
#include <set>
#include "util_type.h"
#ifndef _PARTITIONING_H
#define _PARTITIONING_H

struct ltint
{
  bool operator()(const int s1, const int s2) const
  {
    return s1<s2;
  }
};
#define scalefactor 10

template <class T> class Part2D {

	public:
	typedef std::map<int, std::set<T>, ltint> spatialmap;
	
	void insertbounded(const Rect & r, T v)
	{
		insertbounded(r.getStartPoint().x,r.getStartPoint().y,r.getEndPoint().x,r.getEndPoint().y,v);
	}
	void insertbounded(float x1, float y1, float x2, float y2, T v)
	{
		int ix1 = (int)(scalefactor * x1);
		int ix2 = (int)(scalefactor * x2)+1;
		
		int iy1 = (int)(scalefactor * y1);
		int iy2 = (int)(scalefactor * y2)+1;
		
		for (int x=ix1; x<ix2; x++)
			for (int y=iy1; y<iy2; y++)
				data[xytoq(x,y)].insert(v);
		
	}
	
	void insert(float x, float y, T v)
	{
		std::set<T> * p = &data[xytoq((int)(scalefactor * x), (int)(scalefactor * y))];
		p->insert(v);
	}
	
	
	std::set<T> * retrieveFast(float x, float y)
	{
		int ix = (int)(scalefactor * x);
		int iy = (int)(scalefactor * y);
		return &data[xytoq(ix,iy)];
	}
	
	std::set<T> retrieve(float x, float y, float d)
	{
		return retrieve(x, y, d, d);
	}
	
	std::set<T> retrieve(Rect  r)
	{
		int sx = (int)(scalefactor * (r.getStartPoint().x));
		int sy = (int)(scalefactor * (r.getStartPoint().y));
		
		int ex = (int)(scalefactor * (r.getEndPoint().x)) + 1;
		int ey = (int)(scalefactor * (r.getEndPoint().y)) + 1;
		
		std::set<T> o;
		for (int i=sx; i<ex; i++)
			for (int j=sy; j<ey; j++)
			{
				std::set<T> d = data[xytoq(i,j)];
				o.insert(d.begin(), d.end());
			}
			
		return o;
	
	}
	// retrieves everything in a box thats +-d in both directions, may retrieve more
	std::set<T> retrieve(float x, float y, float dx, float dy)
	{
		int sx = (int)(scalefactor * (x - dx));
		int sy = (int)(scalefactor * (y - dy));
		int ex = (int)(scalefactor * (x + dx)) + 1;
		int ey = (int)(scalefactor * (y + dy)) + 1;
		
		std::set<T> o;
		for (int i=sx; i<ex; i++)
			for (int j=sy; j<ey; j++)
			{
				std::set<T> d = data[xytoq(i,j)];
				o.insert(d.begin(), d.end());
			}
			
		return o;
	}
	
		
	private:
		int xytoq(int x, int y)
		{
			return 2*x+3*y;
		}
		spatialmap data;
};



#endif

