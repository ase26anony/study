/* test_float_conds.c - Generate x86 condition codes for floating-point comparisons */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing crucial operations */
#define NOINLINE __attribute__((noinline, noipa, noclone))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_neg_inf = -INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

volatile long double ld_nan = NAN;
volatile long double ld_inf = INFINITY;
volatile long double ld_zero = 0.0L;
volatile long double ld_one = 1.0L;

/* Test 1: Direct unordered comparisons with != and == operators */
NOINLINE int test_unordered_ordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    
    int result = 0;
    
    /* These should generate UNORDERED condition codes */
    if (a != b) result |= 1;      /* NaN != 1.0 -> true (unordered) */
    if (a != c) result |= 2;      /* NaN != 0.0 -> true (unordered) */
    if (a != a) result |= 4;      /* NaN != NaN -> true (unordered) */
    
    /* These should generate ORDERED condition codes */
    if (b == c) result |= 8;      /* 1.0 == 0.0 -> false (ordered) */
    if (b == b) result |= 16;     /* 1.0 == 1.0 -> true (ordered) */
    
    return result;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_neg_one;
    
    int result = 0;
    
    /* UNORDERED/ORDERED tests */
    if (isunordered(a, b)) result |= 1;    /* UNORDERED */
    if (isordered(b, c))   result |= 2;    /* ORDERED */
    
    /* UNEQ: unordered or equal */
    if (!isgreater(a, b) && !isless(a, b)) result |= 4;  /* UNEQ for NaN */
    if (!isgreater(b, b) && !isless(b, b)) result |= 8;  /* UNEQ for equal numbers */
    
    /* UNGE: not less than (unordered or greater or equal) */
    if (!isless(b, c)) result |= 16;       /* UNGE: 1.0 >= 0.0 */
    if (!isless(a, c)) result |= 32;       /* UNGE: NaN >= 0.0 (unordered) */
    
    /* UNGT: not less than or equal (unordered or greater) */
    if (!islessequal(b, c)) result |= 64;  /* UNGT: 1.0 > 0.0 */
    
    /* UNLE: unordered or less or equal */
    if (!isgreater(c, b)) result |= 128;   /* UNLE: 0.0 <= 1.0 */
    if (!isgreater(a, b)) result |= 256;   /* UNLE: NaN <= 1.0 (unordered) */
    
    /* UNLT: unordered or less than */
    if (!isgreaterequal(d, c)) result |= 512;  /* UNLT: -1.0 < 0.0 */
    
    /* LTGT: less than or greater than (not equal, not unordered) */
    if (islessgreater(b, c)) result |= 1024;   /* LTGT: 1.0 != 0.0 */
    
    return result;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_asm_condition_codes(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    int result = 0;
    int r1, r2, r3, r4;
    
    /* Test UNORDERED with inline assembly */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(r1)
        : "x"(a), "x"(b)
        : "cc"
    );
    result |= (r1 & 1);
    
    /* Test ORDERED */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C1 %0"
        : "=r"(r2)
        : "x"(b), "x"(c)
        , "C"(ORDERED)  /* Force ORDERED condition code */
        : "cc"
    );
    result |= (r2 & 1) << 1;
    
    /* Test UNEQ */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C2 %0"
        : "=r"(r3)
        : "x"(a), "x"(a)
        , "C"(UNEQ)     /* Force UNEQ condition code */
        : "cc"
    );
    result |= (r3 & 1) << 2;
    
    /* Test UNGE */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C3 %0"
        : "=r"(r4)
        : "x"(b), "x"(c)
        , "C"(UNGE)     /* Force UNGE condition code */
        : "cc"
    );
    result |= (r4 & 1) << 3;
    
    return result;
}

