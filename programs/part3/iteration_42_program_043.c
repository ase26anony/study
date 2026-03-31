/* test_condition_codes.c - Target uncovered lines in i386.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent constant folding */
static volatile double vd1 = 1.0;
static volatile double vd2 = 2.0;
static volatile long double vld1 = 3.0L;
static volatile long double vld2 = 4.0L;
static volatile int selector = 0;

/* Condition code constants matching i386.h */
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* Function to test UNORDERED condition */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* Use x87 floating compare with unordered condition */
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

/* Function to test ORDERED condition */
__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    /* SSE2 compare with ordered condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

/* Function to test UNEQ condition */
__attribute__((noinline))
static int test_uneq(long double a, long double b) {
    int result;
    /* x87 compare with UNEQ condition */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNGE condition */
__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    /* Mixed x87/SSE with UNGE (nlt) condition */
    asm volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "comisd %%xmm1, %%xmm0\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGE)
        : "xmm0", "xmm1", "cc"
    );
    return result;
}

/* Function to test UNGT condition */
__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    /* x87 with UNGT (nle) condition */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

/* Function to test UNLE condition */
__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    /* SSE with UNLE condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLE)
        : "cc"
    );
    return result;
}

/* Function to test UNLT condition */
__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    /* x87 with UNLT condition */
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

/* Function to test LTGT condition */
__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    /* SSE with LTGT (une) condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(LTGT)
        : "cc"
    );
    return result;
}

/* Helper function that uses condition code from parameter */
__attribute__((noinline))
static int test_cond_code(int cc, double a, double b) {
    int result;
    
    /* Switch to potentially trigger default case in output logic */
    switch (cc) {
        case UNORDERED:
        case ORDERED:
        case UNEQ:
        case UNGE:
        case UNGT:
        case UNLE:
        case UNLT:
        case LTGT:
            /* Valid condition codes - use in assembly */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c3 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cc)
                : "cc"
            );
            break;
        default:
            /* This might trigger output_operand_lossage if cc is out of range */
            result = -1;
            break;
    }
    return result;
}

/* Function with complex control flow to obscure optimization */
__attribute__((noinline))
static int complex_condition_test(double *arr, int n, volatile int *sel) {
    int total = 0;
    long double ld_total = 0.0L;
    
    for (int i = 0; i < n - 1; i++) {
        /* Mix different condition code tests */
        total += test_unordered(arr[i], arr[i + 1]);
        total += test_ordered(arr[i], arr[i + 1]);
        
        /* Convert to long double for x87 tests */
        long double ld1 = arr[i];
        long double ld2 = arr[i + 1];
        total += test_uneq(ld1, ld2);
        total += test_unge(arr[i], arr[i + 1]);
        total += test_ungt(ld1, ld2);
        
        /* Use selector to choose condition code */
        int cc = (*sel + i) % 8;
        total += test_cond_code(cc, arr[i], arr[i + 1]);
        
        /* Regular C comparisons for context */
        if (arr[i] != arr[i + 1]) {
            total += test_unle(arr[i], arr[i + 1]);
        }
        if (arr[i] >= arr[i + 1]) {
            total += test_unlt(ld1, ld2);
        }
        
        /* LTGT test */
        total += test_ltgt(arr[i], arr[i + 1]);
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    int loop_count = 100;
    volatile int accumulator = 0;
    
    /* Parse loop count from command line */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Create array with volatile initialization */
    double *arr = malloc(loop_count * sizeof(double));
    if (!arr) return 1;
    
    /* Initialize with pattern that includes NaN and infinities */
    for (int i = 0; i < loop_count; i++) {
        volatile double init = (double)i;
        arr[i] = init;
        
        /* Create some special values */
        if (i % 7 == 0) arr[i] = 0.0 / 0.0;  /* NaN */
        if (i % 11 == 0) arr[i] = 1.0 / 0.0; /* Inf */
        if (i % 13 == 0) arr[i] = -1.0 / 0.0; /* -Inf */
    }
    
    /* Main test loop */
    for (int iter = 0; iter < 10; iter++) {
        /* Update selector to vary condition codes */
        selector = (selector + iter) % 10;  /* Goes beyond valid range sometimes */
        
        /* Call complex test function */
        accumulator += complex_condition_test(arr, loop_count, &selector);
        
        /* Direct calls to individual test functions */
        accumulator += test_unordered(vd1, vd2);
        accumulator += test_ordered(vd1, vd2);
        
        long double ld1 = vld1 + iter;
        long double ld2 = vld2 + iter;
        accumulator += test_uneq(ld1, ld2);
        accumulator += test_unge(vd1 + iter, vd2 + iter);
        accumulator += test_ungt(ld1, ld2);
        accumulator += test_unle(vd1 + iter, vd2 + iter);
        accumulator += test_unlt(ld1, ld2);
        accumulator += test_ltgt(vd1 + iter, vd2 + iter);
        
        /* Test with potentially invalid condition code */
        if (selector >= 8) {
            /* This may trigger the default case in output logic */
            accumulator += test_cond_code(selector, vd1, vd2);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated result: %d\n", accumulator);
    
    /* Additional assembly output test */
    {
        int r1, r2, r3, r4, r5, r6, r7, r8;
        
        /* Generate assembly for all condition codes in one block */
        asm volatile (
            "comisd %9, %8\n\t"
            "set%c0 %0\n\t"
            "set%c1 %1\n\t"
            "set%c2 %2\n\t"
            "set%c3 %3\n\t"
            "set%c4 %4\n\t"
            "set%c5 %5\n\t"
            "set%c6 %6\n\t"
            "set%c7 %7"
            : "=r"(r1), "=r"(r2), "=r"(r3), "=r"(r4),
              "=r"(r5), "=r"(r6), "=r"(r7), "=r"(r8)
            : "x"(vd1), "x"(vd2),
              "u"(UNORDERED), "u"(ORDERED), "u"(UNEQ),
              "u"(UNGE), "u"(UNGT), "u"(UNLE),
              "u"(UNLT), "u"(LTGT)
            : "cc"
        );
        
        printf("Multi-condition results: %d %d %d %d %d %d %d %d\n",
               r1, r2, r3, r4, r5, r6, r7, r8);
    }
    
    free(arr);
    return 0;
}
