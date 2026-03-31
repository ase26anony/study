/* Test program to trigger x86 floating-point condition code printing */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations that would remove critical operations */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Global volatile variables to prevent constant folding */
VOLATILE_DOUBLE g_nan = NAN;
VOLATILE_DOUBLE g_inf = INFINITY;
VOLATILE_DOUBLE g_zero = 0.0;
VOLATILE_DOUBLE g_one = 1.0;
VOLATILE_DOUBLE g_two = 2.0;

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    VOLATILE_DOUBLE nan = g_nan;
    VOLATILE_DOUBLE num = g_one;
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan != nan) ? 1 : 0;  /* UNORDERED: NaN != NaN is true */
    results[1] = (nan == nan) ? 1 : 0;  /* ORDERED: NaN == NaN is false */
    results[2] = (num != num) ? 1 : 0;  /* ORDERED: normal != normal is false */
    results[3] = (num == num) ? 1 : 0;  /* ORDERED: normal == normal is true */
    
    /* Mixed comparisons */
    results[4] = (nan != num) ? 1 : 0;
    results[5] = (num != nan) ? 1 : 0;
    results[6] = (nan == num) ? 1 : 0;
    results[7] = (num == nan) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    VOLATILE_DOUBLE a = g_nan;
    VOLATILE_DOUBLE b = g_one;
    VOLATILE_DOUBLE c = g_two;
    VOLATILE_DOUBLE d = g_inf;
    
    int results[12] = {0};
    
    /* These map to specific condition codes */
    results[0] = isunordered(a, b);   /* UNORDERED */
    results[1] = isordered(a, b);     /* ORDERED */
    results[2] = isgreater(b, c);     /* UNLE? Actually generates GT */
    results[3] = isgreaterequal(b, c); /* UNLT? */
    results[4] = isless(b, c);        /* UNGE? */
    results[5] = islessequal(b, c);   /* UNGT? */
    
    /* More complex cases */
    results[6] = !isgreater(b, c) && !isless(b, c) && isordered(b, c);  /* UNEQ? */
    results[7] = isunordered(a, a);   /* UNORDERED */
    results[8] = isordered(b, c);     /* ORDERED */
    
    /* Comparisons with infinity */
    results[9] = isgreater(d, b);     /* d > b (inf > 1) */
    results[10] = isless(b, d);       /* b < d (1 < inf) */
    results[11] = isunordered(d, d);  /* inf is ordered */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    VOLATILE_DOUBLE x = g_one;
    VOLATILE_DOUBLE y = g_two;
    VOLATILE_DOUBLE z = g_nan;
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Test with normal numbers - should generate various condition codes */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(x), "x"(y)
        : "cc"
    );
    
    /* Test with NaN - should trigger UNORDERED */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "x"(z), "x"(x)
        : "cc"
    );
    
    /* Test with two NaNs */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "x"(z), "x"(z)
        : "cc"
    );
    
    return result1 + result2 + result3;
}

/* Test 4: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_one = 1.0L;
    VOLATILE_LONG_DOUBLE ld_two = 2.0L;
    
    int results = 0;
    
    /* x87 comparisons */
    if (ld_nan != ld_nan) results += 1;      /* UNORDERED */
    if (ld_one == ld_one) results += 2;      /* ORDERED */
    if (ld_one < ld_two) results += 4;       /* UNGE? Actually generates LT */
    if (ld_two > ld_one) results += 8;       /* UNLE? Actually generates GT */
    
    /* Complex expression that might generate UNEQ */
    if (!(ld_one > ld_two) && !(ld_one < ld_two) && (ld_one == ld_one)) {
        results += 16;  /* UNEQ case */
    }
    
    return results;
}

