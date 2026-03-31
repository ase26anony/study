/* Test program to trigger x86 floating-point condition code printing */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimizations from removing critical code */
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
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b) ? 1 : 0;  /* UNORDERED when a is NaN */
    results[1] = (a == a) ? 1 : 0;  /* UNORDERED when comparing NaN to itself */
    results[2] = (b == b) ? 1 : 0;  /* ORDERED for normal numbers */
    results[3] = (c != d) ? 1 : 0;  /* UNORDERED when d is NaN */
    
    /* Mixed comparisons */
    results[4] = (a > b) ? 1 : 0;   /* UNORDERED */
    results[5] = (a < b) ? 1 : 0;   /* UNORDERED */
    results[6] = (b > c) ? 1 : 0;   /* ORDERED */
    results[7] = (c < b) ? 1 : 0;   /* ORDERED */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NOINLINE int test_math_macros(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int results[12] = {0};
    
    /* These map to specific condition codes */
    results[0] = isunordered(a, b);    /* UNORDERED */
    results[1] = isordered(b, c);      /* ORDERED */
    results[2] = isgreater(b, c);      /* GT */
    results[3] = isgreaterequal(b, c); /* GE */
    results[4] = isless(b, c);         /* LT */
    results[5] = islessequal(b, c);    /* LE */
    
    /* Unordered versions from the uncovered lines */
    results[6] = !isgreater(a, b);     /* UNLE? */
    results[7] = !isless(a, b);        /* UNGE? */
    results[8] = !isgreater(b, a);     /* UNLE? */
    results[9] = !isless(b, a);        /* UNGE? */
    
    /* Special cases */
    results[10] = (b != c) && !isunordered(b, c);  /* LTGT */
    results[11] = (b == c) || isunordered(b, c);   /* UNEQ? */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOINLINE int test_inline_asm(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    
    unsigned char results[6] = {0};
    
    /* Test various condition codes via inline assembly */
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C0 %0"
        : "=r"(results[0])
        : "x"(a), "x"(b)
        : "cc"
    );
    
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C1 %0"
        : "=r"(results[1])
        : "x"(b), "x"(a), "i"(6)  /* 6 = UNORDERED condition */
        : "cc"
    );
    
    __asm__ volatile (
        "comisd %1, %2\n\t"
        "set%C2 %0"
        : "=r"(results[2])
        : "x"(b), "x"(c), "i"(2)  /* 2 = ORDERED condition */
        : "cc"
    );
    
    /* Test with x87 instructions for long double */
    volatile long double ld1 = g_nan;
    volatile long double ld2 = g_one;
    
    __asm__ volatile (
        "fldt %1\n\t"
        "fldt %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C3 %0"
        : "=r"(results[3])
        : "m"(ld1), "m"(ld2), "i"(6)  /* UNORDERED */
        : "cc", "st"
    );
    
    __asm__ volatile (
        "fldt %2\n\t"
        "fldt %1\n\t"
        "fucomip %%st(1), %%st\n\t"
        "fstp %%st(0)\n\t"
        "set%C4 %0"
        : "=r"(results[4])
        : "m"(ld2), "m"(ld1), "i"(2)  /* ORDERED */
        : "cc", "st"
    );
    
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 4: Array processing with various comparisons */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        if (i % 4 == 0) {
            arr1[i] = g_nan;
            arr2[i] = i * 0.5;
        } else if (i % 4 == 1) {
            arr1[i] = i * 0.5;
            arr2[i] = g_nan;
        } else if (i % 4 == 2) {
            arr1[i] = i * 0.5;
            arr2[i] = (i + 1) * 0.5;
        } else {
            arr1[i] = i * 0.5;
            arr2[i] = i * 0.5;
        }
    }
    
    int counts[8] = {0};
    
    for (int i = 0; i < 8; i++) {
        /* Test all condition code types */
        if (isunordered(arr1[i], arr2[i])) counts[0]++;      /* UNORDERED */
        if (isordered(arr1[i], arr2[i])) counts[1]++;        /* ORDERED */
        if (isgreater(arr1[i], arr2[i])) counts[2]++;        /* GT */
        if (isgreaterequal(arr1[i], arr2[i])) counts[3]++;   /* GE */
        if (isless(arr1[i], arr2[i])) counts[4]++;           /* LT */
        if (islessequal(arr1[i], arr2[i])) counts[5]++;      /* LE */
        if (!isgreater(arr1[i], arr2[i])) counts[6]++;       /* UNLE */
        if (!isless(arr1[i], arr2[i])) counts[7]++;          /* UNGE */
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 5: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int result = 0;
    
    /* Force multiple comparison types */
    if (isunordered(a, b)) {
        result |= 1;  /* UNORDERED */
    }
    
    if (isordered(b, c)) {
        result |= 2;  /* ORDERED */
    }
    
    if (!isgreater(a, b) && !isunordered(a, b)) {
        result |= 4;  /* UNLE */
    }
    
    if (!isless(a, b) && !isunordered(a, b)) {
        result |= 8;  /* UNGE */
    }
    
    if ((b != c) && !isunordered(b, c)) {
        result |= 16; /* LTGT */
    }
    
    if ((b == c) || isunordered(b, c)) {
        result |= 32; /* UNEQ */
    }
    
    /* Complex expression that might generate UNGT */
    if (isgreater(d, b) || isunordered(d, b)) {
        result |= 64;
    }
    
    /* Complex expression that might generate UNLT */
    if (isless(b, d) || isunordered(b, d)) {
        result |= 128;
    }
    
    return result;
}

/* Test 6: Long double specific tests (x87) */
NOINLINE int test_long_double(void) {
    volatile long double ld_nan = g_nan;
    volatile long double ld_one = 1.0L;
    volatile long double ld_zero = 0.0L;
    volatile long double ld_inf = INFINITY;
    
    int results = 0;
    
    /* x87 comparisons often generate different condition codes */
    if (ld_nan != ld_one) results += 1;      /* UNORDERED */
    if (ld_one == ld_one) results += 2;      /* ORDERED */
    if (ld_one > ld_zero) results += 4;      /* GT */
    if (ld_one >= ld_zero) results += 8;     /* GE */
    if (ld_zero < ld_one) results += 16;     /* LT */
    if (ld_zero <= ld_one) results += 32;    /* LE */
    
    /* Force UNEQ, UNGE, UNGT, UNLE, UNLT, LTGT */
    if (!(ld_nan > ld_one) || ld_nan != ld_nan) results += 64;
    if (!(ld_nan < ld_one) || ld_nan != ld_nan) results += 128;
    if ((ld_one != ld_zero) && (ld_one == ld_one)) results += 256;
    
    return results;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 floating-point condition codes...\n");
    
    /* Run all tests to trigger various condition code usages */
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_array_comparisons();
    total += test_switch_comparisons();
    total += test_long_double();
    
    printf("Total checksum: %d\n", total);
    
    /* Also test with volatile function calls to prevent optimization */
    volatile int (*funcs[])(void) = {
        test_unordered_comparisons,
        test_math_macros,
        test_inline_asm,
        test_array_comparisons,
        test_switch_comparisons,
        test_long_double
    };
    
    int volatile_sum = 0;
    for (size_t i = 0; i < sizeof(funcs)/sizeof(funcs[0]); i++) {
        volatile_sum += funcs[i]();
    }
    
    printf("Volatile checksum: %d\n", volatile_sum);
    
    return (total > 0) ? 0 : 1;
}
