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
    
    /* Normal number comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Comparisons with NaN - will trigger UNORDERED paths */
    if (v1 == v_nan) result ^= 64;      /* Always false, unordered */
    if (v1 != v_nan) result ^= 128;     /* Always true when v1 is not NaN */
    if (v1 < v_nan)  result ^= 256;     /* Always false, unordered */
    if (v1 <= v_nan) result ^= 512;     /* Always false, unordered */
    if (v1 > v_nan)  result ^= 1024;    /* Always false, unordered */
    if (v1 >= v_nan) result ^= 2048;    /* Always false, unordered */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096; /* Always false, unordered */
    if (v_nan != v_nan) result ^= 8192; /* Always true, UNORDERED case */
    
    /* Comparisons with infinity */
    if (v1 == v_inf) result ^= 16384;
    if (v1 < v_inf)  result ^= 32768;
    if (v_inf > v_neg_inf) result ^= 65536;
    
    /* Use conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? 1.0 : 2.0;
    result ^= (int)cmov_result;
    
    cmov_result = (v1 != v_nan) ? 3.0 : 4.0;
    result ^= (int)cmov_result;
    
    /* Complex conditional expressions */
    int complex_cond = (v1 < v2) ? ((v1 > v_neg_inf) ? 1 : 2) : 
                     ((v1 == v_nan) ? 3 : 4);
    result ^= complex_cond;
    
    return result;
}

/* Function with goto-based control flow to prevent optimization */
static int fp_comparisons_with_goto(double a, double b, double nan_val) {
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
    } else {
        checksum ^= 8;
    }
    
label2:
    if (y != nan) {
        checksum ^= 16;
        goto label4;
    } else {
        checksum ^= 32;
    }
    
label3:
    if (x <= y) {
        checksum ^= 64;
    } else {
        checksum ^= 128;
    }
    
label4:
    if (x >= nan) {
        checksum ^= 256;
    } else {
        checksum ^= 512;
    }
    
    return checksum;
}

/* Vectorized FP comparisons */
static v2di vector_fp_comparisons(v2df a, v2df b, v2df nan_vec) {
    /* These comparisons generate cmppd/ucomisd with various condition codes */
    v2di mask1 = (v2di)(a == b);   /* EQ */
    v2di mask2 = (v2di)(a != b);   /* NEQ/UNORD */
    v2di mask3 = (v2di)(a < b);    /* LT */
    v2di mask4 = (v2di)(a <= b);   /* LE */
    v2di mask5 = (v2di)(a > b);    /* GT */
    v2di mask6 = (v2di)(a >= b);   /* GE */
    
    /* Comparisons with NaN */
    v2di mask7 = (v2di)(a == nan_vec);  /* UNORDERED */
    v2di mask8 = (v2di)(a != nan_vec);  /* ORDERED */
    v2di mask9 = (v2di)(a < nan_vec);   /* UNORDERED */
    
    /* Combine masks */
    v2di result = mask1 ^ mask2 ^ mask3 ^ mask4 ^ mask5 ^ mask6 ^ mask7 ^ mask8 ^ mask9;
    
    return result;
}

/* Inline assembly that explicitly uses FP condition codes */
static int inline_asm_fp_conds(double a, double b, double nan_val) {
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
    result ^= cc_result;
    
    /* ucomisd with seta (above/UNORDERED or GT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= cc_result;
    
    /* ucomisd with setb (below/UNORDERED or LT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= cc_result;
    
    /* ucomisd with sete (equal/UNORDERED or EQ) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= cc_result;
    
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
    
    int final_checksum = 0;
    
    /* Test 1: Exhaustive scalar comparisons */
    final_checksum ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    final_checksum ^= stress_fp_comparisons(normal1, nan_val, nan_val, inf_val, neg_inf_val);
    final_checksum ^= stress_fp_comparisons(inf_val, neg_inf_val, nan_val, inf_val, neg_inf_val);
    
    /* Test 2: Goto-based control flow */
    final_checksum ^= fp_comparisons_with_goto(normal1, normal2, nan_val);
    final_checksum ^= fp_comparisons_with_goto(nan_val, normal1, nan_val);
    
    /* Test 3: Vectorized comparisons */
    v2df vec1 = {normal1, normal2};
    v2df vec2 = {normal2, normal1};
    v2df nan_vec = {nan_val, nan_val};
    
    v2di vec_result = vector_fp_comparisons(vec1, vec2, nan_vec);
    long long* vec_arr = (long long*)&vec_result;
    final_checksum ^= (int)vec_arr[0];
    final_checksum ^= (int)vec_arr[1];
    
    /* Test 4: Array-based vector comparisons in loop */
    double arr1[4] = {1.0, 2.0, 3.0, 4.0};
    double arr2[4] = {4.0, 3.0, 2.0, 1.0};
    double arr_nan[4] = {nan_val, 0.0, nan_val, 0.0};
    
    int mask_results[4] = {0};
    for (int i = 0; i < 4; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        volatile double nan = arr_nan[i];
        
        /* Mix of comparisons in loop */
        if (a < b) mask_results[i] ^= 1;
        if (a == nan) mask_results[i] ^= 2;
        if (a != b) mask_results[i] ^= 4;
        if (b >= a) mask_results[i] ^= 8;
        
        final_checksum ^= mask_results[i];
    }
    
    /* Test 5: Inline assembly with condition codes */
    final_checksum ^= inline_asm_fp_conds(normal1, normal2, nan_val);
    final_checksum ^= inline_asm_fp_conds(nan_val, normal1, nan_val);
    final_checksum ^= inline_asm_fp_conds(inf_val, neg_inf_val, nan_val);
    
    /* Additional unordered comparison tests */
    volatile double v_nan = nan_val;
    volatile double v_num = 42.0;
    
    /* These should generate UNORDERED condition codes */
    int unordered_test = 0;
    unordered_test |= (v_num == v_nan) ? 0 : 1;      /* UNORDERED/ORDERED */
    unordered_test |= (v_nan != v_nan) ? 2 : 0;      /* UNORDERED */
    unordered_test |= (v_num < v_nan) ? 0 : 4;       /* UNORDERED */
    unordered_test |= (v_num > v_nan) ? 0 : 8;       /* UNORDERED */
    unordered_test |= (v_nan <= v_num) ? 0 : 16;     /* UNORDERED */
    unordered_test |= (v_nan >= v_num) ? 0 : 32;     /* UNORDERED */
    
    final_checksum ^= unordered_test;
    
    /* Mixed ordered/unordered comparisons */
    double mixed_result = 0.0;
    mixed_result += (v_num < v_num) ? 0.1 : 0.2;     /* Ordered LT */
    mixed_result += (v_num == v_num) ? 0.3 : 0.4;    /* Ordered EQ */
    mixed_result += (v_nan == v_num) ? 0.5 : 0.6;    /* Unordered */
    mixed_result += (v_num != v_nan) ? 0.7 : 0.8;    /* Ordered NEQ/UNEQ */
    
    final_checksum ^= (int)mixed_result;
    
    /* Prevent dead code elimination */
    printf("Final checksum: %d\n", final_checksum);
    
    return final_checksum != 0 ? 0 : 1;
}
