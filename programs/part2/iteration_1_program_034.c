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
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Comparisons with NaN (unordered cases) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, unordered */
    if (v1 != v_nan) result ^= 128;     /* Should be true if v1 is not NaN */
    if (v1 < v_nan)  result ^= 256;     /* Unordered */
    if (v_nan <= v2) result ^= 512;     /* Unordered */
    if (v_nan > v_inf) result ^= 1024;  /* Unordered */
    if (v_neg_inf >= v_nan) result ^= 2048; /* Unordered */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096;  /* Always false */
    if (v_nan != v_nan) result ^= 8192;  /* Always true */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 16384;
    if (v_neg_inf < v_inf) result ^= 32768;
    if (v_inf > v1) result ^= 65536;
    
    /* Complex conditional expressions using ?: operator */
    double cond_val = (v1 < v_nan) ? 1.0 : 2.0;  /* Unordered comparison */
    result ^= (int)cond_val;
    
    cond_val = (v_nan == v_nan) ? 3.0 : 4.0;  /* NaN equality */
    result ^= (int)cond_val;
    
    /* Goto-based complex control flow to prevent optimization */
    if (v1 != v1) {  /* Check if v1 is NaN */
        goto unordered_path;
    }
    
    if (v1 < v2) {
        result += 1000;
        goto normal_path;
    }
    
unordered_path:
    result += 2000;
    /* More unordered comparisons */
    if (!(v_nan >= v2)) result += 3000;  /* UNLT or UNORDERED */
    
normal_path:
    if (v1 <= v_inf) {
        result += 4000;
    }
    
    return result;
}

/* Function with vectorized comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), 3.0};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Various vector comparisons */
    v2di cmp_eq = (v2di)(vec1 == vec2);
    v2di cmp_neq = (v2di)(vec1 != vec2);
    v2di cmp_lt = (v2di)(vec1 < vec2);
    v2di cmp_le = (v2di)(vec1 <= vec2);
    v2di cmp_gt = (v2di)(vec1 > vec2);
    v2di cmp_ge = (v2di)(vec1 >= vec2);
    
    /* Comparisons with NaN */
    v2di cmp_nan_eq = (v2di)(vec1 == vec_nan);
    v2di cmp_nan_lt = (v2di)(vec1 < vec_nan);
    v2di cmp_nan_ge = (v2di)(vec_nan >= vec2);
    
    /* Extract results */
    long long *eq_ptr = (long long*)&cmp_eq;
    long long *neq_ptr = (long long*)&cmp_neq;
    long long *nan_eq_ptr = (long long*)&cmp_nan_eq;
    
    int result = 0;
    result += (int)(eq_ptr[0] & 1);
    result += (int)(neq_ptr[0] & 1) * 2;
    result += (int)(nan_eq_ptr[0] & 1) * 4;
    
    /* Loop with vector comparisons */
    double arr1[4] = {1.0, 2.0, __builtin_nan(""), __builtin_inf()};
    double arr2[4] = {2.0, 1.0, __builtin_inf(), __builtin_nan("")};
    int mask[4] = {0};
    
    for (int i = 0; i < 4; i++) {
        mask[i] = (arr1[i] < arr2[i]) ? 1 : 0;
        mask[i] |= (arr1[i] == arr2[i]) ? 2 : 0;
        mask[i] |= (arr1[i] != arr2[i]) ? 4 : 0;
        result += mask[i];
    }
    
    return result;
}

/* Function with inline assembly using condition codes */
static int asm_fp_condition_codes(double a, double b, double nan_val) {
    int result = 0;
    char setp_result, setnp_result, seta_result, setb_result;
    
    /* ucomisd with setp (parity/unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(setp_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result += setp_result;
    
    /* ucomisd with setnp (not parity/ordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(setnp_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += setnp_result * 2;
    
    /* ucomisd with seta (above/UNGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(seta_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result += seta_result * 4;
    
    /* ucomisd with setb (below/UNLT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(setb_result)
        : "x"(b), "x"(a)  /* reversed for below */
        : "cc"
    );
    result += setb_result * 8;
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovbe %3, %0"
        : "=t"(cmov_result)
        : "u"(a), "u"(b), "t"(3.14159)
        : "cc"
    );
    result += (int)cmov_result;
    
    return result;
}

/* Main function that orchestrates all tests */
int main(void) {
    double normal1 = 1.5;
    double normal2 = 2.5;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int checksum = 0;
    
    /* Test 1: Exhaustive scalar comparisons */
    checksum += fp_comparison_stress(normal1, normal2, nan_val, inf_val, neg_inf_val);
    checksum += fp_comparison_stress(nan_val, normal1, nan_val, inf_val, neg_inf_val);
    checksum += fp_comparison_stress(inf_val, neg_inf_val, nan_val, inf_val, neg_inf_val);
    checksum += fp_comparison_stress(nan_val, nan_val, nan_val, inf_val, neg_inf_val);
    
    /* Test 2: Vectorized comparisons */
    checksum += vector_fp_comparisons();
    
    /* Test 3: Inline assembly with condition codes */
    checksum += asm_fp_condition_codes(normal1, normal2, nan_val);
    checksum += asm_fp_condition_codes(nan_val, normal1, nan_val);
    checksum += asm_fp_condition_codes(inf_val, neg_inf_val, nan_val);
    
    /* Additional unordered comparison patterns */
    volatile double v_nan = nan_val;
    volatile double v_inf = inf_val;
    
    /* UNORDERED/ORDERED tests */
    if (!(v_nan < v_inf) && !(v_nan >= v_inf)) {
        checksum += 100000;  /* Both false when unordered */
    }
    
    /* UNEQ test (unordered or equal) */
    if (v_nan == v_nan || v_nan != v_nan) {  /* One is always true */
        checksum += 200000;
    }
    
    /* UNGE test (not less than) */
    if (!(v_nan < normal1)) {
        checksum += 300000;
    }
    
    /* UNGT test (not less than or equal) */
    if (!(v_nan <= normal1)) {
        checksum += 400000;
    }
    
    /* UNLE test (unordered or less than or equal) */
    if (v_nan <= v_nan || normal1 <= normal2) {
        checksum += 500000;
    }
    
    /* UNLT test (unordered or less than) */
    if (v_nan < v_nan || normal1 < normal2) {
        checksum += 600000;
    }
    
    /* LTGT test (less than or greater than, but not equal and not unordered) */
    if ((normal1 < normal2) || (normal1 > normal2)) {
        checksum += 700000;
    }
    
    /* Final output to prevent dead code elimination */
    printf("FP comparison checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
