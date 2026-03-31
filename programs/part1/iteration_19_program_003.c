/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations from removing critical operations */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile long double g_ld_nan = NAN;
volatile long double g_ld_inf = INFINITY;
volatile long double g_ld_zero = 0.0L;

/* Test 1: Direct unordered comparisons with NaN */
NO_OPT int test_unordered_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_inf;
    volatile double d = g_zero;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b);  /* UNORDERED comparison */
    results[1] = (a == a);  /* UNORDERED comparison with NaN */
    results[2] = (b == d);  /* ORDERED comparison */
    results[3] = (c != d);  /* ORDERED comparison */
    
    /* Mixed comparisons */
    results[4] = (a < b);   /* UNORDERED comparison */
    results[5] = (a > b);   /* UNORDERED comparison */
    results[6] = (b < c);   /* ORDERED comparison */
    results[7] = (c > d);   /* ORDERED comparison */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NO_OPT int test_asm_condition_codes(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_zero;
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Test UNORDERED condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    /* Test ORDERED condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(z), "x"(y)
        : "cc"
    );
    
    /* Test UNEQ condition code */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "x"(z), "x"(z)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Test 3: Math.h comparison macros */
NO_OPT int test_math_macros(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : g_nan;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : g_inf;
    }
    
    int counts[7] = {0};  /* For different comparison results */
    
    for (int i = 0; i < 8; i++) {
        /* These macros should generate various condition codes */
        counts[0] += isunordered(arr1[i], arr2[i]);   /* UNORDERED */
        counts[1] += isgreater(arr1[i], arr2[i]);     /* UNLE? Actually generates GT */
        counts[2] += isless(arr1[i], arr2[i]);        /* UNGE? Actually generates LT */
        counts[3] += isgreaterequal(arr1[i], arr2[i]); /* UNLT? */
        counts[4] += islessequal(arr1[i], arr2[i]);   /* UNGT? */
        counts[5] += !isunordered(arr1[i], arr2[i]) && 
                     (arr1[i] == arr2[i]);            /* UNEQ */
        counts[6] += (arr1[i] != arr2[i]) && 
                     !isunordered(arr1[i], arr2[i]);  /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 4: Long double x87 operations */
NO_OPT int test_long_double_ops(void) {
    volatile long double a = g_ld_nan;
    volatile long double b = 3.14159265358979323846L;
    volatile long double c = g_ld_inf;
    volatile long double d = g_ld_zero;
    
    int results = 0;
    int temp;
    
    /* x87 style comparisons - may generate different condition codes */
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(temp)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    results += temp;
    
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(temp)
        : "m"(c), "m"(d)
        : "cc", "st"
    );
    results += temp;
    
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(temp)
        : "m"(b), "m"(b)
        : "cc", "st"
    );
    results += temp;
    
    return results;
}

/* Test 5: Complex control flow based on FP comparisons */
NO_OPT int test_complex_control_flow(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_zero;
    volatile double w = g_inf;
    
    int result = 0;
    
    /* Switch-like behavior using comparisons */
    if (isunordered(x, y)) {
        result += 1;  /* UNORDERED */
    }
    
    if (!isunordered(z, y) && (z != y)) {
        result += 2;  /* LTGT */
    }
    
    if (!isunordered(z, z) && (z == z)) {
        result += 4;  /* UNEQ */
    }
    
    if (isgreater(w, z)) {
        result += 8;  /* UNLE? */
    }
    
    if (isless(z, w)) {
        result += 16; /* UNGE? */
    }
    
    /* Force multiple branches */
    for (int i = 0; i < 4; i++) {
        volatile double v1 = (i & 1) ? g_nan : (double)i;
        volatile double v2 = (i & 2) ? g_inf : (double)(i * 2);
        
        if (isunordered(v1, v2)) {
            result += 32;
        } else if (v1 > v2) {
            result += 64;
        } else if (v1 < v2) {
            result += 128;
        } else {
            result += 256;
        }
    }
    
    return result;
}

/* Test 6: Mixed SSE and x87 operations */
NO_OPT int test_mixed_fpu(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile long double ld1 = g_ld_nan;
    volatile long double ld2 = 2.71828182845904523536L;
    
    int result = 0;
    
    /* SSE2 comparison */
    int sse_result;
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "ucomisd %%xmm1, %%xmm0\n\t"
        "set%C0 %0"
        : "=r"(sse_result)
        : "m"(d1), "m"(d2)
        : "xmm0", "xmm1", "cc"
    );
    result += sse_result;
    
    /* x87 comparison */
    int x87_result;
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0"
        : "=r"(x87_result)
        : "m"(ld1), "m"(ld2)
        : "cc", "st"
    );
    result += x87_result;
    
    /* Mixed: convert long double to double and compare */
    volatile double d3 = (double)ld1;
    volatile double d4 = (double)ld2;
    
    int mixed_result;
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(mixed_result)
        : "x"(d3), "x"(d4)
        : "cc"
    );
    result += mixed_result;
    
    return result;
}

int main(void) {
    printf("Testing x86 condition code generation...\n");
    
    int checksum = 0;
    
    checksum += test_unordered_comparisons();
    printf("Test 1 result: %d\n", checksum);
    
    checksum += test_asm_condition_codes();
    printf("Test 2 result: %d\n", checksum);
    
    checksum += test_math_macros();
    printf("Test 3 result: %d\n", checksum);
    
    checksum += test_long_double_ops();
    printf("Test 4 result: %d\n", checksum);
    
    checksum += test_complex_control_flow();
    printf("Test 5 result: %d\n", checksum);
    
    checksum += test_mixed_fpu();
    printf("Test 6 result: %d\n", checksum);
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
