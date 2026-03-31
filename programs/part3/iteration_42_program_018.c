/* test_condition_codes.c - Target coverage for i386.cc lines 13992-14017 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent constant folding */
static volatile double g_d1 = 1.0;
static volatile double g_d2 = 2.0;
static volatile long double g_ld1 = 3.0L;
static volatile long double g_ld2 = 4.0L;
static volatile int g_selector = 0;

/* Condition code constants matching i386.h definitions */
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* ========== Individual condition code test functions ========== */

/* Each function tests a specific condition code with x87 instructions */
__attribute__((noinline))
static int test_unordered(double a, double b) {
    int result;
    /* x87 unordered comparison */
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ordered(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(ORDERED)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_uneq(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unge(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGE)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ungt(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unle(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLE)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_unlt(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

__attribute__((noinline))
static int test_ltgt(double a, double b) {
    int result;
    asm volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(LTGT)
        : "cc", "st"
    );
    return result;
}

/* ========== Mixed x87/SSE functions ========== */

__attribute__((noinline))
static int test_mixed_x87_sse(long double a, double b) {
    int r1, r2;
    
    /* x87 comparison with long double */
    asm volatile (
        "fldt %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(r1)
        : "m"(a), "m"(b), "u"(UNORDERED)
        : "cc", "st"
    );
    
    /* SSE comparison with double */
    asm volatile (
        "comisd %1, %2\n\t"
        "set%c0 %0"
        : "=r"(r2)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    
    return r1 & r2;
}

/* ========== Function with switch to force condition code printing ========== */

__attribute__((noinline))
static int test_switch_based(int cc, double a, double b) {
    int result = 0;
    
    /* Switch to select condition code - prevents constant propagation */
    switch (cc & 7) {  /* Mask to 0-7 range */
        case UNORDERED:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNORDERED)
                : "cc", "st"
            );
            break;
            
        case ORDERED:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(ORDERED)
                : "cc", "st"
            );
            break;
            
        case UNEQ:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNEQ)
                : "cc", "st"
            );
            break;
            
        case UNGE:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNGE)
                : "cc", "st"
            );
            break;
            
        case UNGT:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNGT)
                : "cc", "st"
            );
            break;
            
        case UNLE:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNLE)
                : "cc", "st"
            );
            break;
            
        case UNLT:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(UNLT)
                : "cc", "st"
            );
            break;
            
        case LTGT:
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(LTGT)
                : "cc", "st"
            );
            break;
            
        default:
            /* This should trigger output_operand_lossage for invalid code */
            asm volatile (
                "fldl %1\n\t"
                "fldl %2\n\t"
                "fucomip %%st(1), %%st(0)\n\t"
                "set%c0 %0\n\t"
                "fstp %%st(0)"
                : "=r"(result)
                : "m"(a), "m"(b), "u"(cc)  /* Potentially invalid condition code */
                : "cc", "st"
            );
            break;
    }
    
    return result;
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int loop_count = 100;
    
    /* Parse loop count from command line if provided */
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    /* Initialize test arrays with volatile values */
    double d_array[8];
    long double ld_array[8];
    
    for (int i = 0; i < 8; i++) {
        d_array[i] = g_d1 + i * 0.5;
        ld_array[i] = g_ld1 + i * 0.5L;
    }
    
    /* Main test loop - prevents optimization */
    for (int iter = 0; iter < loop_count; iter++) {
        int idx = iter & 7;  /* 0-7 index */
        
        /* Test all individual condition code functions */
        accumulator += test_unordered(d_array[idx], d_array[(idx + 1) & 7]);
        accumulator += test_ordered(d_array[idx], d_array[(idx + 2) & 7]);
        accumulator += test_uneq(d_array[idx], d_array[(idx + 3) & 7]);
        accumulator += test_unge(d_array[idx], d_array[(idx + 4) & 7]);
        accumulator += test_ungt(d_array[idx], d_array[(idx + 5) & 7]);
        accumulator += test_unle(d_array[idx], d_array[(idx + 6) & 7]);
        accumulator += test_unlt(d_array[idx], d_array[(idx + 7) & 7]);
        accumulator += test_ltgt(d_array[idx], d_array[idx]);
        
        /* Test mixed x87/SSE */
        accumulator += test_mixed_x87_sse(ld_array[idx], d_array[idx]);
        
        /* Test switch-based with varying condition codes */
        int cc_select = (iter + g_selector) & 15;  /* 0-15, may exceed valid range */
        accumulator += test_switch_based(cc_select, d_array[idx], d_array[(idx + 1) & 7]);
        
        /* Mix with regular C comparisons to provide context */
        if (d_array[idx] != d_array[(idx + 1) & 7]) {
            accumulator++;
        }
        if (d_array[idx] >= d_array[(idx + 2) & 7]) {
            accumulator++;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Accumulated result: %d\n", accumulator);
    
    /* Force assembly output with condition codes */
    asm volatile (
        "# Force condition code output\n"
        :
        :
        : "memory"
    );
    
    return accumulator != 0 ? 0 : 1;
}
