/* test_condition_codes.c - Target i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile globals to prevent constant folding */
volatile double g_d1 = 1.0, g_d2 = 2.0;
volatile long double g_ld1 = 3.0L, g_ld2 = 4.0L;
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

/* ========== Individual condition code test functions ========== */

/* Test UNORDERED condition code with x87 */
__attribute__((noinline))
static int test_unordered_x87(long double a, long double b) {
    int result;
    /* Use fucomip to set FPU flags, then condition code in set instruction */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNORDERED), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Test ORDERED condition code with SSE */
__attribute__((noinline))
static int test_ordered_sse(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

/* Test UNEQ condition code with mixed operations */
__attribute__((noinline))
static int test_uneq_mixed(double a, long double b) {
    int result1, result2;
    /* First with SSE */
    asm volatile (
        "comisd %3, %2\n\t"
        "set%c0 %0"
        : "=r"(result1)
        : "u"(UNEQ), "x"(a), "x"((double)b)
        : "cc"
    );
    
    /* Then with x87 */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNEQ), "=r"(result2)
        : "t"((long double)a), "u"(b)
        : "cc", "st"
    );
    
    return result1 | result2;
}

/* Test UNGE condition code */
__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNGE)
        : "cc"
    );
    return result;
}

/* Test UNGT condition code */
__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNGT), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Test UNLE condition code */
__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLE)
        : "cc"
    );
    return result;
}

/* Test UNLT condition code */
__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNLT), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition code */
__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* ========== Helper function with switch to force printing ========== */

/* This function uses a switch to select condition codes, potentially
   triggering the printing logic during RTL generation */
__attribute__((noinline))
static int dispatch_condition_code(int cc, double a, double b) {
    int result = 0;
    
    switch (cc) {
        case UNORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNORDERED)
                : "cc"
            );
            break;
            
        case ORDERED:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(ORDERED)
                : "cc"
            );
            break;
            
        case UNEQ:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNEQ)
                : "cc"
            );
            break;
            
        case UNGE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGE)
                : "cc"
            );
            break;
            
        case UNGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGT)
                : "cc"
            );
            break;
            
        case UNLE:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLE)
                : "cc"
            );
            break;
            
        case UNLT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLT)
                : "cc"
            );
            break;
            
        case LTGT:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(LTGT)
                : "cc"
            );
            break;
            
        default:
            /* This might trigger output_operand_lossage if cc is out of range */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cc)  /* Potentially invalid condition code */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create arrays with various floating point values */
    double d_vals[] = {0.0, 1.0, -1.0, INFINITY, -INFINITY, NAN, 2.5, -2.5};
    long double ld_vals[] = {0.0L, 1.0L, -1.0L, INFINITY, -INFINITY, NAN, 3.5L, -3.5L};
    int num_vals = sizeof(d_vals) / sizeof(d_vals[0]);
    
    /* Seed volatile selector with something non-constant */
    g_selector = argc;
    
    printf("Testing x86 condition code printing logic (iterations: %d)\n", iterations);
    
    for (int i = 0; i < iterations; i++) {
        /* Use volatile globals and loop index to prevent optimization */
        double d1 = g_d1 + i * 0.1;
        double d2 = g_d2 - i * 0.1;
        long double ld1 = g_ld1 + i * 0.1L;
        long double ld2 = g_ld2 - i * 0.1L;
        
        /* Mix array values with computed values */
        int idx = i % num_vals;
        double d_array = d_vals[idx];
        long double ld_array = ld_vals[idx];
        
        /* Call all condition code test functions */
        accumulator += test_unordered_x87(ld1, ld2);
        accumulator += test_ordered_sse(d1, d2);
        accumulator += test_uneq_mixed(d_array, ld_array);
        accumulator += test_unge(d1, d_array);
        accumulator += test_ungt(ld1, ld_array);
        accumulator += test_unle(d_array, d2);
        accumulator += test_unlt(ld_array, ld2);
        accumulator += test_ltgt(d1, d2);
        
        /* Also use regular C comparisons to provide context */
        if (d1 != d2) accumulator++;
        if (ld1 >= ld2) accumulator++;
        if (!(d_array < d2)) accumulator++;
        
        /* Use dispatch function with varying condition codes */
        int cc = (i + g_selector) % 9;  /* 9 instead of 8 to potentially hit default case */
        accumulator += dispatch_condition_code(cc, d1, d2);
        
        /* Mix in some special values */
        if (i % 7 == 0) {
            accumulator += test_unordered_x87(NAN, ld1);
            accumulator += test_ordered_sse(d1, NAN);
        }
    }
    
    /* Force result to be used */
    printf("Final accumulator value: %d\n", accumulator);
    
    /* Additional test that might trigger the default case printing */
    if (argc > 2) {
        /* Try with an explicitly out-of-range condition code */
        int invalid_cc = atoi(argv[2]);
        int result = dispatch_condition_code(invalid_cc, g_d1, g_d2);
        printf("Invalid condition code test result: %d\n", result);
    }
    
    return accumulator != 0 ? 0 : 1;
}
