/* Generated program to trigger x86 condition code printing logic in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent constant folding */
volatile double vd1 = 1.0;
volatile double vd2 = 2.0;
volatile long double vld1 = 3.0L;
volatile long double vld2 = 4.0L;
volatile int selector = 0;

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
static int test_mixed(long double a, long double b) __attribute__((noinline));
static int test_with_switch(int cc, double a, double b) __attribute__((noinline));
static void force_condition_printing(int cc) __attribute__((noinline));

/* Test functions for each condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 instruction with UNORDERED condition */
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
    /* Using SSE instruction with ORDERED condition */
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
    /* Mix x87 and condition code */
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
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* Mixed x87 and SSE with long double */
static int test_mixed(long double a, long double b) {
    int result1, result2;
    
    /* x87 comparison */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    
    /* Convert to double and use SSE */
    double da = (double)a;
    double db = (double)b;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"(da), "x"(db), "u"(ORDERED)
        : "cc"
    );
    
    return result1 & result2;
}

/* Function that uses switch to select condition code */
static int test_with_switch(int cc, double a, double b) {
    int result;
    
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
            /* This might trigger output_operand_lossage */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cc)  /* Invalid condition code */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* Function to force condition code printing */
static void force_condition_printing(int cc) {
    /* This function creates a scenario where the compiler needs to 
       output condition code names during RTL generation */
    double a = vd1;
    double b = vd2;
    
    /* Use inline asm with condition code operand */
    asm volatile (
        "# Condition code: %c3\n\t"
        "comisd %2, %1\n\t"
        "set%c3 %0"
        : "=r"(selector)
        : "x"(a), "x"(b), "u"(cc)
        : "cc"
    );
}

int main(int argc, char *argv[]) {
    int i, iterations = 100;
    volatile int total = 0;
    
    /* Parse iterations from command line if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize arrays with volatile values to prevent optimization */
    double darray[8];
    long double ldarray[8];
    
    for (i = 0; i < 8; i++) {
        darray[i] = vd1 + i * 0.5;
        ldarray[i] = vld1 + i * 0.5L;
    }
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        int idx = i % 8;
        int idx2 = (i + 1) % 8;
        
        /* Test all condition codes */
        total += test_unordered(darray[idx], darray[idx2]);
        total += test_ordered(darray[idx], darray[idx2]);
        total += test_uneq(darray[idx], darray[idx2]);
        total += test_unge(darray[idx], darray[idx2]);
        total += test_ungt(darray[idx], darray[idx2]);
        total += test_unle(darray[idx], darray[idx2]);
        total += test_unlt(darray[idx], darray[idx2]);
        total += test_ltgt(darray[idx], darray[idx2]);
        
        /* Test mixed x87/SSE */
        total += test_mixed(ldarray[idx], ldarray[idx2]);
        
        /* Test with switch - use volatile selector to prevent constant folding */
        selector = i % 9;  /* 8 valid + 1 potentially invalid */
        total += test_with_switch(selector, darray[idx], darray[idx2]);
        
        /* Force condition printing with various codes */
        force_condition_printing(i % 8);
        
        /* Also test with NaN values to trigger UNORDERED cases */
        if (i % 10 == 0) {
            double nan_val = 0.0 / 0.0;
            total += test_unordered(darray[idx], nan_val);
            total += test_ordered(darray[idx], nan_val);
        }
    }
    
    /* Mix with regular C comparisons to provide context */
    for (i = 0; i < iterations; i++) {
        double a = darray[i % 8];
        double b = darray[(i + 1) % 8];
        
        /* These may generate condition codes that need to be printed */
        if (a != b) total++;
        if (a >= b) total++;
        if (a <= b) total++;
        if (a > b) total++;
        if (a < b) total++;
        if (!(a == b)) total++;
    }
    
    printf("Total: %d\n", total);
    
    /* Try to trigger the default case in the switch from uncovered lines */
    /* by using an out-of-range condition code */
    if (argc > 2 && strcmp(argv[2], "test-default") == 0) {
        double a = 1.0, b = 2.0;
        int result;
        
        /* This might trigger output_operand_lossage with invalid condition code */
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(result)
            : "x"(a), "x"(b), "u"(15)  /* Invalid condition code */
            : "cc"
        );
        
        printf("Result with invalid cc: %d\n", result);
    }
    
    return total != 0 ? 0 : 1;
}
