#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>

#include "ose.h"
#include "ose_util.h"
#include "ose_context.h"
#include "ose_print.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

jmp_buf env;
const int32_t ASSERTION_FAILED = 0x66666666;
const int32_t ASSERTION_PASSED = 0x33333333;

void printit(const char * const refbndl, const char * const testbndl)
{
    for(int32_t i = 0; i < ntohl(*((int32_t *)refbndl)) + 4; ++i)
    {
        printf("%d: 0x%x 0x%x\n", i - 4, refbndl[i], testbndl[i]);
    }
}

#define COMPARE_BUNDLES(refbndl, testbndl)                  \
    memcmp(refbndl, testbndl, 4 + ntohl(*((int32_t *)refbndl)))

void sighandler(int signo)
{
	if(signo == SIGABRT){
		longjmp(env, ASSERTION_FAILED);
	}
}

#define MAX_BNDLSIZE 1024
char *align(char *p)
{
	while((long)p % 4){
		p++;
	}
	return p;
}

int verbose = 0;
int grand_total_failctr;
int grand_total_testctr;
int test_functions_skipped;
int failctr;
int testctr;
int tests_skipped;

void pbndl(ose_bundle bundle, const char * const str)
{
    char buf[65536];
    memset(buf, 0, 65536);
    ose_pprintBundle(bundle, buf, 65536);
    fprintf(stderr, "\n\r%s>>>>>\n\r%s\n\r%s<<<<<\n\r",
            str, buf, str);
}

void pbytes(ose_bundle bundle, int32_t start, int32_t end)
{
    char *b = ose_getBundlePtr(bundle);
    for(int32_t i = start; i < end; i++){
        fprintf(stderr, "%d: %c %d\n\r", i,
                (unsigned char)b[i],
                (unsigned char)b[i]);
    }
}

void print_passed(const char * const fname)
{
	printf(ANSI_COLOR_GREEN "%d / %d tests PASSED" ANSI_COLOR_RESET "\n",
	       testctr - tests_skipped, testctr);
	if(tests_skipped){
		printf(ANSI_COLOR_YELLOW "%d tests SKIPPED" ANSI_COLOR_RESET "\n",
		       tests_skipped);
	}
}

void print_failed(const char * const fname)
{
	printf(ANSI_COLOR_RED "...%d of %d tests FAILED" ANSI_COLOR_RESET "\n",
	       failctr, testctr);
}

void print_results(const char * const fname)
{
	if(!testctr){
		printf(ANSI_COLOR_RED
		       "NO TESTS PERFORMED for this unit"
		       ANSI_COLOR_RESET
		       "\n");
		return;
	}
	if(!failctr){
		print_passed(fname);
	}else{
		print_failed(fname);
	}
}

void print_startTest(const char * const fname)
{
	failctr = 0;
	testctr = 0;
	printf(ANSI_COLOR_CYAN
	       "Testing %s..."
	       ANSI_COLOR_RESET
	       "\n",
	       fname);
}

