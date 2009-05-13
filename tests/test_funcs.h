#include <setjmp.h>

void start_section(char * name);

void start_test(char * name);
bool __test_assert(bool val, char*str);
bool __test_output(bool val, char*str);
bool __test_equals_f(float a, float b, char*str);
bool __test_equals_i(int a, int b, char*str);
bool __test_equals_x(int a, int b, char*str);

#define __tostr2(x) #x
#define __tostr(x) __tostr2(x)
#define TEST_ASSERT(a) if (!__test_assert(a, "Assert Failed:" __FILE__ ":" __tostr( __LINE__ )" " #a)) longjmp(env,1);

#define TEST_OUTPUT(a) if (!__test_output(a, #a)) longjmp(env,1);
#define TEST_EQUALS_F(a, b) if (!__test_equals_f(a, b, #a)) longjmp(env,1);
#define TEST_EQUALS_I(a, b) if (!__test_equals_i(a, b, #a)) longjmp(env,1);
#define TEST_EQUALS_X(a, b) if (!__test_equals_x(a, b, #a)) longjmp(env,1);


extern jmp_buf env;

#define START_TEST(x) if (setjmp(env)) end_test(); start_test(x)
#define END_TEST() end_test()
void end_test();

void polymath_tests(void);