/* Test 4: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    int counts[8] = {0};
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 8; i++) {
        switch (i % 4) {
            case 0: arr1[i] = g_nan; arr2[i] = i * 0.5; break;
            case 1: arr1[i] = i * 1.0; arr2[i] = g_nan; break;
            case 2: arr1[i] = i * 1.0; arr2[i] = (i + 1) * 1.0; break;
            case 3: arr1[i] = i * 1.0; arr2[i] = i * 1.0; break;
        }
    }
    
    /* Perform various comparisons that should generate different condition codes */
    for (int i = 0; i < 8; i++) {
        if (isunordered(arr1[i], arr2[i])) counts[0]++;      /* UNORDERED */
        if (isordered(arr1[i], arr2[i]))   counts[1]++;      /* ORDERED */
        if (!isgreater(arr1[i], arr2[i]) && !isless(arr1[i], arr2[i])) counts[2]++; /* UNEQ */
        if (!isless(arr1[i], arr2[i]))     counts[3]++;      /* UNGE */
        if (!islessequal(arr1[i], arr2[i])) counts[4]++;     /* UNGT */
        if (!isgreater(arr1[i], arr2[i]))  counts[5]++;      /* UNLE */
        if (!isgreaterequal(arr1[i], arr2[i])) counts[6]++;  /* UNLT */
        if (islessgreater(arr1[i], arr2[i])) counts[7]++;    /* LTGT */
    }
    
    int result = 0;
    for (int i = 0; i < 8; i++) {
        result ^= (counts[i] << (i * 3));
    }
    return result;
}

/* Test 5: Long double (x87) operations */
NOINLINE int test_long_double_comparisons(void) {
    volatile long double a = ld_nan;
    volatile long double b = ld_one;
    volatile long double c = ld_zero;
    
    int result = 0;
    
    /* x87 style comparisons - these often generate explicit condition codes */
    if (a != b) result |= 1;      /* Should use UNORDERED */
    if (b == c) result |= 2;      /* Should use ORDERED */
    
    /* Complex expression to force multiple condition codes */
    if ((a > b) || (b <= c) || (a != a)) {
        result |= 4;
    }
    
    /* Switch based on comparison results */
    switch (fpclassify(b)) {
        case FP_NAN: result |= 8; break;
        case FP_INFINITE: result |= 16; break;
        case FP_ZERO: result |= 32; break;
        case FP_SUBNORMAL: result |= 64; break;
        case FP_NORMAL: result |= 128; break;
    }
    
    return result;
}

/* Test 6: Mixed SSE and x87 operations */
NOINLINE int test_mixed_fpu(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile long double ld1 = ld_nan;
    volatile long double ld2 = ld_one;
    
    int result = 0;
    
    /* SSE2 double comparison */
    if (d1 != d2) {
        result |= 1;
    }
    
    /* x87 long double comparison */
    if (ld1 == ld2) {
        result |= 2;
    }
    
    /* Mixed: convert and compare */
    double d_from_ld = (double)ld1;
    if (isunordered(d_from_ld, d2)) {
        result |= 4;
    }
    
    /* Force x87 stack operations */
    volatile long double sum = ld1 + ld2;
    if (sum != sum) {  /* Check for NaN */
        result |= 8;
    }
    
    return result;
}

/* Test 7: Conditional moves based on floating comparisons */
NOINLINE int test_conditional_moves(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    
    double x, y, z;
    
    /* These should generate condition codes for conditional moves */
    x = (a != b) ? 1.0 : 0.0;      /* UNORDERED */
    y = (b == c) ? 1.0 : 0.0;      /* ORDERED */
    z = (!isless(a, b)) ? 1.0 : 0.0; /* UNGE */
    
    int result = 0;
    result |= (int)(x * 100);
    result |= (int)(y * 100) << 8;
    result |= (int)(z * 100) << 16;
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests and accumulate results */
    checksum ^= test_unordered_ordered();
    checksum ^= test_math_macros();
    checksum ^= test_asm_condition_codes();
    checksum ^= test_array_comparisons();
    checksum ^= test_long_double_comparisons();
    checksum ^= test_mixed_fpu();
    checksum ^= test_conditional_moves();
    
    printf("Final checksum: %d\n", checksum);
    printf("(Non-zero checksum indicates tests executed)\n");
    
    return checksum == 0 ? 0 : 1;
}
