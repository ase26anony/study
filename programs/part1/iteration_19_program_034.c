/* test_float_conds.c - Generate x86 condition codes for floating-point comparisons */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
    
    /* UNORDERED: NaN != NaN (unordered comparison) */
    results[0] = (nan != nan) ? 1 : 0;
    
    /* ORDERED: NaN == NaN (ordered comparison) */
    results[1] = (nan == nan) ? 1 : 0;
    
    /* More unordered comparisons */
    results[2] = (nan != num) ? 1 : 0;
    results[3] = (num != nan) ? 1 : 0;
    
    /* Ordered comparisons with normal numbers */
    results[4] = (num == num) ? 1 : 0;
    results[5] = (num != g_zero) ? 1 : 0;
    
    /* Mixed comparisons */
    results[6] = (g_inf != g_inf) ? 1 : 0;
    results[7] = (g_inf == g_inf) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    int results[12] = {0};
    VOLATILE_DOUBLE nan = g_nan;
    VOLATILE_DOUBLE a = g_one;
    VOLATILE_DOUBLE b = g_zero;
    VOLATILE_DOUBLE c = g_neg_one;
    
    /* UNORDERED: isunordered */
    results[0] = isunordered(nan, nan);
    results[1] = isunordered(nan, a);
    results[2] = isunordered(a, nan);
    
    /* ORDERED: !isunordered */
    results[3] = !isunordered(a, b);
    results[4] = !isunordered(b, c);
    
    /* UNEQ: !islessgreater && !isunordered */
    results[5] = (!islessgreater(a, a) && !isunordered(a, a));
    
    /* UNGE: !isless */
    results[6] = !isless(a, b);  /* 1.0 < 0.0 is false, so !isless is true */
    
    /* UNGT: !islessequal */
    results[7] = !islessequal(b, a);  /* 0.0 <= 1.0 is true, so !islessequal is false */
    
    /* UNLE: !isgreater */
    results[8] = !isgreater(b, a);  /* 0.0 > 1.0 is false, so !isgreater is true */
    
    /* UNLT: !isgreaterequal */
    results[9] = !isgreaterequal(a, b);  /* 1.0 >= 0.0 is true, so !isgreaterequal is false */
    
    /* LTGT: islessgreater */
    results[10] = islessgreater(a, b);  /* 1.0 != 0.0, so true */
    results[11] = islessgreater(a, a);  /* 1.0 != 1.0, so false */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) sum += results[i];
    return sum;
}

/* Test 3: Inline assembly with %C modifier for condition codes */
NOINLINE int test_inline_asm(void) {
    int results[8] = {0};
    VOLATILE_DOUBLE x = g_one;
    VOLATILE_DOUBLE y = g_zero;
    VOLATILE_DOUBLE nan = g_nan;
    
    /* Using x87 floating-point compare */
    for (int i = 0; i < 4; i++) {
        int result;
        VOLATILE_DOUBLE a = (i & 1) ? nan : x;
        VOLATILE_DOUBLE b = (i & 2) ? nan : y;
        
        /* fucomip sets flags, set%C0 uses appropriate condition code */
        __asm__ volatile (
            "fldl %2\n\t"
            "fldl %3\n\t"
            "fucomip %%st(1), %%st\n\t"
            "fstp %%st(0)\n\t"
            "set%C0 %0\n\t"
            : "=r"(result)
            : "0"(0), "m"(a), "m"(b)
            : "cc", "st"
        );
        results[i] = result;
    }
    
    /* Using SSE2 compares */
    for (int i = 0; i < 4; i++) {
        int result;
        VOLATILE_DOUBLE a = (i & 1) ? nan : x;
        VOLATILE_DOUBLE b = (i & 2) ? nan : y;
        
        /* ucomisd sets flags, set%C0 uses condition code */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C0 %0\n\t"
            : "=r"(result)
            : "x"(a), "x"(b)
            : "cc"
        );
        results[i + 4] = result;
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 4: Array operations with unordered comparisons */
NOINLINE int test_array_comparisons(void) {
    VOLATILE_DOUBLE arr1[16];
    VOLATILE_DOUBLE arr2[16];
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 16; i++) {
        if (i % 5 == 0) {
            arr1[i] = g_nan;
            arr2[i] = g_nan;
        } else if (i % 5 == 1) {
            arr1[i] = g_nan;
            arr2[i] = (double)i;
        } else if (i % 5 == 2) {
            arr1[i] = (double)i;
            arr2[i] = g_nan;
        } else {
            arr1[i] = (double)i;
            arr2[i] = (double)(15 - i);
        }
    }
    
    int counts[7] = {0};
    
    for (int i = 0; i < 16; i++) {
        /* Count various comparison results */
        counts[0] += isunordered(arr1[i], arr2[i]);      /* UNORDERED */
        counts[1] += !isunordered(arr1[i], arr2[i]);     /* ORDERED */
        counts[2] += (!islessgreater(arr1[i], arr2[i]) && 
                     !isunordered(arr1[i], arr2[i]));    /* UNEQ */
        counts[3] += !isless(arr1[i], arr2[i]);          /* UNGE */
        counts[4] += !islessequal(arr1[i], arr2[i]);     /* UNGT */
        counts[5] += !isgreater(arr1[i], arr2[i]);       /* UNLE */
        counts[6] += islessgreater(arr1[i], arr2[i]);    /* LTGT */
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) sum += counts[i];
    return sum;
}

