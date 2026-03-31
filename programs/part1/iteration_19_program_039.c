/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))

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

/* Test 1: Direct unordered comparisons using != and == operators */
NOINLINE int test_unordered_ordered(void) {
    int results[8] = {0};
    volatile double d1, d2;
    
    /* UNORDERED: Compare NaN with anything using != */
    d1 = g_nan; d2 = g_one;
    results[0] = (d1 != d2) ? 1 : 0;  /* Should be unordered */
    
    /* ORDERED: Compare NaN with anything using == */
    d1 = g_nan; d2 = g_one;
    results[1] = (d1 == d2) ? 1 : 0;  /* Should be ordered (false) */
    
    /* More unordered tests */
    d1 = g_one; d2 = g_nan;
    results[2] = (d1 != d2) ? 1 : 0;
    
    d1 = g_nan; d2 = g_nan;
    results[3] = (d1 != d2) ? 1 : 0;
    
    /* Test with infinity */
    d1 = g_inf; d2 = g_neg_inf;
    results[4] = (d1 != d2) ? 1 : 0;
    
    d1 = g_inf; d2 = g_inf;
    results[5] = (d1 == d2) ? 1 : 0;
    
    /* Mixed tests */
    d1 = g_zero; d2 = g_nan;
    results[6] = (d1 == d2) ? 1 : 0;
    
    d1 = g_nan; d2 = g_zero;
    results[7] = (d1 != d2) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int results[12] = {0};
    volatile double a, b;
    
    /* UNEQ: unordered or equal */
    a = g_nan; b = g_one;
    results[0] = !isunordered(a, b) && (a == b) ? 1 : 0;
    
    a = g_one; b = g_one;
    results[1] = !isunordered(a, b) && (a == b) ? 1 : 0;
    
    /* UNGE: unordered or greater or equal (not less than) */
    a = g_nan; b = g_one;
    results[2] = isunordered(a, b) || (a >= b) ? 1 : 0;
    
    a = g_one; b = g_zero;
    results[3] = isunordered(a, b) || (a >= b) ? 1 : 0;
    
    /* UNGT: unordered or greater (not less or equal) */
    a = g_nan; b = g_one;
    results[4] = isunordered(a, b) || (a > b) ? 1 : 0;
    
    a = g_one; b = g_zero;
    results[5] = isunordered(a, b) || (a > b) ? 1 : 0;
    
    /* UNLE: unordered or less or equal */
    a = g_nan; b = g_one;
    results[6] = isunordered(a, b) || (a <= b) ? 1 : 0;
    
    a = g_zero; b = g_one;
    results[7] = isunordered(a, b) || (a <= b) ? 1 : 0;
    
    /* UNLT: unordered or less than */
    a = g_nan; b = g_one;
    results[8] = isunordered(a, b) || (a < b) ? 1 : 0;
    
    a = g_zero; b = g_one;
    results[9] = isunordered(a, b) || (a < b) ? 1 : 0;
    
    /* LTGT: less or greater (unordered or equal excluded) */
    a = g_one; b = g_zero;
    results[10] = (a < b) || (a > b) ? 1 : 0;
    
    a = g_one; b = g_one;
    results[11] = (a < b) || (a > b) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 12; i++) sum += results[i];
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    int results[8] = {0};
    volatile double x, y;
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < 8; i++) {
        switch (i) {
            case 0: x = g_nan; y = g_one; break;
            case 1: x = g_one; y = g_nan; break;
            case 2: x = g_one; y = g_zero; break;
            case 3: x = g_zero; y = g_one; break;
            case 4: x = g_one; y = g_one; break;
            case 5: x = g_inf; y = g_inf; break;
            case 6: x = g_inf; y = g_neg_inf; break;
            case 7: x = g_neg_one; y = g_one; break;
        }
        
        unsigned char result;
        /* Use %C to get condition code name */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(result)
            : "x"(x), "x"(y)
            : "cc"
        );
        results[i] = result;
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 4: Array operations with unordered comparisons */
NOINLINE int test_array_operations(void) {
    volatile double arr1[16], arr2[16];
    int counts[6] = {0};
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 16; i++) {
        switch (i % 4) {
            case 0: arr1[i] = (double)i; arr2[i] = (double)(i * 2); break;
            case 1: arr1[i] = g_nan; arr2[i] = (double)i; break;
            case 2: arr1[i] = (double)i; arr2[i] = g_nan; break;
            case 3: arr1[i] = g_nan; arr2[i] = g_nan; break;
        }
    }
    
    /* Count various comparison results */
    for (int i = 0; i < 16; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;
        counts[1] += !isunordered(arr1[i], arr2[i]) ? 1 : 0;
        counts[2] += isgreater(arr1[i], arr2[i]) ? 1 : 0;
        counts[3] += isless(arr1[i], arr2[i]) ? 1 : 0;
        counts[4] += isgreaterequal(arr1[i], arr2[i]) ? 1 : 0;
        counts[5] += islessequal(arr1[i], arr2[i]) ? 1 : 0;
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) sum += counts[i];
    return sum;
}

