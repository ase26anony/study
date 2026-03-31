/* Test program to trigger x86 condition code printing for floating-point comparisons */
#include <stdio.h>
#include <math.h>
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

/* Test 1: Direct unordered comparisons using != and == operators */
NOINLINE int test_unordered_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_nan;
    
    int result = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    if (a != b) result |= 1;      /* UNORDERED for NaN != 1.0 */
    if (c == d) result |= 2;      /* UNORDERED for 0.0 == NaN */
    if (b == c) result |= 4;      /* ORDERED for 1.0 == 0.0 */
    if (a != a) result |= 8;      /* UNORDERED for NaN != NaN */
    
    /* Mixed comparisons to trigger different codes */
    if (!(a > b)) result |= 16;   /* UNORDERED for !(NaN > 1.0) */
    if (!(b < a)) result |= 32;  /* UNORDERED for !(1.0 < NaN) */
    
    return result;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    volatile double nan1 = g_nan;
    volatile double nan2 = g_nan;
    volatile double num1 = g_one;
    volatile double num2 = g_neg_one;
    
    int result = 0;
    
    /* These map to specific condition codes */
    if (isunordered(nan1, num1)) result |= 1;      /* UNORDERED */
    if (isgreater(num1, num2)) result |= 2;        /* UNGT */
    if (isless(num2, num1)) result |= 4;           /* UNLT */
    if (islessequal(num2, num1)) result |= 8;      /* UNLE */
    if (isgreaterequal(num1, num2)) result |= 16;  /* UNGE */
    
    /* UNEQ: unordered or equal */
    if (!islessgreater(nan1, nan2)) result |= 32;  /* UNEQ for NaN vs NaN */
    if (!islessgreater(num1, num1)) result |= 64;  /* UNEQ for 1.0 vs 1.0 */
    
    /* LTGT: less or greater (ordered and not equal) */
    if (islessgreater(num1, num2)) result |= 128;  /* LTGT */
    
    return result;
}

/* Test 3: Inline assembly with %C modifier for condition codes */
NOINLINE int test_inline_asm(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Using x87 floating-point compare */
    __asm__ volatile (
        "fldl %2\n\t"
        "fldl %3\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C0 %0\n\t"
        : "=r"(result1)
        : "m"(a), "m"(b)
        : "cc", "st"
    );
    
    /* SSE2 compare with unordered check */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "ucomisd %%xmm1, %%xmm0\n\t"
        "set%C0 %0\n\t"
        : "=r"(result2)
        : "m"(a), "m"(c)
        : "xmm0", "xmm1", "cc"
    );
    
    /* Ordered compare */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "comisd %%xmm1, %%xmm0\n\t"
        "set%C0 %0\n\t"
        : "=r"(result3)
        : "m"(b), "m"(c)
        : "xmm0", "xmm1", "cc"
    );
    
    return (result1 << 0) | (result2 << 8) | (result3 << 16);
}

/* Test 4: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    int counts[7] = {0}; /* Count for each condition type */
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 3 == 0) ? g_nan : (double)i;
        arr2[i] = (i % 4 == 0) ? g_nan : (double)(7 - i);
    }
    
    /* Perform various comparisons */
    for (int i = 0; i < 8; i++) {
        if (isunordered(arr1[i], arr2[i])) counts[0]++;      /* UNORDERED */
        if (isordered(arr1[i], arr2[i])) counts[1]++;        /* ORDERED */
        if (!islessgreater(arr1[i], arr2[i])) counts[2]++;   /* UNEQ */
        if (!isless(arr1[i], arr2[i])) counts[3]++;          /* UNGE */
        if (!islessequal(arr1[i], arr2[i])) counts[4]++;     /* UNGT */
        if (islessequal(arr1[i], arr2[i])) counts[5]++;      /* UNLE */
        if (isless(arr1[i], arr2[i])) counts[6]++;           /* UNLT */
    }
    
    /* Combine counts into a single checksum */
    int checksum = 0;
    for (int i = 0; i < 7; i++) {
        checksum = (checksum * 31) + counts[i];
    }
    return checksum;
}

/* Test 5: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_zero = 0.0L;
    volatile long double ld_one = 1.0L;
    
    int result = 0;
    
    /* x87 style comparisons */
    if (ld_nan != ld_one) result |= 1;
    if (ld_one == ld_zero) result |= 2;
    if (ld_nan == ld_nan) result |= 4;
    if (ld_one > ld_zero) result |= 8;
    if (ld_zero < ld_one) result |= 16;
    
    /* Complex expression to force multiple condition codes */
    if ((ld_nan < ld_one) || (ld_one > ld_inf)) result |= 32;
    if (!(ld_zero >= ld_nan)) result |= 64;
    if (!(ld_nan <= ld_one)) result |= 128;
    
    return result;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    int result = 0;
    
    /* Force compiler to generate multiple conditional jumps */
    int cmp_result = 0;
    if (isunordered(a, b)) cmp_result = 1;
    else if (isgreater(a, b)) cmp_result = 2;
    else if (isless(a, b)) cmp_result = 3;
    else if (!islessgreater(a, b)) cmp_result = 4;
    
    switch (cmp_result) {
        case 1: result = 100; break;  /* UNORDERED */
        case 2: result = 200; break;  /* UNGT */
        case 3: result = 300; break;  /* UNLT */
        case 4: result = 400; break;  /* UNEQ */
        default: result = 500; break;
    }
    
    /* Another switch with different comparisons */
    if (!isless(b, c)) result += 1000;   /* UNGE */
    if (islessequal(c, b)) result += 2000; /* UNLE */
    
    return result;
}

/* Test 7: Using GCC builtins for direct SSE2 unordered compares */
NOINLINE int test_gcc_builtins(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    int result = 0;
    
    /* Use GCC's x86 intrinsics */
    int cmp1 = __builtin_ia32_ucomisd(a, b);
    int cmp2 = __builtin_ia32_ucomisd(b, c);
    int cmp3 = __builtin_ia32_ucomisd(a, a);
    
    /* Check comparison results */
    if (cmp1 & 0x45) result |= 1;   /* UNORDERED or PF=1 */
    if (cmp2 == 0) result |= 2;     /* ORDERED and equal */
    if ((cmp3 & 0x45) == 0x45) result |= 4; /* UNORDERED */
    
    /* Conditional move based on compare */
    double d = (__builtin_ia32_ucomisd(b, c) & 1) ? g_one : g_zero;
    result += (int)d;
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 condition code generation for floating-point comparisons\n");
    
    /* Run all tests and combine results */
    checksum ^= test_unordered_comparisons();
    checksum ^= test_math_macros() << 1;
    checksum ^= test_inline_asm() << 2;
    checksum ^= test_array_comparisons() << 3;
    checksum ^= test_long_double() << 4;
    checksum ^= test_switch_comparisons() << 5;
    checksum ^= test_gcc_builtins() << 6;
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile double verify = g_nan;
    if (isunordered(verify, verify)) {
        printf("NaN detected (expected)\n");
    }
    
    return checksum != 0 ? 0 : 1;
}
