/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -o fp_conditions fp_conditions.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math -o fp_conditions_vec fp_conditions.c */
/* And: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline -o fp_conditions_32 fp_conditions.c */

#include <stdint.h>
#include <stdio.h>

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
    
    /* Normal vs NaN comparisons (unordered cases) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, unordered */
    if (v1 != v_nan) result ^= 128;     /* Should be true, unordered */
    if (v1 < v_nan)  result ^= 256;     /* Should be false, unordered */
    if (v1 <= v_nan) result ^= 512;     /* Should be false, unordered */
    if (v1 > v_nan)  result ^= 1024;    /* Should be false, unordered */
    if (v1 >= v_nan) result ^= 2048;    /* Should be false, unordered */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096;  /* Should be false, unordered */
    if (v_nan != v_nan) result ^= 8192;  /* Should be true, unordered */
    
    /* Normal vs Infinity comparisons */
    if (v1 == v_inf) result ^= 16384;
    if (v1 != v_inf) result ^= 32768;
    if (v1 < v_inf)  result ^= 65536;
    if (v1 <= v_inf) result ^= 131072;
    if (v1 > v_inf)  result ^= 262144;
    if (v1 >= v_inf) result ^= 524288;
    
    /* Infinity vs Negative Infinity */
    if (v_inf == v_neg_inf) result ^= 1048576;
    if (v_inf != v_neg_inf) result ^= 2097152;
    if (v_inf < v_neg_inf)  result ^= 4194304;
    if (v_inf <= v_neg_inf) result ^= 8388608;
    if (v_inf > v_neg_inf)  result ^= 16777216;
    if (v_inf >= v_neg_inf) result ^= 33554432;
    
    /* Conditional moves using FP comparison results */
    double cmov_result = (v1 < v2) ? v1 : v2;
    result ^= (int)(cmov_result * 1000);
    
    cmov_result = (v1 != v_nan) ? v1 : v2;
    result ^= (int)(cmov_result * 1000);
    
    /* Complex control flow with goto to prevent optimization */
    if (v1 == v_nan) goto unordered_path;
    if (v1 < v2) goto lt_path;
    if (v1 > v2) goto gt_path;
    goto eq_path;
    
unordered_path:
    result += 1000000;
    goto end_comparisons;
    
lt_path:
    result += 2000000;
    if (v1 <= v_nan) result += 3000000;  /* This should be taken due to unordered */
    goto end_comparisons;
    
gt_path:
    result += 4000000;
    if (v1 >= v_nan) result += 5000000;  /* This should be taken due to unordered */
    goto end_comparisons;
    
eq_path:
    result += 6000000;
    if (v_nan != v_nan) result += 7000000;  /* This should be taken */
    
end_comparisons:
    
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_nan("")};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    int result = 0;
    
    /* Vector comparisons generate cmppd/ucomisd with condition codes */
    v2di cmp_eq = (v2di)(vec1 == vec2);
    v2di cmp_ne = (v2di)(vec1 != vec2);
    v2di cmp_lt = (v2di)(vec1 < vec2);
    v2di cmp_le = (v2di)(vec1 <= vec2);
    v2di cmp_gt = (v2di)(vec1 > vec2);
    v2di cmp_ge = (v2di)(vec1 >= vec2);
    
    /* Unordered vector comparisons */
    v2di cmp_unord = (v2di)(vec1 == vec_nan);
    v2di cmp_ord = (v2di)(vec1 != vec_nan);
    
    /* Extract results to prevent optimization */
    long long *eq_ptr = (long long*)&cmp_eq;
    long long *ne_ptr = (long long*)&cmp_ne;
    long long *unord_ptr = (long long*)&cmp_unord;
    
    result ^= (int)(eq_ptr[0] + eq_ptr[1]);
    result ^= (int)(ne_ptr[0] + ne_ptr[1]);
    result ^= (int)(unord_ptr[0] + unord_ptr[1]);
    
    /* Loop with vector comparisons */
    double arr1[4] = {1.0, 2.0, 3.0, 4.0};
    double arr2[4] = {4.0, 3.0, 2.0, 1.0};
    double arr_nan[4] = {__builtin_nan(""), __builtin_nan(""), 
                         __builtin_nan(""), __builtin_nan("")};
    
    for (int i = 0; i < 4; i++) {
        v2df v1 = {arr1[i], arr1[(i+1)%4]};
        v2df v2 = {arr2[i], arr2[(i+1)%4]};
        v2df v_nan = {arr_nan[i], arr_nan[(i+1)%4]};
        
        v2di mask1 = (v2di)(v1 < v2);
        v2di mask2 = (v2di)(v1 == v_nan);
        v2di mask3 = (v2di)(v1 != v_nan);
        
        long long *m1 = (long long*)&mask1;
        long long *m2 = (long long*)&mask2;
        
        result ^= (int)(m1[0] ^ m1[1]);
        result ^= (int)(m2[0] ^ m2[1]);
    }
    
    return result;
}

