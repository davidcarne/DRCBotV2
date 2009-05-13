#include <stdio.h>
#include <stdlib.h>

#include "test_funcs.h"
#include "../src/polymath.h"
#include "../src/polymath_internal.h"
#define assert TEST_ASSERT

void GH_insertAfter_Test()
{
	START_TEST("GH_insertAfter");
	GH_vertex * v1 = alloc_GH_vertex();
	GH_vertex * v2 = alloc_GH_vertex();
	GH_vertex * vi = alloc_GH_vertex();
	v2->intersect = true;
	
	GH_insertAfter(v1, v2);
	TEST_OUTPUT(v1->next == v2);
	TEST_OUTPUT(v2->prev == v1);
	TEST_OUTPUT(v1->prev == NULL);
	TEST_OUTPUT(v2->next == NULL);
		
	GH_insertAfter(v1, vi);
	TEST_OUTPUT(v1->next == vi);
	TEST_OUTPUT(vi->next == v2);
	TEST_OUTPUT(v2->next == NULL);
	TEST_OUTPUT(v1->prev == NULL);
	TEST_OUTPUT(vi->prev == v1);
	TEST_OUTPUT(v2->prev == vi);

	END_TEST();

	START_TEST("GH_unlink");
	GH_unlink(vi);
	TEST_OUTPUT(v1->next == v2);
	TEST_OUTPUT(v2->prev == v1);
	END_TEST();

	START_TEST("GH_polyHasIntersectingNotDone (true)");
	TEST_OUTPUT(GH_polyHasIntersectingNotDone(v1));
	END_TEST();
	v2->done = true;
	START_TEST("GH_polyHasIntersectingNotDone (false - done)");
	TEST_OUTPUT(!GH_polyHasIntersectingNotDone(v1));
	END_TEST();
	v2->done = false;
	v2->intersect = false;
	START_TEST("GH_polyHasIntersectingNotDone (false - not intersect)");
	TEST_OUTPUT(!GH_polyHasIntersectingNotDone(v1));
	END_TEST();

	free(v1);
	free(v2);
	free(vi);

}

void GH_createIVertex_Test()
{
	START_TEST("GH_createIVertex");
	GH_point A, B;
	A.x = 3, A.y = 7;
	B.x = -1, B.y = 14;

	GH_vertex * v = GH_createIVertex(&A,&B,0.25);
	TEST_OUTPUT(v!=NULL);
	TEST_OUTPUT(v->intersect);
	TEST_EQUALS_F(v->c.x, 2);
	TEST_EQUALS_F(v->c.y, 8.75);
	TEST_EQUALS_F(v->alpha, 0.25);
	TEST_OUTPUT(v->neighbor == NULL);
	free(v);
	END_TEST();
}

void GH_linkVerticies_Test()
{
	START_TEST("GH_linkVerticies");
	GH_vertex * l1 = alloc_GH_vertex();
	GH_vertex * l2 = alloc_GH_vertex();
	GH_linkVerticies(l1,l2);
	TEST_OUTPUT(l1->neighbor == l2);
	TEST_OUTPUT(l2->neighbor == l1);
	free(l1);
	free(l2);
	END_TEST();
}

