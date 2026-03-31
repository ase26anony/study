/* fp_condition_stress.c - Exhaustively test FP comparison condition code generation */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Function to stress FP comparisons with complex control flow */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf_val) {
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    volatile double v_neg_inf = neg_inf_val;
    
    int result = 0;
    
    /* Label for goto-based control flow */
    start_comparisons:
    
    /* 1. Exhaustive scalar comparisons with all condition codes */
    
    /* UNORDERED cases (involving NaN) */
    if (v1 != v1) {  /* v1 is NaN check */
        result |= 1;
        goto unordered_path;
    }
    
    if (v_nan == v2) {  /* Always false, but generates comparison */
        /* Never taken */
        result |= 2;
    }
    
    unordered_path:
    if (v_nan < v1) {  /* Unordered comparison */
        result |= 4;
    }
    
    if (v1 > v_nan) {  /* Another unordered comparison */
        result |= 8;
    }
    
    /* ORDERED cases (both operands are numbers) */
    if (v1 == v2) {  /* EQ */
        result |= 16;
        goto ordered_path;
    }
    
    if (v1 < v2) {  /* LT */
        result |= 32;
    }
    
    ordered_path:
    if (v1 <= v2) {  /* LE */
        result |= 64;
    }
    
    if (v1 >= v2) {  /* GE */
        result |= 128;
    }
    
    if (v1 > v2) {  /* GT */
        result |= 256;
    }
    
    /* UNEQ (unordered or equal) - using conditional operator */
    double uneq_test = (v_nan == v_nan) ? 1.0 : 0.0;  /* false */
    sink = uneq_test;
    result |= (uneq_test == 0.0) ? 512 : 0;
    
    /* UNGE (unordered or greater than or equal) */
    if (!(v_nan < v1)) {  /* NOT LT = UNGE */
        result |= 1024;
    }
    
    /* UNGT (unordered or greater than) */
    if (!(v_nan <= v1)) {  /* NOT LE = UNGT */
        result |= 2048;
    }
    
    /* UNLE (unordered or less than or equal) */
    if (v1 <= v_nan) {  /* Direct UNLE comparison */
        result |= 4096;
    }
    
    /* UNLT (unordered or less than) */
    if (v1 < v_nan) {  /* Direct UNLT comparison */
        result |= 8192;
    }
    
    /* LTGT (less than or greater than, but not equal and not unordered) */
    if (v1 != v2 && v1 == v1 && v2 == v2) {  /* Both ordered and not equal */
        result |= 16384;
    }
    
    /* 2. Complex conditional expressions mixing ordered/unordered */
    double cond_result = 
        (v1 < v2) ? 1.0 :
        (v1 > v2) ? 2.0 :
        (v1 == v2) ? 3.0 :
        (v1 != v1 || v2 != v2) ? 4.0 : 5.0;
    
    sink = cond_result;
    result |= (int)(cond_result * 1000) & 0xFF;
    
    /* 3. Vectorized comparisons using GCC vector extensions */
    v2df vec_a = {v1, v2};
    v2df vec_b = {v2, v1};
    v2df vec_nan = {v_nan, v_nan};
    
    /* Compare vectors - generates cmppd/ucomisd variants */
    v2di mask_eq = (v2di)(vec_a == vec_b);
    v2di mask_lt = (v2di)(vec_a < vec_b);
    v2di mask_le = (v2di)(vec_a <= vec_b);
    v2di mask_gt = (v2di)(vec_a > vec_b);
    v2di mask_ge = (v2di)(vec_a >= vec_b);
    v2di mask_neq = (v2di)(vec_a != vec_b);
    
    /* Unordered vector comparisons */
    v2di mask_unordered = (v2di)(vec_a != vec_a);
    v2di mask_nan_cmp = (v2di)(vec_a < vec_nan);
    
    /* Store results to prevent elimination */
    long long* masks[] = {&mask_eq[0], &mask_lt[0], &mask_le[0], 
                         &mask_gt[0], &mask_ge[0], &mask_neq[0],
                         &mask_unordered[0], &mask_nan_cmp[0]};
    
    for (int i = 0; i < 8; i++) {
        result ^= (int)(*masks[i] & 0xFFFFFFFF);
    }
    
    /* 4. Inline assembly with explicit condition code usage */
    int cc_result = 0;
    
    /* ucomisd with setp (parity/UNORDERED) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(v1), "x"(v_nan)
        : "cc", "eax"
    );
    result ^= (cc_result << 16);
    
    /* ucomisd with seta (ABOVE/UNORDERED or GT) */
    asm volatile (
        "ucomisd %1, %2\n\t"
        "seta %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(v1), "x"(v2)
        : "cc", "eax"
    );
    result ^= (cc_result << 17);
    
    /* ucomisd with setb (BELOW/UNORDERED or LT) */
    asm volatile (
        "ucomisd %2, %1\n\t"  /* swapped order for BELOW */
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r"(cc_result)
        : "x"(v1), "x"(v2)
        : "cc", "eax"
    );
    result ^= (cc_result << 18);
    
    /* 5. Infinity comparisons */
    if (v1 == v_inf) {
        result |= 0x100000;
    }
    
    if (v_neg_inf < v1) {
        result |= 0x200000;
    }
    
    if (v_inf <= v_inf) {  /* EQ case for infinity */
        result |= 0x400000;
    }
    
    /* 6. Mixed NaN/infinity/normal comparisons */
    double mixed_test = 
        (v_nan < v_inf) ? 1.0 :  /* false (unordered) */
        (v_neg_inf < v_inf) ? 2.0 :  /* true */
        (v1 == v_nan) ? 3.0 :  /* false */
        4.0;
    
    sink = mixed_test;
    result |= ((int)mixed_test) << 24;
    
    /* Loop with varying comparison types */
    for (int i = 0; i < 3; i++) {
        volatile double loop_var = v1 + i;
        
        switch (i) {
            case 0:
                if (loop_var != loop_var) result |= 0x80000000;
                break;
            case 1:
                if (loop_var < v_nan) result |= 0x40000000;
                break;
            case 2:
                if (v_inf > loop_var) result |= 0x20000000;
                break;
        }
    }
    
    return result;
}

