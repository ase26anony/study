/* Test program to trigger x86 condition code mnemonics for floating-point comparisons */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimization from removing crucial operations */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Global volatile variables to prevent constant folding */
VOLATILE_DOUBLE g_nan = NAN;
VOLATILE_DOUBLE g_inf = INFINITY;
VOLATILE_DOUBLE g_zero = 0.0;
VOLATILE_DOUBLE g_one = 1.0;
VOLATILE_DOUBLE g_two = 2.0;

/* Test 1: Direct unordered comparisons using != and == operators */
NOINLINE int test_unordered_comparisons(void) {
    int results = 0;
    VOLATILE_DOUBLE nan = g_nan;
    VOLATILE_DOUBLE num = g_one;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results |= (nan != num) ? 0x01 : 0;  /* UNORDERED: NaN != 1.0 */
    results |= (nan == nan) ? 0x02 : 0;  /* UNORDERED: NaN == NaN */
    results |= (num == num) ? 0x04 : 0;  /* ORDERED: 1.0 == 1.0 */
    results |= (num != nan) ? 0x08 : 0;  /* ORDERED: 1.0 != NaN */
    
    return results;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int results = 0;
    VOLATILE_DOUBLE nan = g_nan;
    VOLATILE_DOUBLE a = g_one;
    VOLATILE_DOUBLE b = g_two;
    VOLATILE_DOUBLE c = g_zero;
    
    /* These map to various condition codes */
    results |= isunordered(nan, a) ? 0x10 : 0;    /* UNORDERED */
    results |= !isunordered(a, b) ? 0x20 : 0;     /* ORDERED */
    results |= isgreater(a, b) ? 0x40 : 0;        /* UNLE? (not greater) */
    results |= isless(a, b) ? 0x80 : 0;           /* UNGE? (not less) */
    results |= isgreaterequal(b, a) ? 0x100 : 0;  /* UNLT? (not less than) */
    results |= islessequal(a, b) ? 0x200 : 0;     /* UNGT? (not greater than) */
    
    /* UNEQ: unordered or equal */
    results |= (!isgreater(a, c) && !isless(a, c)) ? 0x400 : 0;
    
    return results;
}

/* Test 3: Inline assembly with %C modifier for condition codes */
NOINLINE int test_inline_asm(void) {
    int result = 0;
    VOLATILE_DOUBLE x = g_one;
    VOLATILE_DOUBLE y = g_nan;
    VOLATILE_DOUBLE z = g_two;
    
    /* Test with unordered comparison */
    {
        unsigned char cmp_result;
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(cmp_result)
            : "x"(x), "x"(y)
            : "cc"
        );
        result |= cmp_result ? 0x1000 : 0;
    }
    
    /* Test with ordered comparison */
    {
        unsigned char cmp_result;
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(cmp_result)
            : "x"(x), "x"(z)
            : "cc"
        );
        result |= cmp_result ? 0x2000 : 0;
    }
    
    return result;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double(void) {
    int results = 0;
    VOLATILE_LONG_DOUBLE ld_nan = g_nan;
    VOLATILE_LONG_DOUBLE ld_one = g_one;
    VOLATILE_LONG_DOUBLE ld_two = g_two;
    
    /* x87 style comparisons - may generate different condition codes */
    results |= (ld_nan != ld_one) ? 0x10000 : 0;
    results |= (ld_one == ld_one) ? 0x20000 : 0;
    results |= (ld_one < ld_two) ? 0x40000 : 0;
    results |= (ld_two > ld_one) ? 0x80000 : 0;
    
    /* Explicit unordered check */
    results |= isunordered(ld_nan, ld_one) ? 0x100000 : 0;
    
    return results;
}

/* Test 5: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    VOLATILE_DOUBLE arr1[8];
    VOLATILE_DOUBLE arr2[8];
    int counts[8] = {0};
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : g_nan;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : g_nan;
    }
    
    /* Perform various comparisons in loop */
    for (int i = 0; i < 8; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;      /* UNORDERED */
        counts[1] += !isunordered(arr1[i], arr2[i]) ? 1 : 0;     /* ORDERED */
        counts[2] += (arr1[i] == arr2[i]) ? 1 : 0;               /* UNEQ? */
        counts[3] += (arr1[i] != arr2[i]) ? 1 : 0;               /* LTGT? */
        counts[4] += isgreater(arr1[i], arr2[i]) ? 1 : 0;        /* UNLE? */
        counts[5] += isless(arr1[i], arr2[i]) ? 1 : 0;           /* UNGE? */
        counts[6] += isgreaterequal(arr1[i], arr2[i]) ? 1 : 0;   /* UNLT? */
        counts[7] += islessequal(arr1[i], arr2[i]) ? 1 : 0;      /* UNGT? */
    }
    
    /* Combine results */
    int result = 0;
    for (int i = 0; i < 8; i++) {
        result ^= (counts[i] << (i * 3));
    }
    
    return result;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE a = g_one;
    VOLATILE_DOUBLE b = g_nan;
    VOLATILE_DOUBLE c = g_two;
    int result = 0;
    
    /* Force multiple comparison types */
    if (isunordered(a, b)) {
        result |= 0x01;  /* UNORDERED */
    }
    
    if (!isunordered(a, c)) {
        result |= 0x02;  /* ORDERED */
    }
    
    if (a == c) {
        result |= 0x04;  /* UNEQ */
    }
    
    if (a != b) {
        result |= 0x08;  /* LTGT */
    }
    
    if (a > c) {
        result |= 0x10;  /* UNLE */
    }
    
    if (a < c) {
        result |= 0x20;  /* UNGE */
    }
    
    if (a >= c) {
        result |= 0x40;  /* UNLT */
    }
    
    if (a <= c) {
        result |= 0x80;  /* UNGT */
    }
    
    return result;
}

/* Test 7: Using GCC builtins for direct SSE2 unordered compares */
NOINLINE int test_gcc_builtins(void) {
    int result = 0;
    double d1 = g_one;
    double d2 = g_nan;
    double d3 = g_two;
    
    /* Use GCC's x86 specific builtins */
    int cmp1 = __builtin_ia32_ucomisd(d1, d2);
    int cmp2 = __builtin_ia32_ucomisd(d1, d3);
    int cmp3 = __builtin_ia32_ucomisd(d3, d3);
    
    /* Extract condition code bits */
    result |= (cmp1 & 0x40) ? 0x1000000 : 0;  /* ZF: equal */
    result |= (cmp1 & 0x01) ? 0x2000000 : 0;  /* CF: less than */
    result |= (cmp1 & 0x44) == 0x44 ? 0x4000000 : 0;  /* Unordered */
    
    result |= (cmp2 & 0x40) ? 0x8000000 : 0;
    result |= (cmp2 & 0x01) ? 0x10000000 : 0;
    
    result |= (cmp3 & 0x40) ? 0x20000000 : 0;
    result |= (cmp3 & 0x01) ? 0x40000000 : 0;
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests to trigger various condition codes */
    checksum ^= test_unordered_comparisons();
    checksum ^= test_math_macros();
    checksum ^= test_inline_asm();
    checksum ^= test_long_double();
    checksum ^= test_array_comparisons();
    checksum ^= test_switch_comparisons();
    checksum ^= test_gcc_builtins();
    
    printf("Final checksum: %d (0x%08x)\n", checksum, checksum);
    
    /* Use results to prevent dead code elimination */
    if (checksum == 0x12345678) {  /* Unlikely value */
        printf("Impossible!\n");
    }
    
    return 0;
}
