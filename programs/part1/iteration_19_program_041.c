/* test_condition_codes.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <float.h>

/* Prevent optimizations that would remove critical operations */
#define NO_OPT __attribute__((noinline, noipa, noclone))

/* Test 1: Unordered comparisons with NaN using != operator */
NO_OPT int test_unordered_comparisons(void) {
    volatile double nan_val = NAN;
    volatile double normal_val = 3.14159;
    volatile double inf_val = INFINITY;
    volatile double neg_inf = -INFINITY;
    
    int results[8] = {0};
    
    /* These should generate UNORDERED/ORDERED condition codes */
    results[0] = (nan_val != normal_val) ? 1 : 0;      /* UNORDERED */
    results[1] = (normal_val == normal_val) ? 1 : 0;   /* ORDERED */
    results[2] = (nan_val == nan_val) ? 1 : 0;         /* UNORDERED */
    results[3] = (normal_val != inf_val) ? 1 : 0;      /* ORDERED */
    
    /* Mixed comparisons */
    results[4] = (inf_val != neg_inf) ? 1 : 0;
    results[5] = (normal_val != nan_val) ? 1 : 0;
    results[6] = (inf_val == inf_val) ? 1 : 0;
    results[7] = (neg_inf == neg_inf) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 2: Using math.h comparison macros */
NO_OPT int test_math_macros(void) {
    volatile double a = NAN;
    volatile double b = 2.71828;
    volatile double c = INFINITY;
    volatile double d = -INFINITY;
    volatile double e = 0.0;
    
    int results[12] = {0};
    
    /* These map to various condition codes */
    results[0] = isunordered(a, b);    /* UNORDERED */
    results[1] = isordered(b, c);      /* ORDERED */
    results[2] = !isgreater(a, b);     /* UNLE/UNLT */
    results[3] = !isless(b, a);        /* UNGE/UNGT */
    results[4] = islessequal(d, e);    /* UNLE/LE */
    results[5] = isgreaterequal(c, e); /* UNGE/GE */
    results[6] = islessgreater(b, c);  /* LTGT */
    
    /* More complex expressions */
    results[7] = isunordered(a, a) && isordered(b, b);
    results[8] = !isgreater(c, d) && !isless(d, c);
    results[9] = islessequal(e, e) || isgreaterequal(e, e);
    results[10] = islessgreater(c, d) && isordered(c, d);
    results[11] = !isunordered(b, e) && isgreater(b, e);
    
    int sum = 0;
    for (int i = 0; i < 12; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 3: Inline assembly with %C modifier */
NO_OPT int test_inline_asm(void) {
    volatile double x = NAN;
    volatile double y = 1.41421;
    volatile double z = 0.0;
    volatile double w = -1.0;
    
    int results[16] = {0};
    unsigned char byte_result;
    
    /* Test various condition codes via inline assembly */
    for (int i = 0; i < 4; i++) {
        volatile double* ptrs[4] = {&x, &y, &z, &w};
        volatile double val1 = *ptrs[i];
        volatile double val2 = *ptrs[(i + 1) % 4];
        
        /* Using %C to get condition code name */
        __asm__ volatile (
            "ucomisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(byte_result)
            : "x"(val1), "x"(val2)
            : "cc"
        );
        results[i * 4] = byte_result;
        
        /* Different condition */
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(byte_result)
            : "x"(val1), "x"(val2)
            : "cc"
        );
        results[i * 4 + 1] = byte_result;
        
        /* Test with memory operand */
        __asm__ volatile (
            "ucomisd %1, %1\n\t"
            "set%C0 %0"
            : "=r"(byte_result)
            : "m"(val1)
            : "cc"
        );
        results[i * 4 + 2] = byte_result;
        
        /* Another variant */
        __asm__ volatile (
            "comisd %2, %1\n\t"
            "set%C0 %0"
            : "=r"(byte_result)
            : "x"(val2), "x"(val1)
            : "cc"
        );
        results[i * 4 + 3] = byte_result;
    }
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 4: Long double (x87) operations */
NO_OPT int test_long_double(void) {
    volatile long double ld_nan = NAN;
    volatile long double ld_inf = INFINITY;
    volatile long double ld_normal = 3.14159265358979323846L;
    volatile long double ld_zero = 0.0L;
    volatile long double ld_neg = -2.5L;
    
    int results[10] = {0};
    
    /* x87 comparisons generate different condition codes */
    results[0] = (ld_nan != ld_normal) ? 1 : 0;
    results[1] = (ld_normal == ld_normal) ? 1 : 0;
    results[2] = (ld_inf > ld_normal) ? 1 : 0;
    results[3] = (ld_normal < ld_inf) ? 1 : 0;
    results[4] = (ld_zero >= ld_neg) ? 1 : 0;
    results[5] = (ld_neg <= ld_zero) ? 1 : 0;
    
    /* Complex long double expressions */
    volatile long double temp = ld_normal + ld_neg;
    results[6] = (temp != ld_nan) ? 1 : 0;
    results[7] = (ld_inf == ld_inf) ? 1 : 0;
    results[8] = (ld_nan == ld_nan) ? 1 : 0;
    results[9] = (ld_zero != ld_inf) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 5: Array operations with unordered checks */
NO_OPT int test_array_operations(void) {
    volatile double arr1[8];
    volatile double arr2[8];
    
    /* Initialize with mix of values */
    arr1[0] = NAN;
    arr2[0] = 1.0;
    arr1[1] = 2.0;
    arr2[1] = NAN;
    arr1[2] = INFINITY;
    arr2[2] = INFINITY;
    arr1[3] = -INFINITY;
    arr2[3] = 0.0;
    arr1[4] = 3.14;
    arr2[4] = 2.71;
    arr1[5] = NAN;
    arr2[5] = NAN;
    arr1[6] = 0.0;
    arr2[6] = -0.0;
    arr1[7] = 100.0;
    arr2[7] = 100.0;
    
    int counts[6] = {0};
    
    for (int i = 0; i < 8; i++) {
        counts[0] += isunordered(arr1[i], arr2[i]) ? 1 : 0;  /* UNORDERED */
        counts[1] += isordered(arr1[i], arr2[i]) ? 1 : 0;    /* ORDERED */
        counts[2] += isgreater(arr1[i], arr2[i]) ? 1 : 0;    /* UNLE inverse */
        counts[3] += isless(arr1[i], arr2[i]) ? 1 : 0;       /* UNGE inverse */
        counts[4] += islessequal(arr1[i], arr2[i]) ? 1 : 0;  /* UNGT inverse */
        counts[5] += islessgreater(arr1[i], arr2[i]) ? 1 : 0; /* UNEQ inverse */
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += counts[i];
    }
    return sum;
}

/* Test 6: Switch based on fpclassify results */
NO_OPT int test_fpclassify_switch(void) {
    volatile double values[6] = {
        NAN,
        INFINITY,
        -INFINITY,
        0.0,
        -0.0,
        42.0
    };
    
    int results[6] = {0};
    
    for (int i = 0; i < 6; i++) {
        switch (fpclassify(values[i])) {
            case FP_NAN:
                results[i] = 1;
                break;
            case FP_INFINITE:
                results[i] = 2;
                break;
            case FP_ZERO:
                results[i] = 3;
                break;
            case FP_SUBNORMAL:
                results[i] = 4;
                break;
            case FP_NORMAL:
                results[i] = 5;
                break;
            default:
                results[i] = 0;
        }
    }
    
    int sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += results[i];
    }
    return sum;
}

/* Test 7: Mixed SSE and x87 operations */
NO_OPT int test_mixed_operations(void) {
    volatile float f_nan = NAN;
    volatile float f_val = 1.5f;
    volatile double d_nan = NAN;
    volatile double d_val = 2.5;
    volatile long double ld_nan = NAN;
    volatile long double ld_val = 3.5L;
    
    int results[8] = {0};
    
    /* Mixed precision comparisons */
    results[0] = (f_nan != f_val) ? 1 : 0;
    results[1] = (d_nan == d_nan) ? 1 : 0;
    results[2] = (ld_nan != ld_val) ? 1 : 0;
    
    /* Cross-type comparisons */
    results[3] = ((double)f_nan != d_val) ? 1 : 0;
    results[4] = ((long double)d_nan == ld_val) ? 1 : 0;
    results[5] = (f_val < (float)d_val) ? 1 : 0;
    results[6] = (d_val > (double)ld_val) ? 1 : 0;
    results[7] = (ld_val != (long double)f_nan) ? 1 : 0;
    
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += results[i];
    }
    return sum;
}

/* Main function that runs all tests */
int main(void) {
    int total = 0;
    
    printf("Running condition code tests...\n");
    
    total += test_unordered_comparisons();
    total += test_math_macros();
    total += test_inline_asm();
    total += test_long_double();
    total += test_array_operations();
    total += test_fpclassify_switch();
    total += test_mixed_operations();
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 1000) {
        printf("Unexpectedly large result\n");
    }
    
    return total & 0xFF;
}
