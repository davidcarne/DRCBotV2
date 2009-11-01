#ifndef _PAIR_H_
#define _PAIR_H_

#include "stdio.h"
template<class T1, class T2>
struct PairToTupleConverter {
	static PyObject* convert(const std::pair<T1, T2>& pair) {
		using namespace boost::python;
		return incref(make_tuple(pair.first, pair.second).ptr());
	}
};

#endif

