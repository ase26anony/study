/* test_float_conds.c - Generate x86 floating-point condition codes */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Global volatile variables to prevent constant folding */
VOLATILE_DOUBLE g_nan = NAN;
VOLATILE_DOUBLE g_inf = INFINITY;
VOLATILE_DOUBLE g_neg_inf = -INFINITY;
VOLATILE_DOUBLE g_zero = 0.0;
VOLATILE_DOUBLE g_one = 1.0;
VOLATILE_DOUBLE g_neg_one = -1.0;

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    int results[8] = {0};
    VOLATILE_DOUBLE nan = g_nan;
    VOLATILE_DOUBLE val = g_one;
    
    /* UNORDERED: NaN != NaN (unordered) */
    results[0] = (nan != nan) ? 1 : 0;
    
    /* ORDERED: NaN == NaN (ordered) - should be false */
    results[1] = (nan == nan) ? 1 : 0;
    
    /* Various unordered comparisons */
    results[2] = (nan != val) ? 1 : 0;  /* UNORDERED */
    results[3] = (val != nan) ? 1 : 0;  /* UNORDERED */
    
    /* Test with INFINITY */
    VOLATILE_DOUBLE inf = g_inf;
    results[4] = (inf != inf) ? 1 : 0;  /* Should be 0 (ordered) */
    results[5] = (inf == inf) ? 1 : 0;  /* Should be 1 (ordered) */
    
    /* Mixed comparisons */
    results[6] = (nan < val) ? 1 : 0;   /* UNORDERED comparison */
    results[7] = (val > nan) ? 1 : 0;   /* UNORDERED comparison */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int results[12] = {0};
    VOLATILE_DOUBLE nan = g_nan;
    VOLATILE_DOUBLE a = g_one;
    VOLATILE_DOUBLE b = g_neg_one;
    VOLATILE_DOUBLE zero = g_zero;
    
    /* isunordered - UNORDERED */
    results[0] = isunordered(nan, a);
    results[1] = isunordered(a, nan);
    results[2] = isunordered(nan, nan);
    
    /* isgreater - UNLE? Actually generates GT condition */
    results[3] = isgreater(a, b);    /* 1 > -1 = true */
    results[4] = isgreater(b, a);    /* -1 > 1 = false */
    results[5] = isgreater(nan, a);  /* false (unordered) */
    
    /* isless - UNGE? Actually generates LT condition */
    results[6] = isless(b, a);       /* -1 < 1 = true */
    results[7] = isless(a, b);       /* 1 < -1 = false */
    results[8] = isless(nan, a);     /* false (unordered) */
    
    /* islessgreater - LTGT */
    results[9] = islessgreater(a, b);   /* 1 != -1 = true */
    results[10] = islessgreater(a, a);  /* 1 != 1 = false */
    results[11] = islessgreater(nan, a); /* false (unordered) */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    int results[6] = {0};
    VOLATILE_DOUBLE x = g_one;
    VOLATILE_DOUBLE y = g_neg_one;
    VOLATILE_DOUBLE nan = g_nan;
    
    /* Test with regular values */
    {
        int result;
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(result)
            : "x"(x), "x"(y)
            : "cc"
        );
        results[0] = result;  /* Should set based on comparison */
    }
    
    /* Test with NaN (unordered) */
    {
        int result;
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(result)
            : "x"(nan), "x"(x)
            : "cc"
        );
        results[1] = result;
    }
    
    /* Test with two NaNs */
    {
        int result;
        VOLATILE_DOUBLE nan2 = g_nan;
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(result)
            : "x"(nan), "x"(nan2)
            : "cc"
        );
        results[2] = result;
    }
    
    /* Test different condition codes explicitly */
    {
        int result_a, result_b;
        /* Test for UNORDERED */
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "set%Cu %0"
            : "=r"(result_a)
            : "x"(nan), "x"(x)
            : "cc"
        );
        
        /* Test for ORDERED */
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "set%Co %0"
            : "=r"(result_b)
            : "x"(x), "x"(y)
            : "cc"
        );
        
        results[3] = result_a;
        results[4] = result_b;
    }
    
    /* Test with ucomisd (unordered compare) */
    {
        int result;
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(result)
            : "x"(nan), "x"(x)
            : "cc"
        );
        results[5] = result;
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double(void) {
    int results[8] = {0};
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_one = 1.0L;
    VOLATILE_LONG_DOUBLE ld_neg_one = -1.0L;
    VOLATILE_LONG_DOUBLE ld_zero = 0.0L;
    
    /* Direct comparisons with long double */
    results[0] = (ld_nan != ld_nan) ? 1 : 0;      /* UNORDERED */
    results[1] = (ld_one != ld_nan) ? 1 : 0;      /* UNORDERED */
    results[2] = (ld_one > ld_neg_one) ? 1 : 0;   /* Ordered GT */
    results[3] = (ld_neg_one < ld_one) ? 1 : 0;   /* Ordered LT */
    
    /* Complex expression that might generate UNEQ, UNGE, etc. */
    VOLATILE_LONG_DOUBLE a = ld_one;
    VOLATILE_LONG_DOUBLE b = ld_neg_one;
    VOLATILE_LONG_DOUBLE c = ld_zero;
    
    /* Chain of comparisons */
    results[4] = (a >= b) ? 1 : 0;      /* GE */
    results[5] = (b <= a) ? 1 : 0;      /* LE */
    
    /* Compare with NaN */
    results[6] = (a >= ld_nan) ? 1 : 0;  /* UNORDERED - should be false */
    results[7] = (ld_nan <= b) ? 1 : 0;  /* UNORDERED - should be false */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 5: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    VOLATILE_DOUBLE arr1[8];
    VOLATILE_DOUBLE arr2[8];
    int counts[6] = {0};
    
    /* Initialize arrays with mix of values */
    arr1[0] = g_nan;
    arr2[0] = g_one;
    
    arr1[1] = g_one;
    arr2[1] = g_nan;
    
    arr1[2] = g_nan;
    arr2[2] = g_nan;
    
    arr1[3] = g_one;
    arr2[3] = g_neg_one;
    
    arr1[4] = g_neg_one;
    arr2[4] = g_one;
    
    arr1[5] = g_zero;
    arr2[5] = g_zero;
    
    arr1[6] = g_inf;
    arr2[6] = g_inf;
    
    arr1[7] = g_inf;
    arr2[7] = g_neg_inf;
    
    /* Count various comparison results */
    for (int i = 0; i < 8; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]);      /* UNORDERED */
        counts[1] += !isunordered(arr1[i], arr2[i]);     /* ORDERED */
        counts[2] += (arr1[i] == arr2[i]) ? 1 : 0;       /* EQ */
        counts[3] += (arr1[i] > arr2[i]) ? 1 : 0;        /* GT */
        counts[4] += (arr1[i] < arr2[i]) ? 1 : 0;        /* LT */
        counts[5] += (arr1[i] != arr2[i]) ? 1 : 0;       /* NEQ/UNEQ */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 6: Switch based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE a = g_one;
    VOLATILE_DOUBLE b = g_neg_one;
    VOLATILE_DOUBLE nan = g_nan;
    
    int result = 0;
    
    /* Multiple comparisons in switch-like logic */
    if (isunordered(a, nan)) {
        result |= 1;    /* UNORDERED */
    }
    if (!isunordered(a, b)) {
        result |= 2;    /* ORDERED */
    }
    if (a == b) {
        result |= 4;    /* EQ */
    }
    if (a != b) {
        result |= 8;    /* NEQ/UNEQ */
    }
    if (a > b) {
        result |= 16;   /* GT */
    }
    if (a < b) {
        result |= 32;   /* LT */
    }
    if (isgreater(a, b)) {
        result |= 64;   /* Greater (not less or equal) */
    }
    if (isless(a, b)) {
        result |= 128;  /* Less (not greater or equal) */
    }
    
    /* Test with NaN */
    if (isunordered(nan, a)) {
        result |= 256;  /* UNORDERED with NaN */
    }
    if (nan == nan) {   /* Should be false */
        result |= 512;
    }
    if (nan != nan) {   /* Should be true (unordered) */
        result |= 1024;
    }
    
    return result;
}

