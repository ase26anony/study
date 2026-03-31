/* Test program to trigger x86 floating-point condition code output */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <stdint.h>

/* Prevent optimizations from removing crucial operations */
#define NOINLINE __attribute__((noinline, noipa))
#define VOLATILE_DOUBLE volatile double
#define VOLATILE_LONG_DOUBLE volatile long double

/* Global volatile variables to prevent constant folding */
VOLATILE_DOUBLE g_nan = NAN;
VOLATILE_DOUBLE g_inf = INFINITY;
VOLATILE_DOUBLE g_zero = 0.0;
VOLATILE_DOUBLE g_one = 1.0;
VOLATILE_DOUBLE g_neg_one = -1.0;

/* Test 1: Direct unordered comparisons with NaN */
NOINLINE int test_unordered_comparisons(void) {
    int results[8] = {0};
    VOLATILE_DOUBLE d1 = g_nan;
    VOLATILE_DOUBLE d2 = g_one;
    VOLATILE_DOUBLE d3 = g_zero;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (d1 != d2);  /* UNORDERED comparison */
    results[1] = (d1 == d1);  /* Should be false for NaN, may generate ORDERED */
    results[2] = (d2 == d2);  /* ORDERED comparison */
    results[3] = (d3 != d3);  /* Should be false for non-NaN */
    
    /* Force multiple branches to prevent optimization */
    if (isunordered(d1, d2)) results[4] = 1;
    if (isunordered(d2, d3)) results[5] = 0;
    
    /* Use all results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 6; i++) sum += results[i];
    return sum;
}

/* Test 2: Inline assembly with %C modifier for condition codes */
NOINLINE int test_asm_condition_codes(void) {
    int results = 0;
    VOLATILE_DOUBLE a = g_nan;
    VOLATILE_DOUBLE b = g_one;
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < 4; i++) {
        uint8_t result1 = 0, result2 = 0, result3 = 0;
        
        /* UNORDERED comparison */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(result1)
            : "x"(a), "x"(b)
            : "cc"
        );
        
        /* ORDERED comparison */
        VOLATILE_DOUBLE c = g_one;
        VOLATILE_DOUBLE d = g_zero;
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C1 %0"
            : "=r"(result2)
            : "x"(c), "x"(d)
            : "cc"
        );
        
        /* UNEQ comparison (unordered or equal) */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C2 %0"
            : "=r"(result3)
            : "x"(a), "x"(a)  /* Compare NaN with itself */
            : "cc"
        );
        
        results += result1 + result2 + result3;
        
        /* Modify values to prevent loop unrolling from eliminating variations */
        a += 1.0;
        b += 2.0;
    }
    
    return results;
}

/* Test 3: Math.h comparison macros that map to specific condition codes */
NOINLINE int test_math_comparisons(void) {
    VOLATILE_DOUBLE arr1[8];
    VOLATILE_DOUBLE arr2[8];
    int counts[7] = {0};  /* For different comparison types */
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 8; i++) {
        arr1[i] = (i % 3 == 0) ? g_nan : (double)i;
        arr2[i] = (i % 4 == 0) ? g_nan : (double)(i * 2);
    }
    
    /* Perform various comparisons that should generate different condition codes */
    for (int i = 0; i < 8; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]);   /* UNORDERED */
        counts[1] += !isunordered(arr1[i], arr2[i]);  /* ORDERED */
        counts[2] += isgreater(arr1[i], arr2[i]);     /* UNLE? Actually generates GT */
        counts[3] += isless(arr1[i], arr2[i]);        /* UNGE? Actually generates LT */
        counts[4] += isgreaterequal(arr1[i], arr2[i]); /* UNLT? */
        counts[5] += islessequal(arr1[i], arr2[i]);   /* UNGT? */
        
        /* LTGT (not equal and ordered) */
        counts[6] += (arr1[i] < arr2[i] || arr1[i] > arr2[i]);
    }
    
    int sum = 0;
    for (int i = 0; i < 7; i++) sum += counts[i];
    return sum;
}

