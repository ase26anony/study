#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations that might remove our test code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    VOLATILE_DOUBLE nan_val = NAN;
    VOLATILE_DOUBLE normal_val = 3.14159;
    VOLATILE_DOUBLE zero = 0.0;
    VOLATILE_DOUBLE inf_val = INFINITY;
    
    int results[8] = {0};
    
    /* UNORDERED: Compare NaN with anything using != */
    results[0] = (nan_val != normal_val) ? 1 : 0;
    
    /* ORDERED: Compare normal values using == */
    results[1] = (normal_val == normal_val) ? 1 : 0;
    
    /* UNEQ: unordered or equal - use isunordered() and == */
    results[2] = (isunordered(nan_val, normal_val) || (nan_val == normal_val)) ? 1 : 0;
    
    /* UNGE: not less than (unordered or greater or equal) */
    results[3] = (!(nan_val < normal_val)) ? 1 : 0;
    
    /* UNGT: not less or equal (unordered or greater) */
    results[4] = (!(nan_val <= normal_val)) ? 1 : 0;
    
    /* UNLE: unordered or less or equal */
    results[5] = (isunordered(nan_val, normal_val) || (nan_val <= normal_val)) ? 1 : 0;
    
    /* UNLT: unordered or less than */
    results[6] = (isunordered(nan_val, normal_val) || (nan_val < normal_val)) ? 1 : 0;
    
    /* LTGT: less than or greater than (unordered) */
    results[7] = ((nan_val < normal_val) || (nan_val > normal_val)) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    VOLATILE_DOUBLE a = NAN;
    VOLATILE_DOUBLE b = 2.71828;
    VOLATILE_DOUBLE c = 1.41421;
    VOLATILE_DOUBLE d = -INFINITY;
    
    unsigned char results[8] = {0};
    
    /* Using x87 floating point compare */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "set%C0 %0\n\t"
        : "=r"(results[0])
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* SSE2 compare */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "ucomisd %%xmm1, %%xmm0\n\t"
        "set%C0 %0\n\t"
        : "=r"(results[1])
        : "m"(c), "m"(b)
        : "xmm0", "xmm1", "cc"
    );
    
    /* Test with ordered comparison */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "comisd %%xmm1, %%xmm0\n\t"
        "set%C0 %0\n\t"
        : "=r"(results[2])
        : "m"(b), "m"(c)
        : "xmm0", "xmm1", "cc"
    );
    
    /* Test with infinity */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "ucomisd %%xmm1, %%xmm0\n\t"
        "set%C0 %0\n\t"
        : "=r"(results[3])
        : "m"(d), "m"(b)
        : "xmm0", "xmm1", "cc"
    );
    
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Array operations with various comparison macros */
NOINLINE int test_array_comparisons(void) {
    const int SIZE = 16;
    VOLATILE_DOUBLE arr1[SIZE];
    VOLATILE_DOUBLE arr2[SIZE];
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < SIZE; i++) {
        if (i % 4 == 0) {
            arr1[i] = NAN;
            arr2[i] = (double)i;
        } else if (i % 4 == 1) {
            arr1[i] = (double)i;
            arr2[i] = NAN;
        } else if (i % 4 == 2) {
            arr1[i] = INFINITY;
            arr2[i] = (double)i;
        } else {
            arr1[i] = (double)i;
            arr2[i] = (double)(i * 2);
        }
    }
    
    int counts[7] = {0};
    
    for (int i = 0; i < SIZE; i++) {
        /* Use various comparison functions to trigger different condition codes */
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;
        counts[1] += isgreater(arr1[i], arr2[i]) ? 1 : 0;
        counts[2] += isless(arr1[i], arr2[i]) ? 1 : 0;
        counts[3] += isgreaterequal(arr1[i], arr2[i]) ? 1 : 0;
        counts[4] += islessequal(arr1[i], arr2[i]) ? 1 : 0;
        counts[5] += (arr1[i] != arr2[i]) ? 1 : 0;  /* UNORDERED */
        counts[6] += (arr1[i] == arr2[i]) ? 1 : 0;  /* ORDERED */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 4: Long double (x87) operations */
NOINLINE int test_long_double_ops(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_inf = INFINITY;
    VOLATILE_LONG_DOUBLE ld_normal = 3.14159265358979323846L;
    VOLATILE_LONG_DOUBLE ld_zero = 0.0L;
    
    int results = 0;
    
    /* Force x87 comparisons with long double */
    if (isunordered(ld_nan, ld_normal)) results |= 1;
    if (!isunordered(ld_normal, ld_zero)) results |= 2;
    if (ld_nan != ld_normal) results |= 4;      /* UNORDERED */
    if (ld_normal == ld_normal) results |= 8;   /* ORDERED */
    
    /* Complex expression to force multiple condition codes */
    VOLATILE_LONG_DOUBLE a = ld_nan;
    VOLATILE_LONG_DOUBLE b = ld_normal;
    VOLATILE_LONG_DOUBLE c = ld_inf;
    
    /* This should generate various condition code checks */
    if ((a < b) || (a > b)) results |= 16;      /* LTGT */
    if (!(a < b)) results |= 32;                /* UNGE */
    if (!(a <= b)) results |= 64;               /* UNGT */
    if (isunordered(a, b) || (a <= b)) results |= 128;  /* UNLE */
    if (isunordered(a, b) || (a < b)) results |= 256;   /* UNLT */
    
    return results;
}

/* Test 5: Switch statement based on fpclassify */
NOINLINE int test_fpclassify_switch(void) {
    VOLATILE_DOUBLE values[] = {NAN, INFINITY, -INFINITY, 0.0, 1.0, -1.0, DBL_MIN, DBL_MAX};
    int results = 0;
    
    for (int i = 0; i < 8; i++) {
        switch (fpclassify(values[i])) {
            case FP_NAN:
                results += 1;
                /* Force unordered comparison */
                if (values[i] != 0.0) results += 2;
                break;
            case FP_INFINITE:
                results += 4;
                /* Force ordered comparison */
                if (values[i] == values[i]) results += 8;
                break;
            case FP_ZERO:
                results += 16;
                break;
            case FP_SUBNORMAL:
                results += 32;
                break;
            case FP_NORMAL:
                results += 64;
                /* Various comparisons */
                if (isgreater(values[i], 0.0)) results += 128;
                if (isless(values[i], 0.0)) results += 256;
                if (isgreaterequal(values[i], 0.0)) results += 512;
                if (islessequal(values[i], 0.0)) results += 1024;
                break;
        }
    }
    
    return results;
}

/* Test 6: Mixed SSE and x87 operations */
NOINLINE int test_mixed_fp_units(void) {
    VOLATILE_DOUBLE sse_val1 = NAN;
    VOLATILE_DOUBLE sse_val2 = 2.5;
    VOLATILE_LONG_DOUBLE x87_val1 = NAN;
    VOLATILE_LONG_DOUBLE x87_val2 = 3.5L;
    
    int result = 0;
    
    /* SSE operations */
    if (__builtin_ia32_ucomisd(sse_val1, sse_val2)) result |= 1;
    if (__builtin_ia32_comisd(sse_val2, sse_val1)) result |= 2;
    
    /* Mixed: convert double to long double and compare */
    VOLATILE_LONG_DOUBLE converted = (VOLATILE_LONG_DOUBLE)sse_val1;
    if (isunordered(converted, x87_val2)) result |= 4;
    
    /* Force x87 compare */
    if (x87_val1 != x87_val2) result |= 8;      /* UNORDERED */
    if (x87_val2 == x87_val2) result |= 16;     /* ORDERED */
    
    /* Complex expression mixing units */
    VOLATILE_DOUBLE temp = (VOLATILE_DOUBLE)x87_val1;
    if (isunordered(temp, sse_val2)) result |= 32;
    if (!(temp < sse_val2)) result |= 64;       /* UNGE */
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting floating-point condition code tests...\n");
    
    /* Run all tests and accumulate results */
    checksum += test_unordered_comparisons();
    checksum += test_asm_condition_codes();
    checksum += test_array_comparisons();
    checksum += test_long_double_ops();
    checksum += test_fpclassify_switch();
    checksum += test_mixed_fp_units();
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile operations to prevent dead code elimination */
    VOLATILE_DOUBLE final_check = NAN;
    VOLATILE_DOUBLE final_normal = 42.0;
    
    /* One more unordered compare at the end */
    if (final_check != final_normal) {
        checksum += 1000;
    }
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
