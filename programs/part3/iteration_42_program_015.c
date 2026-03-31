/* test_i386_condcodes.c
 * Targets uncovered lines 13992-14017 in i386.cc
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -S test_i386_condcodes.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile double g_d1 = 1.0;
volatile double g_d2 = 2.0;
volatile long double g_ld1 = 3.0L;
volatile long double g_ld2 = 4.0L;
volatile int g_selector = 0;

/* Condition code constants matching i386.h */
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* Force noinline to ensure separate functions */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Use %c modifier to output condition code name */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNORDERED), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(ORDERED), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_uneq(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNEQ), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGE), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_ungt(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGT), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLE), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unlt(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLT), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(LTGT), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

/* Mixed x87 and SSE operations */
__attribute__((noinline))
static int test_mixed_operations(long double ld1, long double ld2, 
                                 double d1, double d2) {
    int results = 0;
    
    /* x87 operations with long double */
    results += test_unordered((double)ld1, (double)ld2);
    results += test_ordered((double)ld1, (double)ld2);
    
    /* SSE operations mixed in */
    asm volatile ("comisd %1, %0" : : "x"(d1), "x"(d2) : "cc");
    
    results += test_uneq(d1, d2);
    results += test_unge(d1, d2);
    
    return results;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int test_switch_based(int selector, double a, double b) {
    int result = 0;
    
    switch (selector & 0x7) {
        case UNORDERED:
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNORDERED));
            break;
        case ORDERED:
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(ORDERED));
            break;
        case UNEQ:
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNEQ));
            break;
        case UNGE:
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNGE));
            break;
        case UNGT:
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNGT));
            break;
        case UNLE:
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNLE));
            break;
        case UNLT:
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(UNLT));
            break;
        case LTGT:
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(LTGT));
            break;
        default:
            /* This should trigger output_operand_lossage */
            asm volatile ("set%c0 %0" : "=r"(result) : "u"(selector));
            break;
    }
    
    return result;
}

/* Helper to create NaN values */
static double make_nan() {
    return 0.0 / 0.0;
}

static long double make_long_nan() {
    return (long double)(0.0L / 0.0L);
}

int main(int argc, char *argv[]) {
    int i, iterations = 100;
    volatile int total = 0;  /* Prevent optimization */
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create some special floating point values */
    double nan_val = make_nan();
    double inf_val = 1.0 / 0.0;
    long double long_nan = make_long_nan();
    
    /* Array of test values */
    double test_doubles[] = {g_d1, g_d2, nan_val, inf_val, -inf_val, 0.0, -0.0};
    long double test_long_doubles[] = {g_ld1, g_ld2, long_nan, 
                                       (long double)inf_val, 0.0L};
    
    int num_doubles = sizeof(test_doubles) / sizeof(test_doubles[0]);
    int num_long_doubles = sizeof(test_long_doubles) / sizeof(test_long_doubles[0]);
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        /* Vary inputs to prevent constant folding */
        int idx1 = i % num_doubles;
        int idx2 = (i + 1) % num_doubles;
        int idx3 = i % num_long_doubles;
        int idx4 = (i + 2) % num_long_doubles;
        
        /* Test all condition codes */
        total += test_unordered(test_doubles[idx1], test_doubles[idx2]);
        total += test_ordered(test_doubles[idx1], test_doubles[idx2]);
        total += test_uneq(test_doubles[idx1], test_doubles[idx2]);
        total += test_unge(test_doubles[idx1], test_doubles[idx2]);
        total += test_ungt(test_doubles[idx1], test_doubles[idx2]);
        total += test_unle(test_doubles[idx1], test_doubles[idx2]);
        total += test_unlt(test_doubles[idx1], test_doubles[idx2]);
        total += test_ltgt(test_doubles[idx1], test_doubles[idx2]);
        
        /* Test mixed operations */
        total += test_mixed_operations(
            test_long_doubles[idx3],
            test_long_doubles[idx4],
            test_doubles[idx1],
            test_doubles[idx2]
        );
        
        /* Test switch-based selection with volatile selector */
        g_selector = i;
        total += test_switch_based(g_selector, 
                                  test_doubles[idx1], 
                                  test_doubles[idx2]);
        
        /* Occasionally test with out-of-range selector to trigger default case */
        if ((i % 17) == 0) {
            volatile int bad_selector = 15;  /* Out of range */
            total += test_switch_based(bad_selector,
                                      test_doubles[idx1],
                                      test_doubles[idx2]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    /* Additional test with inline assembly that should generate 
       condition code output in assembly file */
    asm volatile ("# ===== Condition Code Test Block =====\n"
                  "fucomip %%st(1), %%st(0)\n"
                  "set%c0 %%al\n"
                  "set%c1 %%bl\n"
                  "set%c2 %%cl\n"
                  "set%c3 %%dl\n"
                  : 
                  : "u"(UNORDERED), "u"(ORDERED), "u"(UNEQ), "u"(UNGE)
                  : "al", "bl", "cl", "dl", "cc", "st");
    
    asm volatile ("# ===== More Condition Codes =====\n"
                  "set%c0 %%al\n"
                  "set%c1 %%bl\n"
                  : 
                  : "u"(UNGT), "u"(UNLE));
    
    asm volatile ("# ===== Final Condition Codes =====\n"
                  "set%c0 %%al\n"
                  "set%c1 %%bl\n"
                  : 
                  : "u"(UNLT), "u"(LTGT));
    
    return total != 0;
}
