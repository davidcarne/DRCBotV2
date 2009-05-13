#include <gd.h>
	
class plotOptions {
	public:
	plotOptions(){showBoundingBoxes = false; colorByGroups = false;};
	
	bool showBoundingBoxes;
	bool colorByGroups;
	bool colorByRandom;
	bool colorByOVR;

	unsigned char ovr,ovg,ovb;
	
	bool drawfilled;
	bool drawinverted;
	int alpha;

	double scale;
	double offX;
	double offY;
	int rw,rh;

};
gdImagePtr plot_vector(Vector_Outp * vf,  plotOptions & p);
void plot_vector_onto(Vector_Outp * vf,  plotOptions & p, gdImagePtr im);


