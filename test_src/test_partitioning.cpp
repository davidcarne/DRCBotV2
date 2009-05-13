#include "partitioning.h"
#include <iostream>

int main()
{
	Part2D<int> a;
	
	a.insert(0,0,3);
	a.insert(0,0,2);
	a.insert(1,1,4);
	a.insert(2,2,5);
	std::set<int> n = a.retrieve(0,0,2);
	
	std::set<int>::iterator it = n.begin();
	for (;it!=n.end(); it++)
		std::cout << *it << std::endl;
}

