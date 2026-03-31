/* i386_condition_codes.c - Trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical operations */
#define NOOPT __attribute__((noinline, noipa, optimize("O0")))

/* Global volatile variables to inhibit constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg = -1.0;

/* Test 1: Direct unordered comparisons with != operator */
NOOPT int test_unordered_comparisons(double a, double b, double c, double d) {
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b);  /* UNORDERED when either is NaN */
    results[1] = (c == d);  /* ORDERED when both are numbers */
    
    /* Generate various condition codes through explicit comparisons */
    results[2] = !(a < b) && !isunordered(a, b);   /* UNGE: not less than */
    results[3] = !(a <= b) && !isunordered(a, b);  /* UNGT: not less or equal */
    results[4] = (a <= b) || isunordered(a, b);    /* UNLE: less or equal or unordered */
    results[5] = (a < b) || isunordered(a, b);     /* UNLT: less than or unordered */
    results[6] = (a != b) && !isunordered(a, b);   /* LTGT: not equal but ordered */
    results[7] = (a == b) || isunordered(a, b);    /* UNEQ: equal or unordered */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOOPT int test_asm_condition_codes(double x, double y) {
    int result = 0;
    
    /* UNORDERED condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    int result2 = 0;
    /* ORDERED condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C1 %0"
        : "=r"(result2)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    return result + result2;
}

/* Test 3: Loop with various comparison macros */
NOOPT int test_comparison_macros(const volatile double* arr1, 
                                 const volatile double* arr2, 
                                 int n) {
    int counts[7] = {0};  /* For different condition types */
    
    for (int i = 0; i < n; i++) {
        double a = arr1[i];
        double b = arr2[i];
        
        /* Each of these may generate different condition codes */
        if (isunordered(a, b)) counts[0]++;      /* UNORDERED */
        if (!isunordered(a, b)) counts[1]++;     /* ORDERED */
        if (isgreater(a, b)) counts[2]++;        /* GT (ordered greater than) */
        if (isless(a, b)) counts[3]++;           /* LT (ordered less than) */
        if (isgreaterequal(a, b)) counts[4]++;   /* GE (ordered greater or equal) */
        if (islessequal(a, b)) counts[5]++;      /* LE (ordered less or equal) */
        if (!(a == b) && !isunordered(a, b)) counts[6]++; /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 4: Long double (x87) operations */
NOOPT int test_long_double_ops(volatile long double a, volatile long double b) {
    int results = 0;
    
    /* x87 comparisons generate explicit condition codes */
    if (a != b) results |= 1;      /* UNORDERED when NaN */
    if (a == b) results |= 2;      /* ORDERED equal */
    if (a < b) results |= 4;       /* LT */
    if (a <= b) results |= 8;      /* LE */
    if (a > b) results |= 16;      /* GT */
    if (a >= b) results |= 32;     /* GE */
    
    /* Force multiple branches to prevent optimization */
    switch (fpclassify(a)) {
        case FP_NAN:
            results |= 64;
            break;
        case FP_INFINITE:
            results |= 128;
            break;
        case FP_ZERO:
            results |= 256;
            break;
        case FP_NORMAL:
            results |= 512;
            break;
        case FP_SUBNORMAL:
            results |= 1024;
            break;
    }
    
    return results;
}

/* Test 5: Mixed SSE and x87 operations */
NOOPT int test_mixed_operations(volatile double d1, volatile double d2,
                                volatile long double ld1, volatile long double ld2) {
    int result = 0;
    
    /* SSE2 comparison */
    if (__builtin_ia32_ucomisd(d1, d2)) {
        result += 1;
    }
    
    /* x87 style comparison through builtin */
    if (d1 != d2) {
        result += 2;
    }
    
    /* Long double comparison (forces x87) */
    if (ld1 > ld2) {
        result += 4;
    }
    
    /* Complex expression that might generate UNEQ */
    if ((d1 == d2) || isunordered(d1, d2)) {
        result += 8;
    }
    
    /* Complex expression that might generate UNGE */
    if (!(d1 < d2) && !isunordered(d1, d2)) {
        result += 16;
    }
    
    return result;
}

/* Test 6: Switch based on comparison results */
NOOPT int test_switch_on_comparison(double a, double b) {
    int result = 0;
    
    /* This switch should generate multiple condition code checks */
    int cmp_result = 0;
    if (isunordered(a, b)) cmp_result = 0;
    else if (a < b) cmp_result = 1;
    else if (a > b) cmp_result = 2;
    else cmp_result = 3;  /* a == b */
    
    switch (cmp_result) {
        case 0:  /* UNORDERED */
            result = 100;
            break;
        case 1:  /* LT */
            result = 200;
            break;
        case 2:  /* GT */
            result = 300;
            break;
        case 3:  /* EQ */
            result = 400;
            break;
    }
    
    return result;
}

/* Main test driver */
int main(void) {
    int total_checksum = 0;
    
    /* Initialize test arrays with mixed values */
    volatile double arr1[10], arr2[10];
    for (int i = 0; i < 10; i++) {
        arr1[i] = (i % 3 == 0) ? NAN : (double)i;
        arr2[i] = (i % 4 == 0) ? NAN : (double)(i * 0.5);
    }
    
    /* Test 1: Unordered comparisons */
    total_checksum += test_unordered_comparisons(
        g_nan, g_one,    /* NaN vs number */
        g_one, g_one,    /* number vs same number */
        g_one, g_zero,   /* number vs different number */
        g_inf, g_neg     /* infinity vs negative */
    );
    
    /* Test 2: Inline assembly */
    total_checksum += test_asm_condition_codes(g_nan, g_one);
    total_checksum += test_asm_condition_codes(g_one, g_one);
    
    /* Test 3: Comparison macros in loop */
    total_checksum += test_comparison_macros(arr1, arr2, 10);
    
    /* Test 4: Long double operations */
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_num = 3.14159265358979323846L;
    
    total_checksum += test_long_double_ops(ld_nan, ld_num);
    total_checksum += test_long_double_ops(ld_num, ld_inf);
    
    /* Test 5: Mixed operations */
    total_checksum += test_mixed_operations(
        g_nan, g_one,
        ld_nan, ld_num
    );
    
    /* Test 6: Switch on comparison */
    total_checksum += test_switch_on_comparison(g_nan, g_one);
    total_checksum += test_switch_on_comparison(g_one, g_zero);
    total_checksum += test_switch_on_comparison(g_one, g_one);
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Also print some diagnostic info */
    printf("NaN: %f, Inf: %f, Zero: %f\n", g_nan, g_inf, g_zero);
    
    return 0;
}
