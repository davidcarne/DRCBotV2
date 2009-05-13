#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>

#include "test_funcs.h"

#define CSI "\x1B["
#define RESET CSI "0m"
#define GREEN CSI "92m"
#define YELLOW CSI "93m"
#define BLUE  CSI "94m"
#define RED   CSI "91m"
#define WHITE CSI "97m"


/*
struct malloc_ent {
	struct malloc_ent * next;
	void * caller;
	size_t size;
};

struct malloc_ent * head;

struct malloc_ent ** ptr;

void  * (old_malloc_hook*)(size_t size, const void * caller);
void (old_free_hook*)(void * block, const void * caller);

static void  free_counter(void * block, const void * caller);
static void  * malloc_counter(size_t size, const void * caller);


static void  * malloc_counter(size_t size, const void * caller)
{
	__malloc_hook = old_malloc_hook;
	__free_hook = old_free_hook;
	
	void * block = malloc(size);

	*ptr = (struct malloc_ent *)size;
	ptr = &(*ptr->next);

	return block;
}

static void  free_counter(void * block, const void * caller)
{
}*/

jmp_buf env;

bool mstats[40];
char * messages[40];
int mint = 0;

char * testname;
bool test_ok = true;
void start_test(char * val)
{
	test_ok = true;
	testname = val;
	mint = 0;
}

bool __test_assert(bool v, char * str)
{
	test_ok = test_ok && v;
	if (!v)
	{
		mstats[mint] = v;
		messages[mint] = str;
		mint++;
	}
	return v;
}

bool __test_output(bool v, char * str)
{
	test_ok = test_ok && v;
	mstats[mint] = v;
	messages[mint] = str;
	mint++;	
	return v;
}

bool __test_equals_f(float v, float c, char * str)
{
	bool ok = fabs(v - c) < 0.00001;


	char * strbuf = (char *)malloc(80);
	test_ok = test_ok && ok;
	mstats[mint] = ok;
	sprintf(strbuf, "%s == %f was [%f]", str, c, v);
	messages[mint] = strbuf;
	mint++;	
	return ok;
}
bool __test_equals_i(int v, int c, char * str)
{
	char * strbuf = (char *)malloc(80);
	test_ok = test_ok && (v == c);
	mstats[mint] = (v == c);
	sprintf(strbuf, "%s == %d was [%d]", str, c, v);
	messages[mint] = strbuf;
	mint++;	
	return v == c;
}
bool __test_equals_x(int v, int c, char * str)
{
	char * strbuf = (char *)malloc(80);
	test_ok = test_ok && (v == c);
	mstats[mint] = (v == c);
	sprintf(strbuf, "%s == %x was [%x]", str, c, v);
	messages[mint] = strbuf;
	mint++;	
	return v == c;
}

void end_test()
{
	if (!test_ok)
		printf(" "WHITE"["RED"!!"WHITE"] "RESET);
	else if (mint > 0)
		printf(" "WHITE"["GREEN"ok"WHITE"] "RESET);
	else 
		printf(" "WHITE"["YELLOW"??"WHITE"] "RESET);

	printf("%s\n", testname);
	if (!test_ok)
	for(int i=0; i<mint; i++)
	{
		printf("\t%s%s"RESET"\n", mstats[i] ? GREEN:RED,messages[i]);
	}
	if (!test_ok)
		exit(1);
}
int main(int argc, char** argv)
{
	printf(BLUE "Starting Tests" RESET "\n");
	polymath_tests();
}

