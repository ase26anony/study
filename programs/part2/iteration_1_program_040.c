/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP comparison condition code generation */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Comparisons with NaN - will trigger UNORDERED paths */
    if (v1 == v_nan) result ^= 64;      /* UNORDERED/UNEQ */
    if (v1 != v_nan) result ^= 128;     /* ORDERED/LTGT */
    if (v1 < v_nan)  result ^= 256;     /* UNORDERED/UNLT */
    if (v1 <= v_nan) result ^= 512;     /* UNORDERED/UNLE */
    if (v1 > v_nan)  result ^= 1024;    /* UNORDERED/UNGT */
    if (v1 >= v_nan) result ^= 2048;    /* UNORDERED/UNGE */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096; /* Always false - UNORDERED */
    if (v_nan != v_nan) result ^= 8192; /* Always true - UNORDERED */
    
    /* Comparisons with infinity */
    if (v1 == v_inf) result ^= 16384;
    if (v_inf <= v_neg_inf) result ^= 32768;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v_nan) ? 1.0 : 2.0;
    result += (int)cond_result;
    
    cond_result = (v_nan >= v2) ? 3.0 : 4.0;
    result += (int)cond_result;
    
    /* Goto-based control flow to prevent optimization */
    if (v1 != v1) { /* Check for NaN */
        goto unordered_path;
    }
    
    if (v1 < v2) {
        goto lt_path;
    } else {
        goto ge_path;
    }

lt_path:
    result += 100;
    goto continue_main;

ge_path:
    result += 200;
    goto continue_main;

unordered_path:
    result += 300;
    /* More unordered comparisons */
    if (v_nan < v_inf) result += 400;  /* UNORDERED/UNLT */
    if (v_inf > v_nan) result += 500;  /* UNORDERED/UNGT */
    goto continue_main;

continue_main:
    
    /* Vectorized comparisons using GCC vector extensions */
    v2df vec1 = {a, b};
    v2df vec2 = {b, a};
    v2df vec_nan = {nan_val, nan_val};
    
    /* Vector comparisons generate cmppd/ucomisd with condition codes */
    v2di mask_eq = (v2di)(vec1 == vec2);
    v2di mask_neq = (v2di)(vec1 != vec2);
    v2di mask_lt = (v2di)(vec1 < vec2);
    v2di mask_le = (v2di)(vec1 <= vec2);
    v2di mask_gt = (v2di)(vec1 > vec2);
    v2di mask_ge = (v2di)(vec1 >= vec2);
    
    /* Vector comparisons with NaN */
    v2di mask_nan_eq = (v2di)(vec1 == vec_nan);
    v2di mask_nan_neq = (v2di)(vec1 != vec_nan);
    v2di mask_nan_lt = (v2di)(vec1 < vec_nan);
    v2di mask_nan_le = (v2di)(vec1 <= vec_nan);
    
    /* Extract results from vector masks */
    long long *mask_ptr = (long long*)&mask_eq;
    result += (int)(mask_ptr[0] & 1);
    result += (int)(mask_ptr[1] & 1);
    
    mask_ptr = (long long*)&mask_nan_eq;
    result += (int)(mask_ptr[0] & 1);
    result += (int)(mask_ptr[1] & 1);
    
    /* Loop with vector comparisons */
    double arr1[4] = {a, b, nan_val, inf_val};
    double arr2[4] = {b, a, inf_val, nan_val};
    int cmp_results[4] = {0};
    
    for (int i = 0; i < 4; i++) {
        volatile double x = arr1[i];
        volatile double y = arr2[i];
        
        /* Exhaustive comparison in loop */
        cmp_results[i] = (x == y) ? 1 : 0;
        cmp_results[i] += (x != y) ? 2 : 0;
        cmp_results[i] += (x < y) ? 4 : 0;
        cmp_results[i] += (x <= y) ? 8 : 0;
        cmp_results[i] += (x > y) ? 16 : 0;
        cmp_results[i] += (x >= y) ? 32 : 0;
        
        result += cmp_results[i];
    }
    
    return result;
}

/* Function with inline assembly that uses FP condition codes */
static int inline_asm_fp_conds(double a, double b, double nan_val) {
    int result = 0;
    int cc_result;
    
    /* Inline assembly that reads FP condition codes */
    
    /* Test for UNORDERED (parity flag) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (nan_val)
        : "cc", "eax"
    );
    result += cc_result;
    
    /* Test for ORDERED (not parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "cc", "eax"
    );
    result += cc_result * 2;
    
    /* Test for less than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "cc", "eax"
    );
    result += cc_result * 4;
    
    /* Test for less or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (a), "x" (b)
        : "cc", "eax"
    );
    result += cc_result * 8;
    
    /* Test for greater than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (b), "x" (a)  /* reversed for greater than */
        : "cc", "eax"
    );
    result += cc_result * 16;
    
    /* Test for greater or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (b), "x" (a)  /* reversed for greater or equal */
        : "cc", "eax"
    );
    result += cc_result * 32;
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fld1\n\t"
        "fldz\n\t"
        "fcmovbe %%st(1), %%st(0)\n\t"
        "fstp %0\n\t"
        "fstp %%st(0)"
        : "=m" (cmov_result)
        : "x" (a), "x" (b)
        : "cc", "st", "st(1)"
    );
    
    result += (int)cmov_result;
    
    return result;
}

int main(void) {
    /* Initialize FP special values */
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double zero = 0.0;
    double neg_zero = -0.0;
    
    int final_result = 0;
    
    /* Test various combinations of values */
    final_result += stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    final_result += stress_fp_comparisons(normal1, nan_val, nan_val, inf_val, neg_inf_val);
    final_result += stress_fp_comparisons(nan_val, normal2, nan_val, inf_val, neg_inf_val);
    final_result += stress_fp_comparisons(inf_val, normal1, nan_val, inf_val, neg_inf_val);
    final_result += stress_fp_comparisons(neg_inf_val, inf_val, nan_val, inf_val, neg_inf_val);
    final_result += stress_fp_comparisons(zero, neg_zero, nan_val, inf_val, neg_inf_val);
    
    /* Test inline assembly paths */
    final_result += inline_asm_fp_conds(normal1, normal2, nan_val);
    final_result += inline_asm_fp_conds(nan_val, normal1, nan_val);
    final_result += inline_asm_fp_conds(inf_val, neg_inf_val, nan_val);
    
    /* Additional unordered comparison tests */
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    
    /* These should trigger UNORDERED, UNLT, UNLE, UNGT, UNGE, UNEQ */
    if (!(v_nan < v_inf)) final_result += 1;   /* UNGE (nlt) */
    if (!(v_inf > v_nan)) final_result += 2;   /* UNLE (nle) */
    if (v_nan == v_nan)   final_result += 4;   /* Always false - UNORDERED */
    if (v_nan != v_nan)   final_result += 8;   /* Always true - UNORDERED */
    
    /* LTGT (une) - ordered and not equal */
    if (normal1 != normal2 && !(normal1 != normal1) && !(normal2 != normal2)) {
        final_result += 16;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
