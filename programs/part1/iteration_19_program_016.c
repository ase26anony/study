/* test_float_conds.c - Trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg = -1.0;

/* Test function 1: Direct unordered comparisons using != and == operators */
NOINLINE int test_unordered_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_nan;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b) ? 1 : 0;  /* UNORDERED for NaN != 1.0 */
    results[1] = (a == a) ? 1 : 0;  /* ORDERED for NaN == NaN? Actually false */
    results[2] = (b == c) ? 1 : 0;  /* Regular compare */
    results[3] = (a != a) ? 1 : 0;  /* UNORDERED: NaN != NaN is true! */
    
    /* Mix with regular comparisons */
    results[4] = (b < c) ? 1 : 0;
    results[5] = (b > c) ? 1 : 0;
    results[6] = (b <= c) ? 1 : 0;
    results[7] = (b >= c) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test function 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    volatile double arr1[4] = {g_nan, g_one, g_inf, g_zero};
    volatile double arr2[4] = {g_one, g_nan, g_neg, g_inf};
    
    int results[24] = {0};
    int idx = 0;
    
    for (int i = 0; i < 4; i++) {
        /* These macros handle NaN correctly */
        results[idx++] = isunordered(arr1[i], arr2[i]) ? 1 : 0;  /* UNORDERED */
        results[idx++] = isgreater(arr1[i], arr2[i]) ? 1 : 0;    /* UNLE? Actually GT */
        results[idx++] = isless(arr1[i], arr2[i]) ? 1 : 0;       /* UNGE? Actually LT */
        results[idx++] = isgreaterequal(arr1[i], arr2[i]) ? 1 : 0; /* UNLT? Actually GE */
        results[idx++] = islessequal(arr1[i], arr2[i]) ? 1 : 0;  /* UNGT? Actually LE */
        
        /* Direct comparisons that might generate UNEQ/LTGT */
        results[idx++] = (arr1[i] == arr2[i]) ? 1 : 0;  /* Might generate UNEQ */
    }
    
    int sum = 0;
    for (int i = 0; i < idx; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test function 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    unsigned char results[8] = {0};
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < 2; i++) {
        volatile double x = (i == 0) ? a : b;
        volatile double y = (i == 0) ? c : d;
        
        /* Using x87 floating-point compare */
        __asm__ volatile (
            "fldl %2\n\t"
            "fldl %3\n\t"
            "fucomip %%st(1), %%st\n\t"
            "set%C0 %0\n\t"
            "fstp %%st(0)"
            : "=r"(results[i*4 + 0])
            : "0"(0), "m"(x), "m"(y)
            : "cc", "st"
        );
        
        /* Test different condition codes */
        __asm__ volatile (
            "movsd %2, %%xmm0\n\t"
            "movsd %3, %%xmm1\n\t"
            "ucomisd %%xmm1, %%xmm0\n\t"
            "set%C1 %0"
            : "=r"(results[i*4 + 1])
            : "C"(i == 0 ? 5 : 6), "m"(x), "m"(y)  /* 5=PNZ? Actually need proper codes */
            : "cc", "xmm0", "xmm1"
        );
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test function 4: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    volatile long double a = g_nan;
    volatile long double b = 1.0L;
    volatile long double c = 0.0L;
    volatile long double d = g_inf;
    
    int results = 0;
    
    /* x87 long double comparisons */
    if (a != b) results += 1;      /* UNORDERED */
    if (b > c) results += 2;       /* Regular compare */
    if (!(a == a)) results += 4;   /* UNORDERED: NaN == NaN is false */
    if (isunordered(a, c)) results += 8;  /* UNORDERED */
    
    /* Complex expression to force multiple condition codes */
    volatile long double x = a * b + c;
    volatile long double y = b / c;  /* Infinity */
    
    if (x != y) results += 16;
    if (x < y) results += 32;
    if (x > y) results += 64;
    if (x == y) results += 128;
    
    return results;
}

