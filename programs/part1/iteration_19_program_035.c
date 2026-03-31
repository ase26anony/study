/* Test program to trigger x86 floating-point condition code output */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimizations from removing crucial operations */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_nan;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b) ? 1 : 0;  /* UNORDERED for NaN != 1.0 */
    results[1] = (a == a) ? 1 : 0;  /* UNORDERED for NaN == NaN */
    results[2] = (b == b) ? 1 : 0;  /* ORDERED for 1.0 == 1.0 */
    results[3] = (a < b) ? 1 : 0;   /* UNORDERED for NaN < 1.0 */
    results[4] = (b < a) ? 1 : 0;   /* UNORDERED for 1.0 < NaN */
    results[5] = (c > d) ? 1 : 0;   /* UNORDERED for 0.0 > NaN */
    results[6] = (d <= c) ? 1 : 0;  /* UNORDERED for NaN <= 0.0 */
    results[7] = (c >= d) ? 1 : 0;  /* UNORDERED for 0.0 >= NaN */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int results[12] = {0};
    
    /* These map to various condition codes */
    results[0] = isunordered(a, b);  /* UNORDERED */
    results[1] = isordered(a, b);    /* ORDERED */
    results[2] = isgreater(b, c);    /* UNLE? Actually GT */
    results[3] = isgreaterequal(b, c); /* UNLT? Actually GE */
    results[4] = isless(b, c);       /* UNGE? Actually LT */
    results[5] = islessequal(b, c);  /* UNGT? Actually LE */
    results[6] = islessgreater(b, c); /* LTGT */
    results[7] = isunordered(a, a);  /* UNORDERED */
    results[8] = isordered(b, c);    /* ORDERED */
    results[9] = isgreater(d, b);    /* UNLE? */
    results[10] = isless(c, d);      /* UNGE? */
    results[11] = islessgreater(a, b); /* LTGT with NaN */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Test UNORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Test ORDERED condition */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(b), "x"(c)
        : "cc"
    );
    
    /* Test with different condition codes */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "x"(c), "x"(b)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Test 4: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        if (i % 4 == 0) arr1[i] = g_nan;
        else if (i % 4 == 1) arr1[i] = g_inf;
        else if (i % 4 == 2) arr1[i] = g_zero;
        else arr1[i] = g_one;
        
        if (i % 3 == 0) arr2[i] = g_nan;
        else if (i % 3 == 1) arr2[i] = g_one;
        else arr2[i] = g_zero;
    }
    
    int counts[6] = {0};
    
    for (int i = 0; i < 8; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]);  /* UNORDERED */
        counts[1] += isordered(arr1[i], arr2[i]);    /* ORDERED */
        counts[2] += isgreater(arr1[i], arr2[i]);    /* UNLE? */
        counts[3] += isless(arr1[i], arr2[i]);       /* UNGE? */
        counts[4] += islessequal(arr1[i], arr2[i]);  /* UNGT? */
        counts[5] += islessgreater(arr1[i], arr2[i]); /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 5: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    volatile long double a = g_nan;
    volatile long double b = 1.0L;
    volatile long double c = 0.0L;
    
    int results = 0;
    
    /* x87 comparisons */
    results += (a != b) ? 1 : 0;
    results += (b == b) ? 1 : 0;
    results += (a > c) ? 1 : 0;
    results += (c < b) ? 1 : 0;
    results += (a == a) ? 1 : 0;
    
    /* Force x87 usage with explicit operations */
    volatile long double d = a * b;
    volatile long double e = c / b;
    results += (d != e) ? 1 : 0;
    results += (e == e) ? 1 : 0;
    
    return results;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int result = 0;
    
    /* Multiple comparisons in switch to force different condition codes */
    if (isunordered(a, b)) {
        result += 1;  /* UNORDERED */
    }
    
    if (isordered(b, c)) {
        result += 2;  /* ORDERED */
    }
    
    if (isgreater(d, b)) {
        result += 4;  /* UNLE? */
    }
    
    if (isless(c, d)) {
        result += 8;  /* UNGE? */
    }
    
    if (islessgreater(b, c)) {
        result += 16; /* LTGT */
    }
    
    if (isunordered(a, a)) {
        result += 32; /* UNORDERED */
    }
    
    return result;
}

/* Test 7: Mixed SSE and x87 operations */
NOINLINE int test_mixed_operations(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile long double c = 2.0L;
    volatile long double d = g_nan;
    
    int results = 0;
    
    /* SSE2 double comparison */
    results += (a != b) ? 1 : 0;
    
    /* x87 long double comparison */
    results += (c == c) ? 2 : 0;
    results += (d != c) ? 4 : 0;
    
    /* Mixed type operation */
    volatile double e = (double)c + b;
    results += (e > a) ? 8 : 0;
    
    return results;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition code generation...\n");
    
    /* Run all tests to trigger various condition codes */
    checksum += test_unordered_comparisons();
    checksum += test_math_macros();
    checksum += test_inline_asm();
    checksum += test_array_comparisons();
    checksum += test_long_double();
    checksum += test_switch_comparisons();
    checksum += test_mixed_operations();
    
    printf("Checksum: %d\n", checksum);
    
    /* Also test with volatile function calls to prevent optimization */
    volatile int (*funcs[])(void) = {
        test_unordered_comparisons,
        test_math_macros,
        test_inline_asm,
        test_array_comparisons,
        test_long_double,
        test_switch_comparisons,
        test_mixed_operations
    };
    
    int final_result = 0;
    for (size_t i = 0; i < sizeof(funcs)/sizeof(funcs[0]); i++) {
        final_result += funcs[i]();
    }
    
    printf("Final result: %d\n", final_result);
    
    return 0;
}
