/* i386_condition_codes.c - Target coverage for x86 condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent constant folding */
volatile double vd1 = 1.0, vd2 = 2.0, vd3 = 0.0, vd4 = -1.0;
volatile long double vld1 = 3.14159L, vld2 = 2.71828L;
volatile int vi1 = 0, vi2 = 1, vi3 = 2;

/* Condition code enum matching i386.h */
enum cmp_code {
    UNORDERED = 16,
    ORDERED = 17,
    UNEQ = 18,
    UNGE = 19,
    UNGT = 20,
    UNLE = 21,
    UNLT = 22,
    LTGT = 23
};

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
static void use_condition_code(enum cmp_code cc, double a, double b) __attribute__((noinline));

/* Individual test functions for each condition code */
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
    /* Mixed x87/SSE approach */
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
    double d = (double)a;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"(d), "x"(b), "u"(ORDERED)
        : "cc"
    );
    
    return result1 | result2;
}

/* Function that uses condition code via switch - may trigger printing */
static void use_condition_code(enum cmp_code cc, double a, double b) {
    int result;
    
    switch (cc) {
        case UNORDERED:
        case ORDERED:
        case UNEQ:
        case UNGE:
        case UNGT:
        case UNLE:
        case UNLT:
        case LTGT:
            /* Use the condition code in assembly */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cc)
                : "cc"
            );
            break;
        default:
            /* This might trigger output_operand_lossage */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cc)
                : "cc"
            );
            break;
    }
    
    /* Use result to prevent optimization */
    vi1 += result;
}

int main(int argc, char *argv[]) {
    volatile int sum = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Array of test values */
    double test_values[] = {0.0, 1.0, -1.0, 2.0, -2.0, 0.5, -0.5};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Loop to prevent constant folding */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile indices to prevent optimization */
        int idx1 = vi1 % num_values;
        int idx2 = vi2 % num_values;
        int idx3 = vi3 % num_values;
        
        double a = test_values[idx1];
        double b = test_values[idx2];
        long double c = (long double)test_values[idx3];
        
        /* Call all test functions */
        sum += test_unordered(a, b);
        sum += test_ordered(a, b);
        sum += test_uneq(a, b);
        sum += test_unge(a, b);
        sum += test_ungt(a, b);
        sum += test_unle(a, b);
        sum += test_unlt(a, b);
        sum += test_ltgt(a, b);
        sum += test_mixed_x87_sse(c, b);
        
        /* Also use regular C comparisons for context */
        if (a != b) sum++;
        if (a >= b) sum++;
        
        /* Use switch with volatile control to potentially trigger default case */
        enum cmp_code cc = (i % 9); /* 8 valid codes + 1 potentially invalid */
        use_condition_code(cc, a, b);
        
        /* Modify volatile variables */
        vi1 = (vi1 * 1103515245 + 12345) & 0x7fffffff;
        vi2 = (vi2 * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated sum: %d\n", sum);
    
    /* Additional test with NaN to trigger UNORDERED cases */
    double nan_value = 0.0 / 0.0;
    sum += test_unordered(nan_value, 1.0);
    sum += test_ordered(nan_value, 1.0);
    
    printf("Final sum with NaN tests: %d\n", sum);
    
    return 0;
}