/* Function with inline assembly using FP condition codes */
static int asm_fp_conditions(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that uses FP comparison condition codes */
    
    /* Test for UNORDERED (parity flag) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan)
        : "cc"
    );
    result ^= (cc_result << 0);
    
    /* Test for ORDERED (not parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    /* Test for less than */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    /* Test for less or equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    /* Test for not less than (UNGE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 4);
    
    /* Test for not less or equal (UNGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnbe %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 5);
    
    /* Test for equal */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 6);
    
    /* Test for not equal (LTGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 7);
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovb %3, %0"
        : "=t"(cmov_result)
        : "u"(x), "u"(y), "u"(nan)
        : "cc"
    );
    result ^= (int)(cmov_result * 1000);
    
    return result;
}

int main(void) {
    /* Initialize FP values */
    double normal1 = 1.5;
    double normal2 = 2.5;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int checksum = 0;
    
    /* Stress FP comparisons with different value pairs */
    checksum ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(normal2, normal1, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(normal1, normal1, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(inf_val, neg_inf_val, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(nan_val, normal1, nan_val, inf_val, neg_inf_val);
    
    /* Vectorized comparisons */
    checksum ^= vector_fp_comparisons();
    
    /* Inline assembly with condition codes */
    checksum ^= asm_fp_conditions(normal1, normal2, nan_val);
    checksum ^= asm_fp_conditions(normal1, nan_val, nan_val);
    checksum ^= asm_fp_conditions(inf_val, neg_inf_val, nan_val);
    
    /* Additional unordered comparisons in main */
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    
    /* Direct comparisons that should trigger UNORDERED paths */
    if (!(v_nan == v_nan)) checksum += 1;      /* UNORDERED: NaN != NaN */
    if (v_nan != v_nan) checksum += 2;         /* UNORDERED: NaN != NaN is true */
    if (!(normal1 < v_nan)) checksum += 4;     /* UNORDERED: not less than NaN */
    if (!(normal1 <= v_nan)) checksum += 8;    /* UNORDERED: not less or equal to NaN */
    if (!(normal1 > v_nan)) checksum += 16;    /* UNORDERED: not greater than NaN */
    if (!(normal1 >= v_nan)) checksum += 32;   /* UNORDERED: not greater or equal to NaN */
    
    /* ORDERED comparisons */
    if (normal1 == normal2) checksum += 64;
    if (normal1 != normal2) checksum += 128;
    if (normal1 < normal2) checksum += 256;
    if (normal1 <= normal2) checksum += 512;
    if (normal1 > normal2) checksum += 1024;
    if (normal1 >= normal2) checksum += 2048;
    
    /* UNEQ (unordered or equal) - compare with infinity */
    if (!(v_inf == v_inf)) checksum += 4096;   /* Should be false */
    if (v_inf == v_inf) checksum += 8192;      /* Should be true (ordered equal) */
    
    /* Complex conditional expression chain */
    double a = normal1, b = normal2, c = nan_val, d = inf_val;
    
    checksum += (a < b) ? 100 : 200;
    checksum += (a != c) ? 300 : 400;      /* Should take true branch (unordered not equal) */
    checksum += (c == c) ? 500 : 600;      /* Should take false branch (unordered) */
    checksum += (a <= d) ? 700 : 800;      /* Normal <= Inf */
    checksum += (d >= a) ? 900 : 1000;     /* Inf >= Normal */
    checksum += (c <= c) ? 1100 : 1200;    /* Should take false branch (unordered) */
    
    printf("Final checksum: %d\n", checksum);
    return checksum != 0 ? 0 : 1;
}
