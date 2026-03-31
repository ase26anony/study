/* Test program to trigger x86 condition code printing logic */
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
    
    int result = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    if (a != b) result |= 1;      /* UNORDERED for NaN != 1.0 */
    if (c == d) result |= 2;      /* UNORDERED for 0.0 == NaN */
    if (!(a == b)) result |= 4;   /* ORDERED for !(NaN == 1.0) */
    if (!(c != d)) result |= 8;   /* ORDERED for !(0.0 != NaN) */
    
    return result;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = 2.0;
    volatile double d = 3.0;
    
    int result = 0;
    
    /* These use different condition codes */
    if (isunordered(a, b)) result |= 1;    /* UNORDERED */
    if (isgreater(c, d)) result |= 2;      /* UNGT? Actually GT */
    if (isless(c, d)) result |= 4;         /* UNLT? Actually LT */
    if (isgreaterequal(c, d)) result |= 8; /* UNGE? Actually GE */
    if (islessequal(c, d)) result |= 16;   /* UNLE? Actually LE */
    
    /* UNEQ: unordered or equal */
    if (!isgreater(c, c) && !isless(c, c)) result |= 32;
    
    return result;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = 1.0;
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Test UNORDERED condition code */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* Test ORDERED condition code */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    /* Test UNEQ condition code */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "x"(c), "x"(c)
        : "cc"
    );
    
    return (result1 << 0) | (result2 << 8) | (result3 << 16);
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double(void) {
    volatile long double a = g_nan;
    volatile long double b = 1.0L;
    volatile long double c = 0.0L;
    volatile long double d = g_nan;
    
    int result = 0;
    
    /* x87 comparisons */
    if (a != b) result |= 1;
    if (c == d) result |= 2;
    if (a > b) result |= 4;
    if (c < d) result |= 8;
    if (a >= b) result |= 16;
    if (c <= d) result |= 32;
    
    return result;
}

/* Test 5: Array operations with mixed comparisons */
NOINLINE int test_array_operations(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : g_nan;
        arr2[i] = (i % 3 == 0) ? g_nan : (double)(i * 2);
    }
    
    int counts[6] = {0};
    
    /* Count various comparison results */
    for (int i = 0; i < 8; i++) {
        if (isunordered(arr1[i], arr2[i])) counts[0]++;  /* UNORDERED */
        if (isgreater(arr1[i], arr2[i])) counts[1]++;    /* UNGT? */
        if (isless(arr1[i], arr2[i])) counts[2]++;       /* UNLT? */
        if (isgreaterequal(arr1[i], arr2[i])) counts[3]++; /* UNGE? */
        if (islessequal(arr1[i], arr2[i])) counts[4]++;  /* UNLE? */
        if (!isgreater(arr1[i], arr2[i]) && !isless(arr1[i], arr2[i])) 
            counts[5]++;  /* UNEQ */
    }
    
    /* Combine counts into a single hash */
    int result = 0;
    for (int i = 0; i < 6; i++) {
        result = (result * 31) + counts[i];
    }
    
    return result;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    
    int result = 0;
    
    /* Force multiple comparison types */
    if (a != b) {
        switch (fpclassify(a)) {
            case FP_NAN: result |= 1; break;
            case FP_INFINITE: result |= 2; break;
            case FP_ZERO: result |= 4; break;
            case FP_SUBNORMAL: result |= 8; break;
            case FP_NORMAL: result |= 16; break;
        }
    }
    
    if (c == b) {
        result |= 32;
    } else if (c < b) {
        result |= 64;
    } else if (c > b) {
        result |= 128;
    }
    
    /* Additional unordered checks */
    if (isunordered(a, c)) {
        result |= 256;
    }
    
    return result;
}

/* Test 7: Mixed SSE and x87 operations */
NOINLINE int test_mixed_operations(void) {
    volatile double d1 = g_nan;
    volatile double d2 = 2.0;
    volatile float f1 = NAN;
    volatile float f2 = 3.0f;
    volatile long double ld1 = g_nan;
    volatile long double ld2 = 4.0L;
    
    int result = 0;
    
    /* SSE2 double comparison */
    if (d1 != d2) result |= 1;
    
    /* SSE float comparison */
    if (f1 == f2) result |= 2;
    
    /* x87 long double comparison */
    if (ld1 > ld2) result |= 4;
    
    /* Mixed type comparison */
    if ((double)ld1 != d1) result |= 8;
    
    /* Complex expression */
    if ((d1 != d2) && (f1 == f2) && (ld1 > ld2)) {
        result |= 16;
    }
    
    return result;
}

/* Test 8: Direct builtin usage */
NOINLINE int test_builtins(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    
    int result = 0;
    
    /* Use GCC x86 builtins */
    int cmp1 = __builtin_ia32_ucomisd(a, b);
    int cmp2 = __builtin_ia32_ucomisd(b, b);
    int cmp3 = __builtin_ia32_ucomisd(g_zero, g_one);
    
    /* Check comparison results */
    if (cmp1 & 0x40) result |= 1;  /* Unordered flag */
    if (cmp1 & 0x01) result |= 2;  /* Parity flag */
    if (cmp2 & 0x40) result |= 4;
    if (cmp3 & 0x41) result |= 8;  /* Less than */
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int checksum = 0;
    
    printf("Running condition code tests...\n");
    
    /* Run all tests and combine results */
    checksum ^= test_unordered_comparisons();
    checksum ^= test_math_macros() << 1;
    checksum ^= test_inline_asm() << 2;
    checksum ^= test_long_double() << 3;
    checksum ^= test_array_operations() << 4;
    checksum ^= test_switch_comparisons() << 5;
    checksum ^= test_mixed_operations() << 6;
    checksum ^= test_builtins() << 7;
    
    printf("Final checksum: %d\n", checksum);
    printf("Hex: 0x%08x\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
