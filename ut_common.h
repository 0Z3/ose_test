#include "ose_test_bundleLiteral.h"

#define UNIT_TEST_FUNCTION(fn) TEST_FUNCTION(ut_, fn)
#define SKIP_UNIT_TEST_FUNCTION(fn) SKIP_TEST_FUNCTION(fn)

void printb(const char * const b)
{
	if(b){
		// prevent load of misaligned address runtime error
		char buf[8];
		char *p = align(buf);
		for(int i = 0; i < 4; i++){
			p[i] = b[i];
		}
		int32_t s = htonl(*((int32_t *)p));
		for(int32_t i = 0; i < s + 4; i++){
			if(b[i] <= 32 || b[i] > 127){
				printf("\\x%02x", b[i]);
			}else{
				printf("%c", b[i]);
			}
		}
	}else{
		printf("NULL");
	}
}

#define UNIT_TEST_IMPL(test,                            \
                       expected_result,                 \
                       desc,                            \
                       file,                            \
                       func,                            \
                       line)                            \
	{                                                   \
		testctr++;                                      \
		int32_t r = ASSERTION_FAILED;                   \
		if(!setjmp(env)){                               \
			r = test;                                   \
		}                                               \
		if(r != expected_result){                       \
			printf(ANSI_COLOR_RED                       \
			       "TEST %d FAILED: "                   \
			       ANSI_COLOR_RESET                     \
			       "%s:%s:%d:\n%s\n"                    \
			       "expected %d, but got %d\n",         \
			       testctr,                             \
			       file, func, line,                    \
			       #test, expected_result, r);          \
            failctr++;                                  \
		}else if(verbose){                              \
			printf(ANSI_COLOR_GREEN                     \
			       "TEST %d PASSED:\n"                  \
			       ANSI_COLOR_RESET                     \
			       "%s = %d\n",                         \
			       testctr, #test, expected_result);    \
		}                                               \
	}                                                   \

#define UNIT_TEST(test, expected_result, desc)		\
	UNIT_TEST_IMPL(test, expected_result, desc,     \
                   __FILE__, __func__, __LINE__);
	

#define UNIT_TEST_WITH_SIMPLE_BUNDLE(b,                         \
                                     test,                      \
                                     expected_result,           \
                                     desc)                      \
	{                                                           \
        char buf[MAX_BNDLSIZE] __attribute__ ((aligned (16)));  \
		char *p = NULL;                                         \
		ose_bundle bundle;                                      \
		if(b){                                                  \
			p = buf;                                            \
			memcpy(p, b, sizeof(b));                            \
			bundle = ose_makeBundle(p + 4);                     \
		}else{                                                  \
            /* for testing NULL pointer input */                \
			bundle = ose_makeBundle(p);                         \
		}                                                       \
		if(verbose){                                            \
			printf("bundle = ");                                \
			printb(b);                                          \
			printf("\n");                                       \
		}                                                       \
		UNIT_TEST(test, expected_result, desc);                 \
	}

#define UNIT_TEST_WITH_CONTEXT_BUNDLE_IMPL(testbndl,                    \
                                           bundle_literal,              \
                                           bundle_literal_size,         \
                                           copy_offset,                 \
                                           call_offset,                 \
                                           test,                        \
                                           expected_result,             \
                                           desc)                        \
    {                                                                   \
        ose_bundle bundle = (ose_bundle){NULL};                         \
        char buf[bundle_literal_size + 4]                           \
                __attribute__ ((aligned (16)));                         \
        if(testbndl){                                                   \
            memcpy(buf, bundle_literal, bundle_literal_size + 4);       \
            char *p = buf + copy_offset;                                \
            bundle.b = buf + call_offset;                               \
            int32_t sizeofb = sizeof(testbndl);                         \
            int32_t size1 = ose_ntohl(*((int32_t *)(p - 4)));           \
            int32_t size2 = ose_ntohl(*((int32_t *)(p + size1)));       \
            *((int32_t *)(p + size1)) = 0;                              \
            memcpy(p - 4, testbndl, sizeofb);                           \
            *((int32_t *)(p + (sizeofb - 4)))                           \
                = ose_htonl(size2 - (sizeofb - 20));                    \
        }else{                                                          \
        }                                                               \
        UNIT_TEST(test, expected_result, desc);                         \
        if(verbose){                                                    \
            printf("result bundle = ");                                 \
            printb(bundle.b + ose_test_bundle_context_message_workingbundle_size_offset); \
            printf("\n");                                               \
        }                                                               \
    }