/* Test 7: GCC builtins for direct FP comparisons */
NOINLINE int test_gcc_builtins(void) {
    int results[4] = {0};
    double a = g_one;
    double b = g_neg_one;
    double nan = g_nan;
    
    /* Use GCC's x86-specific builtins */
    results[0] = __builtin_ia32_ucomisd(a, b);   /* Unordered compare */
    results[1] = __builtin_ia32_ucomisd(nan, a); /* With NaN */
    results[2] = __builtin_ia32_ucomisd(a, nan); /* With NaN reversed */
    results[3] = __builtin_ia32_ucomisd(nan, nan); /* NaN vs NaN */
    
    /* The builtin returns an int with flags, check specific conditions */
    int sum = 0;
    sum += (results[0] & 0x40) ? 1 : 0;  /* ZF flag */
    sum += (results[1] & 0x40) ? 2 : 0;  /* ZF flag with NaN */
    sum += (results[2] & 0x40) ? 4 : 0;  /* ZF flag */
    sum += (results[3] & 0x40) ? 8 : 0;  /* ZF flag NaN vs NaN */
    
    return sum;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_long_double();
    total += test_array_comparisons();
    total += test_switch_comparisons();
    total += test_gcc_builtins();
    
    printf("Total checksum: %d\n", total);
    
    /* Also test some direct prints to see condition codes in asm */
    VOLATILE_DOUBLE x = 1.5;
    VOLATILE_DOUBLE y = 2.5;
    VOLATILE_DOUBLE nan = NAN;
    
    /* Force generation of various condition codes */
    if (x < y) total++;
    if (x > y) total--;
    if (nan == nan) total += 2;      /* Should be false */
    if (nan != nan) total += 3;      /* Should be true (unordered) */
    if (isunordered(x, nan)) total += 4;
    if (!isunordered(x, y)) total += 5;
    
    printf("Final result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
