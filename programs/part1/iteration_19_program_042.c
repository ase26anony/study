/* test_float_conds.c - Generate x86 condition codes for floating-point comparisons */
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
VOLATILE_DOUBLE g_neg_inf = -INFINITY;
VOLATILE_DOUBLE g_zero = 0.0;
VOLATILE_DOUBLE g_one = 1.0;
VOLATILE_DOUBLE g_neg_one = -1.0;

/* Test 1: Direct unordered comparisons that should generate UNORDERED/ORDERED codes */
NOINLINE int test_unordered_comparisons(void) {
    VOLATILE_DOUBLE a = g_nan;
    VOLATILE_DOUBLE b = g_one;
    VOLATILE_DOUBLE c = g_zero;
    
    int result = 0;
    
    /* These should generate UNORDERED condition code */
    if (a != b) result |= 1;      /* NaN != 1.0 -> true (unordered) */
    if (!(a == b)) result |= 2;   /* !(NaN == 1.0) -> true (unordered) */
    
    /* These should generate ORDERED condition code */
    if (b == c) result |= 4;      /* 1.0 == 0.0 -> false (ordered) */
    if (b != c) result |= 8;      /* 1.0 != 0.0 -> true (ordered) */
    
    /* Mixed comparisons */
    if (isunordered(a, b)) result |= 16;
    if (isordered(b, c)) result |= 32;
    
    return result;
}

/* Test 2: Use inline assembly with %C modifier to force condition code output */
NOINLINE int test_asm_condition_codes(void) {
    VOLATILE_DOUBLE x = g_nan;
    VOLATILE_DOUBLE y = g_one;
    VOLATILE_DOUBLE z = g_zero;
    VOLATILE_DOUBLE w = g_inf;
    
    int results[8] = {0};
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < 8; i++) {
        int r = 0;
        switch (i) {
            case 0: /* UNORDERED */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(r)
                    : "x"(x), "x"(y)
                    : "cc"
                );
                break;
            case 1: /* ORDERED */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(r)
                    : "x"(y), "x"(z)
                    : "cc"
                );
                break;
            case 2: /* UNEQ */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(r)
                    : "x"(z), "x"(z)  /* equal values */
                    : "cc"
                );
                break;
            case 3: /* UNGE */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(r)
                    : "x"(w), "x"(z)  /* INF >= 0 */
                    : "cc"
                );
                break;
            case 4: /* UNGT */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(r)
                    : "x"(w), "x"(z)  /* INF > 0 */
                    : "cc"
                );
                break;
            case 5: /* UNLE */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(r)
                    : "x"(z), "x"(w)  /* 0 <= INF */
                    : "cc"
                );
                break;
            case 6: /* UNLT */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(r)
                    : "x"(z), "x"(w)  /* 0 < INF */
                    : "cc"
                );
                break;
            case 7: /* LTGT */
                __asm__ volatile (
                    "ucomisd %1, %2\n\t"
                    "set%C0 %0"
                    : "=r"(r)
                    : "x"(g_one), "x"(g_neg_one)  /* 1.0 != -1.0 */
                    : "cc"
                );
                break;
        }
        results[i] = r;
    }
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i] << i;
    }
    return sum;
}

/* Test 3: Array-based comparisons to force multiple condition codes in loops */
NOINLINE int test_array_comparisons(void) {
    VOLATILE_DOUBLE arr1[16];
    VOLATILE_DOUBLE arr2[16];
    
    /* Initialize with mix of values */
    for (int i = 0; i < 16; i++) {
        switch (i % 5) {
            case 0: arr1[i] = g_nan; arr2[i] = (double)i; break;
            case 1: arr1[i] = (double)i; arr2[i] = g_nan; break;
            case 2: arr1[i] = (double)i; arr2[i] = (double)(i * 2); break;
            case 3: arr1[i] = g_inf; arr2[i] = (double)i; break;
            case 4: arr1[i] = (double)i; arr2[i] = g_neg_inf; break;
        }
    }
    
    int counts[8] = {0}; /* Counters for different comparison results */
    
    for (int i = 0; i < 16; i++) {
        /* Test all the comparison macros that map to condition codes */
        if (isunordered(arr1[i], arr2[i])) counts[0]++;      /* UNORDERED */
        if (isordered(arr1[i], arr2[i])) counts[1]++;        /* ORDERED */
        if (!isgreater(arr1[i], arr2[i]) && !isless(arr1[i], arr2[i])) counts[2]++; /* UNEQ */
        if (!isless(arr1[i], arr2[i])) counts[3]++;          /* UNGE */
        if (!islessequal(arr1[i], arr2[i])) counts[4]++;     /* UNGT */
        if (!isgreater(arr1[i], arr2[i])) counts[5]++;       /* UNLE */
        if (isless(arr1[i], arr2[i])) counts[6]++;           /* UNLT */
        if (isgreater(arr1[i], arr2[i]) || isless(arr1[i], arr2[i])) counts[7]++; /* LTGT */
    }
    
    int result = 0;
    for (int i = 0; i < 8; i++) {
        result ^= counts[i] << (i * 4);
    }
    return result;
}