#define UNIT_TEST_WITH_CONTEXT_BUNDLE(testbndl,                         \
                                      test,                             \
                                      expected_result,                  \
                                      desc)                             \
    UNIT_TEST_WITH_CONTEXT_BUNDLE_IMPL(testbndl,                        \
                                       ose_test_bundle,                 \
                                       ose_test_bundle_main_bundle_size, \
                                       ose_test_bundle_context_message_workingbundle_id_offset, \
                                       ose_test_bundle_context_message_workingbundle_id_offset, \
                                       test,                            \
                                       expected_result,                 \
                                       desc)

#define UNIT_TEST_WITH_VM(testbndl,                                     \
                          test,                                         \
                          expected_result,                              \
                          desc)                                         \
    UNIT_TEST_WITH_CONTEXT_BUNDLE_IMPL(testbndl,                        \
                                       ose_test_vm,                     \
                                       ose_test_vm_main_bundle_size,    \
                                       ose_test_vm_stack_message_workingbundle_id_offset, \
                                       ose_test_vm_context_message_workingbundle_id_offset, \
                                       test,                            \
                                       expected_result,                 \
                                       desc)

#define UNIT_TEST_BUNDLE_TRANSFORM(testfn,                  \
                                   testbndl,                \
                                   resultbndl,              \
                                   desc)                    \
    if(verbose){                                            \
        printf("test bundle = ");                           \
        printb(testbndl);                                   \
        printf("\n");                                       \
        printf("expected result = ");                       \
        printb(resultbndl);                                 \
        printf("\n");                                       \
    }                                                       \
    UNIT_TEST_WITH_CONTEXT_BUNDLE(                          \
        testbndl,                                           \
        (testfn(bundle),                                    \
         COMPARE_BUNDLES(resultbndl,                        \
                         ose_getBundlePtr(bundle) - 4)),    \
        0,                                                  \
        desc);                                              
    

#define UNIT_TEST_BUNDLE_TRANSFORM_ARGS(testfn,             \
                                        testbndl,           \
                                        resultbndl,         \
                                        desc,               \
                                        ...)                \
    UNIT_TEST_WITH_CONTEXT_BUNDLE(                          \
        testbndl,                                           \
        (testfn(bundle, __VA_ARGS__),                       \
         COMPARE_BUNDLES(resultbndl,                        \
                         ose_getBundlePtr(bundle) - 4)),    \
        0,                                                  \
        desc);

#define UNIT_TEST_BUNDLE_TRANSFORM_TRIGGER_ASSERTION(testfn,    \
                                                     testbndl,  \
                                                     desc)      \
    UNIT_TEST_WITH_CONTEXT_BUNDLE(testbndl,                     \
                                  (testfn(bundle), 0),          \
                                  ASSERTION_FAILED,             \
                                  desc);

#define UNIT_TEST_BUNDLE_TRANSFORM_TRIGGER_ASSERTION_ARGS(testfn,   \
                                                          testbndl, \
                                                          desc,     \
                                                          ...)      \
    UNIT_TEST_WITH_CONTEXT_BUNDLE(testbndl,                         \
                                  (testfn(bundle, __VA_ARGS__), 0), \
                                  ASSERTION_FAILED,                 \
                                  desc);

#define UNIT_TEST_VM_ERRNO(testfn,              \
                           testbndl,            \
                           errno,               \
                           desc)                \
    UNIT_TEST_WITH_VM(testbndl,                 \
                      (testfn(bundle),          \
                       ose_errno_get(bundle)),  \
                      errno,                    \
                      desc)

#define UNIT_TEST_VM_STACK_COMPARE(testfn,                              \
                                   testbndl,                            \
                                   resultbndl,                          \
                                   desc)                                \
    UNIT_TEST_WITH_VM(testbndl,                                         \
                      (testfn(bundle),                                  \
        ose_errno_get(bundle) == OSE_ERR_NONE \
                       && !COMPARE_BUNDLES(resultbndl,                   \
                                          buf + ose_test_vm_stack_message_workingbundle_size_offset)), \
                      1,                                                \
                      desc)

#define SKIP_UNIT_TEST_WITH_BUNDLE(b, test, expected_result)	\
	SKIP_TEST(test, expected_result)

#define SKIP_UNIT_TEST(test, expected_result)	\
	SKIP_TEST(test, expected_result)
