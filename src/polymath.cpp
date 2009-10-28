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


#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef INT_ASSERT
#include <assert.h>
#else
#include "../tests/test_funcs.h"
#define assert TEST_ASSERT
#endif

#include "polymath.h"
#include "polymath_internal.h"

#define COMPARE_EPSILON 0.0000001
#define _FC(a,b) (fabs((a) - (b)) < COMPARE_EPSILON)
#define xprintf printf
GH_vertex * __find_non_intersect(GH_vertex * v)
{
	while (v && v->intersect)
		v = v->next;
	return v;
}
GH_vertex * __find_intersect(GH_vertex * v)
{
	while (v && !v->intersect)
		v = v->next;
	return v;
}
GH_vertex * __find_intersect_notdone(GH_vertex * v)
{
	while (v && (!v->intersect || v->done))
		v = v->next;
	return v;
}

GH_vertex * __find_last(GH_vertex * v)
{
	while (v && v->next)
		v = v->next;
	return v;
}


struct GH_vertex * alloc_GH_vertex()
{
	struct GH_vertex * v = (struct GH_vertex *)malloc(sizeof(struct GH_vertex));
	v->c.x = v->c.y = INFINITY;
	v->next = v->prev = v->nextPoly = v->neighbor = NULL;
	v->intersect = false;
	v->flag = FLG_NONE;
	v->cross_change = false;
	v->couple = NULL;
	v->alpha = 0;
	v->done = false;
	return v;
}

struct GH_vertex * GH_createIVertex(GH_point * A, GH_point * B, float a)
{
	assert(A != NULL);
	assert(B != NULL);
	assert(a > 0 && a < 1);

	double x = A->x * (1 - a) + B->x * a;
	double y = A->y * (1 - a) + B->y * a;
	GH_vertex * v = alloc_GH_vertex();
	v->alpha = a;
	v->intersect = true;
	v->c.x = x;
	v->c.y = y;
	return v;
}

void GH_linkVerticies(GH_vertex * a, GH_vertex * b)
{
	assert(a != NULL);
	assert(b != NULL);

	a->neighbor = b;
	b->neighbor = a;
}

// insertion point and where to insert
void GH_insertAfter(GH_vertex * ip, GH_vertex * ti)
{
	assert(ip != NULL);
	assert(ti != NULL);
	assert(ti->next == NULL);
	assert(ti->prev == NULL);


	ti->prev = ip;
	ti->next = ip->next;
	if (ip->next)
		ip->next->prev = ti;

	ip->next = ti;
}
void GH_unlink(GH_vertex * v)
{
	assert(v);
	if (v->prev)
		v->prev->next = v->next;
	if (v->next)
		v->next->prev = v->prev;
}

bool GH_polyHasIntersectingNotDone(GH_vertex * v)
{
	GH_vertex * a;

	FOR_VERTEX_I(v, a)
		if (!a->done)
			return true;
	END_FOR_VERTEX_I(v, a);

	return false;
}

void GH_sortedInsert(GH_vertex * P1, GH_vertex * P2, GH_vertex * I)
{
	assert(P1 != NULL);	assert(P2 != NULL);	assert(I != NULL);
	assert(!P1->intersect);	assert(!P2->intersect);	assert(I->intersect);
	assert(P1->alpha == 0);	assert(P2->alpha == 0);	assert(I->alpha > 0 && I->alpha < 1);

	GH_vertex * J = P1;
	// Need to find vertex J
	// 	such that !J->intersect || J->alpha <= I->alpha
	// 	and	!J->next || !J->next->intersect || J->alpha >= I->alpha
	
	while (	J && !((!J->intersect || J->alpha <= I->alpha) && 
		(!J->next || !J->next->intersect || J->next->alpha >= I->alpha)))
			J = J->next;
	assert(J);
	GH_insertAfter(J,I);


}

float GH_calc_WEC(GH_point * A, GH_point * B, GH_point * C, GH_point * D)
{
	assert(A != NULL); assert(B != NULL); assert(C != NULL); assert(D != NULL);
	// (A - B) dot perpendicular(C - D)
	return (float)(A->y - B->y) * (C->x - D->x) - (A->x - B->x) * (C->y - D->y);
}