void outcode_test(void)
{
	START_TEST("outcode");
	/* 1001 0001 0101
	 *      ----
	 * 1000 |  | 0100
	 *      ----
	 * 1010 0010 0110 
	 *
	 * 0,0
	 */
	GH_point p;
	p.x = 3;
	p.y = 3;
	int oc_top = outcode(&p, 1, 4, 0, 2);
	TEST_EQUALS_X(oc_top, 0x1);
	int oc_bot = outcode(&p, 1, 4, 4, 5);
	TEST_EQUALS_X(oc_bot, 0x2);
	int oc_left = outcode(&p, 4, 5, 2, 5);
	TEST_EQUALS_X(oc_left, 0x8);
	int oc_right = outcode(&p, 1, 2, 1, 5);
	TEST_EQUALS_X(oc_right, 0x4);

	int oc_lefttop = outcode(&p, 4, 5, 0, 2);
	TEST_EQUALS_X(oc_lefttop, 0x9);
	int oc_righttop = outcode(&p, 1, 2, 0, 2);
	TEST_EQUALS_X(oc_righttop, 0x5);
	int oc_leftbot = outcode(&p, 4, 5, 4, 5);
	TEST_EQUALS_X(oc_leftbot, 0xA);
	int oc_rightbot = outcode(&p, 1, 2, 4, 5);
	TEST_EQUALS_X(oc_rightbot, 0x6);

	int oc_inside = outcode(&p, 1, 1, 5, 5);
	TEST_EQUALS_X(oc_inside, 0x6);

	END_TEST();
}
void __find_non_intersect_test(void)
{
	START_TEST("__find_non_intersect");
	{
	GH_vertex * v1 = alloc_GH_vertex();
	GH_vertex * v2 = alloc_GH_vertex();
	GH_vertex * vi = alloc_GH_vertex();
	vi->intersect = 1;

	GH_vertex * f = __find_non_intersect(v1->next);
	TEST_OUTPUT(f == NULL);
	GH_insertAfter(v1, vi);
	f = __find_non_intersect(v1->next);
	TEST_OUTPUT(f == NULL);

	GH_insertAfter(vi, v2);
	f = __find_non_intersect(v1->next);
	TEST_OUTPUT(f == v2);
	free(v1); free(v2); free(vi);

	}
	END_TEST();

	START_TEST("__find_intersect");
	{
	GH_vertex * v1 = alloc_GH_vertex();
	GH_vertex * v2 = alloc_GH_vertex();
	GH_vertex * vi = alloc_GH_vertex();
	vi->intersect = 1;

	GH_vertex * f = __find_intersect(v1);
	TEST_OUTPUT(f == NULL);

	GH_insertAfter(v1, vi);
	f = __find_intersect(v1);
	TEST_OUTPUT(f == vi);

	GH_insertAfter(vi, v2);
	f = __find_intersect(v1);
	TEST_OUTPUT(f == vi);

	free(v1); free(v2); free(vi);
	}
	END_TEST();

	START_TEST("__find_last");
	{
	GH_vertex * v1 = alloc_GH_vertex();
	GH_vertex * v2 = alloc_GH_vertex();
	GH_vertex * vi = alloc_GH_vertex();
	vi->intersect = 1;

	GH_insertAfter(v1, vi);
	GH_insertAfter(vi, v2);

	TEST_OUTPUT(__find_last(v1) == v2);
	free(v1); free(v2); free(vi);
	}
	END_TEST();


}

void GH_calc_WEC_test(void)
{
	START_TEST("GH_calc_WEC");
	GH_point A,B,C,D;
	A.x = -4; A.y = 0; B.x = 4; B.y = 0; 
	C.x = 0; C.y = 4; D.x = 0; D.y = -4;

	float WEC;
	
	WEC = GH_calc_WEC(&A,&C,&D,&C);
	TEST_EQUALS_F(WEC, -32);
	WEC = GH_calc_WEC(&B,&C,&D,&C);
	TEST_EQUALS_F(WEC, 32);

	WEC = GH_calc_WEC(&C,&A,&B,&A);
	TEST_EQUALS_F(WEC, 32);
	WEC = GH_calc_WEC(&D,&A,&B,&A);
	TEST_EQUALS_F(WEC, -32);

	END_TEST();
}
void GH_intersect_test(void)
{
	START_TEST("GH_intersect (intersecting)");

	GH_point A,B,C,D;
	A.x = 0; A.y = 0;
	B.x = 5; B.y = 0;
	C.x = 5; C.y = -1;
	D.x = 0; D.y = 4;
	float a,b;
	enum intertype_e intersect;
	intersect = GH_intersect(&A, &B, &C, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_CROSS);
	TEST_EQUALS_F(a, 0.8);
	TEST_EQUALS_F(b, 0.2);

	A.x = -4; A.y = 0; B.x = 4; B.y = 0; 
	C.x = 0; C.y = 4; D.x = 0; D.y = -4;
	intersect = GH_intersect(&A, &B, &C, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_CROSS);
	TEST_EQUALS_F(a, 0.5);
	TEST_EQUALS_F(b, 0.5);

	D.x = 0; D.y = -0;
	intersect = GH_intersect(&A, &B, &C, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_TOUCH);
	TEST_EQUALS_F(a, 0.5);
	TEST_EQUALS_F(b, 1.0);

	END_TEST();

	START_TEST("GH_intersect (nonintersecting)");
	D.x = 0; D.y = 0.0001;
	intersect = GH_intersect(&A, &B, &C, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_NONE);
	D.x = 4.1; D.y = -0.0001;
	intersect = GH_intersect(&A, &B, &C, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_NONE);
	END_TEST();

	START_TEST("GH_intersect (AB Closed endpoint on CD - intersect)");
	C.x = 4; C.y = 0;
	D.x = 0; D.y = 0;
	A.x = 0; A.y = -1;
	B.x = 2; B.y = 0;
	intersect = GH_intersect(&A, &B, &C, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_TOUCH);
	TEST_EQUALS_F(a, 1.0);
	TEST_EQUALS_F(b, 0.5);
	END_TEST();

	START_TEST("GH_intersect (AB Open endpoint on CD - nonintersect)");
	intersect = GH_intersect(&B, &A, &C, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_NONE);
	TEST_EQUALS_F(a, 0.0);
	TEST_EQUALS_F(b, 0.5);
	END_TEST();

	START_TEST("GH_intersect (CD Closed endpoint on AB - intersect)");
	intersect = GH_intersect(&C, &D, &A, &B, &a, &b);
	TEST_EQUALS_X(intersect,INTER_TOUCH);
	TEST_EQUALS_F(a, 0.5);
	TEST_EQUALS_F(b, 1.0);
	END_TEST();

	START_TEST("GH_intersect (CD Open endpoint on AB - nonintersect)");
	intersect = GH_intersect(&C, &D, &B, &A, &a, &b);
	TEST_EQUALS_X(intersect,INTER_NONE);
	TEST_EQUALS_F(a, 0.5);
	TEST_EQUALS_F(b, 0.0);
	END_TEST();

	START_TEST("GH_intersect (AB coinciding CD - intersect)");
	A.x = 0; A.y = 0;
	B.x = 3; B.y = 3;
	C.x = 1; C.y = 1;
	D.x = 4; D.y = 4;
	intersect = GH_intersect(&A, &B, &C, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_COINCIDE);
	END_TEST();

	START_TEST("GH_intersect (AB coinciding CD, but no intersect)");
	intersect = GH_intersect(&A, &C, &B, &D, &a, &b);
	TEST_EQUALS_X(intersect,INTER_NONE);
	END_TEST();

	START_TEST("GH_intersect (CD coinciding AB, intersect)");
	intersect = GH_intersect(&C, &D, &A, &B, &a, &b);
	TEST_EQUALS_X(intersect,INTER_COINCIDE);
	END_TEST();


}

