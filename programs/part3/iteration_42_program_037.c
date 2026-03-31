/* 
 * Program to trigger x86 condition code printing logic in GCC's i386 backend.
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -S -o output.s this_file.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

/* Force noinline to preserve assembly patterns */
#define NOINLINE __attribute__((noinline))

/* Test UNORDERED condition code */
NOINLINE static int test_unordered(double a, double b)
{
    int result;
    /* Use %c modifier to output condition code name */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNORDERED)
                  : "cc", "st");
    return result;
}

/* Test ORDERED condition code */
NOINLINE static int test_ordered(double a, double b)
{
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(ORDERED)
                  : "cc", "st");
    return result;
}

/* Test UNEQ condition code */
NOINLINE static int test_uneq(double a, double b)
{
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNEQ)
                  : "cc", "st");
    return result;
}

/* Test UNGE condition code */
NOINLINE static int test_unge(double a, double b)
{
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGE)
                  : "cc", "st");
    return result;
}

/* Test UNGT condition code */
NOINLINE static int test_ungt(double a, double b)
{
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNGT)
                  : "cc", "st");
    return result;
}

/* Test UNLE condition code */
NOINLINE static int test_unle(double a, double b)
{
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLE)
                  : "cc", "st");
    return result;
}

/* Test UNLT condition code */
NOINLINE static int test_unlt(double a, double b)
{
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(UNLT)
                  : "cc", "st");
    return result;
}

/* Test LTGT condition code */
NOINLINE static int test_ltgt(double a, double b)
{
    int result;
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result)
                  : "u"(LTGT)
                  : "cc", "st");
    return result;
}

/* Mixed x87 and SSE operations */
NOINLINE static int test_mixed_operations(long double ld, double d)
{
    int result1, result2;
    
    /* x87 operation */
    asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                  : "=r"(result1)
                  : "u"(UNORDERED)
                  : "cc", "st");
    
    /* SSE operation - using comisd */
    asm volatile ("comisd %1, %0; set%c2 %3"
                  : "+x"(d)
                  : "x"(g_d1), "u"(ORDERED), "=r"(result2)
                  : "cc");
    
    return result1 + result2;
}

/* Function that uses a switch to select condition codes */
NOINLINE static int select_condition_code(int cc, double a, double b)
{
    int result = 0;
    
    switch (cc) {
        case UNORDERED:
            asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                          : "=r"(result)
                          : "u"(UNORDERED)
                          : "cc", "st");
            break;
        case ORDERED:
            asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                          : "=r"(result)
                          : "u"(ORDERED)
                          : "cc", "st");
            break;
        case UNEQ:
            asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                          : "=r"(result)
                          : "u"(UNEQ)
                          : "cc", "st");
            break;
        case UNGE:
            asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                          : "=r"(result)
                          : "u"(UNGE)
                          : "cc", "st");
            break;
        case UNGT:
            asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                          : "=r"(result)
                          : "u"(UNGT)
                          : "cc", "st");
            break;
        case UNLE:
            asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                          : "=r"(result)
                          : "u"(UNLE)
                          : "cc", "st");
            break;
        case UNLT:
            asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                          : "=r"(result)
                          : "u"(UNLT)
                          : "cc", "st");
            break;
        case LTGT:
            asm volatile ("fucomip %%st(1), %%st(0); set%c0 %0"
                          : "=r"(result)
                          : "u"(LTGT)
                          : "cc", "st");
            break;
        default:
            /* This might trigger output_operand_lossage if compiler
               tries to output an invalid condition code */
            asm volatile ("# Invalid condition code %c0" : : "u"(cc));
            result = -1;
    }
    
    return result;
}

/* Helper to generate NaN values */
NOINLINE static double generate_nan(void)
{
    volatile uint64_t nan_bits = 0x7FF8000000000000ULL;
    return *(double*)&nan_bits;
}

int main(int argc, char *argv[])
{
    volatile int sum = 0;
    int i, iterations;
    
    /* Parse iterations from command line or use default */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    } else {
        iterations = 100;
    }
    
    /* Create some special floating-point values */
    double nan_val = generate_nan();
    double inf_val = 1.0 / 0.0;  /* Will generate infinity */
    double neg_inf_val = -1.0 / 0.0;
    
    /* Array of test values */
    double test_values[] = {1.0, 2.0, 0.0, -0.0, nan_val, inf_val, neg_inf_val};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    printf("Testing x86 condition codes for %d iterations\n", iterations);
    
    for (i = 0; i < iterations; i++) {
        /* Use volatile index to prevent optimization */
        volatile int idx = i % num_values;
        volatile int idx2 = (i * 7) % num_values;
        
        double a = test_values[idx];
        double b = test_values[idx2];
        long double ld_a = (long double)a;
        long double ld_b = (long double)b;
        
        /* Test all condition codes */
        sum += test_unordered(a, b);
        sum += test_ordered(a, b);
        sum += test_uneq(a, b);
        sum += test_unge(a, b);
        sum += test_ungt(a, b);
        sum += test_unle(a, b);
        sum += test_unlt(a, b);
        sum += test_ltgt(a, b);
        
        /* Test mixed operations */
        sum += test_mixed_operations(ld_a, b);
        
        /* Test condition code selection via switch */
        int cc_select = i % 9;  /* 8 valid codes + 1 potentially invalid */
        sum += select_condition_code(cc_select, a, b);
        
        /* Mix with regular C comparisons to provide context */
        if (a != b) sum += 1;
        if (a >= b) sum += 2;
        if (!(a < b)) sum += 3;
    }
    
    /* Also test with long doubles specifically */
    for (i = 0; i < iterations / 10; i++) {
        sum += test_unordered(g_ld1, g_ld2);
        sum += test_ordered(g_ld1, g_ld2);
    }
    
    /* Force use of different floating-point units */
    {
        double d1 = g_d1;
        double d2 = g_d2;
        long double ld1 = g_ld1;
        long double ld2 = g_ld2;
        
        /* SSE comparison */
        asm volatile ("comisd %1, %0" : "+x"(d1) : "x"(d2) : "cc");
        
        /* x87 comparison */
        asm volatile ("fucomip %%st(1), %%st(0)" : : "u"(UNORDERED) : "cc", "st");
    }
    
    printf("Final sum: %d\n", sum);
    
    /* Try to trigger assembly output with different optimization levels */
    if (sum > 1000) {
        /* This printf format might cause the compiler to generate
           assembly output patterns */
        __builtin_printf("Condition code testing complete. Sum: %d\n", sum);
    }
    
    return sum == 0 ? 0 : 1;
}