int outcode(GH_point * p, double L, double R, double B, double T)
{
	assert(p != NULL);
	assert(L <= R); assert(B <= T);

	int outcode = 0;
	if (p->x < L)
		outcode |= OC_L;
	if (p->x > R)
		outcode |= OC_R;
	if (p->y < B)
		outcode |= OC_B;
	if (p->y > T)
		outcode |= OC_T;

	return outcode;
}

bool GH_pointCompare(GH_point * a, GH_point * b)
{
	return _FC(a->x, b->x) && _FC(a->y, b->y);
}

// Intersects P1->P2
// 	P1 Open, P2 Closed [aka, P1 on Q1->Q2 = NO INTERSECT]
enum intertype_e GH_intersect(GH_point * P1, GH_point * P2, GH_point * Q1, GH_point * Q2,
	float * alphaP, float * alphaQ)
{
	*alphaQ = -INFINITY;
	*alphaP = -INFINITY;
	assert(P1 != NULL); assert(P2 != NULL); assert(Q1 != NULL); assert(Q2 != NULL);
	assert(alphaP != NULL); assert(alphaQ != NULL);

	// First do outcodes
	// LRBT
	int outcode_P1 = 0;
	int outcode_P2 = 0;
	double winL = Q1->x > Q2->x ? Q2->x : Q1->x;
	double winR = Q1->x > Q2->x ? Q1->x : Q2->x;
	double winB = Q1->y > Q2->y ? Q2->y : Q1->y;
	double winT = Q1->y > Q2->y ? Q1->y : Q2->y;
	//printf("Window: %f %f %f %f ", winL, winR, winB, winT);
	outcode_P1 = outcode(P1, winL, winR, winB, winT);
	outcode_P2 = outcode(P2, winL, winR, winB, winT);
	//printf("Outcodes %x %x CMP %e\n", outcode_P1, outcode_P2, P2->y - winT);
	
	/* If any bits set in the outcode, no intersection possible, so abort */
	if (outcode_P1 & outcode_P2)
		return INTER_NONE;

	*alphaQ = INFINITY;
	*alphaP = INFINITY;

	/* Now do the window-edge-coordinate method of finding intersections */
	float WEC_P1 = GH_calc_WEC(P1, Q1, Q2, Q1);
	float WEC_P2 = GH_calc_WEC(P2, Q1, Q2, Q1);

	if (WEC_P1 * WEC_P2 <= 0)
	{
		float WEC_Q1 = GH_calc_WEC(Q1, P1, P2, P1);
		float WEC_Q2 = GH_calc_WEC(Q2, P1, P2, P1);
		if (WEC_Q1 * WEC_Q2 <= 0)
		{
			*alphaP = WEC_P1 / (WEC_P1 - WEC_P2);
			*alphaQ = WEC_Q1 / (WEC_Q1 - WEC_Q2);
			//printf("%f %f\n", *alphaP, *alphaQ);
			//printf("%d %d\n", _FC(*alphaP,1.0), _FC(1.0,*alphaQ));

			assert(isnan(*alphaP) && isnan(*alphaQ) || !isnan(*alphaP) && !isnan(*alphaQ));

			if (isnan(*alphaP) && isnan(*alphaQ))
			{
				return INTER_COINCIDE;
			}

			if (_FC(*alphaP, 0.0))
				return INTER_NONE;

			if (_FC(*alphaQ, 0.0))
				return INTER_NONE;

			if (_FC(*alphaP, 1.0))
				return INTER_TOUCH;
			
			if (_FC(*alphaQ, 1.0))
				return INTER_TOUCH;


			assert(*alphaP > 0.0 && *alphaP <= 1.0 && *alphaQ > 0.0 && *alphaP <= 1.0);

			return INTER_CROSS;
		}
	}
	return INTER_NONE;
}


bool GH_pointInPoly(GH_vertex * poly, GH_point * point)
{
	// TODO: optimize + fix
	GH_point end;
	float aa, ab;

	end.x = point->x+1000000;
	end.y = point->y;

	int c = 0;
	GH_vertex * a, *b;
	
	FOR_VERTEX_PAIR(poly, a, b)
		if (GH_intersect(VERTEX_POINT(a), VERTEX_POINT(b), point, &end, &aa, &ab))
			c++;
	END_FOR_VERTEX_PAIR(poly, a, b);
	return c & 0x1; // Odd winding # means inside poly, even outside.
}

