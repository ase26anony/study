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
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf) {
    int result = 0;
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double vnan = nan_val;
    volatile double vinf = inf_val;
    volatile double vneg_inf = neg_inf;
    
    /* Matrix of comparisons to generate various condition codes */
    
    /* 1. Ordered comparisons (normal numbers) */
    if (v1 == v2) {
        result ^= 1;
        goto label_ordered_eq;
    }
    if (v1 < v2) {
        result ^= 2;
        goto label_ordered_lt;
    }
    if (v1 > v2) {
        result ^= 4;
        goto label_ordered_gt;
    }
    if (v1 <= v2) {
        result ^= 8;
    }
    if (v1 >= v2) {
        result ^= 16;
    }
    if (v1 != v2) {
        result ^= 32;
    }
    
label_ordered_eq:
    /* 2. Comparisons with NaN (unordered cases) */
    if (vnan == v1) {  /* Always false, but generates UNORDERED check */
        result ^= 64;
        goto label_nan_eq;
    }
    
label_ordered_lt:
    if (v1 < vnan) {   /* Generates UNORDERED */
        result ^= 128;
    }
    
label_ordered_gt:
    if (vnan > v2) {   /* Generates UNORDERED */
        result ^= 256;
    }
    
label_nan_eq:
    if (vnan != vnan) { /* NaN != NaN is true, generates UNORDERED */
        result ^= 512;
        goto label_nan_ne;
    }
    
    /* 3. Ordered check */
    if (v1 == v1 && v2 == v2) { /* Both are numbers (not NaN) */
        result ^= 1024;
    }
    
label_nan_ne:
    /* 4. More complex unordered comparisons */
    double cmp1 = (vnan == vinf) ? 1.0 : 0.0;  /* UNORDERED/UNEQ */
    double cmp2 = (v1 >= vnan) ? 2.0 : 0.0;    /* UNORDERED/UNGE */
    double cmp3 = (vnan > v2) ? 3.0 : 0.0;     /* UNORDERED/UNGT */
    double cmp4 = (vneg_inf <= vnan) ? 4.0 : 0.0; /* UNORDERED/UNLE */
    double cmp5 = (vnan < vinf) ? 5.0 : 0.0;   /* UNORDERED/UNLT */
    double cmp6 = (v1 != v2) ? 6.0 : 0.0;      /* LTGT */
    
    sink = cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6;
    
    /* 5. Conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? a : b;
    cmov_result = (vnan == vnan) ? cmov_result : b;  /* Always takes first */
    cmov_result = (v1 >= vnan) ? cmov_result : a;    /* UNORDERED path */
    
    sink = cmov_result;
    
    return result;
}

/* Function with vectorized FP comparisons */
static void vector_fp_comparisons(double *arr1, double *arr2, int *mask, int n) {
    for (int i = 0; i < n; i += 2) {
        /* Load vectors */
        v2df vec1 = {arr1[i], arr1[i+1]};
        v2df vec2 = {arr2[i], arr2[i+1]};
        
        /* Perform various vector comparisons */
        v2di cmp_eq = (v2di)(vec1 == vec2);
        v2di cmp_lt = (v2di)(vec1 < vec2);
        v2di cmp_le = (v2di)(vec1 <= vec2);
        v2di cmp_gt = (v2di)(vec1 > vec2);
        v2di cmp_ge = (v2di)(vec1 >= vec2);
        v2di cmp_ne = (v2di)(vec1 != vec2);
        
        /* Combine results */
        mask[i] = cmp_eq[0] | cmp_lt[0];
        mask[i+1] = cmp_le[0] | cmp_gt[0] | cmp_ge[0] | cmp_ne[0];
    }
}