void GH_LineBits_Test()
{
	int bits;
	GH_point a,b,c,d;
	a.x = 1; a.y = 0;
	b.x = 3; b.y = 0;
	c.x = 0; c.y = 0;
	d.x = 4; d.y = 0;


	START_TEST("GH_LineBits AB on CD [X]");
	bits = GH_lineCoincideBits(&a,&b,&c,&d);
	TEST_EQUALS_X(bits, A_ONCD | B_ONCD);
	END_TEST();

	START_TEST("GH_LineBits CD on AB [X]");
	bits = GH_lineCoincideBits(&c,&d,&a,&b);
	TEST_EQUALS_X(bits, C_ONAB | D_ONAB);
	END_TEST();

	START_TEST("GH_LineBits CD equals AB [X]");
	bits = GH_lineCoincideBits(&a,&b,&a,&b);
	TEST_EQUALS_X(bits, A_IS_C | B_IS_D);
	END_TEST();

	a.y = 1; a.x = 0;
	b.y = 3; b.x = 0;
	c.y = 0; c.x = 0;
	d.y = 4; d.x = 0;

	START_TEST("GH_LineBits AB on CD [Y]");
	bits = GH_lineCoincideBits(&a,&b,&c,&d);
	TEST_EQUALS_X(bits, A_ONCD | B_ONCD);
	END_TEST();

	START_TEST("GH_LineBits CD on AB [Y]");
	bits = GH_lineCoincideBits(&c,&d,&a,&b);
	TEST_EQUALS_X(bits, C_ONAB | D_ONAB);
	END_TEST();

	START_TEST("GH_LineBits CD equals AB [Y]");
	bits = GH_lineCoincideBits(&a,&b,&a,&b);
	TEST_EQUALS_X(bits, A_IS_C | B_IS_D);
	END_TEST();

	START_TEST("GH_LineBits overlap");
	bits = GH_lineCoincideBits(&c,&b,&a,&d);
	TEST_EQUALS_X(bits, B_ONCD | C_ONAB);
	END_TEST();
}