float GH_calcAlpha(GH_point * point, GH_point * start, GH_point * finish)
{
	assert(point);
	assert(start);
	assert(finish);

	double dx = finish->x - start->x;
	double dy = finish->y - start->y;
	assert(!(_FC(dx, 0) && _FC(dy, 0)));

	float a = 0;
	if (!_FC(dx, 0))
	{
		a = (point->x - start->x) / dx;
	} else {
		a = (point->y - start->y) / dy;
	}
	assert(a > 0);
	assert(a < 1);
	assert(!_FC(a, 0));
	assert(!_FC(a, 1));

	return a;
}
void GH_insertLinkClonedVertex(GH_vertex * toclone, GH_vertex * before, GH_vertex * after)
{
	assert(toclone);
	assert(before);
	assert(after);
	assert(!toclone->neighbor);
	assert(before != after);
	assert(toclone != before);
	assert(toclone != after);

	GH_vertex * i1 = alloc_GH_vertex();

	i1->alpha = GH_calcAlpha(VERTEX_POINT(toclone), VERTEX_POINT(before), VERTEX_POINT(after));


	i1->intersect = true;
	i1->c.x = toclone->c.x;
	i1->c.y = toclone->c.y;

	// NOTE: we do NOT mark the point is an intersect yet, as that would munge up
	// processing [as its a corner, and therefore must be considered!]
	// toclone->intersect = true;
	GH_linkVerticies(i1, toclone);
	GH_sortedInsert(before, after, i1);
}
/*
 * Quoted from Greiner-Hormann '98
 *
 * Search for all intersection points by testing whether each edge of
 * the subject polygon and each of the clip polygon intersect or not.
 *
 * If they do, the intersection routine will deliver two numbers between
 * 0 and 1, the alpha values, which indicate where the intersection
 * point lies relatively to the start and end points of both edges.
 *
 * With respect to these alpha values, we create new verticies and insert
 * them into the data structures of subject and clip polygons between the 
 * start and end points of the edges they intersect. If no intersection
 * points are detected we know that either the subject polygon lies entirely
 * inside the clip polygon or vice versa, or both polygons are disjoint.
 *
 * By performing even-odd rule we can easily decide which case we have
 * and simply return either the inner polygon as the clipped polygon, 
 * or nothing at all
 *
 */
