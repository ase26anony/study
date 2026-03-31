/* test_i386_condcodes.c - Program to trigger x86 condition code printing logic */
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

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static int test_mixed_x87_sse(long double a, double b) __attribute__((noinline));
static void use_cond_code_in_switch(int cc) __attribute__((noinline));
static int helper_with_cond_code(int cc, double a, double b) __attribute__((noinline));

/* Test functions for each condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 unordered comparison */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

static int test_ordered(double a, double b) {
    int result;
    /* Using SSE ordered comparison */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

static int test_uneq(double a, double b) {
    int result;
    /* Mixed comparison with unordered or equal */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    return result;
}

static int test_unge(double a, double b) {
    int result;
    /* Unordered or greater than or equal */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNGE)
        : "cc"
    );
    return result;
}

static int test_ungt(double a, double b) {
    int result;
    /* Unordered or greater than */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

static int test_unle(double a, double b) {
    int result;
    /* Unordered or less than or equal */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLE)
        : "cc"
    );
    return result;
}

static int test_unlt(double a, double b) {
    int result;
    /* Unordered or less than */
    asm volatile (
        "fldl %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

static int test_ltgt(double a, double b) {
    int result;
    /* Less than or greater than (ordered and not equal) */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* Mixed x87 and SSE operations */
static int test_mixed_x87_sse(long double a, double b) {
    int result1, result2;
    
    /* x87 operation */
    asm volatile (
        "fldt %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "m"(b), "m"(a), "u"(UNORDERED)
        : "cc", "st"
    );
    
    /* SSE operation */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"((double)b), "x"((double)a), "u"(ORDERED)
        : "cc"
    );
    
    return result1 & result2;
}

/* Helper function that uses condition code in assembly */
static int helper_with_cond_code(int cc, double a, double b) {
    int result;
    
    /* Dynamic condition code usage - compiler must handle %c expansion */
    asm volatile (
        "comisd %3, %2\n\t"
        "set%c1 %0"
        : "=r"(result)
        : "u"(cc), "x"(a), "x"(b)
        : "cc"
    );
    
    return result;
}

/* Function with switch to potentially trigger default case */
static void use_cond_code_in_switch(int cc) {
    int result;
    double a = g_d1;
    double b = g_d2;
    
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
            /* This might trigger output_operand_lossage if cc is invalid */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c1 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cc)
                : "cc"
            );
            break;
    }
    
    /* Use result to prevent dead code elimination */
    g_selector = result;
}

int main(int argc, char *argv[]) {
    int i, iterations = 100;
    volatile int total = 0;
    
    /* Parse iteration count from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize test values with some NaN and infinity values */
    double dvals[] = {0.0, 1.0, -1.0, INFINITY, -INFINITY, NAN, 2.0, 3.0};
    long double ldvals[] = {0.0L, 1.0L, -1.0L, INFINITY, -INFINITY, NAN, 2.0L, 3.0L};
    int num_vals = sizeof(dvals) / sizeof(dvals[0]);
    
    printf("Testing x86 condition code printing logic for %d iterations...\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        int idx1 = i % num_vals;
        int idx2 = (i + 1) % num_vals;
        
        /* Test all condition codes */
        total += test_unordered(dvals[idx1], dvals[idx2]);
        total += test_ordered(dvals[idx1], dvals[idx2]);
        total += test_uneq(dvals[idx1], dvals[idx2]);
        total += test_unge(dvals[idx1], dvals[idx2]);
        total += test_ungt(dvals[idx1], dvals[idx2]);
        total += test_unle(dvals[idx1], dvals[idx2]);
        total += test_unlt(dvals[idx1], dvals[idx2]);
        total += test_ltgt(dvals[idx1], dvals[idx2]);
        
        /* Test mixed x87/SSE */
        total += test_mixed_x87_sse(ldvals[idx1], dvals[idx2]);
        
        /* Test with dynamic condition code selection */
        int cc = i % 9;  /* 8 valid codes + 1 potentially invalid */
        total += helper_with_cond_code(cc, dvals[idx1], dvals[idx2]);
        
        /* Use switch with condition codes */
        use_cond_code_in_switch(cc);
        
        /* Mix with regular C comparisons to provide context */
        if (dvals[idx1] != dvals[idx2]) {
            total += 1;
        }
        if (dvals[idx1] >= dvals[idx2]) {
            total += 2;
        }
    }
    
    printf("Total accumulated: %d\n", total);
    
    /* Try to trigger potential invalid condition code */
    if (iterations > 50) {
        /* Force a potentially invalid condition code */
        int invalid_cc = 15;  /* Invalid condition code */
        asm volatile (
            "# Potential invalid condition code:\n\t"
            "comisd %2, %1\n\t"
            "set%c1 %0"
            : "=r"(total)
            : "x"(g_d1), "x"(g_d2), "u"(invalid_cc)
            : "cc"
        );
    }
    
    return total != 0 ? 0 : 1;
}
