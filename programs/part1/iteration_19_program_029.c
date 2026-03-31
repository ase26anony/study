/* test_condition_codes.c */
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
volatile double g_neg_inf = -INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile double g_neg_one = -1.0;

/* Test 1: Direct unordered comparisons using != and == operators */
NOINLINE int test_unordered_ordered(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_zero;
    
    int results = 0;
    
    /* UNORDERED: a != a (NaN != NaN is true for unordered) */
    if (a != a) results |= 1 << 0;
    
    /* ORDERED: c == d (0.0 == 0.0 is true for ordered) */
    if (c == d) results |= 1 << 1;
    
    /* UNEQ: Use isunordered() check */
    if (!isunordered(a, b) && a == b) results |= 1 << 2;  /* This won't be true, but generates code */
    
    return results;
}

/* Test 2: Various comparison macros that map to condition codes */
NOINLINE int test_comparison_macros(void) {
    volatile double nan1 = g_nan;
    volatile double nan2 = -g_nan;  /* Another NaN */
    volatile double x = g_one;
    volatile double y = g_neg_one;
    
    int results = 0;
    
    /* UNORDERED: isunordered */
    if (isunordered(nan1, x)) results |= 1 << 3;
    
    /* ORDERED: !isunordered */
    if (!isunordered(x, y)) results |= 1 << 4;
    
    /* UNGE: !isless (not less than) */
    if (!isless(x, y)) results |= 1 << 5;  /* 1.0 < -1.0 is false, so !isless is true */
    
    /* UNGT: !islessequal (not less than or equal) */
    if (!islessequal(y, x)) results |= 1 << 6;  /* -1.0 <= 1.0 is true, so !islessequal is false */
    
    /* UNLE: islessequal with NaN handling */
    if (islessequal(x, x)) results |= 1 << 7;  /* 1.0 <= 1.0 is true */
    
    /* UNLT: isless with NaN handling */
    if (isless(y, x)) results |= 1 << 8;  /* -1.0 < 1.0 is true */
    
    /* LTGT: !islessgreater (not less than or greater than) */
    if (!islessgreater(x, x)) results |= 1 << 9;  /* 1.0 != 1.0 is false, so !islessgreater is true */
    
    return results;
}

/* Test 3: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_zero;
    
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* UNORDERED comparison using inline assembly */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C0 %0"
        : "=r"(result1)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    /* ORDERED comparison */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C1 %0"
        : "=r"(result2)
        : "x"(c), "x"(d)
        : "cc"
    );
    
    /* UNEQ comparison */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "set%C3 %0"  /* Note: e=equal, 3=parity (unordered) */
        : "=r"(result3)
        : "x"(d), "x"(d)  /* Compare zero with zero */
        : "cc"
    );
    
    return (result1 << 10) | (result2 << 11) | (result3 << 12);
}

/* Test 4: Loop with array comparisons to force code generation */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 2 == 0) ? (double)i : g_nan;
        arr2[i] = (i % 3 == 0) ? (double)(i * 2) : g_nan;
    }
    
    int counts[7] = {0};  /* For different comparison types */
    
    for (int i = 0; i < 8; i++) {
        /* Test various conditions */
        if (isunordered(arr1[i], arr2[i])) counts[0]++;  /* UNORDERED */
        if (!isunordered(arr1[i], arr2[i])) counts[1]++; /* ORDERED */
        if (!isunordered(arr1[i], arr2[i]) && arr1[i] == arr2[i]) counts[2]++; /* UNEQ */
        if (!isless(arr1[i], arr2[i])) counts[3]++;      /* UNGE */
        if (!islessequal(arr1[i], arr2[i])) counts[4]++; /* UNGT */
        if (islessequal(arr1[i], arr2[i])) counts[5]++;  /* UNLE */
        if (isless(arr1[i], arr2[i])) counts[6]++;       /* UNLT */
    }
    
    /* Combine counts into a single hash */
    int hash = 0;
    for (int i = 0; i < 7; i++) {
        hash = hash * 31 + counts[i];
    }
    
    return hash;
}