bool GH_phase_one(struct GH_vertex * subject, struct GH_vertex * clip)
{
	assert(subject != NULL);
	assert(clip != NULL);

	/*
	 * Pseudo code for phase one
	 * for each vertex Si of subject do
	 * 	for each vertex Cj of clip do
	 * 		if intersect Si, Si+1, Cj, Cj + 1, a, b
	 * 			I1 = CreateVertex(Si, Si+1, a)
	 * 			I2 = CreateVertex(Cj, Cj+1, b)
	 * 			link I1 + I2
	 * 			add I1 to subject poly
	 * 			add I2 to clip poly
	 * 		end if
	 * 	end for
	 * end for
	 */
	GH_vertex * s0, * s1;
	GH_vertex * c0, * c1;

	bool intersect_found = false;

	/* poly insertion list pointers */
	GH_vertex * sI = NULL, * cI = NULL;
	FOR_VERTEX_NI_PAIR(subject, s0, s1)
		FOR_VERTEX_NI_PAIR(clip, c0, c1)
			float a, b;
			enum intertype_e inter_type = GH_intersect(
					VERTEX_POINT(s0), VERTEX_POINT(s1), 
					VERTEX_POINT(c0), VERTEX_POINT(c1),
					&a, &b);

			/*printf("Intersection %1.20f %1.20f -> %1.20f %1.20f & %1.20f %1.20f -> %1.20f %1.20f == %d (%f %f)\n", s0->c.x, s0->c.y, s1->c.x, s1->c.y,
				c0->c.x, c0->c.y, c1->c.x, c1->c.y, inter_type,a,b);
*/
			if (inter_type != INTER_NONE)
			{

				// If the lines are simply crossing
				if (inter_type == INTER_CROSS)
				{
					GH_vertex * i1 = GH_createIVertex(
						VERTEX_POINT(s0), VERTEX_POINT(s1),
						a);
					GH_vertex * i2 = GH_createIVertex(
						VERTEX_POINT(c0), VERTEX_POINT(c1),
						b);
				
					GH_linkVerticies(i1, i2);

					GH_sortedInsert(s0, s1, i1);
					GH_sortedInsert(c0, c1, i2);
				} else if (inter_type == INTER_TOUCH) {
					assert(!_FC(a, 0.0));
					assert(!_FC(b, 0.0));

					assert(a < 1.0 && !(b < 1.0) || !(b<1.0) && a < 1.0);

					// The edge that is being touched gets an intersect vertex
					if (a < 1.0)
						GH_insertLinkClonedVertex(c1, s0, s1);
					else
						GH_insertLinkClonedVertex(s1, c0, c1);

				} else if (inter_type == INTER_COINCIDE) {
					int bits = GH_lineCoincideBits(
						VERTEX_POINT(s0), VERTEX_POINT(s1),
						VERTEX_POINT(c0), VERTEX_POINT(c1));
					
					/* A on CD is already generated by touch comparison
					 *if (bits & A_ONCD)
					 *	GH_insertLinkClonedVertex(s0, c0, c1);*/
					if (bits & B_ONCD)
						GH_insertLinkClonedVertex(s1, c0, c1);
					/* C on AB is already generated by touch comparison
					 * if (bits & C_ONAB)
						GH_insertLinkClonedVertex(c0, s0, s1);*/
					if (bits & D_ONAB)
						GH_insertLinkClonedVertex(c1, s0, s1);
					
					if (bits & A_IS_C)
						GH_linkVerticies(s0, c0);
					if (bits & A_IS_D)
						GH_linkVerticies(s0, c1);
					if (bits & B_IS_C)
						GH_linkVerticies(s1, c0);
					if (bits & B_IS_D)
						GH_linkVerticies(s1, c1);

				}
				intersect_found = true;
			}
		END_FOR_VERTEX_NI_PAIR(clip, c0, c1);
	END_FOR_VERTEX_NI_PAIR(subject, s0, s1);

	GH_vertex * i;
	FOR_VERTEX(clip, i)
		if (i->neighbor)
			i->intersect = true;
	END_FOR_VERTEX(clip, i);
	FOR_VERTEX(subject, i)
		if (i->neighbor)
			i->intersect = true;
	END_FOR_VERTEX(subject, i);

	return intersect_found;
}

bool GH_is_on(GH_vertex * a, GH_vertex * b)
{
	if (!a->intersect || !b->intersect)
		return false;

	assert(a->neighbor);
	assert(b->neighbor);

	GH_vertex * n_a = a->neighbor;
	GH_vertex * n_b = b->neighbor;

	// wraparound case
	if (n_a->next == NULL && n_b->prev == NULL ||
		n_a->prev == NULL && n_b->next == NULL)
		return true;
	
	// standard case
	if (n_a->next == n_b || n_b->next == n_a)
		return true;

	return false;
}

enum edge_status_t {
	edge_in,
	edge_on,
	edge_out
};

GH_point GH_midPoint(GH_point * a, GH_point * b)
{
	assert(a);
	assert(b);
	GH_point mid;
	mid.x = (a->x + b->x) / 2.0;
	mid.y = (a->y + b->y) / 2.0;
	return mid;
}
/*
 * Calculates edge status according to Kim-Kim 2006 4.1
 */
enum edge_status_t GHKK_edgeStatus(GH_vertex * a, GH_vertex * b, GH_vertex * other)
{
	assert(a);
	assert(b);
	assert(other);

	if (GH_is_on(a, b))
		return edge_in;

	GH_point mid = GH_midPoint(VERTEX_POINT(a),VERTEX_POINT(b));
	return GH_pointInPoly(other, &mid) ? edge_in : edge_out;
	
}