/* Function with inline assembly using condition codes */
static int asm_fp_conditions(double x, double y, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Test UNORDERED (parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan_val)
        : "cc"
    );
    result |= (cc_result << 0);
    
    /* Test ORDERED */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 1);
    
    /* Test UNEQ (unordered or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(nan_val), "x"(nan_val)
        : "cc"
    );
    result |= (cc_result << 2);
    
    /* Test UNGE (not less than) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan_val)
        : "cc"
    );
    result |= (cc_result << 3);
    
    /* Test UNGT (not less or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 4);
    
    /* Test UNLE (unordered or less or equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setna %0"
        : "=r"(cc_result)
        : "x"(nan_val), "x"(x)
        : "cc"
    );
    result |= (cc_result << 5);
    
    /* Test UNLT (unordered or less than) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan_val)
        : "cc"
    );
    result |= (cc_result << 6);
    
    /* Test LTGT (less than or greater than) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 7);
    
    return result;
}

/* Complex control flow with FP comparisons */
static int complex_fp_control_flow(double a, double b, double nan_val) {
    int checksum = 0;
    
    /* Create a web of goto labels to prevent optimization */
    if (a < b) {
        checksum ^= 1;
        goto block1;
    } else {
        checksum ^= 2;
        goto block2;
    }
    
block1:
    if (nan_val == nan_val) {  /* False */
        checksum ^= 4;
        goto block3;
    } else {
        checksum ^= 8;
        goto block4;
    }
    
block2:
    if (a != a) {  /* False for normal numbers */
        checksum ^= 16;
        goto block5;
    } else {
        checksum ^= 32;
        goto block6;
    }
    
block3:
    if (b >= nan_val) {  /* UNORDERED */
        checksum ^= 64;
        goto block7;
    }
    
block4:
    if (nan_val <= a) {  /* UNORDERED */
        checksum ^= 128;
    }
    goto block8;
    
block5:
    if (a > nan_val) {  /* UNORDERED */
        checksum ^= 256;
    }
    
block6:
    if (nan_val < b) {  /* UNORDERED */
        checksum ^= 512;
    }
    goto block9;
    
block7:
    if (a == b) {
        checksum ^= 1024;
    }
    
block8:
    /* Use GCC builtin for direct comparison */
    v2df vec_a = {a, b};
    v2df vec_b = {b, a};
    v2df vec_cmp = __builtin_ia32_cmpeqsd(vec_a, vec_b);
    checksum ^= ((int)vec_cmp[0]) << 10;
    
block9:
    return checksum;
}

int main(void) {
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf = -__builtin_inf();
    
    int total_result = 0;
    
    /* Test 1: Scalar FP comparisons */
    total_result ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf);
    total_result ^= stress_fp_comparisons(normal1, nan_val, nan_val, inf_val, neg_inf);
    total_result ^= stress_fp_comparisons(nan_val, inf_val, nan_val, inf_val, neg_inf);
    total_result ^= stress_fp_comparisons(inf_val, neg_inf, nan_val, inf_val, neg_inf);
    
    /* Test 2: Vectorized comparisons */
    double arr1[8] = {1.0, 2.0, 3.0, 4.0, nan_val, 6.0, inf_val, 8.0};
    double arr2[8] = {1.0, 3.0, 3.0, 5.0, 5.0, nan_val, inf_val, neg_inf};
    int mask[8];
    vector_fp_comparisons(arr1, arr2, mask, 8);
    
    for (int i = 0; i < 8; i++) {
        total_result ^= mask[i];
    }
    
    /* Test 3: Inline assembly with condition codes */
    total_result ^= asm_fp_conditions(normal1, normal2, nan_val);
    total_result ^= asm_fp_conditions(nan_val, inf_val, nan_val);
    
    /* Test 4: Complex control flow */
    total_result ^= complex_fp_control_flow(normal1, normal2, nan_val);
    total_result ^= complex_fp_control_flow(nan_val, inf_val, nan_val);
    
    /* Additional exhaustive pairwise comparisons */
    double values[] = {normal1, normal2, nan_val, inf_val, neg_inf, 0.0, -0.0};
    int num_values = sizeof(values) / sizeof(values[0]);
    
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            /* Use all comparison operators */
            volatile double x = values[i];
            volatile double y = values[j];
            
            if (x == y) total_result += 1;
            if (x != y) total_result += 2;
            if (x < y)  total_result += 4;
            if (x <= y) total_result += 8;
            if (x > y)  total_result += 16;
            if (x >= y) total_result += 32;
            
            /* Conditional operator */
            double tmp = (x < y) ? x : y;
            sink = tmp;
            
            /* Compound condition */
            if ((x == y) || (x != x) || (y != y)) {
                total_result += 64;
            }
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result checksum: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}
