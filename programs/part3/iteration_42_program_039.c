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

/* Prevent inlining to ensure separate RTL generation */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* x87 unordered comparison */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNORDERED)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    /* SSE ordered comparison */
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(ORDERED)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_uneq(long double a, long double b) {
    int result;
    /* x87 unordered equal */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNEQ)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    /* SSE not less than (unordered or greater or equal) */
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNGE)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    /* x87 not less or equal (unordered or greater) */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGT)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    /* SSE unordered or less or equal */
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNLE)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    /* x87 unordered or less than */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLT)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    /* SSE unordered or not equal */
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(LTGT)
                  : "cc");
    return result;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int test_conditional_switch(int cc, double a, double b) {
    int result = 0;
    
    switch (cc) {
        case UNORDERED:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNORDERED)
                          : "cc");
            break;
        case ORDERED:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(ORDERED)
                          : "cc");
            break;
        case UNEQ:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNEQ)
                          : "cc");
            break;
        case UNGE:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNGE)
                          : "cc");
            break;
        case UNGT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNGT)
                          : "cc");
            break;
        case UNLE:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNLE)
                          : "cc");
            break;
        case UNLT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNLT)
                          : "cc");
            break;
        case LTGT:
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(LTGT)
                          : "cc");
            break;
        default:
            /* This should trigger output_operand_lossage for invalid code */
            asm volatile ("comisd %1, %2; set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(cc)  /* Invalid condition code */
                          : "cc");
            break;
    }
    return result;
}

/* Mixed x87 and SSE operations in one function */
__attribute__((noinline))
static int test_mixed_fpu(double d1, double d2, long double ld1, long double ld2) {
    int r1, r2, r3;
    
    /* SSE comparison */
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(r1)
                  : "x"(d1), "x"(d2), "u"(UNORDERED)
                  : "cc");
    
    /* x87 comparison */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(r2)
                  : "u"(ORDERED)
                  : "cc", "st");
    
    /* Another SSE with different condition */
    asm volatile ("comisd %1, %2; set%c0 %0"
                  : "=r"(r3)
                  : "x"(ld1), "x"(ld2), "u"(UNEQ)
                  : "cc");
    
    return r1 + r2 + r3;
}

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize test values with some NaN and infinity cases */
    double dvals[] = {1.0, 2.0, 0.0/0.0, 1.0/0.0, -1.0/0.0};
    long double ldvals[] = {3.0L, 4.0L, 0.0L/0.0L, 1.0L/0.0L, -1.0L/0.0L};
    int dcount = sizeof(dvals)/sizeof(dvals[0]);
    int ldcount = sizeof(ldvals)/sizeof(ldvals[0]);
    
    /* Force values to be used from memory to prevent constant propagation */
    volatile double *vd = dvals;
    volatile long double *vld = ldvals;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % (dcount - 1);
        
        /* Test all condition codes with various value combinations */
        accumulator += test_unordered(vd[idx], vd[idx+1]);
        accumulator += test_ordered(vd[idx], vd[idx+1]);
        accumulator += test_uneq(vld[idx], vld[idx+1]);
        accumulator += test_unge(vd[idx], vd[idx+1]);
        accumulator += test_ungt(vld[idx], vld[idx+1]);
        accumulator += test_unle(vd[idx], vd[idx+1]);
        accumulator += test_unlt(vld[idx], vld[idx+1]);
        accumulator += test_ltgt(vd[idx], vd[idx+1]);
        
        /* Test mixed FPU operations */
        accumulator += test_mixed_fpu(vd[idx], vd[idx+1], 
                                      vld[idx], vld[idx+1]);
        
        /* Test conditional switch with volatile selector */
        g_selector = (i % 9);  /* 8 valid codes + 1 invalid */
        accumulator += test_conditional_switch(g_selector, 
                                               vd[idx], vd[idx+1]);
        
        /* Mix with regular C comparisons to provide context */
        if (vd[idx] != vd[idx+1]) {
            accumulator += 1;
        }
        if (vld[idx] >= vld[idx+1]) {
            accumulator -= 1;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulator: %d\n", accumulator);
    
    /* Force assembly output of condition codes via inline asm with modifiers */
    asm volatile ("# Condition code test complete\n"
                  : : : "memory");
    
    return 0;
}
