enum GH_op_t {
	GH_op_subtract,
	GH_op_intersect,
	GH_op_union
	};

struct GH_vertex;

// Pass NULL to start a new polygon
GH_vertex * GH_polyPoint(GH_vertex * v, double x, double y);
GH_vertex * GH_getPolyPoint(GH_vertex *v, double * x, double * y);

GH_vertex * GH_polygon_boolean(GH_vertex * p1, GH_vertex * p2, enum GH_op_t op);
void GH_free_polygons(GH_vertex * polys);