enum flag_type_e GHKK_calcVertexFlag(enum edge_status_t gamma_p, enum edge_status_t gamma_n)
{
	switch (gamma_p)
	{
		case edge_in:
			switch (gamma_n)
			{
				case edge_in:
					return FLG_EX_EN;
				case edge_on:
				case edge_out:
					return FLG_EX;
			}
			assert(false);

		case edge_on:
			switch (gamma_n)
			{
				case edge_in:
					return FLG_EN;
				case edge_on:
					return FLG_NONE;
				case edge_out:
					return FLG_EX;
			}
			assert(false);

		case edge_out:
			switch (gamma_n)
			{
				case edge_in:
				case edge_on:
					return FLG_EN;
				case edge_out:
					return FLG_EN_EX;
			}
			assert(false);
	}
	assert(false);
}

void GHKK_phase_two_firstpoly(struct GH_vertex * p, struct GH_vertex * other, enum GH_op_t op)
{
	enum edge_status_t gamma_p, gamma_n;

	GH_vertex * Cip, * Ci, * Cin;
	FOR_VERTEX_I_CENTRI(p, Cip, Ci, Cin)
		gamma_p = GHKK_edgeStatus(Cip, Ci, other);
		gamma_n = GHKK_edgeStatus(Ci, Cin, other);
		Ci->flag = GHKK_calcVertexFlag(gamma_p, gamma_n);
		if (Ci->flag == FLG_NONE)
		{
			Ci->intersect = false;
			Ci->neighbor->intersect = false;
			Ci->neighbor->neighbor = NULL;
			Ci->neighbor = NULL;
		}
	END_FOR_VERTEX_I_CENTRI(p, Cip, Ci, Cin);
}

enum flag_type_e GHKK_invertFlag(flag_type_e e)
{
	switch (e)
	{	
		case FLG_NONE:
			return FLG_NONE;
		case FLG_EN:
			return FLG_EX;
		case FLG_EX:
			return FLG_EN;
		case FLG_EN_EX:
			return FLG_EX_EN;
		case FLG_EX_EN:
			return FLG_EN_EX;
		default:
			assert(false);
	}
}
void GHKK_phase_two_secondpoly(struct GH_vertex * p, struct GH_vertex * other, enum GH_op_t op)
{
	GH_vertex * Ci = __find_intersect(p);
	GH_vertex * Cip = Ci->prev;
	GH_vertex * Cin = Ci->next;
	enum edge_status_t gamma_p, gamma_n;

	assert(Ci); assert(Cip); assert(Cin); assert(Ci->neighbor); assert(Ci->intersect);

	gamma_p = GHKK_edgeStatus(Cip, Ci, other);
	gamma_n = GHKK_edgeStatus(Ci, Cin, other);
	Ci->flag = GHKK_calcVertexFlag(gamma_p, gamma_n);

	enum flag_type_e Ciflag_inv = GHKK_invertFlag(Ci->flag);

	printf("gamma_p = %x, gamma_n = %x, Ci->flag = %x, CiInv = %x, Ci->neighbor->flag = %x\n", gamma_p, gamma_n, Ci->flag, Ciflag_inv, Ci->neighbor->flag);
	assert(Ci->flag == Ci->neighbor->flag || Ciflag_inv == Ci->neighbor->flag);

	bool flipflags = Ciflag_inv == Ci->neighbor->flag;

	GH_vertex * i;
	FOR_VERTEX_I(p, i)
		if (flipflags)
			i->flag = GHKK_invertFlag(i->neighbor->flag);
		else
			i->flag = i->neighbor->flag;
	END_FOR_VERTEX_I(p, i);

}
void GH_phase_two(struct GH_vertex * p1, struct GH_vertex * p2, enum GH_op_t op)
{
	GHKK_phase_two_firstpoly(p2, p1, op);
	GHKK_phase_two_secondpoly(p1, p2, op);
}

#if 0
/*
 * Quoted from Greiner-Hormann '98
 *
 * Phase two is analogous to the chalk cart in Section 3. We trace each
 * polygon once and mark entry and exit points to the other
 * polygons interior. We start at the polygons first vertex and detect
 * using the even/odd rule whether this point lies inside the other 
 * polygon or not. Then we move along the polygon verticies and mark the
 * intersecting points that have been inserted in phase one (and marked
 * by the intersect flag) alternately as entry and exit points respectively
 */
