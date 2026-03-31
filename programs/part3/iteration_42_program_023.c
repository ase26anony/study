/* test_condition_codes.c - Target coverage of i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile globals to prevent constant propagation */
volatile double g_dbl1 = 1.0;
volatile double g_dbl2 = 2.0;
volatile long double g_ldbl1 = 3.0L;
volatile long double g_ldbl2 = 4.0L;
volatile int g_selector = 0;

/* Condition code constants matching i386.h */
enum x86_cond_code {
    UNORDERED = 16,
    ORDERED = 17,
    UNEQ = 18,
    UNGE = 19,
    UNGT = 20,
    UNLE = 21,
    UNLT = 22,
    LTGT = 23
};

/* ========== Individual condition code test functions ========== */

/* Test UNORDERED condition with x87 */
static int __attribute__((noinline)) test_unordered_x87(long double a, long double b)
{
    int result;
    /* x87 floating compare with unordered check */
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNORDERED), "m"(a), "m"(b)
        : "cc", "st"
    );
    return result;
}

/* Test ORDERED condition with SSE */
static int __attribute__((noinline)) test_ordered_sse(double a, double b)
{
    int result;
    /* SSE2 compare with ordered check */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

/* Test UNEQ condition with mixed operations */
static int __attribute__((noinline)) test_uneq_mixed(double a, long double b)
{
    int result1, result2;
    /* First do x87 comparison */
    asm volatile (
        "fldt %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "u"(UNEQ), "m"(b), "m"(a)
        : "cc", "st"
    );
    
    /* Then SSE comparison for same condition */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"((double)b), "x"(a), "u"(UNEQ)
        : "cc"
    );
    
    return result1 & result2;
}

/* Test UNGE condition (nlt) */
static int __attribute__((noinline)) test_unge(double a, double b)
{
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

/* Test UNGT condition (nle) */
static int __attribute__((noinline)) test_ungt(long double a, long double b)
{
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNGT), "m"(b), "m"(a)
        : "cc", "st"
    );
    return result;
}

/* Test UNLE condition (ule) */
static int __attribute__((noinline)) test_unle(double a, double b)
{
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

/* Test UNLT condition (ult) */
static int __attribute__((noinline)) test_unlt(long double a, long double b)
{
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %3\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "u"(UNLT), "m"(b), "m"(a)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition (une) */
static int __attribute__((noinline)) test_ltgt(double a, double b)
{
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

/* ========== Helper function with switch to force code generation ========== */

/* This function uses a switch to select condition codes, potentially 
   triggering the default case in output logic */
static int __attribute__((noinline)) 
conditional_test(double a, double b, int cond_code)
{
    int result = 0;
    
    /* Use switch to create multiple paths to condition code usage */
    switch (cond_code & 0x7) {
        case 0:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNORDERED)
                : "cc"
            );
            break;
        case 1:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(ORDERED)
                : "cc"
            );
            break;
        case 2:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNEQ)
                : "cc"
            );
            break;
        case 3:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGE)
                : "cc"
            );
            break;
        case 4:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNGT)
                : "cc"
            );
            break;
        case 5:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLE)
                : "cc"
            );
            break;
        case 6:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(UNLT)
                : "cc"
            );
            break;
        case 7:
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(LTGT)
                : "cc"
            );
            break;
        default:
            /* This might trigger the default case in output_operand_lossage
               if the compiler can't statically determine the condition code */
            asm volatile (
                "comisd %2, %1\n\t"
                "set%c0 %0"
                : "=r"(result)
                : "x"(a), "x"(b), "u"(cond_code)  /* Dynamic condition code! */
                : "cc"
            );
            break;
    }
    
    return result;
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[])
{
    volatile int accumulator = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create volatile arrays to prevent optimization */
    volatile double dbl_array[8];
    volatile long double ldbl_array[8];
    
    /* Initialize with varying values */
    for (int i = 0; i < 8; i++) {
        dbl_array[i] = g_dbl1 + i * 0.5;
        ldbl_array[i] = g_ldbl1 + i * 0.3L;
    }
    
    printf("Testing x86 condition code printing logic...\n");
    printf("Iterations: %d\n", iterations);
    
    /* Main test loop with mixed operations */
    for (int i = 0; i < iterations; i++) {
        int idx = i & 7;  /* Use lower 3 bits for array index */
        
        /* Call all individual test functions */
        accumulator += test_unordered_x87(ldbl_array[idx], ldbl_array[(idx + 1) & 7]);
        accumulator += test_ordered_sse(dbl_array[idx], dbl_array[(idx + 2) & 7]);
        accumulator += test_uneq_mixed(dbl_array[idx], ldbl_array[(idx + 3) & 7]);
        accumulator += test_unge(dbl_array[(idx + 1) & 7], dbl_array[idx]);
        accumulator += test_ungt(ldbl_array[(idx + 2) & 7], ldbl_array[idx]);
        accumulator += test_unle(dbl_array[(idx + 3) & 7], dbl_array[idx]);
        accumulator += test_unlt(ldbl_array[(idx + 4) & 7], ldbl_array[idx]);
        accumulator += test_ltgt(dbl_array[(idx + 5) & 7], dbl_array[idx]);
        
        /* Use the switch-based function with varying condition codes */
        int cond_selector = (i + g_selector) & 0xF;  /* Could be 0-15 */
        accumulator += conditional_test(dbl_array[idx], dbl_array[(idx + 6) & 7], 
                                       cond_selector);
        
        /* Mix with regular C comparisons to provide context */
        if (dbl_array[idx] != dbl_array[(idx + 1) & 7]) {
            accumulator += 1;
        }
        
        if (ldbl_array[idx] >= ldbl_array[(idx + 2) & 7]) {
            accumulator += 2;
        }
        
        /* Update volatile selector to prevent loop unrolling */
        g_selector += (i & 1);
    }
    
    /* Also test with NaN values to trigger UNORDERED cases */
    double nan_val = 0.0 / 0.0;
    long double nan_ldbl = 0.0L / 0.0L;
    
    accumulator += test_unordered_x87(nan_ldbl, ldbl_array[0]);
    accumulator += test_ordered_sse(nan_val, dbl_array[0]);
    accumulator += test_uneq_mixed(nan_val, nan_ldbl);
    
    printf("Final accumulator value: %d\n", accumulator);
    
    /* Force assembly output of condition codes via inline asm with constraints */
    asm volatile (
        "# Force condition code output for coverage\n"
        "comisd %1, %0\n\t"
        "set%c2 %3"
        : 
        : "x"(g_dbl1), "x"(g_dbl2), "u"(UNORDERED), "m"(accumulator)
        : "cc"
    );
    
    return accumulator != 0 ? 0 : 1;
}