/* Secondary function with different optimization context */
static int more_fp_conditions(double x, double y) {
    volatile double a = x;
    volatile double b = y;
    int r = 0;
    
    /* Create all possible condition code usages */
    r |= (a == b) ? 1 : 0;
    r |= (a != b) ? 2 : 0;
    r |= (a < b) ? 4 : 0;
    r |= (a <= b) ? 8 : 0;
    r |= (a > b) ? 16 : 0;
    r |= (a >= b) ? 32 : 0;
    
    /* Force unordered comparisons */
    double nan = __builtin_nan("");
    r |= (a == nan) ? 64 : 0;
    r |= (nan == nan) ? 128 : 0;  /* false */
    r |= (a < nan) ? 256 : 0;
    r |= (nan > b) ? 512 : 0;
    
    /* Complex boolean combinations */
    if ((a < b) || (a != a) || (b != b)) {
        r |= 1024;
    }
    
    if ((a == b) && (a == a) && (b == b)) {
        r |= 2048;
    }
    
    return r;
}

int main(void) {
    /* Initialize FP special values */
    double nan = __builtin_nan("");
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double zero = 0.0;
    double neg_zero = -0.0;
    
    printf("Starting FP comparison stress test...\n");
    
    /* Test with various value pairs to cover all condition codes */
    int result = 0;
    
    /* normal vs normal */
    result ^= stress_fp_comparisons(normal1, normal2, nan, inf, neg_inf);
    
    /* normal vs NaN */
    result ^= stress_fp_comparisons(normal1, nan, nan, inf, neg_inf);
    
    /* NaN vs NaN */
    result ^= stress_fp_comparisons(nan, nan, nan, inf, neg_inf);
    
    /* normal vs inf */
    result ^= stress_fp_comparisons(normal1, inf, nan, inf, neg_inf);
    
    /* inf vs -inf */
    result ^= stress_fp_comparisons(inf, neg_inf, nan, inf, neg_inf);
    
    /* zero vs -zero (equal but distinct bit patterns) */
    result ^= stress_fp_comparisons(zero, neg_zero, nan, inf, neg_inf);
    
    /* Call secondary function with different patterns */
    result ^= more_fp_conditions(normal1, normal2);
    result ^= more_fp_conditions(nan, normal1);
    result ^= more_fp_conditions(inf, neg_inf);
    
    /* Array-based vector comparisons */
    double arr1[4] = {normal1, nan, inf, zero};
    double arr2[4] = {normal2, normal1, neg_inf, neg_zero};
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            volatile double cmp1 = arr1[i];
            volatile double cmp2 = arr2[j];
            
            /* Generate various condition codes */
            if (cmp1 < cmp2) result++;
            if (cmp1 <= cmp2) result ^= 0x55;
            if (cmp1 > cmp2) result ^= 0xAA;
            if (cmp1 >= cmp2) result ^= 0xFF;
            if (cmp1 == cmp2) result ^= 0x33;
            if (cmp1 != cmp2) result ^= 0xCC;
        }
    }
    
    printf("Result checksum: %d\n", result);
    printf("FP comparison stress test complete.\n");
    
    /* Use sink to prevent dead code elimination */
    printf("Sink value: %f\n", sink);
    
    return result != 0 ? 0 : 1;
}
