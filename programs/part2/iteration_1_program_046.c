/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_conds fp_conds.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_conds_vec fp_conds.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_conds_32 fp_conds.c */

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
    
    /* Normal vs Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Normal vs NaN - triggers UNORDERED cases */
    if (v1 == v_nan) result ^= 64;
    if (v1 != v_nan) result ^= 128;
    if (v1 < v_nan)  result ^= 256;
    if (v1 <= v_nan) result ^= 512;
    if (v1 > v_nan)  result ^= 1024;
    if (v1 >= v_nan) result ^= 2048;
    
    /* NaN vs Normal */
    if (v_nan == v2) result ^= 4096;
    if (v_nan != v2) result ^= 8192;
    if (v_nan < v2)  result ^= 16384;
    if (v_nan <= v2) result ^= 32768;
    if (v_nan > v2)  result ^= 65536;
    if (v_nan >= v2) result ^= 131072;
    
    /* NaN vs NaN */
    if (v_nan == v_nan) result ^= 262144;  /* Always false */
    if (v_nan != v_nan) result ^= 524288;  /* Always true */
    if (v_nan < v_nan)  result ^= 1048576;
    if (v_nan <= v_nan) result ^= 2097152;
    if (v_nan > v_nan)  result ^= 4194304;
    if (v_nan >= v_nan) result ^= 8388608;
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 16777216;
    if (v_neg_inf < v_inf) result ^= 33554432;
    if (v_inf > v_neg_inf) result ^= 67108864;
    
    /* Conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? 1.0 : 2.0;
    result ^= (int)cmov_result;
    
    cmov_result = (v1 != v_nan) ? 3.0 : 4.0;
    result ^= (int)cmov_result;
    
    /* Complex conditional expressions */
    double complex_cond = ((v1 < v2) && (v1 != v_nan)) ? 5.0 : 
                         ((v1 > v2) || (v_nan == v_nan)) ? 6.0 : 7.0;
    result ^= (int)complex_cond;
    
    return result;
}

/* Function with goto-based control flow to prevent optimization */
static int stress_with_goto(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    int checksum = 0;
    
    /* Use goto to create non-linear control flow */
    if (x < y) {
        checksum ^= 1;
        goto label1;
    } else {
        checksum ^= 2;
        goto label2;
    }
    
label1:
    if (x == nan) {
        checksum ^= 4;
        goto label3;
    }
    if (x != nan) {
        checksum ^= 8;
    }
    goto label4;
    
label2:
    if (y > nan) {
        checksum ^= 16;
        goto label4;
    }
    goto label3;
    
label3:
    if (nan <= x) {
        checksum ^= 32;
    }
    /* Fall through */
    
label4:
    if (x >= y) {
        checksum ^= 64;
    }
    
    return checksum;
}

/* Vectorized FP comparisons */
static v2di vector_fp_comparisons(v2df a, v2df b, v2df nan_vec) {
    /* Compare vectors - generates cmppd/ucomisd with condition codes */
    v2df cmp_eq = a == b;      /* EQ */
    v2df cmp_neq = a != b;     /* NEQ/UNORDERED */
    v2df cmp_lt = a < b;       /* LT */
    v2df cmp_le = a <= b;      /* LE */
    v2df cmp_gt = a > b;       /* GT */
    v2df cmp_ge = a >= b;      /* GE */
    
    /* Comparisons with NaN */
    v2df cmp_nan_eq = a == nan_vec;    /* UNORDERED */
    v2df cmp_nan_neq = a != nan_vec;   /* ORDERED */
    v2df cmp_nan_lt = a < nan_vec;     /* UNORDERED */
    
    /* Combine results */
    v2di mask = (v2di)cmp_eq | (v2di)cmp_neq | (v2di)cmp_lt |
                (v2di)cmp_le | (v2di)cmp_gt | (v2di)cmp_ge |
                (v2di)cmp_nan_eq | (v2di)cmp_nan_neq | (v2di)cmp_nan_lt;
    
    return mask;
}

