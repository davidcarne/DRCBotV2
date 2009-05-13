struct drcSettings
{
	float minTraceWidth;
	float minTraceSpace;
	
};

bool doDRC(Vector_Outp * v, struct drcSettings * s, bool drawErrors);

void initDRC(struct drcSettings * s);

