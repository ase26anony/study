/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_cond_test_vec fp_cond_test.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_cond_test_32 fp_cond_test.c */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Stress function with exhaustive FP comparisons */
static int fp_comparison_stress(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    int temp = 0;
    
    /* Complex control flow with goto to prevent optimization */
    if (v1 == v2) goto label_eq;
    if (v1 != v2) goto label_neq;
    
label_eq:
    /* ORDERED and UNEQ paths */
    if (v1 == v2) {
        /* This should generate ordered equal comparison */
        result += 1;
    }
    
    /* UNORDERED path - comparisons with NaN */
    if (v_nan == v1) {  /* Always false, but compiler doesn't know */
        result += 2;
    }
    
    if (v1 == v_nan) {  /* Another unordered comparison */
        result += 4;
    }
    
    /* LTGT path (ordered and not equal) */
    if (v1 < v2 || v1 > v2) {
        result += 8;
    }
    
label_neq:
    /* UNLT path (unordered or less than) */
    if (v1 < v_nan) {  /* Unordered comparison */
        result += 16;
    }
    
    /* UNLE path (unordered or less than or equal) */
    if (v1 <= v_nan) {
        result += 32;
    }
    
    /* UNGE path (unordered or greater than or equal) */
    if (v_nan >= v1) {
        result += 64;
    }
    
    /* UNGT path (unordered or greater than) */
    if (v_nan > v1) {
        result += 128;
    }
    
    /* More complex comparisons mixing NaN and normal values */
    double arr[5] = {v1, v2, v_nan, v_inf, v_neg_inf};
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            /* Exhaustive comparison matrix */
            if (arr[i] == arr[j]) result ^= (i * j + 1);
            if (arr[i] != arr[j]) result ^= (i * j + 2);
            if (arr[i] < arr[j])  result ^= (i * j + 3);
            if (arr[i] <= arr[j]) result ^= (i * j + 4);
            if (arr[i] > arr[j])  result ^= (i * j + 5);
            if (arr[i] >= arr[j]) result ^= (i * j + 6);
        }
    }
    
    /* Conditional moves using FP comparison results */
    double cmov_result = (v1 < v2) ? v1 : v2;
    cmov_result = (v_nan == v_nan) ? cmov_result : v_inf;  /* NaN != NaN */
    cmov_result = (v1 <= v_inf) ? cmov_result : v_neg_inf;
    
    /* Cast to int to use in result */
    result += (int)cmov_result;
    
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), 3.0};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Vector comparisons generating cmppd/ucomisd with condition codes */
    v2di mask_eq = (v2di)(vec1 == vec2);
    v2di mask_neq = (v2di)(vec1 != vec2);
    v2di mask_lt = (v2di)(vec1 < vec2);
    v2di mask_le = (v2di)(vec1 <= vec2);
    v2di mask_gt = (v2di)(vec1 > vec2);
    v2di mask_ge = (v2di)(vec1 >= vec2);
    
    /* Comparisons with NaN */
    v2di mask_nan_eq = (v2di)(vec1 == vec_nan);
    v2di mask_nan_neq = (v2di)(vec1 != vec_nan);
    v2di mask_nan_lt = (v2di)(vec1 < vec_nan);
    v2di mask_nan_le = (v2di)(vec1 <= vec_nan);
    
    /* Extract results */
    long long *m1 = (long long*)&mask_eq;
    long long *m2 = (long long*)&mask_nan_eq;
    
    return (int)(m1[0] ^ m1[1] ^ m2[0] ^ m2[1]);
}

/* Function with inline assembly using condition codes */
static int asm_fp_condition_codes(double a, double b, double nan_val) {
    int result = 0;
    int unordered_flag = 0;
    int equal_flag = 0;
    int less_flag = 0;
    
    /* Inline assembly that reads FP condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"           /* Compare a and b */
        "setp %%al\n\t"                /* Set if unordered (parity) */
        "sete %%bl\n\t"                /* Set if equal */
        "setb %%cl\n\t"                /* Set if below (less than) */
        : "=a"(unordered_flag), "=b"(equal_flag), "=c"(less_flag)
        : "x"(a), "x"(b)
        : "cc"
    );
    
    result = unordered_flag * 1 + equal_flag * 2 + less_flag * 4;
    
    /* Another asm with different condition */
    int greater_flag = 0;
    int not_less_equal = 0;
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %%al\n\t"                /* Set if above (greater than) */
        "setnbe %%bl\n\t"              /* Set if not below or equal */
        : "=a"(greater_flag), "=b"(not_less_equal)
        : "x"(b), "x"(a)               /* Swapped order */
        : "cc"
    );
    
    result += greater_flag * 8 + not_less_equal * 16;
    
    /* Comparison with NaN */
    int nan_unordered = 0;
    asm volatile (
        "ucomisd %1, %1\n\t"           /* NaN comparison with itself */
        "setp %%al\n\t"
        : "=a"(nan_unordered)
        : "x"(nan_val)
        : "cc"
    );
    
    result += nan_unordered * 32;
    
    return result;
}

/* Main function with all test cases */
int main(void) {
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double zero = 0.0;
    double neg_zero = -0.0;
    
    printf("Starting FP comparison coverage test...\n");
    
    /* Test 1: Exhaustive scalar comparisons */
    int result1 = fp_comparison_stress(normal1, normal2, nan_val, inf_val, neg_inf_val);
    printf("Scalar comparisons result: %d\n", result1);
    
    /* Test 2: Special value combinations */
    int result2 = fp_comparison_stress(inf_val, neg_inf_val, nan_val, inf_val, neg_inf_val);
    result2 += fp_comparison_stress(zero, neg_zero, nan_val, inf_val, neg_inf_val);
    result2 += fp_comparison_stress(normal1, inf_val, nan_val, inf_val, neg_inf_val);
    printf("Special value comparisons result: %d\n", result2);
    
    /* Test 3: Vectorized comparisons */
    int result3 = vector_fp_comparisons();
    printf("Vector comparisons result: %d\n", result3);
    
    /* Test 4: Inline assembly with condition codes */
    int result4 = asm_fp_condition_codes(normal1, normal2, nan_val);
    result4 += asm_fp_condition_codes(inf_val, normal1, nan_val);
    result4 += asm_fp_condition_codes(nan_val, normal1, nan_val);
    printf("Inline assembly result: %d\n", result4);
    
    /* Additional complex control flow with FP comparisons */
    volatile double x = normal1;
    volatile double y = normal2;
    int final_result = 0;
    
    /* Complex if-else chain covering different conditions */
    if (x < y) {
        final_result += 1;
        if (x != y) {
            final_result += 2;
            if (x <= y) {
                final_result += 4;
            }
        }
    } else if (x > y) {
        final_result += 8;
    } else {
        final_result += 16;
    }
    
    /* NaN comparisons in control flow */
    if (!(nan_val == nan_val)) {  /* NaN != NaN is true */
        final_result += 32;
    }
    
    if (x < nan_val || nan_val < x) {
        final_result += 64;
    }
    
    if (x <= nan_val && nan_val <= x) {
        final_result += 128;
    }
    
    /* Loop with FP comparisons */
    for (int i = 0; i < 10; i++) {
        volatile double loop_var = i * 0.1;
        if (loop_var < x) final_result ^= i;
        if (loop_var > y) final_result ^= (i * 2);
        if (loop_var == x) final_result += i;
        if (loop_var != y) final_result -= i;
    }
    
    printf("Final aggregated result: %d\n", 
           result1 + result2 + result3 + result4 + final_result);
    
    return 0;
}