/* Test 5: Long double (x87) specific operations */
NOINLINE int test_long_double(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_one = 1.0L;
    VOLATILE_LONG_DOUBLE ld_zero = 0.0L;
    VOLATILE_LONG_DOUBLE ld_inf = INFINITY;
    
    int results[10] = {0};
    
    /* Direct comparisons with long double */
    results[0] = (ld_nan != ld_nan);
    results[1] = (ld_nan == ld_nan);
    results[2] = (ld_one != ld_zero);
    results[3] = (ld_one == ld_one);
    
    /* Using builtins for x87 */
    for (int i = 0; i < 3; i++) {
        VOLATILE_LONG_DOUBLE a = (i == 0) ? ld_nan : ld_one;
        VOLATILE_LONG_DOUBLE b = (i == 1) ? ld_nan : ld_zero;
        
        int flags;
        __asm__ volatile (
            "fldt %2\n\t"
            "fldt %3\n\t"
            "fucomip %%st(1), %%st\n\t"
            "fstp %%st(0)\n\t"
            "pushf\n\t"
            "pop %0\n\t"
            : "=r"(flags)
            : "0"(0), "m"(a), "m"(b)
            : "cc", "st"
        );
        
        /* Check various condition flags */
        results[4 + i] = (flags & 0x4500) ? 1 : 0;  /* ZF, PF, CF */
    }
    
    /* Complex expression to force multiple condition codes */
    VOLATILE_LONG_DOUBLE x = ld_one;
    VOLATILE_LONG_DOUBLE y = ld_zero;
    
    for (int i = 0; i < 3; i++) {
        int cmp_result;
        switch (i) {
            case 0:
                cmp_result = (x > y) ? 1 : ((x < y) ? -1 : 0);
                break;
            case 1:
                cmp_result = (x != y) ? 1 : 0;
                break;
            case 2:
                cmp_result = (x == x) ? 1 : 0;
                break;
        }
        results[7 + i] = cmp_result;
    }
    
    int sum = 0;
    for (int i = 0; i < 10; i++) sum += results[i];
    return sum;
}

/* Test 6: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE values[] = {g_nan, g_zero, g_one, g_inf, -g_inf};
    int results[20] = {0};
    int idx = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            VOLATILE_DOUBLE a = values[i];
            VOLATILE_DOUBLE b = values[j];
            
            /* Complex branching to force different condition codes */
            if (isunordered(a, b)) {
                results[idx++] = 1;  /* UNORDERED */
            } else if (!islessgreater(a, b)) {
                results[idx++] = 2;  /* UNEQ or EQ */
            } else if (isless(a, b)) {
                results[idx++] = 3;  /* LT */
            } else if (isgreater(a, b)) {
                results[idx++] = 4;  /* GT */
            } else {
                results[idx++] = 5;  /* Shouldn't happen */
            }
            
            /* Another branch with different condition */
            if (!isless(a, b)) {
                results[idx++] = 6;  /* UNGE */
            } else {
                results[idx++] = 7;
            }
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 25; i++) sum += results[i];
    return sum;
}

/* Main function that runs all tests */
int main(void) {
    printf("Testing x86 floating-point condition codes...\n");
    
    int total = 0;
    
    total += test_unordered_ordered();
    printf("Test 1 (unordered/ordered): %d\n", total);
    
    total += test_math_macros();
    printf("Test 2 (math macros): %d\n", total - test_unordered_ordered());
    
    total += test_inline_asm();
    printf("Test 3 (inline asm): %d\n", total - (test_unordered_ordered() + test_math_macros()));
    
    total += test_array_comparisons();
    printf("Test 4 (array comparisons): %d\n", total - (test_unordered_ordered() + test_math_macros() + test_inline_asm()));
    
    total += test_long_double();
    printf("Test 5 (long double): %d\n", total - (test_unordered_ordered() + test_math_macros() + test_inline_asm() + test_array_comparisons()));
    
    total += test_switch_comparisons();
    printf("Test 6 (switch comparisons): %d\n", total - (test_unordered_ordered() + test_math_macros() + test_inline_asm() + test_array_comparisons() + test_long_double()));
    
    printf("Total checksum: %d\n", total);
    
    /* Verify we used NaN values */
    if (isnan(g_nan)) {
        printf("NaN check passed\n");
    }
    
    return 0;
}