/* Test 5: Long double (x87) operations for different condition codes */
NOINLINE int test_long_double_comparisons(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_zero = 0.0L;
    volatile long double ld_one = 1.0L;
    
    int results = 0;
    
    /* Force x87 unordered compare */
    if (ld_nan != ld_nan) results |= 1 << 13;  /* UNORDERED */
    
    /* Ordered compare */
    if (ld_zero == ld_zero) results |= 1 << 14; /* ORDERED */
    
    /* Various comparisons that might generate different condition codes */
    if (!isless(ld_one, ld_zero)) results |= 1 << 15;  /* UNGE: 1.0 < 0.0 is false */
    if (islessequal(ld_zero, ld_one)) results |= 1 << 16; /* UNLE: 0.0 <= 1.0 is true */
    
    /* Use builtin for direct unordered compare */
    int cmp_result = __builtin_islessgreater(ld_nan, ld_one);
    if (!cmp_result) results |= 1 << 17;  /* LTGT: not less and not greater (unordered) */
    
    return results;
}

/* Test 6: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double vals[4] = {g_nan, g_zero, g_one, g_inf};
    int total = 0;
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int condition = 0;
            
            /* Determine comparison result */
            if (isunordered(vals[i], vals[j])) {
                condition = 0;  /* UNORDERED */
            } else if (vals[i] == vals[j]) {
                condition = 1;  /* ORDERED equal */
            } else if (vals[i] < vals[j]) {
                condition = 2;  /* UNLT */
            } else if (vals[i] > vals[j]) {
                condition = 3;  /* UNGT */
            } else {
                condition = 4;  /* Other */
            }
            
            /* Switch to force different code paths */
            switch (condition) {
                case 0: total += 1; break;  /* UNORDERED */
                case 1: total += 2; break;  /* ORDERED equal */
                case 2: total += 3; break;  /* UNLT */
                case 3: total += 4; break;  /* UNGT */
                case 4: total += 5; break;  /* Other */
            }
        }
    }
    
    return total;
}

/* Test 7: Direct use of GCC builtins for SSE2 unordered compares */
NOINLINE int test_sse2_builtins(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    
    int results = 0;
    
    /* Use __builtin_ia32_ucomisd directly */
    int cmp1 = __builtin_ia32_ucomisd(a, b);
    if (cmp1 & 4) results |= 1 << 18;  /* UNORDERED flag */
    
    int cmp2 = __builtin_ia32_ucomisd(c, c);
    if ((cmp2 & 1) == 0) results |= 1 << 19;  /* ORDERED equal */
    
    /* Force conditional move based on comparison */
    volatile double x = g_one;
    volatile double y = g_neg_one;
    double selected;
    
    /* This should generate conditional move with condition code */
    __asm__ volatile (
        "ucomisd %2, %1\n\t"
        "cmov%C2 %3, %0"
        : "=r"(selected)
        : "x"(x), "x"(y), "r"(x)
        : "cc"
    );
    
    if (selected == x) results |= 1 << 20;
    
    return results;
}

int main(void) {
    printf("Testing x86 condition code generation...\n");
    
    /* Run all tests to trigger condition code usage */
    int result = 0;
    
    result ^= test_unordered_ordered();
    printf("Test 1 result: %d\n", test_unordered_ordered());
    
    result ^= test_comparison_macros();
    printf("Test 2 result: %d\n", test_comparison_macros());
    
    result ^= test_asm_condition_codes();
    printf("Test 3 result: %d\n", test_asm_condition_codes());
    
    result ^= test_array_comparisons();
    printf("Test 4 result: %d\n", test_array_comparisons());
    
    result ^= test_long_double_comparisons();
    printf("Test 5 result: %d\n", test_long_double_comparisons());
    
    result ^= test_switch_comparisons();
    printf("Test 6 result: %d\n", test_switch_comparisons());
    
    result ^= test_sse2_builtins();
    printf("Test 7 result: %d\n", test_sse2_builtins());
    
    printf("Final checksum: %d\n", result);
    
    return 0;
}
