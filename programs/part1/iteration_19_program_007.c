/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing crucial operations */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile long double g_ld_nan = NAN;
volatile long double g_ld_inf = INFINITY;
volatile long double g_ld_zero = 0.0L;

/* Test 1: Unordered comparisons with double using != operator */
NOOPT int test_unordered_comparisons(double a, double b, double c, double d) {
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b) ? 1 : 0;           /* UNORDERED when NaN involved */
    results[1] = (c == d) ? 1 : 0;           /* ORDERED when normal numbers */
    results[2] = (a != c) ? 1 : 0;
    results[3] = (b == d) ? 1 : 0;
    
    /* Mix with other comparisons */
    results[4] = (a < b) ? 1 : 0;            /* UNLT when NaN */
    results[5] = (a > b) ? 1 : 0;            /* UNGT when NaN */
    results[6] = (c <= d) ? 1 : 0;           /* UNLE when NaN */
    results[7] = (c >= d) ? 1 : 0;           /* UNGE when NaN */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOOPT int test_math_macros(double a, double b, double c, double d) {
    int results[12] = {0};
    
    /* These map directly to the uncovered condition codes */
    results[0] = isunordered(a, b);      /* UNORDERED */
    results[1] = !isunordered(c, d);     /* ORDERED (inverse) */
    results[2] = isgreater(a, c);        /* UNGT */
    results[3] = isgreaterequal(b, d);   /* UNGE */
    results[4] = isless(a, b);           /* UNLT */
    results[5] = islessequal(c, d);      /* UNLE */
    
    /* Create UNEQ and LTGT scenarios */
    results[6] = (!isunordered(a, c) && !isgreater(a, c) && !isless(a, c)) ? 1 : 0;  /* UNEQ */
    results[7] = (isgreater(a, d) || isless(a, d)) ? 1 : 0;  /* LTGT (not equal and ordered) */
    
    /* More complex expressions to force code generation */
    results[8] = isunordered(a, a) ? 1 : 0;  /* Self-comparison with NaN */
    results[9] = isunordered(b, b) ? 1 : 0;
    results[10] = isgreater(c, a) ? 1 : 0;
    results[11] = isless(d, b) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOOPT int test_inline_asm(double x, double y) {
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* x87 unordered compare with condition code output */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "set%C0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "m"(x), "m"(y)
        : "cc", "st"
    );
    
    /* SSE2 unordered compare */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    /* Another variant with different condition */
    __asm__ volatile (
        "comisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Test 4: Array processing with mixed comparisons */
NOOPT int test_array_comparisons(const volatile double* arr1, 
                                 const volatile double* arr2, 
                                 int size) {
    int counts[6] = {0};  /* UNORDERED, ORDERED, UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT */
    
    for (int i = 0; i < size; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Use switch to force multiple condition code usages */
        if (isunordered(a, b)) {
            counts[0]++;  /* UNORDERED */
        } else if (!isgreater(a, b) && !isless(a, b)) {
            counts[2]++;  /* UNEQ */
        } else if (isgreater(a, b)) {
            counts[4]++;  /* UNGT */
        } else if (isless(a, b)) {
            counts[5]++;  /* UNLT */
        }
        
        /* Additional comparisons */
        counts[1] += !isunordered(a, b);      /* ORDERED */
        counts[3] += isgreaterequal(a, b);    /* UNGE */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 5: Long double (x87) specific tests */
NOOPT int test_long_double(long double a, long double b, long double c) {
    int results = 0;
    
    /* x87 specific comparisons */
    results += (a != b) ? 1 : 0;
    results += (a == c) ? 2 : 0;
    results += (a < b) ? 4 : 0;
    results += (a > c) ? 8 : 0;
    results += (a <= b) ? 16 : 0;
    results += (a >= c) ? 32 : 0;
    
    /* Force x87 stack operations */
    long double temp1 = a * b;
    long double temp2 = c / a;
    results += (temp1 != temp2) ? 64 : 0;
    results += (temp1 == a) ? 128 : 0;
    
    return results;
}

/* Test 6: Complex control flow based on FP comparisons */
NOOPT int test_complex_control_flow(double a, double b, double c, double d) {
    int result = 0;
    
    /* Switch-like behavior using FP comparisons */
    if (isunordered(a, b)) {
        result = 1;  /* UNORDERED path */
        volatile double* ptr = &a;
        *ptr = b;  /* Prevent dead code elimination */
    } else if (!isgreater(a, b) && !isless(a, b)) {
        result = 2;  /* UNEQ path */
    } else if (isgreater(a, b)) {
        result = 3;  /* UNGT path */
        if (isless(c, d)) {
            result += 10;  /* UNLT nested */
        }
    } else if (isless(a, b)) {
        result = 4;  /* UNLT path */
        if (isgreaterequal(c, d)) {
            result += 20;  /* UNGE nested */
        }
    }
    
    /* Additional branches */
    if (isgreaterequal(a, c)) {
        result += 100;  /* UNGE */
    }
    if (islessequal(b, d)) {
        result += 200;  /* UNLE */
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    int total = 0;
    
    /* Initialize test data with mix of normal and special values */
    volatile double test_doubles[8];
    test_doubles[0] = NAN;
    test_doubles[1] = INFINITY;
    test_doubles[2] = -INFINITY;
    test_doubles[3] = 0.0;
    test_doubles[4] = 1.0;
    test_doubles[5] = -1.0;
    test_doubles[6] = 2.5;
    test_doubles[7] = -2.5;
    
    volatile long double test_ldoubles[4];
    test_ldoubles[0] = NAN;
    test_ldoubles[1] = INFINITY;
    test_ldoubles[2] = 0.0L;
    test_ldoubles[3] = 3.14159265358979323846L;
    
    printf("Starting condition code tests...\n");
    
    /* Run all tests multiple times with different inputs */
    for (int i = 0; i < 4; i++) {
        total += test_unordered_comparisons(
            test_doubles[i], 
            test_doubles[i+1], 
            test_doubles[i+2], 
            test_doubles[i+3]
        );
        
        total += test_math_macros(
            test_doubles[i], 
            test_doubles[i+2], 
            test_doubles[i+1], 
            test_doubles[i+3]
        );
        
        total += test_inline_asm(test_doubles[i], test_doubles[i+1]);
        
        total += test_array_comparisons(test_doubles, test_doubles + 4, 4);
        
        total += test_long_double(
            test_ldoubles[i % 4], 
            test_ldoubles[(i+1) % 4], 
            test_ldoubles[(i+2) % 4]
        );
        
        total += test_complex_control_flow(
            test_doubles[i], 
            test_doubles[i+1], 
            test_doubles[i+2], 
            test_doubles[i+3]
        );
    }
    
    printf("Total checksum: %d\n", total);
    printf("Tests completed.\n");
    
    return 0;
}
