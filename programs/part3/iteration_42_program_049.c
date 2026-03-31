/* test_i386_condcodes.c - Target specific coverage for i386.cc condition code printing */
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
#define ORDERED 1
#define UNEQ 2
#define UNGE 3
#define UNGT 4
#define UNLE 5
#define UNLT 6
#define LTGT 7

/* Function prototypes */
static int test_unordered(double a, double b) __attribute__((noinline));
static int test_ordered(double a, double b) __attribute__((noinline));
static int test_uneq(double a, double b) __attribute__((noinline));
static int test_unge(double a, double b) __attribute__((noinline));
static int test_ungt(double a, double b) __attribute__((noinline));
static int test_unle(double a, double b) __attribute__((noinline));
static int test_unlt(double a, double b) __attribute__((noinline));
static int test_ltgt(double a, double b) __attribute__((noinline));
static void use_cond_code_in_switch(int cc) __attribute__((noinline));
static int helper_with_cond_code(int cc, double a, double b) __attribute__((noinline));

/* Test functions for each condition code using x87 instructions */
static int test_unordered(double a, double b) {
    int result;
    /* Using x87 fucomip with UNORDERED condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNORDERED), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

static int test_ordered(double a, double b) {
    int result;
    /* Using x87 fucomip with ORDERED condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(ORDERED), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

static int test_uneq(double a, double b) {
    int result;
    /* Using x87 fucomip with UNEQ condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNEQ), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unge(double a, double b) {
    int result;
    /* Using x87 fucomip with UNGE condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNGE), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

static int test_ungt(double a, double b) {
    int result;
    /* Using x87 fucomip with UNGT condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNGT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unle(double a, double b) {
    int result;
    /* Using x87 fucomip with UNLE condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNLE), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

static int test_unlt(double a, double b) {
    int result;
    /* Using x87 fucomip with UNLT condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNLT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

static int test_ltgt(double a, double b) {
    int result;
    /* Using x87 fucomip with LTGT condition */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(LTGT), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Mixed x87 and SSE operations */
static int test_mixed_operations(long double ld1, long double ld2, double d1, double d2) {
    int result = 0;
    
    /* x87 operations with long double */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNORDERED), "m"(ld1), "m"(ld2)
        : "cc", "st"
    );
    
    /* SSE operations with double */
    int sse_result;
    asm volatile (
        "comisd %2, %3\n\t"
        "set%c1 %0"
        : "=r"(sse_result)
        : "u"(ORDERED), "x"(d1), "x"(d2)
        : "cc"
    );
    
    return result + sse_result;
}

/* Helper function that uses condition code parameter */
static int helper_with_cond_code(int cc, double a, double b) {
    int result;
    /* Dynamic condition code usage */
    asm volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c1 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(cc), "m"(a), "m"(b)
        : "cc", "st"
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
        case ORDERED:
        case UNEQ:
        case UNGE:
        case UNGT:
        case UNLE:
        case UNLT:
        case LTGT:
            result = helper_with_cond_code(cc, a, b);
            /* Use result to prevent optimization */
            asm volatile ("" : : "r"(result));
            break;
        default:
            /* This might trigger output_operand_lossage if cc is invalid */
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "u"(cc), "m"(a), "m"(b)
                : "cc", "st"
            );
            break;
    }
}

int main(int argc, char *argv[]) {
    volatile int sum = 0;
    int i, iterations;
    
    /* Parse iterations from command line or use default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Initialize volatile floating-point arrays */
    volatile double d_array[8];
    volatile long double ld_array[8];
    
    for (i = 0; i < 8; i++) {
        d_array[i] = (double)i * 1.5;
        ld_array[i] = (long double)i * 2.5L;
    }
    
    /* Main test loop */
    for (i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Test all condition codes */
        sum += test_unordered(d_array[idx], d_array[(idx + 1) % 8]);
        sum += test_ordered(d_array[(idx + 1) % 8], d_array[(idx + 2) % 8]);
        sum += test_uneq(d_array[(idx + 2) % 8], d_array[(idx + 3) % 8]);
        sum += test_unge(d_array[(idx + 3) % 8], d_array[(idx + 4) % 8]);
        sum += test_ungt(d_array[(idx + 4) % 8], d_array[(idx + 5) % 8]);
        sum += test_unle(d_array[(idx + 5) % 8], d_array[(idx + 6) % 8]);
        sum += test_unlt(d_array[(idx + 6) % 8], d_array[(idx + 7) % 8]);
        sum += test_ltgt(d_array[(idx + 7) % 8], d_array[idx]);
        
        /* Test mixed operations */
        sum += test_mixed_operations(ld_array[idx], ld_array[(idx + 1) % 8],
                                    d_array[idx], d_array[(idx + 1) % 8]);
        
        /* Use switch with potentially out-of-range values */
        g_selector = (i * 17) % 12;  /* Could be 8-11, which are invalid */
        use_cond_code_in_switch(g_selector);
        
        /* Regular C comparisons to provide context */
        if (d_array[idx] != d_array[(idx + 1) % 8]) {
            sum += 1;
        }
        if (d_array[idx] >= d_array[(idx + 2) % 8]) {
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
    
    return sum != 0 ? 0 : 1;
}
