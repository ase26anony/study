/* Condition code test program for i386.cc coverage */
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

/* Condition code constants matching i386.cc */
enum x86_cond_code {
    UNORDERED = 16,
    ORDERED = 17,
    UNEQ = 18,
    UNGE = 19,
    UNGT = 20,
    UNLE = 21,
    UNLT = 22,
    LTGT = 23
};

/* Force noinline to ensure separate functions */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Use %c modifier to output condition code name */
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNORDERED), "x"(a), "x"(b)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(ORDERED), "x"(a), "x"(b)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_uneq(long double a, long double b) {
    int result;
    /* x87 floating point comparison */
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "u"(UNEQ), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGE), "x"(a), "x"(b)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "u"(UNGT), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLE), "x"(a), "x"(b)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "u"(UNLT), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "u"(LTGT), "x"(a), "x"(b)
                  : "cc");
    return result;
}

/* Mixed x87 and SSE operations */
__attribute__((noinline))
static int test_mixed_ops(double d1, double d2, long double ld1, long double ld2) {
    int r1, r2, r3;
    
    /* SSE comparison */
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(r1)
                  : "u"(UNORDERED), "x"(d1), "x"(d2)
                  : "cc");
    
    /* x87 comparison */
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(r2)
                  : "u"(ORDERED), "t"(ld1), "u"(ld2)
                  : "cc", "st");
    
    /* Another SSE with different condition */
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(r3)
                  : "u"(LTGT), "x"(d1 * 2.0), "x"(d2 / 2.0)
                  : "cc");
    
    return r1 + r2 + r3;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int dispatch_condition(int selector, double a, double b) {
    int result = 0;
    
    /* Volatile to prevent optimization */
    volatile int sel = selector;
    
    switch (sel & 7) {
        case 0:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNORDERED), "x"(a), "x"(b)
                          : "cc");
            break;
        case 1:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(ORDERED), "x"(a), "x"(b)
                          : "cc");
            break;
        case 2:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNEQ), "x"(a), "x"(b)
                          : "cc");
            break;
        case 3:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNGE), "x"(a), "x"(b)
                          : "cc");
            break;
        case 4:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNGT), "x"(a), "x"(b)
                          : "cc");
            break;
        case 5:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNLE), "x"(a), "x"(b)
                          : "cc");
            break;
        case 6:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(UNLT), "x"(a), "x"(b)
                          : "cc");
            break;
        case 7:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "u"(LTGT), "x"(a), "x"(b)
                          : "cc");
            break;
        default:
            /* This might trigger output_operand_lossage if compiler
               tries to output an invalid condition code */
            asm volatile ("# Invalid condition code %c0"
                          : 
                          : "u"(sel)
                          : "cc");
            break;
    }
    
    return result;
}

/* Helper to generate NaN values */
static double make_nan(void) {
    volatile double zero = 0.0;
    return 0.0 / zero;
}

static long double make_nanl(void) {
    volatile long double zero = 0.0L;
    return 0.0L / zero;
}

int main(int argc, char *argv[]) {
    int i, iterations = 100;
    volatile int total = 0;  /* Prevent optimization */
    
    /* Parse iterations from command line if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create test values including NaN, infinity, normal numbers */
    double dvals[] = {1.0, 2.0, make_nan(), INFINITY, -INFINITY, 0.0};
    long double ldvals[] = {1.0L, 2.0L, make_nanl(), INFINITY, -INFINITY, 0.0L};
    int dcount = sizeof(dvals)/sizeof(dvals[0]);
    int ldcount = sizeof(ldvals)/sizeof(ldvals[0]);
    
    printf("Testing condition codes for %d iterations\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        /* Use volatile index to prevent optimization */
        volatile int idx = i;
        
        /* Test all condition code functions */
        total += test_unordered(dvals[idx % dcount], dvals[(idx + 1) % dcount]);
        total += test_ordered(dvals[idx % dcount], dvals[(idx + 2) % dcount]);
        total += test_uneq(ldvals[idx % ldcount], ldvals[(idx + 1) % ldcount]);
        total += test_unge(dvals[idx % dcount], dvals[(idx + 3) % dcount]);
        total += test_ungt(ldvals[idx % ldcount], ldvals[(idx + 2) % ldcount]);
        total += test_unle(dvals[idx % dcount], dvals[(idx + 4) % dcount]);
        total += test_unlt(ldvals[idx % ldcount], ldvals[(idx + 3) % ldcount]);
        total += test_ltgt(dvals[idx % dcount], dvals[(idx + 5) % dcount]);
        
        /* Test mixed operations */
        total += test_mixed_ops(g_d1 + idx, g_d2 - idx,
                               g_ld1 + idx, g_ld2 - idx);
        
        /* Test dispatch function with varying selector */
        total += dispatch_condition(idx, 
                                   dvals[idx % dcount],
                                   dvals[(idx + 1) % dcount]);
        
        /* Also test with regular C comparisons to provide context */
        if (dvals[idx % dcount] != dvals[(idx + 1) % dcount]) {
            total += 1;
        }
        
        /* Update global selector */
        g_selector = idx;
    }
    
    /* Force use of all condition codes in printf format (indirect) */
    printf("Total: %d\n", total);
    
    /* Additional test that might trigger the default case */
    if (iterations > 1000) {
        /* Potentially out of range condition code */
        int invalid_cc = 255;
        asm volatile ("# Potential invalid condition code %c0"
                      :
                      : "u"(invalid_cc));
    }
    
    return total != 0 ? 0 : 1;
}
