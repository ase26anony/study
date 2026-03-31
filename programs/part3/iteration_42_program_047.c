/* test_i386_condcodes.c
 * Program to trigger x86 condition code printing logic in i386.cc
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -S test_i386_condcodes.c
 */

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
#define COND_MAX  8

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
static void use_cond_code(int cc, double a, double b) __attribute__((noinline));

/* Test functions for each condition code */
static int test_unordered(double a, double b) {
    int result;
    /* Use UNORDERED condition code */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result)
        : "r"(UNORDERED), "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_ordered(double a, double b) {
    int result;
    /* Use ORDERED condition code */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result)
        : "r"(ORDERED), "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_uneq(double a, double b) {
    int result;
    /* Use UNEQ condition code */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result)
        : "r"(UNEQ), "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unge(double a, double b) {
    int result;
    /* Use UNGE condition code - prints as "nlt" */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result)
        : "r"(UNGE), "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_ungt(double a, double b) {
    int result;
    /* Use UNGT condition code - prints as "nle" */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result)
        : "r"(UNGT), "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unle(double a, double b) {
    int result;
    /* Use UNLE condition code - prints as "ule" */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result)
        : "r"(UNLE), "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unlt(double a, double b) {
    int result;
    /* Use UNLT condition code - prints as "ult" */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result)
        : "r"(UNLT), "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

static int test_ltgt(double a, double b) {
    int result;
    /* Use LTGT condition code - prints as "une" */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result)
        : "r"(LTGT), "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Mixed x87 and SSE operations */
static int test_mixed(long double a, long double b) {
    int result1, result2;
    
    /* x87 operation with UNORDERED */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(result1)
        : "r"(UNORDERED), "t"(a), "u"(b)
        : "cc", "st"
    );
    
    /* Convert to double for SSE operation with ORDERED */
    double da = (double)a;
    double db = (double)b;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"(da), "x"(db), "r"(ORDERED)
        : "cc"
    );
    
    return result1 & result2;
}

/* Function that uses condition code from parameter - may trigger printing */
static void use_cond_code(int cc, double a, double b) {
    int result;
    
    /* Switch to make cc non-constant in RTL */
    switch (cc) {
        case UNORDERED:
        case ORDERED:
        case UNEQ:
        case UNGE:
        case UNGT:
        case UNLE:
        case UNLT:
        case LTGT:
            asm volatile (
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(result)
                : "r"(cc), "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
        default:
            /* This might trigger output_operand_lossage for invalid code */
            asm volatile (
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %1"
                : "=u"(result)
                : "r"(cc), "t"(a), "u"(b)
                : "cc", "st"
            );
            break;
    }
    
    /* Use result to prevent optimization */
    if (result) {
        vd1 += 0.001;
    }
}

int main(int argc, char *argv[]) {
    int i, iterations;
    volatile int sum = 0;
    
    /* Get iterations from command line or use default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Initialize arrays with volatile values */
    double darray[8];
    long double ldarray[8];
    
    for (i = 0; i < 8; i++) {
        darray[i] = vd1 + i * 0.5;
        ldarray[i] = vld1 + i * 0.5L;
    }
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Test all condition codes */
        sum += test_unordered(darray[idx], darray[(idx + 1) % 8]);
        sum += test_ordered(darray[(idx + 1) % 8], darray[(idx + 2) % 8]);
        sum += test_uneq(darray[(idx + 2) % 8], darray[(idx + 3) % 8]);
        sum += test_unge(darray[(idx + 3) % 8], darray[(idx + 4) % 8]);
        sum += test_ungt(darray[(idx + 4) % 8], darray[(idx + 5) % 8]);
        sum += test_unle(darray[(idx + 5) % 8], darray[(idx + 6) % 8]);
        sum += test_unlt(darray[(idx + 6) % 8], darray[(idx + 7) % 8]);
        sum += test_ltgt(darray[(idx + 7) % 8], darray[idx]);
        
        /* Test mixed x87/SSE */
        sum += test_mixed(ldarray[idx], ldarray[(idx + 1) % 8]);
        
        /* Use volatile selector to choose condition code */
        selector = (selector + 1) % (COND_MAX + 2); /* +2 to potentially go out of bounds */
        
        /* This may trigger the default case if selector >= COND_MAX */
        use_cond_code(selector, darray[idx], darray[(idx + 1) % 8]);
        
        /* Regular C comparisons to provide context */
        if (darray[idx] != darray[(idx + 1) % 8]) {
            sum++;
        }
        if (darray[idx] >= darray[(idx + 2) % 8]) {
            sum += 2;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final sum: %d\n", sum);
    
    /* Additional test with NaN to trigger UNORDERED cases */
    double nan_val = 0.0 / 0.0;
    sum += test_unordered(nan_val, 1.0);
    sum += test_ordered(1.0, nan_val);
    
    printf("After NaN tests: %d\n", sum);
    
    return 0;
}
