/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Function to stress FP comparison condition code generation */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal vs Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Normal vs NaN - triggers UNORDERED cases */
    if (v1 == v_nan) result ^= 64;      /* Always false, but compiler doesn't know */
    if (v1 != v_nan) result ^= 128;     /* UNORDERED or ORDERED */
    if (v1 < v_nan)  result ^= 256;     /* UNORDERED */
    if (v1 <= v_nan) result ^= 512;     /* UNORDERED */
    if (v1 > v_nan)  result ^= 1024;    /* UNORDERED */
    if (v1 >= v_nan) result ^= 2048;    /* UNORDERED */
    
    /* NaN vs NaN */
    if (v_nan == v_nan) result ^= 4096; /* Always false - UNORDERED */
    if (v_nan != v_nan) result ^= 8192; /* Always true - UNORDERED */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 16384;
    if (v_inf > v_neg_inf) result ^= 32768;
    if (v_neg_inf < v_inf) result ^= 65536;
    
    /* Normal vs Infinity */
    if (v1 < v_inf) result ^= 131072;
    if (v1 > v_neg_inf) result ^= 262144;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 == v2) ? 1.0 : 
                        (v1 != v2) ? 2.0 :
                        (v1 < v2)  ? 3.0 :
                        (v1 <= v2) ? 4.0 :
                        (v1 > v2)  ? 5.0 :
                        (v1 >= v2) ? 6.0 : 7.0;
    
    result ^= (int)cond_result;
    
    /* More complex with NaN */
    cond_result = (v1 == v_nan) ? 8.0 :
                  (v1 != v_nan) ? 9.0 :
                  (v1 < v_nan)  ? 10.0 :
                  (v1 <= v_nan) ? 11.0 :
                  (v1 > v_nan)  ? 12.0 :
                  (v1 >= v_nan) ? 13.0 : 14.0;
    
    result ^= (int)cond_result;
    
    return result;
}

/* Function with goto-based control flow to prevent optimization */
static int stress_fp_with_goto(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    int checksum = 0;
    
    /* Complex control flow with goto */
    if (x == y) {
        checksum ^= 1;
        goto label1;
    }
    
    if (x != y) {
        checksum ^= 2;
        goto label2;
    }
    
label1:
    if (x < y) {
        checksum ^= 4;
        goto label3;
    }
    
label2:
    if (x <= y) {
        checksum ^= 8;
        goto label4;
    }
    
label3:
    if (x > y) {
        checksum ^= 16;
        goto label5;
    }
    
label4:
    if (x >= y) {
        checksum ^= 32;
        goto label6;
    }
    
label5:
    /* NaN comparisons with goto */
    if (x == nan) {
        checksum ^= 64;
        goto label7;
    }
    
label6:
    if (x != nan) {
        checksum ^= 128;
        goto label8;
    }
    
label7:
    if (x < nan) {
        checksum ^= 256;
        goto label9;
    }
    
label8:
    if (x <= nan) {
        checksum ^= 512;
        goto label10;
    }
    
label9:
    if (x > nan) {
        checksum ^= 1024;
        goto label11;
    }
    
label10:
    if (x >= nan) {
        checksum ^= 2048;
    }
    
label11:
    return checksum;
}