/* Test 5: Array operations with mixed comparisons */
NOINLINE int test_array_comparisons(void) {
    VOLATILE_DOUBLE arr1[8];
    VOLATILE_DOUBLE arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        if (i % 4 == 0) {
            arr1[i] = g_nan;
            arr2[i] = (double)i;
        } else if (i % 4 == 1) {
            arr1[i] = (double)i;
            arr2[i] = g_nan;
        } else if (i % 4 == 2) {
            arr1[i] = (double)i;
            arr2[i] = (double)(i * 2);
        } else {
            arr1[i] = (double)i;
            arr2[i] = (double)i;
        }
    }
    
    int counts[6] = {0};
    
    for (int i = 0; i < 8; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;    /* UNORDERED */
        counts[1] += isordered(arr1[i], arr2[i]) ? 1 : 0;      /* ORDERED */
        counts[2] += isgreater(arr1[i], arr2[i]) ? 1 : 0;      /* UNLE? */
        counts[3] += isless(arr1[i], arr2[i]) ? 1 : 0;         /* UNGE? */
        counts[4] += (arr1[i] == arr2[i]) ? 1 : 0;             /* UNEQ? or ORDERED */
        counts[5] += (arr1[i] != arr2[i]) ? 1 : 0;             /* LTGT? or UNORDERED */
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
    VOLATILE_DOUBLE b = g_two;
    VOLATILE_DOUBLE c = g_nan;
    
    int result = 0;
    
    /* Multiple comparisons in switch-like logic */
    if (isunordered(a, b)) {
        result = 1;  /* UNORDERED */
    } else if (isgreater(a, b)) {
        result = 2;  /* UNLE? */
    } else if (isless(a, b)) {
        result = 3;  /* UNGE? */
    } else if (a == b) {
        result = 4;  /* UNEQ? */
    } else {
        result = 5;  /* LTGT? */
    }
    
    /* Another branch with NaN */
    if (isunordered(c, a)) {
        result += 10;  /* UNORDERED */
    }
    
    /* Check for UNGE/UNLE specifically */
    if (!isless(a, b) && isordered(a, b)) {
        result += 100;  /* UNGE */
    }
    
    if (!isgreater(a, b) && isordered(a, b)) {
        result += 200;  /* UNLE */
    }
    
    return result;
}

/* Test 7: Using GCC builtins for direct SSE2 unordered compares */
NOINLINE int test_gcc_builtins(void) {
    double a = g_one;
    double b = g_two;
    double c = g_nan;
    
    int results = 0;
    
    /* Using __builtin_ia32_ucomisd directly */
    int res1 = __builtin_ia32_ucomisd(a, b);
    int res2 = __builtin_ia32_ucomisd(c, a);
    int res3 = __builtin_ia32_ucomisd(c, c);
    
    /* The builtin returns flags, we need to check them */
    if (res1 & 1) results += 1;      /* UNORDERED flag */
    if (res1 & 4) results += 2;      /* Less flag */
    if (res1 & 0x40) results += 4;   /* Equal flag */
    
    if (res2 & 1) results += 8;      /* UNORDERED flag */
    if (res2 & 4) results += 16;     /* Less flag */
    if (res2 & 0x40) results += 32;  /* Equal flag */
    
    if (res3 & 1) results += 64;     /* UNORDERED flag */
    if (res3 & 4) results += 128;    /* Less flag */
    if (res3 & 0x40) results += 256; /* Equal flag */
    
    return results;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Starting floating-point condition code tests...\n");
    
    total += test_unordered_comparisons();
    printf("Test 1 complete: %d\n", total);
    
    total += test_math_macros();
    printf("Test 2 complete: %d\n", total);
    
    total += test_inline_asm();
    printf("Test 3 complete: %d\n", total);
    
    total += test_long_double();
    printf("Test 4 complete: %d\n", total);
    
    total += test_array_comparisons();
    printf("Test 5 complete: %d\n", total);
    
    total += test_switch_comparisons();
    printf("Test 6 complete: %d\n", total);
    
    total += test_gcc_builtins();
    printf("Test 7 complete: %d\n", total);
    
    printf("Final checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("WARNING: All tests returned zero!\n");
    }
    
    return 0;
}