void GH_calcAlpha_Test()
{
	GH_point a,b,p;
	START_TEST("GH_calcAlpha");
	a.x = 3; a.y = 0;
	b.x = 7; a.y = 0;
	p.x = 4; p.y = 0;
	float alpha = GH_calcAlpha(&p,&a,&b);
	TEST_EQUALS_F(alpha, 0.25);
	p.x = 6;
	alpha = GH_calcAlpha(&p,&a,&b);
	TEST_EQUALS_F(alpha, 0.75);
	a.y = 3; a.x = 0;
	b.y = 7; b.x = 0;
	p.y = 4; p.x = 0;
	alpha = GH_calcAlpha(&p,&a,&b);
	TEST_EQUALS_F(alpha, 0.25);
	p.y = 6;
	alpha = GH_calcAlpha(&p,&a,&b);
	TEST_EQUALS_F(alpha, 0.75);
	a.y = 3; a.x = -3;
	b.y = 7; b.x = -7;
	p.y = 4; p.x = -4;
	alpha = GH_calcAlpha(&p,&a,&b);
	TEST_EQUALS_F(alpha, 0.25);
	p.y = 6; p.x = -6;
	alpha = GH_calcAlpha(&p,&a,&b);
	TEST_EQUALS_F(alpha, 0.75);
	END_TEST();
}
void GH_sortedInsert_Test()
{
	START_TEST("GH_sortedInsert");
	GH_vertex * v1 = alloc_GH_vertex();
	GH_vertex * v5 = alloc_GH_vertex();
	GH_vertex * v6 = alloc_GH_vertex();

	GH_vertex * vi2 = alloc_GH_vertex();
	GH_vertex * vi3 = alloc_GH_vertex();
	GH_vertex * vi4 = alloc_GH_vertex();
	GH_vertex * vi7 = alloc_GH_vertex();


	vi3->intersect = vi2->intersect = vi4->intersect = vi7->intersect = true;
	vi3->alpha = 0.5;
	vi2->alpha = 0.2;
	vi4->alpha = 0.7;

	GH_insertAfter(v1, v5);
	TEST_OUTPUT(v1->next == v5);

	GH_sortedInsert(v1, v5, vi3);
	TEST_OUTPUT(v1->next == vi3);
	TEST_OUTPUT(vi3->next == v5);

	GH_sortedInsert(v1, v5, vi2);
	TEST_OUTPUT(v1->next == vi2);
	TEST_OUTPUT(vi2->next == vi3);
	TEST_OUTPUT(vi3->next == v5);

	GH_sortedInsert(v1, v5, vi4);

	TEST_OUTPUT(v1->next == vi2);
	TEST_OUTPUT(vi2->next == vi3);
	TEST_OUTPUT(vi3->next == vi4);
	TEST_OUTPUT(vi4->next == v5);
	END_TEST();
	GH_insertAfter(v5, v6);
	GH_insertAfter(v6, vi7);


	START_TEST("GH_polySize");
	TEST_EQUALS_X(GH_polySize(v1), 7);
	END_TEST();
	int i=0;
	GH_vertex * va, *vb;
	/***********************************************/
	i=0;
	START_TEST("FOR / END VertexPair looptest");
	
	FOR_VERTEX_PAIR(v1, va, vb) 
	switch (i)
	{
		case 0:
			TEST_OUTPUT(va == v1);
			TEST_OUTPUT(vb == vi2);
			break;
		case 1:
			TEST_OUTPUT(va == vi2);
			TEST_OUTPUT(vb == vi3);
			break;
		case 2:
			TEST_OUTPUT(va == vi3);
			TEST_OUTPUT(vb == vi4);
			break;	
		case 3:
			TEST_OUTPUT(va == vi4);
			TEST_OUTPUT(vb == v5);
			break;
		case 4:
			TEST_OUTPUT(va == v5);
			TEST_OUTPUT(vb == v6);
			break;
		case 5:
			TEST_OUTPUT(va == v6);
			TEST_OUTPUT(vb == vi7);
			break;
		case 6:
			TEST_OUTPUT(va == vi7);
			TEST_OUTPUT(vb == v1);
			break;


		default:
			TEST_OUTPUT(i < 7);

	}
	i++;
	END_FOR_VERTEX_PAIR(v1, va, vb);
	TEST_OUTPUT(i==7);
	END_TEST();
	/***********************************************/
	i=0;
	START_TEST("FOR / END Vertex looptest");
	
	FOR_VERTEX(v1, va) 
	switch (i)
	{
		case 0:
			TEST_OUTPUT(va == v1);
			break;
		case 1:
			TEST_OUTPUT(va == vi2);
			break;
		case 2:
			TEST_OUTPUT(va == vi3);
			break;	
		case 3:
			TEST_OUTPUT(va == vi4);
			break;
		case 4:
			TEST_OUTPUT(va == v5);
			break;
		case 5:
			TEST_OUTPUT(va == v6);
			break;
		case 6:
			TEST_OUTPUT(va == vi7);
			break;


		default:
			TEST_OUTPUT(i < 7);

	}
	i++;
	END_FOR_VERTEX(v1, va);
	TEST_OUTPUT(i==7);
	END_TEST();

	/***********************************************/
	i=0;
	START_TEST("FOR / END NI VertexPair looptest");
	
	FOR_VERTEX_NI_PAIR(v1, va, vb) 

	switch (i)
	{
		case 0:
			TEST_OUTPUT(va == v1);
			TEST_OUTPUT(vb == v5);
			break;
		case 1:
			TEST_OUTPUT(va == v5);
			TEST_OUTPUT(vb == v6);
			break;
		case 2:
			TEST_OUTPUT(va == v6);
			TEST_OUTPUT(vb == v1);
			break;
		default:
			TEST_OUTPUT(i < 3);

	}
	i++;
	END_FOR_VERTEX_NI_PAIR(v1, va, vb);
	TEST_OUTPUT(i==3);
	END_TEST();

	/***********************************************/
	i=0;
	START_TEST("FOR / END I Vertex");
	
	FOR_VERTEX_I(v1, va) 

	switch (i)
	{
		case 0:
			TEST_OUTPUT(va == vi2);
			break;
		case 1:
			TEST_OUTPUT(va == vi3);
			break;
		case 2:
			TEST_OUTPUT(va == vi4);
			break;
		case 3:
			TEST_OUTPUT(va == vi7);
			break;

		default:
			TEST_OUTPUT(i < 4);

	}
	i++;
	END_FOR_VERTEX_I(v1, va);
	TEST_OUTPUT(i==4);
	END_TEST();
	i=0;
	START_TEST("FOR / END I Vertex");
	
	GH_vertex * vp, * vn;
	FOR_VERTEX_I_CENTRI(v1, vp,va,vn) 

	switch (i)
	{
		case 0:
			TEST_OUTPUT(vp == v1);
			TEST_OUTPUT(va == vi2);
			TEST_OUTPUT(vn == vi3);
			break;
		case 1:	
			TEST_OUTPUT(vp == vi2);
			TEST_OUTPUT(va == vi3);
			TEST_OUTPUT(vn == vi4);
			break;
		case 2:
			TEST_OUTPUT(vp == vi3);
			TEST_OUTPUT(va == vi4);
			TEST_OUTPUT(vn == v5);
			break;
		case 3:
			TEST_OUTPUT(vp == v6);
			TEST_OUTPUT(va == vi7);
			TEST_OUTPUT(vn == v1);
			break;

		default:
			TEST_OUTPUT(i < 4);

	}
	i++;
	END_FOR_VERTEX_I_CENTRI(v1, vp,va,vn);
	TEST_OUTPUT(i==4);
	END_TEST();

	free(v1); free(v5); free(v6);
	free(vi2); free(vi3); free(vi4); free(vi7);
}