void GH_phase_two_perpoly(struct GH_vertex * p, struct GH_vertex * other, enum GH_op_t op)
{
	/* pseudo code for phase 2:
	 * If p0 inside other polygon
	 * 	status = exit
	 * else
	 * 	status = entry
	 * end if
	 * for each vertex Pi of polygon do
	 * 	if (pi->intersect then
	 * 		pi->entry_exit = status
	 * 		status = !status
	 * 	end if
	 * end for
	 */
	
	/* Entry true, exit false */
	flag_type_e status;

	switch (op)
	{
		case GH_op_intersect: /* clipping */
			status = !GH_pointInPoly(other, VERTEX_POINT(p)) ? FLG_EN : FLG_EX;
			break;
		case GH_op_union:
			status = GH_pointInPoly(other, VERTEX_POINT(p)) ? FLG_EN : FLG_EX;			
	}
	GH_vertex * v;
	
	FOR_VERTEX_I(p, v);
	{
		v->flag = status;
		switch (op)
		{
			case GH_op_intersect: /* clipping */
				status = status ? FLG_EX : FLG_EN;
			break;	
		}
	}
	END_FOR_VERTEX_I(p, v);
}

void GH_phase_two(struct GH_vertex * p1, struct GH_vertex * p2, enum GH_op_t op)
{

	//GH_phase_two_perpoly(p2, p1, op);
	//GH_phase_two_perpoly(p2, p1, op);
}

#endif



/* Writes a new polygon at the current poly ptr + moves the pointer to the new location */

#define GH_NEWPOLYGON() { current_poly_start = output_polygon_pos; \
	if (*output_polygon_pos) \
	output_vertex_pos = output_polygon_pos = &((*output_polygon_pos)->nextPoly);}

/* allocate a new vertex, store its location where the vertex_output_ptr points to + advance the
 * vertex output ptr to the ->next field */ 
void GH_newVertex(GH_vertex * current_vertex, GH_vertex *** vertex_output_ptr)
{
	**vertex_output_ptr = alloc_GH_vertex();
	(**vertex_output_ptr)->c = current_vertex->c;
	*vertex_output_ptr = &((**vertex_output_ptr)->next);
	printf("Create point %f %f [%p] %d\n", current_vertex->c.x, current_vertex->c.y, current_vertex, current_vertex->done);
}

int GH_polySize(GH_vertex * a)
{
	assert(a);
	int c = 1;
	while (a = a->next)
	{
		c++;
	}
	return c;
}

void polyDump(GH_vertex * p)
{
	do {
		xprintf("$$ X=%lf Y=%lf I=%d FLG=%d\n", p->c.x, p->c.y, p->intersect, p->flag);
	} while (p = p->next);
}

#define POINT_COMPARE(a, b) (_FC((a)->x, (b)->x) && _FC((a)->y, (b)->y))
#define RECT_BETWEEN(a, c,d)	((a->x > c->x && a->x < d->x || a->x > d->x && a->x < c->x) || \
		(a->y > c->y && a->y < d->y || a->y > d->x && a->y < c->y))
int GH_lineCoincideBits(GH_point * a, GH_point * b, GH_point * c, GH_point * d)
{
	int bits = 0;
	assert(!POINT_COMPARE(a,b));
	assert(!POINT_COMPARE(c,d));

	if (RECT_BETWEEN(a,c,d))
		bits |= A_ONCD;
	if (RECT_BETWEEN(b,c,d))
		bits |= B_ONCD;
	if (RECT_BETWEEN(c,a,b))
		bits |= C_ONAB;
	if (RECT_BETWEEN(d,a,b))
		bits |= D_ONAB;


	if (POINT_COMPARE(a,c))
		bits |= A_IS_C;
	if (POINT_COMPARE(b,c))
		bits |= B_IS_C;
	if (POINT_COMPARE(a,d))
		bits |= A_IS_D;
	if (POINT_COMPARE(b,d))
		bits |= B_IS_D;
	
	return bits;
}

