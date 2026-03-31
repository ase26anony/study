/* Test program to trigger x86 floating-point condition code mnemonics */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing critical code */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Global volatile variables to prevent constant folding */
VOLATILE_DOUBLE g_nan = NAN;
VOLATILE_DOUBLE g_inf = INFINITY;
VOLATILE_DOUBLE g_zero = 0.0;
VOLATILE_DOUBLE g_one = 1.0;
VOLATILE_DOUBLE g_neg_one = -1.0;

/* Test 1: Direct unordered comparisons using != and == operators */
NOINLINE int test_unordered_ordered(void) {
    int results[8] = {0};
    VOLATILE_DOUBLE nan = g_nan;
    VOLATILE_DOUBLE num = g_one;
    
    /* UNORDERED: NaN != NaN should trigger unord */
    results[0] = (nan != nan) ? 1 : 0;
    
    /* ORDERED: 1.0 == 1.0 should trigger ord */
    results[1] = (num == num) ? 1 : 0;
    
    /* More unordered comparisons */
    results[2] = (nan != num) ? 1 : 0;
    results[3] = (num != nan) ? 1 : 0;
    
    /* Ordered comparisons */
    results[4] = (num == g_one) ? 1 : 0;
    results[5] = (g_zero == -g_zero) ? 1 : 0;  /* +0 == -0 */
    
    /* Mixed */
    results[6] = (g_inf == g_inf) ? 1 : 0;
    results[7] = (g_inf != g_inf) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int results[12] = {0};
    VOLATILE_DOUBLE nan = g_nan;
    VOLATILE_DOUBLE a = 2.0;
    VOLATILE_DOUBLE b = 1.0;
    VOLATILE_DOUBLE c = -1.0;
    
    /* UNEQ: unordered or equal */
    results[0] = !isgreater(nan, b) && !isless(nan, b);
    results[1] = !isgreater(a, a) && !isless(a, a);
    
    /* UNGE: unordered or greater or equal (not less than) */
    results[2] = !isless(nan, b);
    results[3] = !isless(a, b);
    results[4] = !isless(a, a);
    
    /* UNGT: unordered or greater (not less or equal) */
    results[5] = !islessequal(nan, b);
    results[6] = !islessequal(a, b);
    
    /* UNLE: unordered or less or equal */
    results[7] = !isgreater(nan, b);
    results[8] = !isgreater(b, a);
    results[9] = !isgreater(a, a);
    
    /* UNLT: unordered or less than */
    results[10] = !isgreaterequal(nan, b);
    results[11] = !isgreaterequal(b, a);
    
    int sum = 0;
    for (int i = 0; i < 12; i++) sum += results[i];
    return sum;
}

/* Test 3: LTGT (not equal and ordered) */
NOINLINE int test_ltgt(void) {
    int results[4] = {0};
    VOLATILE_DOUBLE a = 1.0;
    VOLATILE_DOUBLE b = 2.0;
    VOLATILE_DOUBLE nan = g_nan;
    
    /* LTGT: a != b and both are numbers (not NaN) */
    results[0] = (a != b) && !isunordered(a, b);
    
    /* Not LTGT: equal numbers */
    results[1] = (a != a) && !isunordered(a, a);
    
    /* Not LTGT: unordered */
    results[2] = (nan != a) && !isunordered(nan, a);
    
    /* LTGT: different numbers */
    results[3] = (b != a) && !isunordered(b, a);
    
    int sum = 0;
    for (int i = 0; i < 4; i++) sum += results[i];
    return sum;
}