void print_vertex_graphviz(GH_vertex * a)
{
	printf("_%010p", a);
}
void print_poly_graphviz(GH_vertex * a, char * s)
{
	printf("\tsubgraph cluster_%s {\n", s);
	printf("\t\tstyle=filled;\n\t\tcolor=lightgrey;\n");
	printf("\t\tlabel=\"%s\";\n", s);
	
	GH_vertex * i = a;	
	while (i)
	{	printf("\t\t");

		print_vertex_graphviz(i);
		printf(" [label=\"(%.3f,%.3f)\"];\n",i->c.x, i->c.y);
		i = i->next;

	}

	printf("\t\t");
	i = a;	
	print_vertex_graphviz(i);
	i = i->next;

	while (i)
	{
		printf(" -> ");
		print_vertex_graphviz(i);
		i = i->next;
	}
	printf(";\n");
	printf("\t}\n");

	i = a;
	while (i)
	{
	if (i->intersect)
	{
		printf("\t");
		print_vertex_graphviz(i);
		printf(" -> ");
		print_vertex_graphviz(i->neighbor);
		printf(";\n");
	}
		i = i->next;
	}
}
void print_polygons_graphviz(GH_vertex * a, GH_vertex * b)
{
	printf("digraph G {\n");
	print_poly_graphviz(a, "subject");
	print_poly_graphviz(b, "clip");
	printf("}\n");

	
}
void print_polygon(GH_vertex * p)
{
	while (p)
	{
		printf("%010p %f,%f %d %010p\n", p, p->c.x, p->c.y, p->intersect, p->neighbor);
		p = p->next;
	}
}


void GH_pointCompare_test()
{
	GH_point a,b;
	a.x = 40;
	a.y = 27;
	b.x = 40;
	b.y = 3*9;

	bool m;
	START_TEST("GH_pointCompare (true)");
	m = GH_pointCompare(&a, &b);
	TEST_OUTPUT(m);
	END_TEST();
	a.x = 0;

	START_TEST("GH_pointCompare (false)");
	m = GH_pointCompare(&a, &b);
	TEST_OUTPUT(!m);
	END_TEST();

}