/* Test 4: Long double (x87) comparisons */
NOINLINE int test_long_double_comparisons(void) {
    VOLATILE_LONG_DOUBLE ld_nan = NAN;
    VOLATILE_LONG_DOUBLE ld1 = 1.0L;
    VOLATILE_LONG_DOUBLE ld2 = 2.0L;
    VOLATILE_LONG_DOUBLE ld3 = g_nan;  /* double NaN promoted to long double */
    
    int results = 0;
    
    /* x87 style comparisons - these often generate explicit condition code checks */
    if (ld_nan != ld1) results |= 1;      /* UNORDERED */
    if (ld1 == ld1) results |= 2;         /* ORDERED */
    if (!(ld_nan < ld1)) results |= 4;    /* UNGE (nlt) */
    if (!(ld_nan <= ld1)) results |= 8;   /* UNGT (nle) */
    if (ld_nan <= ld_nan) results |= 16;  /* UNLE */
    if (ld_nan < ld_nan) results |= 32;   /* UNLT */
    if (ld1 != ld2) results |= 64;        /* LTGT (une) */
    
    /* Complex expression to force multiple condition code uses */
    VOLATILE_LONG_DOUBLE a = ld1;
    VOLATILE_LONG_DOUBLE b = ld2;
    for (int i = 0; i < 4; i++) {
        int cmp;
        __asm__ volatile (
            "fldt %1\n\t"
            "fldt %2\n\t"
            "fucomip %%st(1), %%st\n\t"
            "fstp %%st(0)\n\t"
            "setne %0"
            : "=r"(cmp)
            : "m"(a), "m"(b)
            : "cc", "st"
        );
        results += cmp;
        a += 1.0L;
        b += 2.0L;
    }
    
    return results;
}

/* Test 5: Switch statement based on comparison results */
NOINLINE int test_switch_comparisons(void) {
    VOLATILE_DOUBLE vals[6] = {g_nan, g_inf, g_zero, g_one, g_neg_one, -g_inf};
    int result = 0;
    
    for (int i = 0; i < 6; i++) {
        VOLATILE_DOUBLE a = vals[i];
        VOLATILE_DOUBLE b = vals[(i + 1) % 6];
        
        /* This switch should generate various condition code checks */
        switch (fpclassify(a)) {
            case FP_NAN:
                if (isunordered(a, b)) result += 1;  /* UNORDERED */
                break;
            case FP_INFINITE:
                if (isgreater(a, b)) result += 2;    /* Generates specific condition */
                break;
            case FP_ZERO:
                if (islessequal(a, b)) result += 3;  /* UNGT? */
                break;
            case FP_NORMAL:
                if (isless(a, b)) result += 4;       /* UNGE? */
                break;
            case FP_SUBNORMAL:
                if (isgreaterequal(a, b)) result += 5; /* UNLT? */
                break;
        }
        
        /* Additional comparisons to use more condition codes */
        if (a != b) result += 6;          /* LTGT (une) when ordered */
        if (!(a < b)) result += 7;        /* UNGE (nlt) */
        if (!(a > b)) result += 8;        /* UNLE (nle) */
    }
    
    return result;
}

/* Test 6: Mixed SSE and x87 operations */
NOINLINE int test_mixed_operations(void) {
    double sse_result = 0.0;
    long double x87_result = 0.0L;
    int checksum = 0;
    
    /* SSE2 operations */
    for (int i = 0; i < 10; i++) {
        VOLATILE_DOUBLE a = g_nan + (double)i;
        VOLATILE_DOUBLE b = (double)(i * 2);
        
        /* Force unordered compare */
        if (a != b) {
            sse_result += 1.0;
            checksum++;
        }
        
        /* Force ordered compare */
        if (b == b) {
            sse_result -= 0.5;
            checksum--;
        }
    }
    
    /* x87 operations with long double */
    for (int i = 0; i < 5; i++) {
        VOLATILE_LONG_DOUBLE a = (long double)i / 3.0L;
        VOLATILE_LONG_DOUBLE b = g_nan;
        
        /* Various comparisons that should use different condition codes */
        x87_result += (a != b) ? 1.0L : 0.0L;      /* UNORDERED when b is NaN */
        x87_result += (a == a) ? 0.5L : 0.0L;      /* ORDERED */
        x87_result += (!(a < b)) ? 0.25L : 0.0L;   /* UNGE (nlt) */
        x87_result += (!(a > b)) ? 0.125L : 0.0L;  /* UNLE (nle) */
    }
    
    /* Use results to prevent optimization */
    checksum += (int)sse_result + (int)x87_result;
    return checksum;
}

int main(void) {
    int total = 0;
    
    printf("Testing x86 floating-point condition code generation...\n");
    
    /* Run all tests to trigger various condition code paths */
    total += test_unordered_comparisons();
    total += test_asm_condition_codes();
    total += test_math_comparisons();
    total += test_long_double_comparisons();
    total += test_switch_comparisons();
    total += test_mixed_operations();
    
    printf("Result checksum: %d\n", total);
    
    /* Additional volatile operations to ensure all code executes */
    VOLATILE_DOUBLE final_check = g_nan;
    if (isunordered(final_check, g_one)) {
        printf("Final unordered check passed\n");
    }
    
    return total != 0 ? 0 : 1;
}
