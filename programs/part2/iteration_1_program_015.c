/* fp_condition_stress.c - Exhaustive test of FP comparison condition codes */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Function to stress FP comparisons with complex control flow */
void stress_fp_comparisons(double a, double b, double nan_val, double inf_val, 
                          double neg_inf, double zero) {
    /* Exhaustive comparison matrix */
    double values[] = {a, b, nan_val, inf_val, neg_inf, zero, -zero};
    int n_values = sizeof(values) / sizeof(values[0]);
    
    int result = 0;
    
    /* Complex control flow with goto to prevent simplification */
    int i = 0;
    comparison_loop:
    for (int j = 0; j < n_values; j++) {
        volatile double x = values[i];
        volatile double y = values[j];
        
        /* All standard comparisons */
        if (x == y) result ^= 1;
        if (x != y) result ^= 2;
        if (x < y)  result ^= 4;
        if (x <= y) result ^= 8;
        if (x > y)  result ^= 16;
        if (x >= y) result ^= 32;
        
        /* Unordered comparisons - critical for uncovered paths */
        int unordered_test = (x != x) || (y != y);
        if (unordered_test) result ^= 64;  /* UNORDERED */
        
        /* Ordered comparison */
        int ordered_test = (x == x) && (y == y);
        if (ordered_test) result ^= 128;  /* ORDERED */
        
        /* UNEQ: unordered or equal */
        if (unordered_test || (x == y)) result ^= 256;
        
        /* UNGE: unordered or greater-or-equal */
        if (unordered_test || (x >= y)) result ^= 512;
        
        /* UNGT: unordered or greater */
        if (unordered_test || (x > y)) result ^= 1024;
        
        /* UNLE: unordered or less-or-equal */
        if (unordered_test || (x <= y)) result ^= 2048;
        
        /* UNLT: unordered or less */
        if (unordered_test || (x < y)) result ^= 4096;
        
        /* LTGT: less or greater (ordered and not equal) */
        if ((x < y) || (x > y)) result ^= 8192;
        
        /* Conditional moves using ternary operator */
        double cmov_result = (x < y) ? x : y;
        sink = cmov_result;
        cmov_result = (x >= y) ? x : y;
        sink = cmov_result;
        cmov_result = (x != y) ? x : y;
        sink = cmov_result;
        cmov_result = (x == y) ? x : y;
        sink = cmov_result;
    }
    
    if (++i < n_values) goto comparison_loop;
    
    /* Vectorized comparisons using GCC vector extensions */
    typedef double v2df __attribute__((vector_size(16)));
    typedef long long v2di __attribute__((vector_size(16)));
    
    v2df vec_a = {a, b};
    v2df vec_b = {b, a};
    v2df vec_nan = {nan_val, nan_val};
    
    /* Vector comparisons generating cmppd/ucomisd instructions */
    v2di cmp_eq = (v2di)(vec_a == vec_b);
    v2di cmp_lt = (v2di)(vec_a < vec_b);
    v2di cmp_le = (v2di)(vec_a <= vec_b);
    v2di cmp_gt = (v2di)(vec_a > vec_b);
    v2di cmp_ge = (v2di)(vec_a >= vec_b);
    v2di cmp_neq = (v2di)(vec_a != vec_b);
    
    /* Comparisons with NaN */
    v2di cmp_nan_eq = (v2di)(vec_a == vec_nan);
    v2di cmp_nan_lt = (v2di)(vec_a < vec_nan);
    v2di cmp_nan_gt = (v2di)(vec_a > vec_nan);
    
    /* Store results to prevent elimination */
    volatile v2di store;
    store = cmp_eq; store = cmp_lt; store = cmp_le;
    store = cmp_gt; store = cmp_ge; store = cmp_neq;
    store = cmp_nan_eq; store = cmp_nan_lt; store = cmp_nan_gt;
    
    /* Inline assembly with explicit condition code handling */
    double asm_a = a;
    double asm_b = b;
    double asm_nan = nan_val;
    
    /* Various inline asm patterns that use FP condition codes */
    int cc_result;
    
    /* UNORDERED/ORDERED tests */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (asm_a), "x" (asm_nan)
        : "al", "cc"
    );
    result ^= cc_result;
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (asm_a), "x" (asm_b)
        : "al", "cc"
    );
    result ^= cc_result;
    
    /* UNEQ test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %%al\n\t"
        "setp %%cl\n\t"
        "orb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (asm_a), "x" (asm_nan)
        : "al", "cl", "cc"
    );
    result ^= cc_result;
    
    /* UNGE test (not less) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (asm_a), "x" (asm_b)
        : "al", "cc"
    );
    result ^= cc_result;
    
    /* UNGT test (not less or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (asm_a), "x" (asm_b)
        : "al", "cc"
    );
    result ^= cc_result;
    
    /* UNLE test (unordered or less or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setna %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (asm_nan), "x" (asm_b)
        : "al", "cc"
    );
    result ^= cc_result;
    
    /* UNLT test (unordered or less) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (asm_nan), "x" (asm_b)
        : "al", "cc"
    );
    result ^= cc_result;
    
    /* LTGT test (not equal and ordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %%al\n\t"
        "setnp %%cl\n\t"
        "andb %%cl, %%al\n\t"
        "movzbl %%al, %0"
        : "=r" (cc_result)
        : "x" (asm_a), "x" (asm_b)
        : "al", "cl", "cc"
    );
    result ^= cc_result;
    
    /* Conditional move based on FP comparison */
    double cmov_dest;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "cmova %3, %0"
        : "=r" (cmov_dest)
        : "x" (asm_a), "x" (asm_b), "r" (asm_a)
        : "cc"
    );
    sink = cmov_dest;
    
    /* Final sink to prevent dead code elimination */
    sink = result;
}

