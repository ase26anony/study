/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations from removing critical operations */
#define NOOPT __attribute__((noinline, noipa, optimize("no-tree-vectorize")))

/* Global volatile variables to prevent constant folding */
volatile double g_nan = NAN;
volatile double g_inf = INFINITY;
volatile double g_zero = 0.0;
volatile double g_one = 1.0;
volatile long double g_ld_nan = NAN;
volatile long double g_ld_inf = INFINITY;

/* Test 1: Direct unordered comparisons with NaN */
NOOPT int test_unordered_comparisons(void) {
    volatile double d1 = g_nan;
    volatile double d2 = g_one;
    volatile double d3 = g_zero;
    volatile double d4 = g_inf;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (d1 != d2);  /* UNORDERED comparison */
    results[1] = (d1 == d1);  /* UNORDERED comparison */
    results[2] = (d2 == d3);  /* ORDERED comparison */
    results[3] = (d4 != d4);  /* UNORDERED comparison (inf != inf is false) */
    
    /* Mixed comparisons */
    results[4] = (d1 < d2);   /* UNORDERED comparison */
    results[5] = (d2 > d3);   /* ORDERED comparison */
    results[6] = (d1 <= d4);  /* UNORDERED comparison */
    results[7] = (d4 >= d3);  /* ORDERED comparison */
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using <math.h> comparison macros */
NOOPT int test_math_macros(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int results[12] = {0};
    
    /* These macros should generate specific condition codes */
    results[0] = isunordered(a, b);  /* UNORDERED */
    results[1] = isunordered(b, c);  /* ORDERED (false) */
    results[2] = isgreater(b, c);    /* UNLE? Actually generates GT */
    results[3] = isless(b, c);       /* UNGE? Actually generates LT */
    results[4] = isgreaterequal(b, c); /* UNLT? */
    results[5] = islessequal(b, c);    /* UNGT? */
    
    /* More complex cases */
    results[6] = !isunordered(a, b) && (a > b);  /* UNLE? */
    results[7] = !isunordered(b, a) && (b < a);  /* UNGE? */
    results[8] = isunordered(a, a);  /* UNORDERED? */
    results[9] = !isunordered(d, c) && (d != c); /* UNEQ? */
    
    /* LTGT (not equal and ordered) */
    results[10] = !isunordered(b, c) && (b != c); /* LTGT */
    results[11] = !isunordered(a, b) && (a != b); /* UNORDERED prevents LTGT */
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NOOPT int test_inline_asm(void) {
    volatile double x = g_nan;
    volatile double y = g_one;
    volatile double z = g_zero;
    int result1 = 0, result2 = 0, result3 = 0;
    
    /* Force generation of condition codes via inline asm */
    /* Using x87 floating point compare */
    __asm__ volatile (
        "fldl %1\n\t"
        "fldl %2\n\t"
        "fucomip %%st(1), %%st\n\t"
        "set%C0 %0\n\t"
        "fstp %%st(0)"
        : "=r"(result1)
        : "m"(x), "m"(y)
        : "cc", "st"
    );
    
    /* SSE2 compare with NaN */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "ucomisd %%xmm0, %%xmm1\n\t"
        "set%C0 %0"
        : "=r"(result2)
        : "m"(y), "m"(z)
        : "xmm0", "xmm1", "cc"
    );
    
    /* Another compare with different condition */
    __asm__ volatile (
        "movsd %1, %%xmm0\n\t"
        "movsd %2, %%xmm1\n\t"
        "ucomisd %%xmm0, %%xmm1\n\t"
        "set%C0 %0"
        : "=r"(result3)
        : "m"(x), "m"(z)
        : "xmm0", "xmm1", "cc"
    );
    
    return result1 + result2 + result3;
}

/* Test 4: Array operations with unordered comparisons */
NOOPT int test_array_operations(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 8; i++) {
        if (i % 4 == 0) arr1[i] = g_nan;
        else if (i % 4 == 1) arr1[i] = g_inf;
        else if (i % 4 == 2) arr1[i] = -g_inf;
        else arr1[i] = (double)i;
        
        if (i % 3 == 0) arr2[i] = g_nan;
        else if (i % 3 == 1) arr2[i] = g_inf;
        else arr2[i] = (double)(i * 2);
    }
    
    int counts[6] = {0};
    
    for (int i = 0; i < 8; i++) {
        /* Count various comparison results */
        counts[0] += isunordered(arr1[i], arr2[i]);      /* UNORDERED */
        counts[1] += !isunordered(arr1[i], arr2[i]) && 
                     (arr1[i] == arr2[i]);              /* UNEQ? */
        counts[2] += !isunordered(arr1[i], arr2[i]) && 
                     (arr1[i] > arr2[i]);               /* UNLE? */
        counts[3] += !isunordered(arr1[i], arr2[i]) && 
                     (arr1[i] < arr2[i]);               /* UNGE? */
        counts[4] += !isunordered(arr1[i], arr2[i]) && 
                     (arr1[i] >= arr2[i]);              /* UNLT? */
        counts[5] += !isunordered(arr1[i], arr2[i]) && 
                     (arr1[i] <= arr2[i]);              /* UNGT? */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 5: Long double (x87) operations */
NOOPT int test_long_double_ops(void) {
    volatile long double ld1 = g_ld_nan;
    volatile long double ld2 = g_ld_inf;
    volatile long double ld3 = 3.14159265358979323846L;
    volatile long double ld4 = 2.71828182845904523536L;
    
    int results = 0;
    
    /* x87 style comparisons with long double */
    results += (ld1 != ld2);    /* UNORDERED */
    results += (ld2 == ld2);    /* ORDERED (inf == inf) */
    results += (ld3 > ld4);     /* ORDERED */
    results += (ld1 < ld3);     /* UNORDERED */
    results += (ld2 != ld3);    /* ORDERED */
    results += (ld1 == ld1);    /* UNORDERED (NaN == NaN is false) */
    
    /* Force x87 compare and conditional move */
    volatile long double ld5 = 0.0L;
    if (!isunordered(ld3, ld4) && (ld3 != ld4)) {
        ld5 = ld3;  /* LTGT condition */
    }
    
    results += (int)(ld5 > 0.0L);
    
    return results;
}

/* Test 6: Switch based on comparison results */
NOOPT int test_switch_comparisons(void) {
    volatile double a = g_nan;
    volatile double b = g_one;
    volatile double c = g_zero;
    volatile double d = g_inf;
    
    int result = 0;
    
    /* Multiple comparisons in switch to force different condition codes */
    int cmp1 = isunordered(a, b) ? 1 : 0;
    int cmp2 = (!isunordered(b, c) && (b > c)) ? 2 : 0;
    int cmp3 = (!isunordered(c, d) && (c < d)) ? 3 : 0;
    int cmp4 = (!isunordered(a, d) && (a != d)) ? 4 : 0;
    int cmp5 = (!isunordered(b, d) && (b == d)) ? 5 : 0;
    
    switch (cmp1 + cmp2 + cmp3 + cmp4 + cmp5) {
        case 0:
            result = isunordered(a, c) ? 10 : 20;
            break;
        case 1:
            result = (!isunordered(b, d) && (b != d)) ? 30 : 40;
            break;
        case 2:
            result = (!isunordered(c, a) && (c > a)) ? 50 : 60;
            break;
        case 3:
            result = isunordered(d, a) ? 70 : 80;
            break;
        default:
            result = (!isunordered(a, b) && (a == b)) ? 90 : 100;
            break;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running condition code tests...\n");
    
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_array_operations();
    total += test_long_double_ops();
    total += test_switch_comparisons();
    
    printf("Total checksum: %d\n", total);
    
    /* Use results to prevent dead code elimination */
    if (total == 0) {
        printf("Warning: All tests returned zero\n");
    }
    
    return 0;
}
