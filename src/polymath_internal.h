struct GH_point {
	double x, y;
};


enum flag_type_e {
		FLG_NONE,
		FLG_EN,
		FLG_EX,
		FLG_EN_EX,
		FLG_EX_EN,
		};

struct GH_vertex {
	struct GH_point	c; //coords
	
	// Next + prev have special meaning when its an intersection queued
	//  next is next in insertion list, prev is the line segment its on
	struct GH_vertex * next, * prev;
	struct GH_vertex * nextPoly;

	bool	intersect;
	struct GH_vertex * neighbor;

	float	alpha;
	enum flag_type_e flag;
	struct GH_vertex * couple;
	bool	cross_change;
	bool 	done;

};

enum intertype_e {
	INTER_NONE,
	INTER_CROSS,
	INTER_TOUCH,
	INTER_COINCIDE,
};

// iterates over all pairs of nonintersection in a polygon
#define FOR_VERTEX_NI_PAIR(start, va, vb) \
	va = start; \
	vb = __find_non_intersect(start->next); \
	do { \
		if (vb == NULL) \
			vb = start;


#define END_FOR_VERTEX_NI_PAIR(start, va, vb) \
	va = vb; \
	vb = __find_non_intersect(va->next); \
	} while (va != start)

// iterates over all pairs of nonintersection in a polygon
#define FOR_VERTEX_I_CENTRI(start, vp, vi, vn) \
	vi = __find_intersect(start); \
	do { \
		vn = vi->next; \
		if (!vn) \
			vn = start;\
		vp = vi->prev; \
		assert(vp); \


#define END_FOR_VERTEX_I_CENTRI(start, vp, vi, vn) \
	vi = __find_intersect(vi->next); \
	} while (vi != NULL)

// Loop over all pairs in a polygon
#define FOR_VERTEX_PAIR(start, va, vb) \
	va = start; \
	vb = start->next; \
	do { \
		if (vb == NULL) \
			vb = start;

#define END_FOR_VERTEX_PAIR(start, va, vb) \
	va = vb; \
	vb = va->next; \
	} while (va != start)

// Loop over all intersection verticies in a polygon
#define FOR_VERTEX_I(start, va) \
	va = __find_intersect(start); \
	while (va != NULL) {


#define END_FOR_VERTEX_I(start, va) \
	va = __find_intersect(va->next); \
	}

// Loop over all verticies in a polygon
#define FOR_VERTEX(start, va) \
	va = start; \
	while (va != NULL) {


#define END_FOR_VERTEX(start, va) \
	va = va->next; \
	}

#define VERTEX_POINT(v) &((v)->c)

GH_vertex * __find_non_intersect(GH_vertex * v); /*HASTEST*/
GH_vertex * __find_intersect(GH_vertex * v); /*HASTEST*/
GH_vertex * __find_last(GH_vertex * v); /*HASTEST*/
GH_vertex * __find_intersect_notdone(GH_vertex * v);

struct GH_vertex * alloc_GH_vertex();
struct GH_vertex * GH_createIVertex(GH_point * A, GH_point * B, float a); /*HASTEST*/
void GH_linkVerticies(GH_vertex * a, GH_vertex * b); /* HASTEST */
void GH_insertAfter(GH_vertex * ip, GH_vertex * ti); /* HASTEST */
void GH_unlink(GH_vertex * v); /* HASTEST */
bool GH_polyHasIntersectingNotDone(GH_vertex * v); /* HASTEST */
void GH_sortedInsert(GH_vertex * P1, GH_vertex * P2, GH_vertex * I); /* HASTEST */
float GH_calc_WEC(GH_point * A, GH_point * B, GH_point * C, GH_point * D); /* HASTEST */
int outcode(GH_point * p, double L, double R, double B, double T); /* HASTEST */
bool GH_pointCompare(GH_point * a, GH_point * b); /* HASTEST */
enum intertype_e GH_intersect(GH_point * P1, GH_point * P2, GH_point * Q1, GH_point * Q2,
	float * alphaP, float * alphaQ); /* HASTEST */
	

bool GH_pointInPoly(GH_vertex * poly, GH_point * point); /* HASTEST */
float GH_calcAlpha(GH_point * point, GH_point * start, GH_point * finish); /* HASTEST */
int GH_polySize(GH_vertex * a); /* HASTEST */

bool GH_phase_one(struct GH_vertex * subject, struct GH_vertex * clip); /* HASTEST */
void GH_phase_two(struct GH_vertex * p1, struct GH_vertex * p2, enum GH_op_t op);
GH_vertex * GH_phase_three(struct GH_vertex * p1, struct GH_vertex * p2);

/*
 * This function does not indicate bits where A or B = C or D, only when there is an intersection point
 * that needs to be created
 */
int GH_lineCoincideBits(GH_point * a, GH_point * b, GH_point * c, GH_point * d);

#define OC_L (1 << 3)
#define OC_R (1 << 2)
#define OC_B (1 << 1)
#define OC_T (1 << 0)

#define A_ONCD (1<<3)
#define B_ONCD (1<<2)
#define C_ONAB (1<<1)
#define D_ONAB (1<<0)

#define A_IS_C (1<<7)
#define A_IS_D (1<<6)
#define B_IS_C (1<<5)
#define B_IS_D (1<<4)