void GH_phase12_Test()
{
	/* First test: 	1 outside rect, 0,0 - > 4,4
	 *		1 inside triangle 1,1 3,1, 2,4
	 *
	 * Results should be: Triangle untouched, except third point
	 *  intersect + neighbor linked
	 * Rect has new intersect point inserted /w neighbor link
	 */

	START_TEST("GH_phase_one Rect, Tri inside, one point touch");
	{
	GH_vertex * r_bl = GH_polyPoint(NULL, 0,0);
	GH_vertex * r_br = GH_polyPoint(r_bl, 4,0);
	GH_vertex * r_tr = GH_polyPoint(r_br, 4,4);
	GH_vertex * r_tl = GH_polyPoint(r_tr, 0,4);


	GH_vertex * t_0 = GH_polyPoint(NULL, 1,1);
	GH_vertex * t_1 = GH_polyPoint(t_0, 3,1);
	GH_vertex * t_2 = GH_polyPoint(t_1, 2,4);

	GH_phase_one(r_bl, t_0);

	TEST_OUTPUT(r_bl->next == r_br);
	TEST_OUTPUT(!r_bl->intersect);
	TEST_OUTPUT(r_br->next == r_tr);
	TEST_OUTPUT(!r_br->intersect);
	TEST_OUTPUT(!r_tr->intersect);
	GH_vertex * iv = r_tr->next;
	TEST_OUTPUT(iv);
	TEST_OUTPUT(iv->intersect);
	TEST_EQUALS_F(iv->alpha, 0.5)
	TEST_OUTPUT(iv->next == r_tl);
	TEST_OUTPUT(r_tl->next == NULL);
	TEST_OUTPUT(!r_tl->intersect);


	TEST_OUTPUT(t_0->next == t_1);
	TEST_OUTPUT(!t_0->intersect);
	TEST_OUTPUT(t_1->next == t_2);
	TEST_OUTPUT(!t_1->intersect);
	TEST_OUTPUT(t_2->next == NULL);
	TEST_OUTPUT(t_2->intersect);

	TEST_OUTPUT(t_2->neighbor == iv);
	TEST_OUTPUT(iv->neighbor == t_2);

	GH_phase_two(r_bl, t_0, GH_op_union);

	TEST_OUTPUT(iv->flag == FLG_EN_EX);
	TEST_OUTPUT(t_2->flag = FLG_EX_EN);

	}
	END_TEST();

	START_TEST("GH_phase_one Rect, smaller inside, 3 point touch");
	{
	GH_vertex * r1_bl = GH_polyPoint(NULL, 0,0);
	GH_vertex * r1_br = GH_polyPoint(r1_bl, 4,0);
	GH_vertex * r1_tr = GH_polyPoint(r1_br, 4,4);
	GH_vertex * r1_tl = GH_polyPoint(r1_tr, 0,4);

	GH_vertex * r2_bl = GH_polyPoint(NULL, 1,1);
	GH_vertex * r2_br = GH_polyPoint(r2_bl, 4,1);
	GH_vertex * r2_tr = GH_polyPoint(r2_br, 4,4);
	GH_vertex * r2_tl = GH_polyPoint(r2_tr, 1,4);


	GH_phase_one(r1_bl, r2_bl);

/*	print_polygon(r1_bl);

	printf("------------\n");
	print_polygon(r2_bl);
*/
//print_polygons_graphviz(r1_bl, r2_bl);
	TEST_OUTPUT(r1_bl->next == r1_br);
	TEST_OUTPUT(!r1_bl->intersect);
	GH_vertex * iv1 = r1_br->next;
	TEST_OUTPUT(iv1->next == r1_tr);
	GH_vertex * iv2 = r1_tr->next;
	TEST_OUTPUT(iv2->next == r1_tl);

	TEST_OUTPUT(iv1->intersect);
	TEST_OUTPUT(r1_tr->intersect);
	TEST_OUTPUT(iv2->intersect);
	TEST_OUTPUT(!r1_tl->intersect);
	TEST_OUTPUT(!r1_bl->intersect);
	TEST_OUTPUT(!r1_br->intersect);
	}
	END_TEST();

}
void GH_phase_Test_rect_union()
{
#if 0
	START_TEST("GH_phase_one Overlapping Rects Intersection");

	GH_vertex * r1_bl, *r1_br, *r1_tr, *r1_tl, *r1_i1, *r1_i2;
	GH_vertex * r2_bl, *r2_br, *r2_tr, *r2_tl, *r2_i1, *r2_i2;
	r1_bl = alloc_GH_vertex();
	r1_br = alloc_GH_vertex();
	r1_tr = alloc_GH_vertex();
	r1_tl = alloc_GH_vertex();
	GH_insertAfter(r1_bl, r1_br);
	GH_insertAfter(r1_br, r1_tr);
	GH_insertAfter(r1_tr, r1_tl);
	r1_bl->c.x = 0; r1_bl->c.y = 0;
	r1_br->c.x = 2; r1_br->c.y = 0;
	r1_tr->c.x = 2; r1_tr->c.y = 3;
	r1_tl->c.x = 0; r1_tl->c.y = 3;

	r2_bl = alloc_GH_vertex();
	r2_br = alloc_GH_vertex();
	r2_tr = alloc_GH_vertex();
	r2_tl = alloc_GH_vertex();
	GH_insertAfter(r2_bl, r2_br);
	GH_insertAfter(r2_br, r2_tr);
	GH_insertAfter(r2_tr, r2_tl);
	r2_bl->c.x = -1; r2_bl->c.y = 1;
	r2_br->c.x =  1; r2_br->c.y = 1;
	r2_tr->c.x =  1; r2_tr->c.y = 2;
	r2_tl->c.x = -1; r2_tl->c.y = 2;

	
	GH_vertex * result = GH_polygon_boolean(r1_bl, r2_bl, GH_op_union);
	TEST_OUTPUT(result);
	//print_polygon(result);
	END_TEST();
	GH_free_polygons(result);
	GH_free_polygons(r1_bl);
	GH_free_polygons(r2_bl);
	#endif

}
void GH_phase_Test_rect_intersection()
{
	START_TEST("GH_phase_one Overlapping Rects Intersection");

	GH_vertex * r1_bl, *r1_br, *r1_tr, *r1_tl, *r1_i1, *r1_i2;
	GH_vertex * r2_bl, *r2_br, *r2_tr, *r2_tl, *r2_i1, *r2_i2;
	r1_bl = alloc_GH_vertex();
	r1_br = alloc_GH_vertex();
	r1_tr = alloc_GH_vertex();
	r1_tl = alloc_GH_vertex();
	GH_insertAfter(r1_bl, r1_br);
	GH_insertAfter(r1_br, r1_tr);
	GH_insertAfter(r1_tr, r1_tl);
	r1_bl->c.x = 0; r1_bl->c.y = 0;
	r1_br->c.x = 2; r1_br->c.y = 0;
	r1_tr->c.x = 2; r1_tr->c.y = 3;
	r1_tl->c.x = 0; r1_tl->c.y = 3;

	r2_bl = alloc_GH_vertex();
	r2_br = alloc_GH_vertex();
	r2_tr = alloc_GH_vertex();
	r2_tl = alloc_GH_vertex();
	GH_insertAfter(r2_bl, r2_br);
	GH_insertAfter(r2_br, r2_tr);
	GH_insertAfter(r2_tr, r2_tl);
	r2_bl->c.x = -1; r2_bl->c.y = 1;
	r2_br->c.x =  1; r2_br->c.y = 1;
	r2_tr->c.x =  1; r2_tr->c.y = 2;
	r2_tl->c.x = -1; r2_tl->c.y = 2;

	GH_phase_one(r1_bl, r2_bl);

	// First Rectangle Tests
	GH_vertex * it = r1_bl;
	TEST_OUTPUT(it == r1_bl); it = it->next;
	TEST_OUTPUT(it == r1_br); it = it->next;
	TEST_OUTPUT(it == r1_tr); it = it->next;
	TEST_OUTPUT(it == r1_tl); it = it->next;

	// First intersection point
	TEST_OUTPUT(it != NULL); 
	r1_i1 = it;
	TEST_OUTPUT(r1_i1->intersect);
	TEST_OUTPUT(r1_i1->alpha < 0.33334 && r1_i1->alpha > 0.33333);
	TEST_EQUALS_F(r1_i1->c.x, 0);
	TEST_EQUALS_F(r1_i1->c.y, 2.0f);

	// Second intersection point
	it = it->next;
	TEST_OUTPUT(it != NULL); 
	r1_i2 = it;
	TEST_OUTPUT(r1_i2->intersect);
	TEST_OUTPUT(r1_i2->alpha < 0.66667 && r1_i2->alpha > 0.66666);
	TEST_EQUALS_F(r1_i2->c.x, 0);
	TEST_EQUALS_F(r1_i2->c.y, 1);

	// End of First Rectangle
	TEST_OUTPUT(it->next == NULL);


	// First Rectangle Tests
	it = r2_bl;
	TEST_OUTPUT(it == r2_bl); it = it->next;
	// First intersection point
	TEST_OUTPUT(it != NULL);
	r2_i1 = it;
	TEST_OUTPUT(r2_i1->intersect);
	TEST_EQUALS_F(r2_i1->alpha, 0.5);
	TEST_EQUALS_F(r2_i1->c.x, 0);
	TEST_EQUALS_F(r2_i1->c.y, 1);


	it = it->next;
	TEST_OUTPUT(it == r2_br);
	it = it->next;
	TEST_OUTPUT(it == r2_tr);
	it = it->next;
	TEST_OUTPUT(it != NULL);

	r2_i2 = it;
	TEST_OUTPUT(r2_i2->intersect);
	TEST_EQUALS_F(r2_i2->alpha, 0.5);
	TEST_EQUALS_F(r2_i2->c.x, 0);
	TEST_EQUALS_F(r2_i2->c.y, 2);
	it = it->next;
	TEST_OUTPUT(it == r2_tl);
	it = it->next;
	TEST_OUTPUT(it == NULL);

	TEST_OUTPUT(r1_i1->neighbor == r2_i2);
	TEST_OUTPUT(r1_i2->neighbor == r2_i1);
	
	TEST_OUTPUT(r2_i1->neighbor == r1_i2);
	TEST_OUTPUT(r2_i2->neighbor == r1_i1);

	END_TEST();
#if 0
	START_TEST("GH_phase_two Overlapping Rects Intersection");
	GH_phase_two(r1_bl, r2_bl, GH_op_intersect);
	TEST_OUTPUT(r1_i1->flag == FLG_EN);
	TEST_OUTPUT(r1_i2->flag == FLG_EX);
	TEST_OUTPUT(r2_i1->flag == FLG_EN);
	TEST_OUTPUT(r2_i2->flag == FLG_EX);
	END_TEST();

	START_TEST("GH_phase_three Overlapping Rects Intersection");

	GH_vertex * output = GH_phase_three(r1_bl, r2_bl);
	/*print_polygon(output);*/
	TEST_OUTPUT(output);
	TEST_OUTPUT(output->nextPoly == NULL);
	TEST_EQUALS_F(output->c.x, 0);
	TEST_EQUALS_F(output->c.y, 2);
	GH_vertex * i = output->next;
	TEST_OUTPUT(i);
	TEST_EQUALS_F(i->c.x, 0);
	TEST_EQUALS_F(i->c.y, 1);
	i = i->next;
	TEST_OUTPUT(i);
	TEST_EQUALS_F(i->c.x, 1);
	TEST_EQUALS_F(i->c.y, 1);
	i = i->next;
	TEST_OUTPUT(i);
	TEST_EQUALS_F(i->c.x, 1);
	TEST_EQUALS_F(i->c.y, 2);
	i = i->next;
	TEST_OUTPUT(!i);
	//print_polygon(output);
	
	END_TEST();
#endif
	free(r1_bl); free(r1_br); free(r1_tr); free(r1_tl); free(r1_i1); free(r1_i2);
	free(r2_bl); free(r2_br); free(r2_tr); free(r2_tl); free(r2_i1); free(r2_i2);


}
void GH_point_in_poly_Test()
{
	START_TEST("GH_pointInPoly");
	GH_vertex * r1_bl, *r1_br, *r1_tr, *r1_tl;
	r1_bl = alloc_GH_vertex();
	r1_br = alloc_GH_vertex();
	r1_tr = alloc_GH_vertex();
	r1_tl = alloc_GH_vertex();
	GH_insertAfter(r1_bl, r1_br);
	GH_insertAfter(r1_br, r1_tr);
	GH_insertAfter(r1_tr, r1_tl);
	r1_bl->c.x = 0; r1_bl->c.y = 0;
	r1_br->c.x = 2; r1_br->c.y = 0;
	r1_tr->c.x = 2; r1_tr->c.y = 3;
	r1_tl->c.x = 0; r1_tl->c.y = 3;

	GH_point a = {1,1};
	GH_point b = {4,1};

	TEST_OUTPUT(GH_pointInPoly(r1_bl, &a));
	TEST_OUTPUT(!GH_pointInPoly(r1_bl, &b));

	free(r1_bl); free(r1_br); free(r1_tr); free(r1_tl);
	END_TEST();
	
}
void polymath_tests(void)
{
	GH_insertAfter_Test();
	GH_createIVertex_Test();
	GH_linkVerticies_Test();
	outcode_test();
	__find_non_intersect_test();
	GH_pointCompare_test();
	GH_calcAlpha_Test();
	GH_calc_WEC_test();
	GH_intersect_test();
	GH_LineBits_Test();
	GH_sortedInsert_Test();
	GH_point_in_poly_Test();
	GH_phase12_Test();
	GH_phase_Test_rect_intersection();
	GH_phase_Test_rect_union();
}
