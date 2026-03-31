/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations from removing critical operations */
#define NOINLINE __attribute__((noinline, noipa))

/* Test 1: Unordered comparisons with NaN values */
NOINLINE int test_unordered_comparisons(void) {
    volatile double nan_val = NAN;
    volatile double inf_val = INFINITY;
    volatile double normal1 = 3.14159;
    volatile double normal2 = 2.71828;
    volatile double zero = 0.0;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal1) ? 1 : 0;      /* unordered comparison */
    results[1] = (normal1 == normal2) ? 1 : 0;      /* ordered comparison */
    results[2] = (nan_val == nan_val) ? 1 : 0;      /* UNEQ? */
    results[3] = (normal1 != normal2) ? 1 : 0;      /* LTGT? */
    
    /* Force multiple branches to prevent optimization */
    if (isunordered(nan_val, normal1)) results[4] = 1;
    if (isgreater(normal1, normal2))   results[5] = 1;  /* UNLE? */
    if (isless(normal1, normal2))      results[6] = 1;  /* UNGE? */
    if (islessequal(normal1, normal2)) results[7] = 1;  /* UNGT? */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double c = NAN;
    int result = 0;
    
    /* Test various condition codes through inline assembly */
    
    /* UNORDERED comparison */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result)
        : "x"(a), "x"(c)
        : "cc"
    );
    
    int r1 = result;
    
    /* ORDERED comparison */
    __asm__ volatile (
        "ucomisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    int r2 = result;
    
    /* UNEQ comparison */
    __asm__ volatile (
        "ucomisd %1, %1\n\t"  /* Compare NaN with itself */
        "set%C0 %0"
        : "=r"(result)
        : "x"(c)
        : "cc"
    );
    
    return r1 + r2 + result;
}

/* Test 3: Array operations with various comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : NAN;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : INFINITY;
    }
    
    int counts[6] = {0};
    
    for (int i = 0; i < 8; i++) {
        /* Use different comparison functions to trigger different condition codes */
        if (isunordered(arr1[i], arr2[i])) counts[0]++;      /* UNORDERED */
        if (isgreater(arr1[i], arr2[i]))   counts[1]++;      /* UNLE */
        if (isless(arr1[i], arr2[i]))      counts[2]++;      /* UNGE */
        if (islessequal(arr1[i], arr2[i])) counts[3]++;      /* UNGT */
        if (isgreaterequal(arr1[i], arr2[i])) counts[4]++;   /* UNLT */
        if (islessgreater(arr1[i], arr2[i])) counts[5]++;    /* UNEQ */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) sum += counts[i];
    return sum;
}

/* Test 4: Long double (x87) operations */
NOINLINE int test_long_double_ops(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld1 = 3.14159265358979323846L;
    volatile long double ld2 = 2.71828182845904523536L;
    
    int results = 0;
    
    /* x87 style comparisons - may generate different condition codes */
    if (ld_nan != ld1) results |= 1;      /* unordered */
    if (ld1 == ld2)    results |= 2;      /* ordered */
    if (ld1 > ld2)     results |= 4;      /* UNLE? */
    if (ld1 < ld2)     results |= 8;      /* UNGE? */
    if (ld1 >= ld2)    results |= 16;     /* UNLT? */
    if (ld1 <= ld2)    results |= 32;     /* UNGT? */
    
    /* Complex expression to force multiple condition code usages */
    volatile long double ld3 = ld1 * ld2;
    if (isunordered(ld_nan, ld3)) results |= 64;
    if (islessgreater(ld1, ld3))  results |= 128;
    
    return results;
}

/* Test 5: Switch based on floating point classification */
NOINLINE int test_fpclassify_switch(void) {
    volatile double values[] = {NAN, INFINITY, -INFINITY, 0.0, 1.0, -1.0, DBL_MIN, DBL_MAX};
    int results = 0;
    
    for (int i = 0; i < 8; i++) {
        switch (fpclassify(values[i])) {
            case FP_NAN:
                results += 1;
                /* Force comparison with NaN */
                if (values[i] != values[i]) results += 2;  /* UNORDERED */
                break;
            case FP_INFINITE:
                results += 4;
                if (values[i] > 0) results += 8;  /* UNLE? */
                break;
            case FP_ZERO:
                results += 16;
                if (values[i] == 0.0) results += 32;  /* ORDERED */
                break;
            case FP_NORMAL:
                results += 64;
                if (values[i] < 1.0) results += 128;  /* UNGE? */
                break;
            case FP_SUBNORMAL:
                results += 256;
                break;
        }
    }
    
    return results;
}

/* Test 6: Direct use of builtins for SSE2 unordered compares */
NOINLINE int test_sse2_builtins(void) {
    volatile double a = 1.0;
    volatile double b = NAN;
    volatile double c = 2.0;
    
    int res = 0;
    
    /* Use GCC builtins for direct comparison */
    if (__builtin_ia32_ucomisd(a, b)) res |= 1;    /* UNORDERED */
    if (__builtin_ia32_ucomisd(a, c)) res |= 2;    /* ORDERED result */
    
    /* Create complex dependency to prevent optimization */
    volatile double d = a + b;
    if (__builtin_ia32_ucomisd(d, a)) res |= 4;
    
    return res;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 condition code generation...\n");
    
    /* Run all tests to trigger various condition code paths */
    checksum += test_unordered_comparisons();
    checksum += test_asm_condition_codes();
    checksum += test_array_comparisons();
    checksum += test_long_double_ops();
    checksum += test_fpclassify_switch();
    checksum += test_sse2_builtins();
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile double final_check = (double)checksum;
    if (isunordered(final_check, NAN)) {
        printf("Final unordered check triggered\n");
    }
    
    return 0;
}
