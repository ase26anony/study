/* test_i386_condcodes.c - Target coverage for i386.cc condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile double g_d1 = 1.0, g_d2 = 2.0, g_d3 = NAN, g_d4 = INFINITY;
volatile long double g_ld1 = 1.0L, g_ld2 = 2.0L, g_ld3 = NAN, g_ld4 = INFINITY;
volatile int g_selector = 0;

/* Helper to force compiler to generate condition code operands */
#define DECLARE_TEST_FUNC(name, cond) \
    __attribute__((noinline)) \
    static int test_##name(double a, double b) { \
        int result; \
        asm volatile ("comisd %1, %2\n\t" \
                      "set%c0 %0" \
                      : "=r"(result) \
                      : "x"(a), "x"(b), "i"(cond) \
                      : "cc"); \
        return result; \
    }

/* Declare test functions for each condition code */
DECLARE_TEST_FUNC(unordered, UNORDERED)
DECLARE_TEST_FUNC(ordered, ORDERED)
DECLARE_TEST_FUNC(uneq, UNEQ)
DECLARE_TEST_FUNC(unge, UNGE)
DECLARE_TEST_FUNC(ungt, UNGT)
DECLARE_TEST_FUNC(unle, UNLE)
DECLARE_TEST_FUNC(unlt, UNLT)
DECLARE_TEST_FUNC(ltgt, LTGT)

/* x87-specific tests using long double */
__attribute__((noinline))
static int test_x87_unordered(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "u"(UNORDERED), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_x87_ordered(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0\n\t"
                  "fstp %%st(0)"
                  : "=r"(result)
                  : "u"(ORDERED), "t"(a), "u"(b)
                  : "cc", "st");
    return result;
}

/* Mixed x87/SSE test */
__attribute__((noinline))
static int test_mixed_uneq(double a, long double b) {
    int r1, r2;
    /* SSE comparison */
    asm volatile ("comisd %2, %3\n\t"
                  "set%c0 %0"
                  : "=r"(r1)
                  : "i"(UNEQ), "x"(a), "x"((double)b)
                  : "cc");
    
    /* x87 comparison */
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %1\n\t"
                  "fstp %%st(0)"
                  : "=r"(r2)
                  : "i"(UNEQ), "t"(b), "u"((long double)a)
                  : "cc", "st");
    
    return r1 & r2;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int test_switch_cond(double a, double b, int cond_code) {
    int result = 0;
    
    /* This switch may cause the compiler to generate 
       condition code operands in RTL */
    switch (cond_code & 0x7) {
        case 0:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(UNORDERED)
                          : "cc");
            break;
        case 1:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(ORDERED)
                          : "cc");
            break;
        case 2:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(UNEQ)
                          : "cc");
            break;
        case 3:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(UNGE)
                          : "cc");
            break;
        case 4:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(UNGT)
                          : "cc");
            break;
        case 5:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(UNLE)
                          : "cc");
            break;
        case 6:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(UNLT)
                          : "cc");
            break;
        case 7:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(LTGT)
                          : "cc");
            break;
        default:
            /* This might trigger output_operand_lossage if compiler
               generates an invalid condition code */
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(cond_code)  /* Potentially invalid */
                          : "cc");
    }
    return result;
}

/* Function with complex control flow to obscure optimizations */
__attribute__((noinline))
static int test_complex_cond(double a, double b, int iter) {
    int result = 0;
    volatile int local_selector = iter;
    
    for (int i = 0; i < iter; i++) {
        /* Mix regular C comparisons with inline asm */
        if (a != b) {
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "i"(UNORDERED + (i & 7))
                          : "cc");
        }
        
        if (a >= b) {
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "+r"(result)
                          : "x"(b), "x"(a), "i"(UNGE + (i & 3))
                          : "cc");
        }
        
        /* Alternate between condition codes */
        switch (local_selector & 0x3) {
            case 0:
                asm volatile ("comisd %1, %2\n\t"
                              "set%c0 %0"
                              : "+r"(result)
                              : "x"(a), "x"(b), "i"(UNLT)
                              : "cc");
                break;
            case 1:
                asm volatile ("comisd %1, %2\n\t"
                              "set%c0 %0"
                              : "+r"(result)
                              : "x"(a), "x"(b), "i"(UNLE)
                              : "cc");
                break;
            case 2:
                asm volatile ("comisd %1, %2\n\t"
                              "set%c0 %0"
                              : "+r"(result)
                              : "x"(a), "x"(b), "i"(UNGT)
                              : "cc");
                break;
            case 3:
                asm volatile ("comisd %1, %2\n\t"
                              "set%c0 %0"
                              : "+r"(result)
                              : "x"(a), "x"(b), "i"(UNGE)
                              : "cc");
                break;
        }
        
        local_selector = (local_selector * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    volatile int accumulator = 0;
    
    /* Use command line argument to control iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize test values from volatile globals */
    double dvals[] = {g_d1, g_d2, g_d3, g_d4};
    long double ldvals[] = {g_ld1, g_ld2, g_ld3, g_ld4};
    
    printf("Testing i386 condition code printing (targeting lines 13992-14017)\n");
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 4;
        
        /* Test all condition codes */
        accumulator += test_unordered(dvals[idx], dvals[(idx + 1) % 4]);
        accumulator += test_ordered(dvals[idx], dvals[(idx + 2) % 4]);
        accumulator += test_uneq(dvals[idx], dvals[(idx + 3) % 4]);
        accumulator += test_unge(dvals[(idx + 1) % 4], dvals[idx]);
        accumulator += test_ungt(dvals[(idx + 2) % 4], dvals[idx]);
        accumulator += test_unle(dvals[(idx + 3) % 4], dvals[idx]);
        accumulator += test_unlt(dvals[idx], dvals[(idx + 1) % 4]);
        accumulator += test_ltgt(dvals[idx], dvals[(idx + 2) % 4]);
        
        /* Test x87 operations */
        accumulator += test_x87_unordered(ldvals[idx], ldvals[(idx + 1) % 4]);
        accumulator += test_x87_ordered(ldvals[idx], ldvals[(idx + 2) % 4]);
        
        /* Test mixed operations */
        accumulator += test_mixed_uneq(dvals[idx], ldvals[(idx + 3) % 4]);
        
        /* Test switch-based condition selection */
        accumulator += test_switch_cond(dvals[idx], dvals[(idx + 1) % 4], i & 0xF);
        
        /* Test complex control flow */
        accumulator += test_complex_cond(dvals[idx], dvals[(idx + 2) % 4], (i % 8) + 1);
        
        /* Update volatile selector */
        g_selector = (g_selector * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    printf("Accumulator result: %d\n", accumulator);
    
    /* Additional test that might trigger the default case */
    if (argc > 2 && strcmp(argv[2], "test-default") == 0) {
        /* Try to pass an out-of-range condition code */
        int dummy;
        asm volatile ("# Potential invalid condition code\n"
                      : "=r"(dummy)
                      : "i"(255)  /* Invalid condition code */
                      : "cc");
    }
    
    return accumulator != 0 ? 0 : 1;
}
