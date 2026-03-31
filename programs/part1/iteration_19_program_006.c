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
volatile double g_neg = -1.0;

/* Test function 1: Direct unordered comparisons with != operator */
NO_OPT int test_unordered_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_nan;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b) ? 1 : 0;  /* UNORDERED for NaN != 1.0 */
    results[1] = (a == a) ? 1 : 0;  /* ORDERED for NaN == NaN? Actually false */
    results[2] = (b != c) ? 1 : 0;  /* Regular compare */
    results[3] = (d != d) ? 1 : 0;  /* UNORDERED: NaN != NaN is true */
    
    /* Mixed comparisons */
    results[4] = (a < b) ? 1 : 0;   /* UNORDERED: NaN < 1.0 is false */
    results[5] = (a > b) ? 1 : 0;   /* UNORDERED: NaN > 1.0 is false */
    results[6] = (b < c) ? 1 : 0;   /* Regular compare */
    results[7] = (b > c) ? 1 : 0;   /* Regular compare */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test function 2: Inline assembly with %C modifier */
NO_OPT int test_asm_condition_codes(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_zero;
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Using x87 floating-point compare with condition code output */
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
    
    /* SSE2 compare with unordered condition */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "ucomisd %%xmm0, %%xmm1\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "m"(z), "m"(y)
        : "xmm0", "xmm1", "cc"
    );
    
    /* Test with UNEQ condition (unordered or equal) */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "set%C0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result3)
        : "m"(x), "m"(x)  /* NaN vs NaN */
        : "cc", "st"
    );
    
    return result1 + result2 + result3;
}

/* Test function 3: Math.h comparison macros */
NO_OPT int test_math_macros(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 3 == 0) ? g_nan : (double)i;
        arr2[i] = (i % 4 == 0) ? g_nan : (double)(i * 2);
    }
    
    int counts[7] = {0};  /* For different comparison types */
    
    for (int i = 0; i < 8; i++) {
        /* These macros generate specific condition codes */
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;  /* UNORDERED */
        counts[1] += isgreater(arr1[i], arr2[i]) ? 1 : 0;    /* UNLE? Actually generates GT */
        counts[2] += isless(arr1[i], arr2[i]) ? 1 : 0;       /* UNGE? Actually generates LT */
        counts[3] += isgreaterequal(arr1[i], arr2[i]) ? 1 : 0; /* UNLT */
        counts[4] += islessequal(arr1[i], arr2[i]) ? 1 : 0;  /* UNGT */
        
        /* Direct comparisons that might generate UNEQ/LTGT */
        counts[5] += (arr1[i] == arr2[i]) ? 1 : 0;  /* EQ/UNEQ */
        counts[6] += (arr1[i] != arr2[i]) ? 1 : 0;  /* NEQ/LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test function 4: Long double (x87) operations */
NO_OPT int test_long_double_ops(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_one = 1.0L;
    volatile long double ld_zero = 0.0L;
    
    int results = 0;
    
    /* x87 comparisons with long double */
    if (ld_nan != ld_one) results += 1;   /* UNORDERED */
    if (ld_one > ld_zero) results += 2;   /* GT */
    if (ld_one < ld_inf) results += 4;    /* LT */
    if (ld_nan == ld_nan) results += 8;   /* UNORDERED (false) */
    
    /* Complex expression to force multiple condition codes */
    volatile long double a = ld_nan;
    volatile long double b = ld_one;
    volatile long double c = ld_zero;
    
    /* Switch based on comparison results */
    int cmp1 = (a > b) ? 1 : 0;      /* UNORDERED: false */
    int cmp2 = (b > c) ? 1 : 0;      /* true */
    int cmp3 = (a != a) ? 1 : 0;     /* UNORDERED: true (NaN != NaN) */
    int cmp4 = (b != c) ? 1 : 0;     /* true */
    
    results += (cmp1 * 16) + (cmp2 * 32) + (cmp3 * 64) + (cmp4 * 128);
    
    return results;
}

/* Test function 5: Mixed SSE/x87 with control flow */
NO_OPT int test_mixed_fpu(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile long double ld1 = (long double)g_nan;
    volatile long double ld2 = (long double)g_one;
    
    int result = 0;
    
    /* SSE compare */
    if (d1 < d2) result += 1;      /* UNORDERED: false */
    if (d2 > g_zero) result += 2;  /* true */
    
    /* x87 compare */
    if (ld1 > ld2) result += 4;    /* UNORDERED: false */
    if (ld2 < (long double)g_inf) result += 8;  /* true */
    
    /* Force generation of multiple condition codes through switch */
    int classification = fpclassify(d1);
    switch (classification) {
        case FP_NAN:
            result += 16;
            /* Additional unordered compare */
            if (isunordered(d1, d2)) result += 32;
            break;
        case FP_INFINITE:
            result += 64;
            break;
        case FP_ZERO:
            result += 128;
            break;
        case FP_NORMAL:
            result += 256;
            break;
        case FP_SUBNORMAL:
            result += 512;
            break;
    }
    
    return result;
}

/* Main function that calls all tests */
int main(void) {
    printf("Testing x86 condition code generation...\n");
    
    int total = 0;
    
    total += test_unordered_comparisons();
    printf("Test 1 result: %d\n", test_unordered_comparisons());
    
    total += test_asm_condition_codes();
    printf("Test 2 result: %d\n", test_asm_condition_codes());
    
    total += test_math_macros();
    printf("Test 3 result: %d\n", test_math_macros());
    
    total += test_long_double_ops();
    printf("Test 4 result: %d\n", test_long_double_ops());
    
    total += test_mixed_fpu();
    printf("Test 5 result: %d\n", test_mixed_fpu());
    
    printf("Total checksum: %d\n", total);
    
    /* Verify some expected behavior */
    if (g_nan != g_nan) {
        printf("NaN != NaN is true (as expected for unordered)\n");
    }
    
    return 0;
}