/* Array-based vector comparisons */
void vector_array_comparisons(void) {
    #define ARRAY_SIZE 64
    double arr1[ARRAY_SIZE];
    double arr2[ARRAY_SIZE];
    int mask[ARRAY_SIZE];
    
    /* Initialize with mixed values including NaN */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = (i * 1.1) - 32.0;
        arr2[i] = (i % 2 == 0) ? (i * 0.9) : __builtin_nan("");
        if (i % 7 == 0) arr1[i] = __builtin_nan("");
        if (i % 5 == 0) arr2[i] = __builtin_inf();
    }
    
    /* Perform various comparisons in loops */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        
        /* Generate all condition codes */
        mask[i] = 0;
        if (a == b) mask[i] |= 1;
        if (a != b) mask[i] |= 2;
        if (a < b)  mask[i] |= 4;
        if (a <= b) mask[i] |= 8;
        if (a > b)  mask[i] |= 16;
        if (a >= b) mask[i] |= 32;
        
        /* Unordered checks */
        if (a != a || b != b) mask[i] |= 64;
        if (a == a && b == b) mask[i] |= 128;
    }
    
    /* Use mask to prevent elimination */
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum ^= mask[i];
    }
    sink = sum;
}

int main(void) {
    /* Initialize FP special values */
    double normal1 = 3.141592653589793;
    double normal2 = 2.718281828459045;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double zero = 0.0;
    
    printf("Testing exhaustive FP comparisons...\n");
    
    /* Call stress function multiple times with different values */
    stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf, zero);
    stress_fp_comparisons(normal2, normal1, nan_val, inf_val, neg_inf, zero);
    stress_fp_comparisons(nan_val, normal1, nan_val, inf_val, neg_inf, zero);
    stress_fp_comparisons(inf_val, neg_inf, nan_val, inf_val, neg_inf, zero);
    
    /* Test vectorized array comparisons */
    vector_array_comparisons();
    
    printf("FP comparison stress test completed.\n");
    printf("Final sink value (prevent DCE): %f\n", sink);
    
    return 0;
}
