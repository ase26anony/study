/* test_i386_condcodes.c - Target coverage of i386.cc condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to prevent constant folding */
static volatile double g_dbl1 = 1.0;
static volatile double g_dbl2 = 2.0;
static volatile long double g_ldbl1 = 3.0L;
static volatile long double g_ldbl2 = 4.0L;
static volatile int g_selector = 0;

/* Condition code constants matching i386.h */
#define UNORDERED 0
#define ORDERED   1
#define UNEQ      2
#define UNGE      3
#define UNGT      4
#define UNLE      5
#define UNLT      6
#define LTGT      7

/* Function 1: Test UNORDERED condition with x87 */
__attribute__((noinline))
static int test_unordered_x87(long double a, long double b) {
    int result;
    /* Use x87 fucomip instruction with unordered condition */
    asm volatile (
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %1"
        : "=u"(UNORDERED), "=r"(result)
        : "t"(a), "u"(b)
        : "cc", "st"
    );
    return result;
}

/* Function 2: Test ORDERED condition with SSE */
__attribute__((noinline))
static int test_ordered_sse(double a, double b) {
    int result;
    /* Use SSE comisd instruction with ordered condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result), "=x"(a)
        : "x"(b), "0"(ORDERED)
        : "cc"
    );
    return result;
}

/* Function 3: Test UNEQ condition with mixed operations */
__attribute__((noinline))
static int test_uneq_mixed(double a, long double b) {
    int result1, result2;
    /* First do x87 comparison */
    asm volatile (
        "fldt %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result1)
        : "m"(a), "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    
    /* Then SSE comparison for same values */
    double b_dbl = (double)b;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"(a), "x"(b_dbl), "u"(UNEQ)
        : "cc"
    );
    
    return result1 | result2;
}

/* Function 4: Test UNGE condition (nlt) */
__attribute__((noinline))
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

/* Function 5: Test UNGT condition (nle) */
__attribute__((noinline))
static int test_ungt(long double a, long double b) {
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNGT)
        : "cc", "st"
    );
    return result;
}

/* Function 6: Test UNLE condition */
__attribute__((noinline))
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

/* Function 7: Test UNLT condition */
__attribute__((noinline))
static int test_unlt(long double a, long double b) {
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(UNLT)
        : "cc", "st"
    );
    return result;
}

/* Function 8: Test LTGT condition (une) */
__attribute__((noinline))
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

/* Function 9: Complex function that uses switch to select condition code */
__attribute__((noinline))
static int test_conditional_cc(int cc_selector, double a, double b) {
    int result = 0;
    
    /* Volatile to prevent optimization */
    volatile int local_selector = cc_selector;
    
    switch (local_selector & 0x7) {
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
            /* This should trigger output_operand_lossage for invalid code */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(local_selector)  /* Potentially invalid */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* Function 10: Generate NaN values for unordered comparisons */
__attribute__((noinline))
static double generate_nan(void) {
    volatile uint64_t nan_bits = 0x7FF8000000000000ULL; /* Quiet NaN */
    return *(double*)&nan_bits;
}

/* Function 11: Mixed x87 and SSE with runtime condition selection */
__attribute__((noinline))
static int test_mixed_fpu(int mode, double d1, double d2, 
                          long double ld1, long double ld2) {
    int result = 0;
    
    if (mode & 1) {
        /* Use x87 */
        asm volatile (
            "fldt %2\n\t"
            "fldt %1\n\t"
            "fucomip %%st(1), %%st(0)\n\t"
            "set%c0 %0"
            : "=r"(result)
            : "m"(ld1), "m"(ld2), "u"(UNORDERED)
            : "cc", "st"
        );
    } else {
        /* Use SSE */
        asm volatile (
            "comisd %2, %1\n\t"
            "set%c0 %0"
            : "=r"(result)
            : "x"(d1), "x"(d2), "u"(ORDERED)
            : "cc"
        );
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int accumulator = 0;
    int iterations = 100;
    
    /* Parse iteration count from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create some special floating-point values */
    double nan_val = generate_nan();
    double inf_val = 1.0 / 0.0;  /* Will generate inf */
    double neg_inf_val = -1.0 / 0.0;
    double zero_val = 0.0;
    double neg_zero_val = -0.0;
    
    /* Array of test values */
    double test_doubles[] = {1.0, 2.0, nan_val, inf_val, zero_val};
    long double test_ldoubles[] = {1.0L, 2.0L, nan_val, inf_val, zero_val};
    
    printf("Testing i386 condition code printing...\n");
    
    for (int i = 0; i < iterations; i++) {
        /* Update volatile globals */
        g_selector = i;
        g_dbl1 = test_doubles[i % 5];
        g_dbl2 = test_doubles[(i + 1) % 5];
        g_ldbl1 = test_ldoubles[i % 5];
        g_ldbl2 = test_ldoubles[(i + 2) % 5];
        
        /* Call all test functions with different condition codes */
        accumulator += test_unordered_x87(g_ldbl1, g_ldbl2);
        accumulator += test_ordered_sse(g_dbl1, g_dbl2);
        accumulator += test_uneq_mixed(g_dbl1, g_ldbl2);
        accumulator += test_unge(g_dbl1, g_dbl2);
        accumulator += test_ungt(g_ldbl1, g_ldbl2);
        accumulator += test_unle(g_dbl1, g_dbl2);
        accumulator += test_unlt(g_ldbl1, g_ldbl2);
        accumulator += test_ltgt(g_dbl1, g_dbl2);
        
        /* Test with switch-based condition selection */
        accumulator += test_conditional_cc(i & 0x7, g_dbl1, g_dbl2);
        
        /* Test with potentially invalid condition code (for default case) */
        if ((i % 13) == 0) {
            accumulator += test_conditional_cc(8, g_dbl1, g_dbl2); /* Invalid code */
        }
        
        /* Test mixed FPU usage */
        accumulator += test_mixed_fpu(i & 1, g_dbl1, g_dbl2, g_ldbl1, g_ldbl2);
        
        /* Regular C comparisons to provide context */
        if (g_dbl1 != g_dbl2) accumulator++;
        if (g_dbl1 >= g_dbl2) accumulator++;
        if (!(g_dbl1 < g_dbl2)) accumulator++;
    }
    
    printf("Final accumulator value: %d\n", accumulator);
    
    /* Force use of all condition codes in printf format (indirect) */
    printf("Condition codes tested: %d %d %d %d %d %d %d %d\n",
           UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT);
    
    return accumulator != 0 ? 0 : 1;
}