/* Vectorized FP comparisons */
static int vector_fp_comparisons(double a, double b, double nan_val) {
    v2df vec1 = {a, a * 2.0};
    v2df vec2 = {b, b * 3.0};
    v2df vec_nan = {nan_val, nan_val};
    
    /* Various vector comparisons */
    v2di cmp_eq = (vec1 == vec2);
    v2di cmp_ne = (vec1 != vec2);
    v2di cmp_lt = (vec1 < vec2);
    v2di cmp_le = (vec1 <= vec2);
    v2di cmp_gt = (vec1 > vec2);
    v2di cmp_ge = (vec1 >= vec2);
    
    /* Comparisons with NaN */
    v2di cmp_nan_eq = (vec1 == vec_nan);
    v2di cmp_nan_ne = (vec1 != vec_nan);
    v2di cmp_nan_lt = (vec1 < vec_nan);
    v2di cmp_nan_le = (vec1 <= vec_nan);
    
    /* Extract results */
    long long *eq_ptr = (long long*)&cmp_eq;
    long long *ne_ptr = (long long*)&cmp_ne;
    long long *lt_ptr = (long long*)&cmp_lt;
    long long *nan_eq_ptr = (long long*)&cmp_nan_eq;
    
    int result = 0;
    result ^= (int)(eq_ptr[0] & 1);
    result ^= (int)(ne_ptr[0] & 1) << 1;
    result ^= (int)(lt_ptr[0] & 1) << 2;
    result ^= (int)(nan_eq_ptr[0] & 1) << 3;
    
    return result;
}

/* Inline assembly with condition codes */
static int inline_asm_fp_conditions(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    int result = 0;
    char cc_result;
    
    /* ucomisd with setp (parity/UNORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan)
        : "cc"
    );
    result ^= (cc_result & 1);
    
    /* ucomisd with seta (above/UNORDERED or GT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result & 1) << 1;
    
    /* ucomisd with setb (below/UNORDERED or LT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result & 1) << 2;
    
    /* ucomisd with sete (equal/UNORDERED or EQ) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result & 1) << 3;
    
    return result;
}

/* Main test function */
int main(void) {
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int checksum = 0;
    
    /* Test 1: Exhaustive FP comparisons */
    checksum ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    
    /* Test 2: Same value comparisons */
    checksum ^= stress_fp_comparisons(normal1, normal1, nan_val, inf_val, neg_inf_val);
    
    /* Test 3: NaN with itself */
    checksum ^= stress_fp_comparisons(nan_val, nan_val, nan_val, inf_val, neg_inf_val);
    
    /* Test 4: Infinity comparisons */
    checksum ^= stress_fp_comparisons(inf_val, neg_inf_val, nan_val, inf_val, neg_inf_val);
    
    /* Test 5: Goto-based control flow */
    checksum ^= stress_fp_with_goto(normal1, normal2, nan_val);
    checksum ^= stress_fp_with_goto(normal1, normal1, nan_val);
    checksum ^= stress_fp_with_goto(nan_val, normal1, nan_val);
    
    /* Test 6: Vectorized comparisons */
    checksum ^= vector_fp_comparisons(normal1, normal2, nan_val);
    checksum ^= vector_fp_comparisons(normal1, normal1, nan_val);
    checksum ^= vector_fp_comparisons(nan_val, normal2, nan_val);
    
    /* Test 7: Inline assembly */
    checksum ^= inline_asm_fp_conditions(normal1, normal2, nan_val);
    checksum ^= inline_asm_fp_conditions(normal1, nan_val, nan_val);
    checksum ^= inline_asm_fp_conditions(nan_val, nan_val, nan_val);
    
    /* Loop with array comparisons to encourage vectorization */
    double arr1[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double arr2[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    double arr_nan[8];
    
    for (int i = 0; i < 8; i++) {
        arr_nan[i] = (i % 2 == 0) ? nan_val : arr1[i];
    }
    
    int mask_result = 0;
    for (int i = 0; i < 8; i++) {
        volatile double *ptr1 = &arr1[i];
        volatile double *ptr2 = &arr2[i];
        volatile double *ptr_nan = &arr_nan[i];
        
        if (*ptr1 == *ptr2) mask_result ^= (1 << i);
        if (*ptr1 != *ptr_nan) mask_result ^= (1 << (i + 8));
        if (*ptr1 < *ptr2) mask_result ^= (1 << (i + 16));
        if (*ptr_nan > *ptr1) mask_result ^= (1 << (i + 24));
    }
    
    checksum ^= mask_result;
    
    /* Final output to prevent optimization */
    printf("FP condition code test checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
