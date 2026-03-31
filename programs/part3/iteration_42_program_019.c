/* test_i386_condcodes.c
 * Target: i386.cc lines 13992-14017
 * Compile with: gcc -O2 -mfpmath=387 -march=i686 -S test_i386_condcodes.c
 */

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

/* Function 1: Test UNORDERED condition with x87 */
static int __attribute__((noinline)) test_unordered(double a, double b)
{
    int result;
    /* Using x87 fucomip instruction */
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

/* Function 2: Test ORDERED condition with SSE */
static int __attribute__((noinline)) test_ordered(double a, double b)
{
    int result;
    /* Using SSE comisd instruction */
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(ORDERED)
        : "cc"
    );
    return result;
}

/* Function 3: Test UNEQ condition with mixed operations */
static int __attribute__((noinline)) test_uneq(long double a, long double b)
{
    int result;
    /* Using x87 with long double */
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

/* Function 4: Test UNGE condition */
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

/* Function 5: Test UNGT condition with x87 */
static int __attribute__((noinline)) test_ungt(long double a, long double b)
{
    int result;
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

/* Function 6: Test UNLE condition */
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

/* Function 7: Test UNLT condition */
static int __attribute__((noinline)) test_unlt(double a, double b)
{
    int result;
    asm volatile (
        "comisd %2, %1\n\t"
        "set%c0 %0"
        : "=r"(result)
        : "x"(a), "x"(b), "u"(UNLT)
        : "cc"
    );
    return result;
}

/* Function 8: Test LTGT condition with x87 */
static int __attribute__((noinline)) test_ltgt(long double a, long double b)
{
    int result;
    asm volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st(0)\n\t"
        "set%c0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result)
        : "m"(a), "m"(b), "u"(LTGT)
        : "cc", "st"
    );
    return result;
}

/* Helper function that uses switch to select condition code */
static int __attribute__((noinline)) test_cond_switch(int cond_code, double a, double b)
{
    int result = 0;
    
    /* This switch may help trigger the printing logic */
    switch (cond_code) {
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
                : "x"(a), "x"(b), "u"(cond_code)  /* Invalid condition code */
                : "cc"
            );
            break;
    }
    return result;
}

/* Function with complex control flow to obscure optimizations */
static int __attribute__((noinline)) test_complex_flow(double a, double b, long double c, long double d)
{
    volatile int sum = 0;
    
    /* Mix regular C comparisons with inline assembly */
    if (a != b) {
        sum += test_unordered(a, b);
    }
    
    if (a >= b) {
        sum += test_ordered((double)c, (double)d);
    }
    
    /* Use NaN to trigger UNORDERED cases */
    double nan_val = 0.0 / 0.0;
    if (isnan(a) || isnan(b)) {
        sum += test_uneq(c, d);
    }
    
    return sum;
}

int main(int argc, char *argv[])
{
    volatile int total = 0;
    int iterations = 100;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize arrays with volatile values */
    double d_vals[8];
    long double ld_vals[8];
    
    for (int i = 0; i < 8; i++) {
        d_vals[i] = g_d1 + i * 0.5;
        ld_vals[i] = g_ld1 + i * 0.5L;
    }
    
    /* Insert some special values */
    d_vals[2] = 0.0 / 0.0;  /* NaN */
    d_vals[5] = 1.0 / 0.0;  /* Inf */
    ld_vals[3] = -0.0L;
    
    /* Main test loop */
    for (int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Call all condition code test functions */
        total += test_unordered(d_vals[idx], d_vals[(idx + 1) % 8]);
        total += test_ordered(d_vals[idx], d_vals[(idx + 2) % 8]);
        total += test_uneq(ld_vals[idx], ld_vals[(idx + 3) % 8]);
        total += test_unge(d_vals[idx], d_vals[(idx + 4) % 8]);
        total += test_ungt(ld_vals[idx], ld_vals[(idx + 5) % 8]);
        total += test_unle(d_vals[idx], d_vals[(idx + 6) % 8]);
        total += test_unlt(d_vals[idx], d_vals[(idx + 7) % 8]);
        total += test_ltgt(ld_vals[idx], ld_vals[idx]);
        
        /* Use the switch-based function with volatile selector */
        g_selector = (g_selector + 1) % 9;  /* 9 to include invalid case */
        total += test_cond_switch(g_selector, d_vals[idx], d_vals[(idx + 1) % 8]);
        
        /* Complex flow test */
        total += test_complex_flow(d_vals[idx], d_vals[(idx + 2) % 8],
                                  ld_vals[idx], ld_vals[(idx + 3) % 8]);
        
        /* Mix with regular floating-point operations */
        double temp = d_vals[idx] * d_vals[(idx + 1) % 8];
        long double ltemp = ld_vals[idx] / ld_vals[(idx + 2) % 8];
        
        if (temp > 0.0) {
            total += test_unordered(temp, d_vals[idx]);
        }
        
        if (!isnan(ltemp)) {
            total += test_ordered((double)ltemp, d_vals[(idx + 3) % 8]);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total: %d\n", total);
    
    /* Additional test with pointer to function to confuse optimizer */
    {
        int (*funcs[8])(double, double) = {
            (int (*)(double, double))test_unordered,
            (int (*)(double, double))test_ordered,
            (int (*)(double, double))test_unge,
            (int (*)(double, double))test_unle,
            (int (*)(double, double))test_unlt,
            NULL, NULL, NULL
        };
        
        for (int i = 0; i < 6 && funcs[i] != NULL; i++) {
            total += funcs[i](d_vals[i], d_vals[i + 1]);
        }
        
        printf("Final total: %d\n", total);
    }
    
    return total != 0 ? 0 : 1;
}