/* Test 4: Long double (x87) comparisons - more likely to use condition codes */
NOINLINE int test_long_double_comparisons(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld_inf = INFINITY;
    VOLATILE_LONG_DOUBLE ld_one = 1.0L;
    VOLATILE_LONG_DOUBLE ld_zero = 0.0L;
    VOLATILE_LONG_DOUBLE ld_neg = -1.0L;
    
    int result = 0;
    
    /* Force x87 comparisons with long double */
    if (ld_nan != ld_one) result |= 1;      /* Should use UNORDERED */
    if (ld_one == ld_zero) result |= 2;     /* Should use ORDERED */
    if (!(ld_one > ld_zero) && !(ld_one < ld_zero)) result |= 4; /* UNEQ for equal? */
    if (!(ld_one < ld_zero)) result |= 8;   /* UNGE */
    if (!(ld_one <= ld_zero)) result |= 16; /* UNGT */
    if (!(ld_one > ld_zero)) result |= 32;  /* UNLE */
    if (ld_one < ld_zero) result |= 64;     /* UNLT */
    if (ld_one > ld_zero || ld_one < ld_zero) result |= 128; /* LTGT */
    
    /* Complex expression to force multiple condition codes */
    VOLATILE_LONG_DOUBLE a = ld_nan;
    VOLATILE_LONG_DOUBLE b = ld_one;
    VOLATILE_LONG_DOUBLE c = ld_zero;
    
    if (isunordered(a, b) && isordered(b, c)) result |= 256;
    if (!isless(a, b) && !isgreater(a, b)) result |= 512;
    
    return result;
}

/* Test 5: Switch statement based on comparison results */
NOINLINE int test_switch_on_comparisons(void) {
    VOLATILE_DOUBLE x = g_nan;
    VOLATILE_DOUBLE y = g_one;
    VOLATILE_DOUBLE z = g_zero;
    
    int result = 0;
    
    /* This switch should generate different condition codes */
    for (int i = 0; i < 3; i++) {
        VOLATILE_DOUBLE a = (i == 0) ? x : (i == 1) ? y : z;
        VOLATILE_DOUBLE b = (i == 2) ? x : (i == 1) ? z : y;
        
        int cmp_result = 0;
        if (isunordered(a, b)) cmp_result = 1;
        else if (isgreater(a, b)) cmp_result = 2;
        else if (isless(a, b)) cmp_result = 3;
        else if (isgreaterequal(a, b)) cmp_result = 4;
        else if (islessequal(a, b)) cmp_result = 5;
        else cmp_result = 6;
        
        switch (cmp_result) {
            case 1: result |= (1 << (i * 3)); break;     /* UNORDERED */
            case 2: result |= (2 << (i * 3)); break;     /* UNLT? */
            case 3: result |= (3 << (i * 3)); break;     /* UNGT? */
            case 4: result |= (4 << (i * 3)); break;     /* UNLE? */
            case 5: result |= (5 << (i * 3)); break;     /* UNGE? */
            case 6: result |= (6 << (i * 3)); break;     /* UNEQ? */
        }
    }
    
    return result;
}

/* Test 6: Direct use of GCC builtins for SSE2 unordered compares */
NOINLINE int test_sse2_builtins(void) {
    double d1 = g_nan;
    double d2 = g_one;
    double d3 = g_zero;
    double d4 = g_inf;
    
    int result = 0;
    
    /* Use GCC's x86 intrinsics directly */
    int c1 = __builtin_ia32_ucomisd(d1, d2);  /* UNORDERED */
    int c2 = __builtin_ia32_ucomisd(d2, d3);  /* ORDERED, GREATER */
    int c3 = __builtin_ia32_ucomisd(d3, d3);  /* ORDERED, EQUAL */
    int c4 = __builtin_ia32_ucomisd(d4, d3);  /* ORDERED, GREATER */
    
    /* Extract condition code bits */
    if (c1 & 4) result |= 1;   /* PF set for UNORDERED */
    if (!(c2 & 1)) result |= 2; /* ZF clear for GREATER */
    if (c3 & 1) result |= 4;   /* ZF set for EQUAL */
    if (!(c4 & 1) && !(c4 & 4)) result |= 8; /* Neither ZF nor PF for GREATER */
    
    /* Force conditional moves based on comparisons */
    double r1 = (d1 != d2) ? d3 : d4;
    double r2 = (d2 > d3) ? d4 : d1;
    double r3 = (isunordered(d1, d2)) ? d2 : d3;
    
    result ^= (int)(r1 + r2 + r3);
    
    return result;
}

int main(void) {
    printf("Testing x86 floating-point condition code generation...\n");
    
    /* Initialize NaN more portably */
    if (isnan(g_nan) == 0) {
        g_nan = strtod("NAN", NULL);
    }
    
    int checksum = 0;
    
    checksum ^= test_unordered_comparisons();
    printf("Test 1 result: %d\n", checksum);
    
    checksum ^= test_asm_condition_codes();
    printf("Test 2 result: %d\n", checksum);
    
    checksum ^= test_array_comparisons();
    printf("Test 3 result: %d\n", checksum);
    
    checksum ^= test_long_double_comparisons();
    printf("Test 4 result: %d\n", checksum);
    
    checksum ^= test_switch_on_comparisons();
    printf("Test 5 result: %d\n", checksum);
    
    checksum ^= test_sse2_builtins();
    printf("Test 6 result: %d\n", checksum);
    
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