#define TEST_FUNCTION(pfx, fn)					\
	{                                           \
		tests_skipped = 0;                      \
		grand_total_testctr++;                  \
		print_startTest(#fn);                   \
		int fc = failctr;                       \
		pfx ## fn();                            \
		print_results(#fn);                     \
		if(fc != failctr){                      \
			grand_total_failctr++;              \
		}                                       \
	}

#define SKIP_TEST_FUNCTION(fn)                  \
	test_functions_skipped++;                   \
	grand_total_testctr++;                      \
	printf(ANSI_COLOR_YELLOW                    \
	       "SKIPPING %s : "                     \
	       ANSI_COLOR_RESET                     \
	       "\n",                                \
	       #fn)

#define SKIP_TEST(test, expected_result)        \
	{                                           \
		tests_skipped++;                        \
		testctr++;                              \
		printf(ANSI_COLOR_YELLOW                \
		       "\nSKIPPING test %s"             \
		       ANSI_COLOR_RESET                 \
		       "\n",                            \
		       #test);                          \
	}

#ifdef UT_NEW_BUNDLE
/* zero or more zeros for padding */
#define Z0
#define Z1 0
#define Z2 0, 0
#define Z3 0, 0, 0
#define Z4 0, 0, 0, 0

/* unpack a string into a comma separated list of chars */
#define STOC1(s) s[0]
#define STOC2(s) s[0], STOC1((s + 1))
#define STOC3(s) s[0], STOC2((s + 1))
#define STOC4(s) s[0], STOC3((s + 1))

/* NULL padded strings */
#define S0N(s, z) z
#define S1N(s, z) STOC1(s), z
#define S2N(s, z) STOC2(s), z
#define S3N(s, z) STOC3(s), z
#define S4N(s, z) STOC4(s), z
#define S5N(s, z) STOC4(s), STOC1((s + 4)), z
#define S6N(s, z) STOC4(s), STOC2((s + 4)), z
#define S7N(s, z) STOC4(s), STOC3((s + 4)), z
#define S8N(s, z) STOC4(s), STOC4((s + 4)), z
#define S9N(s, z) STOC4(s), STOC4((s + 4)), STOC1((s + 8)), z
#define S10N(s, z) STOC4(s), STOC4((s + 4)), STOC2((s + 8)), z
#define S11N(s, z) STOC4(s), STOC4((s + 4)), STOC3((s + 8)), z
#define S12N(s, z) STOC4(s), STOC4((s + 4)), STOC4((s + 8)), z
#define S13N(s, z) STOC4(s), STOC4((s + 4)), STOC4((s + 8)),    \
        STOC1((s + 12)), z
#define S14N(s, z) STOC4(s), STOC4((s + 4)), STOC4((s + 8)),    \
        STOC2((s + 12)), z
#define S15N(s, z) STOC4(s), STOC4((s + 4)), STOC4((s + 8)),    \
        STOC3((s + 12)), z
#define S16N(s, z) STOC4(s), STOC4((s + 4)), STOC4((s + 8)),    \
        STOC4((s + 12)), z

/* general string type, used for addresses and strings */
#define S0(s) S0N(s, Z4)
#define S1(s) S1N(s, Z3)
#define S2(s) S2N(s, Z2)
#define S3(s) S3N(s, Z1)
#define S4(s) S4N(s, Z4)
#define S5(s) S5N(s, Z3)
#define S6(s) S6N(s, Z2)
#define S7(s) S7N(s, Z1)
#define S8(s) S8N(s, Z4)
#define S9(s) S9N(s, Z3)
#define S10(s) S10N(s, Z2)
#define S11(s) S11N(s, Z1)
#define S12(s) S12N(s, Z4)
#define S13(s) S13N(s, Z3)
#define S14(s) S14N(s, Z2)
#define S15(s) S15N(s, Z1)
#define S16(s) S16N(s, Z4)

/* address */
#define A(n, a) S ## n (a)

/* typetags */
#define T0(s) S0N(s, Z3)
#define T1(s) S1N(s, Z2)
#define T2(s) S2N(s, Z1)
#define T3(s) S3N(s, Z4)
#define T4(s) S4N(s, Z3)
#define T5(s) S5N(s, Z2)
#define T6(s) S6N(s, Z1)
#define T7(s) S7N(s, Z4)
#define T8(s) S8N(s, Z3)
#define T9(s) S9N(s, Z2)
#define T10(s) S10N(s, Z1)
#define T11(s) S11N(s, Z4)
#define T12(s) S12N(s, Z3)
#define T13(s) S13N(s, Z2)
#define T14(s) S14N(s, Z1)
#define T15(s) S15N(s, Z4)
#define T16(s) S16N(s, Z3)

#define T(n, s) ',', T ## n (s)

/* data: int32 */
#define Di(i)                                   \
    ((i & 0xFF000000) >> 24),                   \
        ((i & 0x00FF0000) >> 16),               \
        ((i & 0x0000FF00) >> 8),                \
        (i & 0x000000FF)

/* data: float32 */
union flint
{
    int32_t i;
    float f;
};

#define Df(ff) \
    (((union flint){.f = ff}.i & 0xFF000000) >> 24),   \
        (((union flint){.f = ff}.i & 0x00FF0000) >> 16),   \
        (((union flint){.f = ff}.i & 0x0000FF00) >> 8),    \
        ((union flint){.f = ff}.i & 0x000000FF)

/* data: string */
#define Ds(n, s) S ## n (s)

/* data: blob */
#define BS(i) 0, 0, 0, i,

#define B0(b) Z4
#define B1(b) BS(1) S1N(b, Z3)
#define B2(b) BS(2) S2N(b, Z2)
#define B3(b) BS(3) S3N(b, Z1)
#define B4(b) BS(4) S4N(b, Z0)
#define B5(b) BS(5) S5N(b, Z3)
#define B6(b) BS(6) S6N(b, Z2)
#define B7(b) BS(7) S7N(b, Z1)
#define B8(b) BS(8) S8N(b, Z0)
#define B9(b) BS(9) S9N(b, Z3)
#define B10(b) BS(10) S10N(b, Z2)
#define B11(b) BS(11) S11N(b, Z1)
#define B12(b) BS(12) S12N(b, Z0)
#define B13(b) BS(13) S9N(b, Z3)
#define B14(b) BS(14) S10N(b, Z2)
#define B15(b) BS(15) S11N(b, Z1)
#define B16(b) BS(16) S12N(b, Z0)

#define Db(n, b) B ## n (b)

/* bundle elements */
#define BE_SIZE(...)                                    \
    (sizeof((char []){__VA_ARGS__}) & 0xFF000000),      \
        (sizeof((char []){__VA_ARGS__}) & 0x00FF0000),  \
        (sizeof((char []){__VA_ARGS__}) & 0x0000FF00),  \
        (sizeof((char []){__VA_ARGS__}) & 0x000000FF),  \
        
/* bundle element: message */
#define BE_M(...)                               \
    BE_SIZE(__VA_ARGS__)                        \
        __VA_ARGS__

/* bundle element: bundle */
#define BE_B(...)                               \
    BE_SIZE('#', 'b', 'u', 'n', 'd', 'l', 'e', 0,   \
            0, 0, 0, 0, 0, 0, 0, 0,             \
            __VA_ARGS__)                        \
        '#', 'b', 'u', 'n', 'd', 'l', 'e', 0,   \
            0, 0, 0, 0, 0, 0, 0, 0,             \
        __VA_ARGS__

/* message */
/* #define M(...) ((char []){ __VA_ARGS__ }) */
#define M(...) ((char []){ BE_M(__VA_ARGS__) })

/* bundle */
/* #define B(...) ((char []){                      \ */
/*         '#', 'b', 'u', 'n', 'd', 'l', 'e', 0,   \ */
/*             0, 0, 0, 0, 0, 0, 0, 0,             \ */
/*             __VA_ARGS__                         \ */
/*         }) */
#define B(...) ((char []){ BE_B(__VA_ARGS__) })

/* 
Example:

char b[] = B(BE_M(A(0, ""), T(0, "")),
                 BE_B(BE_M(A(2, "/x"),
                           T(2, "ii"),
                           Di(33),
                           Di(44)),
                      BE_M(A(8, "/foo/bar"),
                           T(4, "ifsb"),
                           Di(10),
                           Df(3.14159),
                           Ds(3, "foo"),
                           Db(3, "\x0\x1\x2"))));
 */
#else

#define H "#bundle\0\0\0\0\0\0\0\0\0"

#define Sm1 "\xFF\xFF\xFF\xFF"
#define S0 "\0\0\0\0"
#define S4 "\0\0\0\x04"
#define S8 "\0\0\0\x08"
#define S12 "\0\0\0\x0C"
#define S16 "\0\0\0\x10"
#define S20 "\0\0\0\x14"
#define S24 "\0\0\0\x18"
#define S28 "\0\0\0\x1C"
#define S32 "\0\0\0\x20"
#define S36 "\0\0\0\x24"
#define S40 "\0\0\0\x28"
#define S44 "\0\0\0\x2C"
#define S48 "\0\0\0\x30"
#define S52 "\0\0\0\x34"
#define S56 "\0\0\0\x38"
#define S60 "\0\0\0\x3C"
#define S64 "\0\0\0\x40"
#define S68 "\0\0\0\x44"
#define S72 "\0\0\0\x48"
#define S76 "\0\0\0\x4C"
#define S80 "\0\0\0\x50"
#define S84 "\0\0\0\x54"
#define S88 "\0\0\0\x58"
#define S92 "\0\0\0\x5C"
#define S96 "\0\0\0\x60"
#define S100 "\0\0\0\x64"
#define S104 "\0\0\0\x68"
#define S108 "\0\0\0\x6C"
#define S112 "\0\0\0\x70"
#define S116 "\0\0\0\x74"
#define S120 "\0\0\0\x78"
#define S124 "\0\0\0\x7C"
#define S128 "\0\0\0\x80"
#define S132 "\0\0\0\x84"
#define S136 "\0\0\0\x88"
#define S140 "\0\0\0\x8C"
#define S144 "\0\0\0\x90"
#define S148 "\0\0\0\x94"
#define S152 "\0\0\0\x98"
#define S156 "\0\0\0\x9C"
#define S160 "\0\0\0\xA0"

#define A0 "\0\0\0\0"
#define A1 "/\0\0\0"
#define A2 "/f\0\0"
#define A3 "/fo\0"
#define A4 "/foo\0\0\0\0"
#define A5 "/fooo\0\0\0"

#define T0 ",\0\0\0"
#define Ti ",i\0\0"
#define Tii ",ii\0"
#define Tiii ",iii\0\0\0\0"
#define Tiiii ",iiii\0\0\0"
#define Tf ",f\0\0"
#define Ts ",s\0\0"
#define Tss ",ss\0"
#define Tsss ",sss\0\0\0\0"
#define Tb ",b\0\0"
#define Tbb ",bb\0"
#define Tbbb ",bbb\0\0\0\0"
#define Tifsb ",ifsb\0\0\0"

#define Di "\0\0\0\x20"
#define Dii "\0\0\0\x30\0\0\0\x31"
#define Diii "\0\0\0\x30\0\0\0\x31\0\0\0\x32"
#define Diiii "\0\0\0\x30\0\0\0\x31\0\0\0\x32\0\0\0\x33"
#define Df "\x44\x08\0\0"
#define Ds0 "\0\0\0\0"
#define Ds1 "f\0\0\0"
#define Ds2 "fo\0\0"
#define Ds3 "foo\0"
#define Ds4 "fooo\0\0\0\0"
#define Ds5 "foooo\0\0\0"
#define Db0 "\0\0\0\x00"
#define Db1 "\0\0\0\x01\x01\0\0\0"
#define Db2 "\0\0\0\x02\x01\x02\0\0"
#define Db3 "\0\0\0\x03\x01\x02\x03\0"
#define Db4 "\0\0\0\x04\x01\x02\x03\x04"
#define Db5 "\0\0\0\x05\x01\x02\x03\x04\x05\0\0\0"

/* const unsigned char all_chars[256] = { */
/* 				      0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, */
/* 				      0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, */
			     
/* 				      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, */
/* 				      0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, */
			     
/* 				      0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, */
/* 				      0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, */
			     
/* 				      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, */
/* 				      0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, */
			     
/* 				      0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, */
/* 				      0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, */
			     
/* 				      0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, */
/* 				      0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, */
			     
/* 				      0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, */
/* 				      0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, */
			     
/* 				      0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, */
/* 				      0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, */
			     
/* 				      0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, */
/* 				      0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, */
			     
/* 				      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, */
/* 				      0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, */
			     
/* 				      0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, */
/* 				      0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, */
			     
/* 				      0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, */
/* 				      0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, */
			     
/* 				      0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, */
/* 				      0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, */
			     
/* 				      0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, */
/* 				      0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, */
			     
/* 				      0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, */
/* 				      0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, */
			     
/* 				      0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, */
/* 				      0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF, */
/* }; */

/* char true256[256]; */
/* char false256[256]; */

#endif

void init(void)
{
	signal(SIGABRT, sighandler);
    /* memset(true256, OSETT_TRUE, 256); */
	/* memset(false256, OSETT_FALSE, 256); */
	srand(time(NULL));
	grand_total_testctr = grand_total_failctr = 0;
	test_functions_skipped = tests_skipped = 0;
}

void finalize(void)
{
	printf("\n**************************************************\n");
	if(!grand_total_failctr){
		printf(ANSI_COLOR_GREEN
		       "%d / %d TESTS PASSED"
		       ANSI_COLOR_RESET
		       "\n",
		       grand_total_testctr - test_functions_skipped,
		       grand_total_testctr);
		if(test_functions_skipped){
			printf(ANSI_COLOR_YELLOW
			       "%d TESTS SKIPPED"
			       ANSI_COLOR_RESET
			       "\n",
			       test_functions_skipped);
		}
	}else{
		printf(ANSI_COLOR_RED
		       "%d / %d TESTS FAILED"
		       ANSI_COLOR_RESET
		       "\n",
		       grand_total_failctr,
		       grand_total_testctr);
	}
	printf("**************************************************\n");
}
