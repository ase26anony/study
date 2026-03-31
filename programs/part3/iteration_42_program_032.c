/* test_condition_codes.c - Target uncovered lines in i386.cc */
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

/* Force noinline to ensure separate functions */
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
    /* Use both x87 and SSE for variety */
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(ORDERED)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_uneq(long double a, long double b) {
    int result;
    /* x87 long double comparison */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNEQ)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNGE)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGT)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    /* Mix with regular C comparison for context */
    if (a != b) {
        asm volatile ("comisd %1, %2\n\t"
                      "set%c0 %0"
                      : "=r"(result)
                      : "x"(a), "x"(b), "u"(UNLE)
                      : "cc");
    } else {
        result = 0;
    }
    return result;
}

__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLT)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(LTGT)
                  : "cc");
    return result;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int test_conditional_switch(int cc, double a, double b) {
    int result = 0;
    
    /* Switch to potentially trigger default case printing */
    switch (cc) {
        case UNORDERED:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNORDERED)
                          : "cc");
            break;
        case ORDERED:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(ORDERED)
                          : "cc");
            break;
        case UNEQ:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNEQ)
                          : "cc");
            break;
        case UNGE:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNGE)
                          : "cc");
            break;
        case UNGT:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNGT)
                          : "cc");
            break;
        case UNLE:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNLE)
                          : "cc");
            break;
        case UNLT:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(UNLT)
                          : "cc");
            break;
        case LTGT:
            asm volatile ("comisd %1, %2\n\t"
                          "set%c0 %0"
                          : "=r"(result)
                          : "x"(a), "x"(b), "u"(LTGT)
                          : "cc");
            break;
        /* Intentionally no default to see if compiler handles it */
    }
    return result;
}

/* Function that might trigger output_operand_lossage */
__attribute__((noinline))
static void test_invalid_cc(int invalid_cc, double a, double b) {
    int result;
    /* This might trigger the default case if invalid_cc > 7 */
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(invalid_cc)
                  : "cc");
    
    /* Use result to prevent elimination */
    g_selector += result;
}

int main(int argc, char *argv[]) {
    volatile int sum = 0;
    int iterations = 100;
    
    /* Use command line to control iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create arrays with various FP values */
    double d_vals[] = {0.0, 1.0, -1.0, INFINITY, -INFINITY, NAN, 2.5, -2.5};
    long double ld_vals[] = {0.0L, 1.0L, -1.0L, INFINITY, -INFINITY, NAN, 3.14L, -3.14L};
    int d_count = sizeof(d_vals)/sizeof(d_vals[0]);
    int ld_count = sizeof(ld_vals)/sizeof(ld_vals[0]);
    
    /* Main test loop */
    for (int i = 0; i < iterations; i++) {
        /* Mix volatile and non-volatile accesses */
        double a = g_d1 + i * 0.1;
        double b = g_d2 - i * 0.1;
        long double la = g_ld1 + i * 0.1L;
        long double lb = g_ld2 - i * 0.1L;
        
        /* Array indices based on volatile to prevent optimization */
        int idx1 = (g_selector + i) % d_count;
        int idx2 = (g_selector + i * 2) % d_count;
        int idx3 = (g_selector + i * 3) % ld_count;
        int idx4 = (g_selector + i * 4) % ld_count;
        
        /* Call all condition code functions */
        sum += test_unordered(d_vals[idx1], d_vals[idx2]);
        sum += test_ordered(a, b);
        sum += test_uneq(ld_vals[idx3], ld_vals[idx4]);
        sum += test_unge(d_vals[idx1], d_vals[idx2]);
        sum += test_ungt(la, lb);
        sum += test_unle(a, b);
        sum += test_unlt(la, lb);
        sum += test_ltgt(d_vals[idx1], d_vals[idx2]);
        
        /* Use switch-based function with varying condition codes */
        int cc = (i + g_selector) % 8;
        sum += test_conditional_switch(cc, a, b);
        
        /* Occasionally test with potentially invalid condition code */
        if ((i % 17) == 0) {
            int invalid_cc = 8 + (i % 4);  /* Values 8-11, which are invalid */
            test_invalid_cc(invalid_cc, a, b);
        }
        
        /* Mix with regular FP comparisons */
        if (a >= b) sum += 1;
        if (la != lb) sum -= 1;
        if (isnan(d_vals[idx1])) sum += 2;
    }
    
    /* Update volatile global */
    g_selector = sum;
    
    printf("Final sum: %d\n", sum);
    
    /* Additional test to ensure assembly output */
    if (sum > 0) {
        /* Force compiler to consider all condition codes */
        asm volatile ("# Condition code test complete");
    }
    
    return 0;
}
