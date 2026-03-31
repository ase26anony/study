/* Test program to trigger x86 floating-point condition code output */
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
volatile double g_two = 2.0;

volatile long double ld_nan;
volatile long double ld_inf;
volatile long double ld_zero = 0.0L;
volatile long double ld_one = 1.0L;

/* Initialize long double NaN/INF */
static void init_long_doubles(void) {
    ld_nan = strtold("NAN", NULL);
    ld_inf = strtold("INF", NULL);
}

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    int results[8] = {0};
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_two;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (a != b) ? 1 : 0;  /* UNORDERED when a is NaN */
    results[1] = (a == a) ? 1 : 0;  /* UNORDERED when comparing NaN with itself */
    results[2] = (b == c) ? 1 : 0;  /* ORDERED comparison */
    results[3] = (b != c) ? 1 : 0;  /* ORDERED comparison */
    
    /* Force use of condition codes in control flow */
    if (isunordered(a, b)) results[4] = 1;
    if (isordered(a, b))   results[5] = 0;
    
    /* UNEQ: unordered or equal */
    if (!(a > b) && !(a < b)) results[6] = 1;  /* a is NaN, so neither > nor < */
    
    /* LTGT: less than or greater than (ordered and not equal) */
    if ((b < c) || (b > c)) results[7] = 1;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) sum += results[i];
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    int results = 0;
    volatile double x = g_nan;
    volatile double y = g_one;
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < 8; i++) {
        int result;
        switch (i) {
            case 0: /* UNORDERED */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(result)
                    : "x"(x), "x"(y)
                    : "cc"
                );
                break;
            case 1: /* ORDERED */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(result)
                    : "x"(y), "x"(x)
                    : "cc"
                );
                break;
            case 2: /* UNEQ */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(result)
                    : "x"(g_zero), "x"(g_zero)
                    : "cc"
                );
                break;
            case 3: /* UNGE */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(result)
                    : "x"(g_one), "x"(g_zero)
                    : "cc"
                );
                break;
            case 4: /* UNGT */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(result)
                    : "x"(g_two), "x"(g_one)
                    : "cc"
                );
                break;
            case 5: /* UNLE */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(result)
                    : "x"(g_zero), "x"(g_one)
                    : "cc"
                );
                break;
            case 6: /* UNLT */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(result)
                    : "x"(g_zero), "x"(g_two)
                    : "cc"
                );
                break;
            case 7: /* LTGT */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(result)
                    : "x"(g_one), "x"(g_two)
                    : "cc"
                );
                break;
        }
        results += result;
    }
    
    return results;
}

/* Test 3: Array comparisons with various condition codes */
NOINLINE int test_array_comparisons(void) {
    volatile double arr1[16];
    volatile double arr2[16];
    int counts[8] = {0};  /* For different condition types */
    
    /* Initialize arrays with mix of values */
    for (int i = 0; i < 16; i++) {
        if (i % 5 == 0) {
            arr1[i] = g_nan;
            arr2[i] = i * 0.5;
        } else if (i % 3 == 0) {
            arr1[i] = g_inf;
            arr2[i] = g_neg_inf;
        } else {
            arr1[i] = i * 1.0;
            arr2[i] = (i - 1) * 1.0;
        }
    }
    
    /* Perform various comparisons that should generate different condition codes */
    for (int i = 0; i < 16; i++) {
        /* Use different comparison functions to trigger different condition codes */
        if (isunordered(arr1[i], arr2[i])) counts[0]++;  /* UNORDERED */
        if (isordered(arr1[i], arr2[i]))  counts[1]++;  /* ORDERED */
        if (!isgreater(arr1[i], arr2[i]) && !isless(arr1[i], arr2[i])) counts[2]++;  /* UNEQ */
        if (!isless(arr1[i], arr2[i]))    counts[3]++;  /* UNGE (not less than) */
        if (!islessequal(arr1[i], arr2[i])) counts[4]++;  /* UNGT (not less or equal) */
        if (islessequal(arr1[i], arr2[i])) counts[5]++;  /* UNLE */
        if (isless(arr1[i], arr2[i]))     counts[6]++;  /* UNLT */
        if (isless(arr1[i], arr2[i]) || isgreater(arr1[i], arr2[i])) counts[7]++;  /* LTGT */
    }
    
    int total = 0;
    for (int i = 0; i < 8; i++) total += counts[i];
    return total;
}