/* Test function 5: Array processing with mixed comparisons */
NOINLINE int test_array_processing(void) {
    volatile double arr1[16];
    volatile double arr2[16];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 16; i++) {
        if (i % 5 == 0) {
            arr1[i] = g_nan;
            arr2[i] = i * 0.1;
        } else if (i % 3 == 0) {
            arr1[i] = i * 0.2;
            arr2[i] = g_nan;
        } else if (i % 7 == 0) {
            arr1[i] = g_inf;
            arr2[i] = -g_inf;
        } else {
            arr1[i] = i * 0.3;
            arr2[i] = (15 - i) * 0.3;
        }
    }
    
    int counts[6] = {0};
    
    for (int i = 0; i < 16; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;  /* UNORDERED */
        counts[1] += isgreater(arr1[i], arr2[i]) ? 1 : 0;    /* UNLE */
        counts[2] += isless(arr1[i], arr2[i]) ? 1 : 0;       /* UNGE */
        counts[3] += (arr1[i] == arr2[i]) ? 1 : 0;           /* UNEQ */
        counts[4] += (arr1[i] != arr2[i]) ? 1 : 0;           /* LTGT? */
        counts[5] += (arr1[i] < arr2[i]) ? 1 : 0;            /* UNGE */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum = sum * 31 + counts[i];
    }
    return sum;
}

/* Test function 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int result = 0;
    
    /* Multiple comparisons in switch-like logic */
    if (isunordered(a, b)) {
        result |= 0x01;  /* UNORDERED */
    }
    if (!isunordered(b, c)) {
        result |= 0x02;  /* ORDERED */
    }
    if (isgreater(b, c)) {
        result |= 0x04;  /* UNLE */
    }
    if (isless(c, b)) {
        result |= 0x08;  /* UNGE */
    }
    if (isgreaterequal(d, b)) {
        result |= 0x10;  /* UNLT */
    }
    if (islessequal(b, d)) {
        result |= 0x20;  /* UNGT */
    }
    
    /* Additional comparisons for other condition codes */
    volatile double x = a + b;  /* NaN */
    volatile double y = c * d;  /* 0 * inf = NaN */
    
    if (x == y) {
        result |= 0x40;  /* UNEQ */
    }
    if (x != y) {
        result |= 0x80;  /* LTGT */
    }
    
    return result;
}

/* Test function 7: Using GCC builtins for direct SSE2 unordered compares */
NOINLINE int test_sse2_builtins(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int results = 0;
    
    /* Use GCC's x86 intrinsics */
    __attribute__((unused)) int cmp1 = __builtin_ia32_ucomisd(a, b);
    __attribute__((unused)) int cmp2 = __builtin_ia32_ucomisd(b, c);
    __attribute__((unused)) int cmp3 = __builtin_ia32_ucomisd(a, a);
    __attribute__((unused)) int cmp4 = __builtin_ia32_ucomisd(d, c);
    
    /* Force use of condition codes through conditional moves */
    double r1 = (a != b) ? 1.0 : 0.0;
    double r2 = (b > c) ? 2.0 : 0.0;
    double r3 = (a == a) ? 3.0 : 0.0;  /* Always false for NaN */
    double r4 = (d < c) ? 4.0 : 0.0;
    
    results = (int)(r1 + r2 + r3 + r4);
    
    return results;
}

/* Main function that runs all tests */
int main(void) {
    printf("Testing x86 floating-point condition code generation...\n");
    
    int total = 0;
    
    total += test_unordered_comparisons();
    printf("Test 1 result: %d\n", test_unordered_comparisons());
    
    total += test_math_macros();
    printf("Test 2 result: %d\n", test_math_macros());
    
    total += test_inline_asm();
    printf("Test 3 result: %d\n", test_inline_asm());
    
    total += test_long_double();
    printf("Test 4 result: %d\n", test_long_double());
    
    total += test_array_processing();
    printf("Test 5 result: %d\n", test_array_processing());
    
    total += test_switch_comparisons();
    printf("Test 6 result: %d\n", test_switch_comparisons());
    
    total += test_sse2_builtins();
    printf("Test 7 result: %d\n", test_sse2_builtins());
    
    printf("Total checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
