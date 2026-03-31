/* fp_condition_stress.c - Exhaustively test FP comparison condition code generation */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Prevent constant folding and optimization */
static volatile double sink;

/* Vector types for SIMD comparisons */
typedef double v2df __attribute__((vector_size(16)));
typedef long long v2di __attribute__((vector_size(16)));

/* Stress function with exhaustive FP comparisons */
static int stress_fp_comparisons(double a, double b, double nan_val, double inf_val, double neg_inf) {
    int result = 0;
    volatile double v1 = a;
    volatile double v2 = b;
    volatile double vnan = nan_val;
    volatile double vinf = inf_val;
    volatile double vneginf = neg_inf;
    
    /* Complex control flow with goto to prevent simplification */
    int state = 0;
    
    /* Block 1: Direct comparisons with NaN (unordered cases) */
    if (vnan == v1) {
        state = 1;
        goto label_unordered;
    }
    
    if (v1 < vnan) {
        state = 2;
        goto label_unordered;
    }
    
    if (vnan <= v2) {
        state = 3;
        goto label_unordered;
    }
    
    /* Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Comparisons involving infinity */
    if (v1 == vinf) result ^= 64;
    if (vinf > v2)  result ^= 128;
    if (vneginf < v1) result ^= 256;
    
    /* NaN self-comparison (always false, but generates UNORDERED) */
    if (vnan != vnan) {
        result ^= 512;  /* This should always execute */
    }
    
    /* Complex conditional expressions (ternary operator) */
    double cond_result = (v1 < vnan) ? 1.0 : 
                        (vnan > v2) ? 2.0 :
                        (v1 == v1) ? 3.0 : 4.0;
    sink = cond_result;
    
    /* More unordered comparisons */
    if (!(v1 >= vnan)) result ^= 1024;  /* UNLT: not (a >= NaN) */
    if (!(vnan <= v2)) result ^= 2048;  /* UNGT: not (NaN <= b) */
    
    /* Check for ordered comparisons */
    if (v1 == v1 && v2 == v2) {  /* Both are numbers (not NaN) */
        if (v1 < v2 || v1 > v2) result ^= 4096;  /* LTGT */
    }
    
    /* Unordered or equal */
    if (!(v1 < v2) && !(v1 > v2)) result ^= 8192;  /* UNEQ or ORDERED */
    
    return result;

label_unordered:
    /* Handle unordered comparison results */
    switch(state) {
        case 1: result ^= 16384; break;  /* UNORDERED path */
        case 2: result ^= 32768; break;  /* UNLT path */
        case 3: result ^= 65536; break;  /* UNLE path */
    }
    
    /* More condition checks after goto */
    if (v1 != v1 || v2 != v2) {  /* Either is NaN */
        result ^= 131072;  /* ORDERED check (false) */
    }
    
    return result;
}

/* Function with vectorized comparisons */
static v2di vector_fp_comparisons(v2df vec1, v2df vec2, v2df vec_nan) {
    /* Various vector comparisons generating different condition codes */
    v2di mask1 = (v2di)(vec1 == vec2);   /* EQ */
    v2di mask2 = (v2di)(vec1 != vec2);   /* NEQ/UNEQ */
    v2di mask3 = (v2di)(vec1 < vec2);    /* LT */
    v2di mask4 = (v2di)(vec1 <= vec2);   /* LE */
    v2di mask5 = (v2di)(vec1 > vec2);    /* GT */
    v2di mask6 = (v2di)(vec1 >= vec2);   /* GE */
    
    /* Unordered comparisons with NaN */
    v2di mask7 = (v2di)(vec1 == vec_nan);  /* UNORDERED */
    v2di mask8 = (v2di)(vec1 < vec_nan);   /* UNLT */
    v2di mask9 = (v2di)(vec_nan <= vec2);  /* UNLE */
    
    /* Combine masks */
    v2di result = mask1 ^ mask2 ^ mask3 ^ mask4 ^ mask5 ^ mask6 ^ mask7 ^ mask8 ^ mask9;
    
    /* Additional comparisons using builtins (direct instruction generation) */
    v2di builtin_cmp;
    builtin_cmp = __builtin_ia32_cmpeqpd((__v2df)vec1, (__v2df)vec2);
    result ^= builtin_cmp;
    
    builtin_cmp = __builtin_ia32_cmpltpd((__v2df)vec1, (__v2df)vec2);
    result ^= builtin_cmp;
    
    builtin_cmp = __builtin_ia32_cmpneqpd((__v2df)vec1, (__v2df)vec2);
    result ^= builtin_cmp;
    
    return result;
}

/* Function with inline assembly using condition codes */
static int inline_asm_fp_conditions(double x, double y, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Test UNORDERED (parity flag) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan_val)
        : "cc"
    );
    result |= (cc_result << 0);
    
    /* Test ORDERED (not parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 1);
    
    /* Test less than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 2);
    
    /* Test less or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 3);
    
    /* Test greater than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 4);
    
    /* Test greater or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 5);
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovb %3, %0"
        : "=t"(cmov_result)
        : "u"(x), "u"(y), "t"(nan_val)
        : "cc"
    );
    sink = cmov_result;
    
    return result;
}

/* Main test driver */
int main(void) {
    /* Initialize FP special values */
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double zero = 0.0;
    double one = 1.0;
    double neg_one = -1.0;
    
    int total_result = 0;
    
    /* Test matrix of value pairs */
    double test_values[] = {zero, one, neg_one, inf_val, neg_inf, nan_val};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Exhaustive pairwise comparisons */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            total_result ^= stress_fp_comparisons(
                test_values[i], 
                test_values[j],
                nan_val,
                inf_val,
                neg_inf
            );
        }
    }
    
    /* Vectorized comparisons */
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {3.0, 1.0};
    v2df vec_nan = {nan_val, nan_val};
    
    v2di vec_result = vector_fp_comparisons(vec1, vec2, vec_nan);
    
    /* Extract results from vector */
    long long* vec_arr = (long long*)&vec_result;
    total_result ^= (int)(vec_arr[0] ^ vec_arr[1]);
    
    /* Inline assembly tests */
    total_result ^= inline_asm_fp_conditions(1.0, 2.0, nan_val);
    total_result ^= inline_asm_fp_conditions(nan_val, 1.0, nan_val);
    total_result ^= inline_asm_fp_conditions(inf_val, neg_inf, nan_val);
    
    /* Array-based comparisons in loop (triggers vectorization) */
    double arr1[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    double arr2[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    double arr_nan[8];
    
    for (int i = 0; i < 8; i++) {
        arr_nan[i] = (i % 3 == 0) ? nan_val : arr1[i];
    }
    
    int mask_result = 0;
    for (int i = 0; i < 8; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        volatile double n = arr_nan[i];
        
        /* Mix of ordered and unordered comparisons */
        if (a < b) mask_result ^= (1 << (i % 32));
        if (a == n) mask_result ^= (1 << ((i + 1) % 32));
        if (!(n <= b)) mask_result ^= (1 << ((i + 2) % 32));
        if (a != a || b != b) mask_result ^= (1 << ((i + 3) % 32));
    }
    total_result ^= mask_result;
    
    /* Final result to prevent dead code elimination */
    printf("Result checksum: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}
