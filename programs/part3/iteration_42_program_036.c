/* test_i386_condcodes.c
 * Target: x86 GCC, specifically i386.cc lines 13992-14017
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -S test_i386_condcodes.c
 * Also try: gcc -O3 -mfpmath=both -march=core2 -ffast-math -fverbose-asm
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
#define UNORDERED 16
#define ORDERED   17
#define UNEQ      18
#define UNGE      19
#define UNGT      20
#define UNLE      21
#define UNLT      22
#define LTGT      23

/* Prevent inlining to ensure separate functions */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Using %c modifier to output condition code name */
    asm volatile ("comisd %1, %0\n\t"
                  "set%c0 %2"
                  : "=@ccu" (result)
                  : "x" (a), "x" (b), "u" (UNORDERED)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    asm volatile ("comisd %1, %0\n\t"
                  "set%c0 %2"
                  : "=@cco" (result)
                  : "x" (a), "x" (b), "u" (ORDERED)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_uneq(double a, double b) {
    int result;
    asm volatile ("comisd %1, %0\n\t"
                  "set%c0 %2"
                  : "=@ccueq" (result)
                  : "x" (a), "x" (b), "u" (UNEQ)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile ("comisd %1, %0\n\t"
                  "set%c0 %2"
                  : "=@ccunge" (result)
                  : "x" (a), "x" (b), "u" (UNGE)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ungt(double a, double b) {
    int result;
    asm volatile ("comisd %1, %0\n\t"
                  "set%c0 %2"
                  : "=@ccungt" (result)
                  : "x" (a), "x" (b), "u" (UNGT)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile ("comisd %1, %0\n\t"
                  "set%c0 %2"
                  : "=@ccunle" (result)
                  : "x" (a), "x" (b), "u" (UNLE)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_unlt(double a, double b) {
    int result;
    asm volatile ("comisd %1, %0\n\t"
                  "set%c0 %2"
                  : "=@ccunlt" (result)
                  : "x" (a), "x" (b), "u" (UNLT)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile ("comisd %1, %0\n\t"
                  "set%c0 %2"
                  : "=@ccltgt" (result)
                  : "x" (a), "x" (b), "u" (LTGT)
                  : "cc");
    return result;
}

/* x87 long double version */
__attribute__((noinline))
static int test_unordered_ld(long double a, long double b) {
    int result;
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0"
                  : "=r" (result)
                  : "u" (UNORDERED), "t" (a), "u" (b)
                  : "cc", "st");
    return result;
}

/* Mixed SSE and x87 operations */
__attribute__((noinline))
static int test_mixed_conditions(double a, long double b) {
    int r1, r2;
    
    /* SSE comparison */
    asm volatile ("comisd %2, %1\n\t"
                  "set%c0 %0"
                  : "=r" (r1)
                  : "x" (a), "x" ((double)b), "u" (UNGE)
                  : "cc");
    
    /* x87 comparison */
    asm volatile ("fucomip %%st(1), %%st(0)\n\t"
                  "set%c0 %0"
                  : "=r" (r2)
                  : "u" (UNLE), "t" (b), "u" ((long double)a)
                  : "cc", "st");
    
    return r1 & r2;
}

/* Function that uses switch to select condition code */
__attribute__((noinline))
static int test_switch_condition(double a, double b, int cond_code) {
    int result = 0;
    
    switch (cond_code) {
        case UNORDERED:
            asm volatile ("comisd %1, %0\n\t"
                          "set%c2 %3"
                          : "=@ccu" (result)
                          : "x" (a), "x" (b), "u" (UNORDERED)
                          : "cc");
            break;
        case ORDERED:
            asm volatile ("comisd %1, %0\n\t"
                          "set%c2 %3"
                          : "=@cco" (result)
                          : "x" (a), "x" (b), "u" (ORDERED)
                          : "cc");
            break;
        case UNEQ:
            asm volatile ("comisd %1, %0\n\t"
                          "set%c2 %3"
                          : "=@ccueq" (result)
                          : "x" (a), "x" (b), "u" (UNEQ)
                          : "cc");
            break;
        case UNGE:
            asm volatile ("comisd %1, %0\n\t"
                          "set%c2 %3"
                          : "=@ccunge" (result)
                          : "x" (a), "x" (b), "u" (UNGE)
                          : "cc");
            break;
        case UNGT:
            asm volatile ("comisd %1, %0\n\t"
                          "set%c2 %3"
                          : "=@ccungt" (result)
                          : "x" (a), "x" (b), "u" (UNGT)
                          : "cc");
            break;
        case UNLE:
            asm volatile ("comisd %1, %0\n\t"
                          "set%c2 %3"
                          : "=@ccunle" (result)
                          : "x" (a), "x" (b), "u" (UNLE)
                          : "cc");
            break;
        case UNLT:
            asm volatile ("comisd %1, %0\n\t"
                          "set%c2 %3"
                          : "=@ccunlt" (result)
                          : "x" (a), "x" (b), "u" (UNLT)
                          : "cc");
            break;
        case LTGT:
            asm volatile ("comisd %1, %0\n\t"
                          "set%c2 %3"
                          : "=@ccltgt" (result)
                          : "x" (a), "x" (b), "u" (LTGT)
                          : "cc");
            break;
        default:
            /* This should trigger output_operand_lossage for invalid code */
            asm volatile ("# Invalid condition code %0" : : "i" (cond_code));
            result = -1;
    }
    
    return result;
}

/* Function that might trigger the default case */
__attribute__((noinline))
static int test_invalid_condition(double a, double b) {
    int result;
    /* Using an invalid condition code - might trigger default case */
    asm volatile ("comisd %1, %0\n\t"
                  "# Potential invalid condition code"
                  : "=@ccu" (result)
                  : "x" (a), "x" (b), "u" (999)  /* Invalid code */
                  : "cc");
    return result;
}

int main(int argc, char *argv[]) {
    volatile int total = 0;
    int i, iterations;
    
    /* Use command line argument or default for iterations */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Create some NaN values for unordered comparisons */
    double nan_val = 0.0 / 0.0;
    double inf_val = 1.0 / 0.0;
    
    /* Array of test values */
    double d_vals[] = {g_d1, g_d2, nan_val, inf_val, -inf_val, 0.0, -0.0};
    long double ld_vals[] = {g_ld1, g_ld2, (long double)nan_val, 
                            (long double)inf_val, (long double)-inf_val, 
                            0.0L, -0.0L};
    int num_vals = sizeof(d_vals) / sizeof(d_vals[0]);
    
    printf("Testing x86 condition codes for %d iterations...\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        int idx1 = i % num_vals;
        int idx2 = (i + 1) % num_vals;
        
        /* Test all condition codes with double */
        total += test_unordered(d_vals[idx1], d_vals[idx2]);
        total += test_ordered(d_vals[idx1], d_vals[idx2]);
        total += test_uneq(d_vals[idx1], d_vals[idx2]);
        total += test_unge(d_vals[idx1], d_vals[idx2]);
        total += test_ungt(d_vals[idx1], d_vals[idx2]);
        total += test_unle(d_vals[idx1], d_vals[idx2]);
        total += test_unlt(d_vals[idx1], d_vals[idx2]);
        total += test_ltgt(d_vals[idx1], d_vals[idx2]);
        
        /* Test with long double (x87) */
        total += test_unordered_ld(ld_vals[idx1], ld_vals[idx2]);
        
        /* Test mixed operations */
        total += test_mixed_conditions(d_vals[idx1], ld_vals[idx2]);
        
        /* Test switch-based condition selection */
        int cond = (i % 9);  /* 8 valid + 1 potentially invalid */
        total += test_switch_condition(d_vals[idx1], d_vals[idx2], 
                                      (cond < 8) ? (UNORDERED + cond) : 999);
        
        /* Regular C comparisons to provide context */
        if (d_vals[idx1] != d_vals[idx2]) {
            total++;
        }
        if (d_vals[idx1] >= d_vals[idx2]) {
            total++;
        }
        
        /* Update volatile selector to prevent optimization */
        g_selector = i;
    }
    
    /* Try to trigger invalid condition code */
    if (iterations > 50) {
        total += test_invalid_condition(g_d1, g_d2);
    }
    
    printf("Total: %d\n", total);
    
    /* Force assembly output of condition codes */
    asm volatile ("# Condition code test complete");
    
    return total != 0 ? 0 : 1;
}
