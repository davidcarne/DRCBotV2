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

#include <boost/python.hpp>
#include "wrap_fns.h"
#include "plot_vector.h"
#include "gcode_interp.h"

int writePNGFile(char * file, gdImagePtr im)
{
	FILE * pngout = fopen(file, "wb");
	if (!pngout)
	{
		perror("");
		return -1;
	}
	gdImagePng(im, pngout);
	fclose(pngout);
	return 0;
}

void plotWrap()
{
	using namespace boost::python;
	
	class_<plotOptions>("plotOptions")
	.def_readwrite("showBoundingBoxes",&plotOptions::showBoundingBoxes)
	.def_readwrite("colorByGroups",&plotOptions::colorByGroups)
	.def_readwrite("colorByRandom",&plotOptions::colorByRandom)
	.def_readwrite("colorByOVR",&plotOptions::colorByOVR)
	.def_readwrite("ovr",&plotOptions::ovr)
	.def_readwrite("ovg",&plotOptions::ovg)
	.def_readwrite("ovb",&plotOptions::ovb)
	.def_readwrite("drawfilled",&plotOptions::drawfilled)
	.def_readwrite("drawinverted",&plotOptions::drawinverted)
	.def_readwrite("alpha",&plotOptions::alpha)
	.def_readwrite("scale",&plotOptions::scale)
	.def_readwrite("offX",&plotOptions::offX)
	.def_readwrite("offY",&plotOptions::offY)
	.def_readwrite("rw",&plotOptions::rw)
	.def_readwrite("rh",&plotOptions::rh);
	
	def("plot_vector", plot_vector, return_value_policy<return_opaque_pointer>());
	def("plot_vector_onto", plot_vector_onto);
	def("writePNGFile", writePNGFile);
}

