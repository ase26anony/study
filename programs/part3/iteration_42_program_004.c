/* test_condition_codes.c - Target uncovered lines in i386.cc condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent constant folding */
volatile double g_dbl1 = 1.0;
volatile double g_dbl2 = 2.0;
volatile long double g_ldbl1 = 3.0L;
volatile long double g_ldbl2 = 4.0L;
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
    /* Use %c modifier to output condition code name */
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNORDERED)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
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
    /* x87 floating point comparison */
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "fstp %%st(0)\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "t"(a), "t"(b), "u"(UNEQ)
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
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "fstp %%st(0)\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "t"(a), "t"(b), "u"(UNGT)
                  : "cc", "st");
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile ("comisd %1, %2\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "x"(a), "x"(b), "u"(UNLE)
                  : "cc");
    return result;
}

__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    asm volatile ("fldt %2\n\t"
                  "fldt %1\n\t"
                  "fucomip %%st(1), %%st(0)\n\t"
                  "fstp %%st(0)\n\t"
                  "set%c0 %0"
                  : "=r"(result)
                  : "t"(a), "t"(b), "u"(UNLT)
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

/* Function that uses switch to select condition code - may trigger printing */
__attribute__((noinline))
static int test_conditional_switch(int cc, double a, double b) {
    int result = 0;
    
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
        default:
            /* This might trigger output_operand_lossage if cc is out of range */
            asm volatile ("# Invalid condition code %c0"
                          : 
                          : "u"(cc));
            result = -1;
    }
    
    return result;
}

/* Mixed x87 and SSE operations */
__attribute__((noinline))
static int test_mixed_operations(double d1, double d2, 
                                 long double ld1, long double ld2) {
    int sum = 0;
    
    /* SSE comparisons */
    sum += test_unordered(d1, d2);
    sum += test_ordered(d1 + 1.0, d2 - 1.0);
    sum += test_unge(d1 * 2.0, d2 / 2.0);
    sum += test_unle(d1, d2);
    sum += test_ltgt(d1, d2);
    
    /* x87 comparisons */
    sum += test_uneq(ld1, ld2);
    sum += test_ungt(ld1 + 1.0L, ld2 - 1.0L);
    sum += test_unlt(ld1 * 2.0L, ld2 / 2.0L);
    
    return sum;
}

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize test values from volatile sources */
    double dvals[4];
    long double ldvals[4];
    
    dvals[0] = g_dbl1;
    dvals[1] = g_dbl2;
    dvals[2] = g_dbl1 * 3.14159;
    dvals[3] = g_dbl2 / 2.71828;
    
    ldvals[0] = g_ldbl1;
    ldvals[1] = g_ldbl2;
    ldvals[2] = g_ldbl1 * 3.14159265358979323846L;
    ldvals[3] = g_ldbl2 / 2.71828182845904523536L;
    
    /* Main test loop - prevents constant folding */
    for (int i = 0; i < iterations; i++) {
        /* Cycle through different value pairs */
        int idx1 = i % 4;
        int idx2 = (i + 1) % 4;
        
        /* Test all condition code functions */
        accumulator += test_unordered(dvals[idx1], dvals[idx2]);
        accumulator += test_ordered(dvals[idx2], dvals[idx1]);
        accumulator += test_uneq(ldvals[idx1], ldvals[idx2]);
        accumulator += test_unge(dvals[idx1] + i, dvals[idx2] - i);
        accumulator += test_ungt(ldvals[idx1] * i, ldvals[idx2] / (i + 1));
        accumulator += test_unle(dvals[idx1], dvals[idx2] * 1.1);
        accumulator += test_unlt(ldvals[idx1] + 0.5L, ldvals[idx2]);
        accumulator += test_ltgt(dvals[idx1] * 0.9, dvals[idx2]);
        
        /* Mixed operations */
        accumulator += test_mixed_operations(
            dvals[idx1], dvals[idx2],
            ldvals[idx1], ldvals[idx2]
        );
        
        /* Use switch-based function with varying condition codes */
        int cc = i % 9;  /* 8 valid codes + 1 potentially invalid */
        accumulator += test_conditional_switch(cc, dvals[idx1], dvals[idx2]);
        
        /* Modify values slightly each iteration */
        dvals[idx1] += 0.1;
        ldvals[idx2] -= 0.05L;
        
        /* Also test with NaN to trigger UNORDERED cases */
        if (i % 7 == 0) {
            double nan_val = 0.0 / 0.0;
            accumulator += test_unordered(dvals[idx1], nan_val);
            accumulator += test_ordered(nan_val, dvals[idx2]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated result: %d\n", accumulator);
    
    /* Force printing of condition codes via inline asm with %c modifier */
    asm volatile ("# Final condition code test block\n\t"
                  "# %c0 %c1 %c2 %c3 %c4 %c5 %c6 %c7"
                  :
                  : "u"(UNORDERED), "u"(ORDERED), "u"(UNEQ),
                    "u"(UNGE), "u"(UNGT), "u"(UNLE),
                    "u"(UNLT), "u"(LTGT));
    
    return accumulator != 0 ? 0 : 1;
}
