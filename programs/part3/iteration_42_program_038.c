/* test_i386_condcodes.c - Target the uncovered condition code printing logic in i386.cc */
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

/* ========== Individual condition code test functions ========== */

/* Test UNORDERED condition code with x87 long double */
static int __attribute__((noinline)) test_unordered_ld(long double a, long double b)
{
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

/* Test ORDERED condition code with SSE double */
static int __attribute__((noinline)) test_ordered_sse(double a, double b)
{
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

/* Test UNEQ condition code with mixed operations */
static int __attribute__((noinline)) test_uneq_mixed(double a, long double b)
{
    int result1, result2;
    /* First do x87 comparison */
    asm volatile (
        "fldt %2\n\t"
        "fldl %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result1), "=t"(a)
        : "m"(b), "u"(UNEQ)
        : "cc", "st"
    );
    
    /* Then SSE comparison for same condition */
    double a_dbl = (double)a;
    double b_dbl = (double)b;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result2)
        : "x"(a_dbl), "x"(b_dbl), "u"(UNEQ)
        : "cc"
    );
    
    return result1 | result2;
}

/* Test UNGE condition code (nlt) */
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

/* Test UNGT condition code (nle) */
static int __attribute__((noinline)) test_ungt(long double a, long double b)
{
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "t"(a), "u"(b), "0"(UNGT)
        : "cc", "st"
    );
    return result;
}

/* Test UNLE condition code */
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

/* Test UNLT condition code */
static int __attribute__((noinline)) test_unlt(long double a, long double b)
{
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "t"(a), "u"(b), "0"(UNLT)
        : "cc", "st"
    );
    return result;
}

/* Test LTGT condition code (une) */
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

/* ========== Function that uses switch to select condition code ========== */

/* This function uses a switch to select condition codes, potentially triggering
   the output logic for different codes */
static int __attribute__((noinline)) test_condcode_switch(int cc, double a, double b)
{
    int result = 0;
    
    switch (cc & 0x7) {  /* Mask to 0-7 range */
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
            /* This might trigger output_operand_lossage if cc is out of range,
               but our masking should prevent this */
            result = -1;
            break;
    }
    
    return result;
}

/* ========== Function with potentially invalid condition code ========== */

/* This function might trigger the default case in the uncovered code
   if the compiler doesn't validate the condition code properly */
static void __attribute__((noinline)) test_potential_invalid_cc(void)
{
    /* Try to create a scenario where an invalid condition code might be used */
    volatile int invalid_cc = 8;  /* Value outside 0-7 range */
    double a = g_d1;
    double b = g_d2;
    int result;
    
    /* This inline asm uses a variable condition code - if the compiler
       doesn't validate it properly during RTL output, it might hit the
       default case */
    asm volatile (
        "# Potential invalid condition code test\n\t"
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(invalid_cc)
        : "cc"
    );
    
    /* Use result to prevent dead code elimination */
    g_selector = result;
}

/* ========== Main test driver ========== */

int main(int argc, char *argv[])
{
    volatile int total = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create arrays of test values */
    double d_vals[8];
    long double ld_vals[8];
    
    /* Initialize with volatile values to prevent constant folding */
    for (int i = 0; i < 8; i++) {
        d_vals[i] = g_d1 * i + g_d2;
        ld_vals[i] = g_ld1 * i + g_ld2;
    }
    
    /* Also include some special floating-point values */
    double special_doubles[] = {0.0, -0.0, INFINITY, -INFINITY, NAN};
    long double special_long_doubles[] = {0.0L, -0.0L, INFINITY, -INFINITY, NAN};
    
    printf("Testing x86 condition code printing logic...\n");
    
    /* Main test loop */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile index to prevent loop unrolling from eliminating
           condition code variability */
        volatile int idx = i % 8;
        
        /* Test each condition code function */
        total += test_unordered_ld(ld_vals[idx], ld_vals[(idx + 1) % 8]);
        total += test_ordered_sse(d_vals[idx], d_vals[(idx + 2) % 8]);
        total += test_uneq_mixed(d_vals[idx], ld_vals[(idx + 3) % 8]);
        total += test_unge(d_vals[idx], d_vals[(idx + 4) % 8]);
        total += test_ungt(ld_vals[idx], ld_vals[(idx + 5) % 8]);
        total += test_unle(d_vals[idx], d_vals[(idx + 6) % 8]);
        total += test_unlt(ld_vals[idx], ld_vals[(idx + 7) % 8]);
        total += test_ltgt(d_vals[idx], d_vals[(idx + 1) % 8]);
        
        /* Test the switch-based function */
        total += test_condcode_switch(idx, d_vals[idx], d_vals[(idx + 2) % 8]);
        
        /* Occasionally test with special values */
        if (i % 13 == 0) {
            int spec_idx = i % 5;
            total += test_unordered_ld(special_long_doubles[spec_idx], 
                                      ld_vals[idx]);
            total += test_ordered_sse(special_doubles[spec_idx], 
                                     d_vals[idx]);
        }
        
        /* Update volatile selector to prevent optimization */
        g_selector = idx;
    }
    
    /* Test the potentially invalid condition code function */
    test_potential_invalid_cc();
    
    /* Mix in some regular C comparisons to provide context */
    for (int i = 0; i < iterations / 10; i++) {
        volatile int idx = i % 8;
        double a = d_vals[idx];
        double b = d_vals[(idx + 1) % 8];
        long double la = ld_vals[idx];
        long double lb = ld_vals[(idx + 1) % 8];
        
        /* Regular C comparisons - these might get lowered to the
           condition codes we're testing */
        if (a != b) total++;
        if (la >= lb) total--;
        if (!(a < b)) total += 2;
        if (isunordered(a, b)) total -= 1;
    }
    
    printf("Final total: %d\n", total);
    
    /* Return non-zero if total is odd (just to have variable output) */
    return total & 1;
}