/* Test 4: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    int results[8] = {0};
    VOLATILE_DOUBLE x, y;
    
    /* Test various comparisons with inline assembly */
    for (int i = 0; i < 4; i++) {
        switch (i) {
            case 0: x = g_nan; y = g_one; break;
            case 1: x = g_one; y = g_nan; break;
            case 2: x = g_one; y = g_one; break;
            case 3: x = g_one; y = g_two; break;
        }
        
        /* Using x87 floating-point compare */
        unsigned char result;
        __asm__ volatile (
            "fldl %2\n\t"
            "fldl %3\n\t"
            "fucomip %%st(1), %%st\n\t"
            "set%C0 %0\n\t"
            "fstp %%st(0)"
            : "=r"(result)
            : "0"(0), "m"(x), "m"(y)
            : "cc", "st"
        );
        results[i] = result;
        
        /* Using SSE2 compare */
        unsigned char sse_result;
        __asm__ volatile (
            "movsd %2, %%xmm0\n\t"
            "movsd %3, %%xmm1\n\t"
            "ucomisd %%xmm0, %%xmm1\n\t"
            "set%C1 %0"
            : "=r"(sse_result)
            : "C" (i), "m"(x), "m"(y)
            : "xmm0", "xmm1", "cc"
        );
        results[i + 4] = sse_result;
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 5: Long double (x87) comparisons */
NOINLINE int test_long_double(void) {
    int results[6] = {0};
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_one = 1.0L;
    VOLATILE_LONG_DOUBLE ld_two = 2.0L;
    VOLATILE_LONG_DOUBLE ld_inf = INFINITY;
    
    /* Various long double comparisons */
    results[0] = (ld_nan != ld_nan);
    results[1] = (ld_one == ld_one);
    results[2] = (ld_one < ld_two);
    results[3] = (ld_two > ld_one);
    results[4] = (ld_inf == ld_inf);
    results[5] = !(ld_nan == ld_one);
    
    /* Force x87 usage with explicit operations */
    VOLATILE_LONG_DOUBLE a = ld_one;
    VOLATILE_LONG_DOUBLE b = ld_two;
    for (int i = 0; i < 3; i++) {
        a = a * b - b / a;
    }
    results[0] += (a != ld_nan);
    
    int sum = 0;
    for (int i = 0; i < 6; i++) sum += results[i];
    return sum;
}

/* Test 6: Array-based unordered comparisons */
NOINLINE int test_array_comparisons(void) {
    VOLATILE_DOUBLE arr1[8], arr2[8];
    int counts[7] = {0};  /* For different condition codes */
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 8; i++) {
        switch (i % 4) {
            case 0: arr1[i] = (double)i; arr2[i] = (double)i; break;
            case 1: arr1[i] = (double)i; arr2[i] = g_nan; break;
            case 2: arr1[i] = g_nan; arr2[i] = (double)i; break;
            case 3: arr1[i] = g_nan; arr2[i] = g_nan; break;
        }
    }
    
    /* Count various comparison results */
    for (int i = 0; i < 8; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]);      /* UNORDERED */
        counts[1] += !isunordered(arr1[i], arr2[i]);     /* ORDERED */
        counts[2] += (arr1[i] == arr2[i]);               /* EQ (part of UNEQ) */
        counts[3] += (arr1[i] >= arr2[i]);               /* GE (part of UNGE) */
        counts[4] += (arr1[i] > arr2[i]);                /* GT (part of UNGT) */
        counts[5] += (arr1[i] <= arr2[i]);               /* LE (part of UNLE) */
        counts[6] += (arr1[i] < arr2[i]);                /* LT (part of UNLT) */
    }
    
    /* Calculate LTGT: not equal and ordered */
    int ltgt_count = 0;
    for (int i = 0; i < 8; i++) {
        if (!isunordered(arr1[i], arr2[i]) && (arr1[i] != arr2[i])) {
            ltgt_count++;
        }
    }
    
    int sum = ltgt_count;
    for (int i = 0; i < 7; i++) sum += counts[i];
    return sum;
}

/* Test 7: Switch based on fpclassify */
NOINLINE int test_fpclassify_switch(void) {
    VOLATILE_DOUBLE values[6] = {
        g_nan, g_inf, -g_inf, g_zero, g_one, -g_one
    };
    
    int results = 0;
    for (int i = 0; i < 6; i++) {
        switch (fpclassify(values[i])) {
            case FP_NAN:
                results |= 1 << 0;
                /* Force unordered comparison */
                if (values[i] != values[i]) results |= 1 << 1;
                break;
            case FP_INFINITE:
                results |= 1 << 2;
                if (values[i] > 0) results |= 1 << 3;
                break;
            case FP_ZERO:
                results |= 1 << 4;
                if (values[i] == 0.0) results |= 1 << 5;
                break;
            case FP_NORMAL:
            case FP_SUBNORMAL:
                results |= 1 << 6;
                if (values[i] == values[i]) results |= 1 << 7;
                break;
        }
    }
    return results;
}

/* Test 8: Complex conditional expressions */
NOINLINE int test_complex_conditionals(void) {
    VOLATILE_DOUBLE a = g_nan;
    VOLATILE_DOUBLE b = 1.0;
    VOLATILE_DOUBLE c = 2.0;
    VOLATILE_DOUBLE d = g_inf;
    
    int result = 0;
    
    /* Complex expression that might generate various condition codes */
    if ((a != a) && (b == b)) result += 1;      /* UNORDERED + ORDERED */
    if (!(a < b) && !isunordered(a, b)) result += 2;  /* UNGE */
    if (!(a <= b) && !isunordered(a, b)) result += 4; /* UNGT */
    if (!(b > a) && !isunordered(b, a)) result += 8;  /* UNLE */
    if (!(b >= a) && !isunordered(b, a)) result += 16; /* UNLT */
    if ((b != c) && !isunordered(b, c)) result += 32; /* LTGT */
    if (!isgreater(a, b) && !isless(a, b)) result += 64; /* UNEQ */
    
    /* More complex chain */
    VOLATILE_DOUBLE x = b;
    VOLATILE_DOUBLE y = c;
    for (int i = 0; i < 10; i++) {
        x = x * 1.1;
        y = y / 1.1;
        if (x != y && !isunordered(x, y)) result += 128; /* LTGT */
        if (!(x < y) && !isunordered(x, y)) result += 256; /* UNGE */
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests */
    total += test_unordered_ordered();
    total += test_math_macros();
    total += test_ltgt();
    total += test_inline_asm();
    total += test_long_double();
    total += test_array_comparisons();
    total += test_fpclassify_switch();
    total += test_complex_conditionals();
    
    printf("Checksum: %d\n", total);
    
    /* Verify NaN behavior */
    if (g_nan != g_nan) {
        printf("NaN != NaN (UNORDERED) confirmed\n");
    }
    if (g_one == g_one) {
        printf("1.0 == 1.0 (ORDERED) confirmed\n");
    }
    
    return 0;
}