#if 0
GH_vertex * GH_phase_three(struct GH_vertex * subject, struct GH_vertex * clip)
{
	GH_vertex * subject_tail = __find_last(subject);
	GH_vertex * clip_tail = __find_last(clip);
	GH_vertex * current;

	// Output variables
	GH_vertex * output_start = NULL;
	GH_vertex ** current_poly_start = NULL;
	GH_vertex ** output_polygon_pos = &output_start;
	GH_vertex ** output_vertex_pos = &output_start;
	
	int safetynet = GH_polySize(subject) + GH_polySize(clip);

	while (GH_polyHasIntersectingNotDone(subject))
	{
		current = __find_intersect_notdone(subject);
		assert(!current->done);

		GH_NEWPOLYGON();
		GH_newVertex(current, &output_vertex_pos);
		safetynet --;
		assert(safetynet);
		if (!safetynet)
		{
			printf("$$ N=Subject\n");
			polyDump(subject);
			printf("$$ N=Clip\n");
			polyDump(clip);
		}

		current->done = true;
		do {
			if (current->flag == FLG_EN)
			{
				printf("Start entry chain\n");
				/* is an entry */
				do {
					GH_vertex * last = current;
					current = current->next;
					if (current == NULL)
					{
						assert(last == subject_tail || last == clip_tail);
	
						// We're at the end of a polygon, fixup depending on which it is
						if (last == subject_tail)
							current = subject;
						else if (last == clip_tail)
							current = clip;
					}

					// Check if we're trying to add the first again
					if (!GH_pointCompare(VERTEX_POINT(current), VERTEX_POINT(*current_poly_start)))
					{
						safetynet --;
						if (!safetynet)
						{
							xprintf("$$ N=Subject\n");
							polyDump(subject);
							xprintf("$$ N=Clip:\n");
							polyDump(clip);
						}

						assert(safetynet);
						GH_newVertex(current, &output_vertex_pos);
					}
				} while (!current->intersect);
			} else if (current->flag = FLG_EX) {
				printf("Start exit chain\n");

				/* is an exit */
				do {
					GH_vertex * last = current;
					current = current->prev;
					if (current == NULL)
					{
						assert(last == subject || last == clip);

						// We're at the end of a polygon, fixup depending on which it is
						if (last == subject)
							current = subject_tail;
						else if (last == clip)
							current = clip_tail;
					}
	
					if (!GH_pointCompare(VERTEX_POINT(current), VERTEX_POINT(*current_poly_start)))
					{
						GH_newVertex(current, &output_vertex_pos);
						safetynet --;
						if (!safetynet)
						{
							printf("$$ N=Subject\n");
							polyDump(subject);
							printf("$$ N=Clip\n");
							polyDump(clip);
						}
						assert(safetynet);
					}
				} while (!current->intersect);
			}
			current->done = true;
			printf("Breakout!\n");
			current = current->neighbor;
		} while (!GH_pointCompare(VERTEX_POINT(current), VERTEX_POINT(*current_poly_start)));
	}
	return output_start;
}
#endif
/*
 *  For clip - p1 = subject, p2 = clip
 *
 *  p1 / p2 will be destroyed!
 */
GH_vertex * GH_polygon_boolean(GH_vertex * p1, GH_vertex * p2, enum GH_op_t op)
{
	if (!GH_phase_one(p1, p2))
		return NULL;

	//GH_phase_two(p1, p2, op);
	//return GH_phase_three(p1, p2);
	return NULL;
}


void GH_free_polygons(GH_vertex * polys)
{
	GH_vertex * cur_vertex;
	while (polys)
	{
		GH_vertex * nextpoly = polys->nextPoly;
		cur_vertex = polys;

		while (cur_vertex)
		{
			GH_vertex * tofree = cur_vertex;
			cur_vertex = cur_vertex->next;
			free(cur_vertex);
		}
		polys = nextpoly;
	}
}

/* Polygon accessor functions */
GH_vertex * GH_polyPoint(GH_vertex * v, double x, double y)
{
	GH_vertex * nv = alloc_GH_vertex();
	nv->c.x = x;
	nv->c.y = y;

	if (v)
		GH_insertAfter(v, nv);
	return nv;
}

GH_vertex * GH_getPolyPoint(GH_vertex *v, double * x, double * y)
{
	assert(v);
	assert(x);
	assert(y);

	*x = v->c.x;
	*y = v->c.y;

	return v->next;
}