/* Test 4: x87 long double comparisons */
NOINLINE int test_x87_long_double(void) {
    int results = 0;
    volatile long double a = ld_nan;
    volatile long double b = ld_one;
    volatile long double c = ld_zero;
    
    /* x87 comparisons - these often use different condition codes */
    if (a != b) results += 1;      /* Should be UNORDERED */
    if (b == b) results += 2;      /* Should be ORDERED */
    if (!(a > b) && !(a < b)) results += 4;  /* UNEQ */
    if (!(b < c)) results += 8;    /* UNGE */
    if (!(b <= c)) results += 16;  /* UNGT */
    if (c <= b) results += 32;     /* UNLE */
    if (c < b) results += 64;      /* UNLT */
    if ((b < c) || (b > c)) results += 128;  /* LTGT */
    
    /* Force x87 stack operations */
    volatile long double temp;
    __asm__ volatile ("fldt %1" : "=t"(temp) : "m"(a));
    __asm__ volatile ("fldt %1" : "=t"(temp) : "m"(b));
    __asm__ volatile ("fucomip %%st(1), %%st" : : : "cc", "st");
    __asm__ volatile ("fstp %%st(0)" : : : "st");
    
    return results;
}

/* Test 5: Mixed SSE/x87 with switch statement */
NOINLINE int test_mixed_switch(void) {
    volatile double vals[] = {g_nan, g_inf, g_neg_inf, g_zero, g_one};
    int results = 0;
    
    for (int i = 0; i < 5; i++) {
        int fpclass = fpclassify(vals[i]);
        
        /* Switch on fpclassify result - forces multiple comparison patterns */
        switch (fpclass) {
            case FP_NAN:
                if (isunordered(vals[i], vals[(i+1)%5])) results += 1;
                break;
            case FP_INFINITE:
                if (isgreater(vals[i], g_zero)) results += 2;
                else if (isless(vals[i], g_zero)) results += 4;
                break;
            case FP_ZERO:
                if (vals[i] == g_zero) results += 8;
                break;
            case FP_NORMAL:
                if (isless(vals[i], g_one)) results += 16;
                else if (isgreater(vals[i], g_one)) results += 32;
                else results += 64;
                break;
            case FP_SUBNORMAL:
                results += 128;
                break;
        }
        
        /* Additional unordered comparisons in each case */
        if (vals[i] != vals[(i+2)%5]) results += 256;
    }
    
    return results;
}

/* Test 6: Complex expression with multiple condition codes */
NOINLINE int test_complex_expressions(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_two;
    int result = 0;
    
    /* Complex expression that should generate various condition codes */
    if ((x != y) && (y == z || y != z)) {
        result += 1;  /* UNORDERED from x != y */
    }
    
    if ((isunordered(x, y) || isordered(y, z)) && !isgreater(x, z)) {
        result += 2;  /* Mix of UNORDERED and ORDERED */
    }
    
    /* Chain of comparisons */
    if (!(x < y) && !(x > y)) result += 4;  /* UNEQ */
    if (!(y < z)) result += 8;              /* UNGE */
    if (!(y <= z)) result += 16;            /* UNGT */
    if (y <= z) result += 32;               /* UNLE */
    if (y < z) result += 64;                /* UNLT */
    if ((y < z) || (y > z)) result += 128;  /* LTGT */
    
    return result;
}

int main(void) {
    init_long_doubles();
    
    printf("Testing x86 floating-point condition code generation...\n");
    
    int total = 0;
    
    total += test_unordered_comparisons();
    total += test_asm_condition_codes();
    total += test_array_comparisons();
    total += test_x87_long_double();
    total += test_mixed_switch();
    total += test_complex_expressions();
    
    printf("Total checksum: %d\n", total);
    printf("If non-zero, tests executed successfully.\n");
    
    return 0;
}