/* Inline assembly that uses FP condition codes directly */
static int inline_asm_fp_conds(double a, double b) {
    int result = 0;
    char cc_result;
    
    /* ucomisd with setp (parity/UNORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= cc_result;
    
    /* ucomisd with seta (above/UNORDERED & GT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    /* ucomisd with setb (below/UNORDERED & LT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    /* ucomisd with sete (equal/EQ & ORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    return result;
}

/* Main test function */
static int test_all_fp_conditions(void) {
    /* Initialize FP values */
    double normal1 = 1.5;
    double normal2 = 2.5;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    int total_checksum = 0;
    
    /* Test various value combinations */
    total_checksum ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    total_checksum ^= stress_fp_comparisons(normal2, normal1, nan_val, inf_val, neg_inf_val);
    total_checksum ^= stress_fp_comparisons(normal1, nan_val, nan_val, inf_val, neg_inf_val);
    total_checksum ^= stress_fp_comparisons(nan_val, normal2, nan_val, inf_val, neg_inf_val);
    total_checksum ^= stress_fp_comparisons(zero, neg_zero, nan_val, inf_val, neg_inf_val);
    total_checksum ^= stress_fp_comparisons(inf_val, neg_inf_val, nan_val, inf_val, neg_inf_val);
    
    /* Test with goto control flow */
    total_checksum ^= stress_with_goto(normal1, normal2, nan_val);
    total_checksum ^= stress_with_goto(nan_val, normal1, nan_val);
    
    /* Vectorized comparisons */
    v2df vec_a = {normal1, normal2};
    v2df vec_b = {normal2, normal1};
    v2df vec_nan = {nan_val, nan_val};
    
    v2di vec_result = vector_fp_comparisons(vec_a, vec_b, vec_nan);
    total_checksum ^= vec_result[0] ^ vec_result[1];
    
    /* More vector tests */
    v2df vec_inf = {inf_val, neg_inf_val};
    v2df vec_mixed = {normal1, nan_val};
    
    vec_result = vector_fp_comparisons(vec_a, vec_inf, vec_nan);
    total_checksum ^= vec_result[0];
    vec_result = vector_fp_comparisons(vec_mixed, vec_b, vec_nan);
    total_checksum ^= vec_result[1];
    
    /* Inline assembly tests */
    total_checksum ^= inline_asm_fp_conds(normal1, normal2);
    total_checksum ^= inline_asm_fp_conds(normal1, nan_val);
    total_checksum ^= inline_asm_fp_conds(nan_val, normal2);
    total_checksum ^= inline_asm_fp_conds(inf_val, normal1);
    total_checksum ^= inline_asm_fp_conds(zero, neg_zero);
    
    /* Loop with array comparisons to force vectorization */
    double arr1[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double arr2[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    double arr_nan[8];
    int mask[8];
    
    /* Fill with NaN and normal values */
    for (int i = 0; i < 8; i++) {
        arr_nan[i] = (i % 3 == 0) ? nan_val : (double)i;
    }
    
    /* Perform various comparisons in loop */
    for (int i = 0; i < 8; i++) {
        mask[i] = 0;
        if (arr1[i] == arr2[i]) mask[i] |= 1;
        if (arr1[i] != arr2[i]) mask[i] |= 2;
        if (arr1[i] < arr2[i])  mask[i] |= 4;
        if (arr1[i] <= arr2[i]) mask[i] |= 8;
        if (arr1[i] > arr2[i])  mask[i] |= 16;
        if (arr1[i] >= arr2[i]) mask[i] |= 32;
        
        /* NaN comparisons */
        if (arr1[i] == arr_nan[i]) mask[i] |= 64;
        if (arr1[i] != arr_nan[i]) mask[i] |= 128;
        if (arr_nan[i] < arr2[i])  mask[i] |= 256;
        
        total_checksum ^= mask[i];
    }
    
    return total_checksum;
}

int main(void) {
    int result = test_all_fp_conditions();
    
    /* Use result to prevent dead code elimination */
    printf("FP condition test checksum: %d\n", result);
    
    /* Additional volatile store to force all computations */
    volatile int sink = result;
    
    return sink != 0 ? 0 : 1;
}