/* Test 5: Long double (x87) operations */
NOINLINE int test_long_double(void) {
    volatile long double a, b, c;
    int results[10] = {0};
    
    /* Various long double comparisons */
    a = ld_nan; b = ld_one;
    results[0] = (a != b) ? 1 : 0;
    results[1] = (a == b) ? 1 : 0;
    
    a = ld_one; b = ld_zero;
    results[2] = (a > b) ? 1 : 0;
    results[3] = (a < b) ? 1 : 0;
    results[4] = (a >= b) ? 1 : 0;
    results[5] = (a <= b) ? 1 : 0;
    
    a = ld_one; b = ld_one;
    results[6] = (a == b) ? 1 : 0;
    results[7] = (a != b) ? 1 : 0;
    
    a = ld_inf; b = ld_one;
    results[8] = (a > b) ? 1 : 0;
    
    a = ld_nan; b = ld_nan;
    results[9] = (a == b) ? 1 : 0;
    
    /* Force x87 usage with long double arithmetic */
    c = a * b + a / b - a;
    volatile int dummy = (int)(c * 100.0L);
    (void)dummy;
    
    int sum = 0;
    for (int i = 0; i < 10; i++) sum += results[i];
    return sum;
}

/* Test 6: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    int result = 0;
    
    /* Complex switch to force multiple condition codes */
    int cmp_result = 0;
    if (isunordered(x, y)) cmp_result = 1;
    else if (x == y) cmp_result = 2;
    else if (x > y) cmp_result = 3;
    else if (x < y) cmp_result = 4;
    else cmp_result = 5;
    
    switch (cmp_result) {
        case 1: result = 100; break;  /* unordered */
        case 2: result = 200; break;  /* equal */
        case 3: result = 300; break;  /* greater */
        case 4: result = 400; break;  /* less */
        default: result = 500; break;
    }
    
    /* More comparisons in switch */
    volatile double a = g_zero, b = g_one;
    if (!isunordered(a, b) && a != b && !(a > b)) {
        result += 50;  /* less */
    }
    
    return result;
}

/* Test 7: GCC builtins for direct comparison */
NOINLINE int test_gcc_builtins(void) {
    volatile double a = g_nan, b = g_one;
    volatile double c = g_one, d = g_zero;
    int results[4] = {0};
    
    /* Use GCC's IA32 builtins */
    results[0] = __builtin_ia32_ucomisd(a, b);
    results[1] = __builtin_ia32_ucomisd(c, d);
    results[2] = __builtin_ia32_ucomisd(d, c);
    results[3] = __builtin_ia32_ucomisd(c, c);
    
    /* Force use of results */
    int sum = 0;
    for (int i = 0; i < 4; i++) sum += results[i];
    
    /* Conditional move based on comparison */
    volatile int cmov_result;
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "cmovc %3, %0"
        : "=r"(cmov_result)
        : "x"(a), "x"(b), "r"(42)
        : "cc"
    );
    
    return sum + cmov_result;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing x86 condition code generation...\n");
    
    /* Run all tests */
    checksum += test_unordered_ordered();
    checksum += test_math_macros();
    checksum += test_inline_asm();
    checksum += test_array_operations();
    checksum += test_long_double();
    checksum += test_switch_comparisons();
    checksum += test_gcc_builtins();
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional volatile operations to prevent dead code elimination */
    volatile double final_check = g_nan * g_one + g_zero / g_nan;
    (void)final_check;
    
    return 0;
}
