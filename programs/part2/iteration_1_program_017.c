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
    if (v1 != v_nan) result ^= 128;     /* Should be true, unordered */
    if (v1 < v_nan)  result ^= 256;     /* Should be false, unordered */
    if (v1 <= v_nan) result ^= 512;     /* Should be false, unordered */
    if (v1 > v_nan)  result ^= 1024;    /* Should be false, unordered */
    if (v1 >= v_nan) result ^= 2048;    /* Should be false, unordered */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096; /* Should be false, unordered */
    if (v_nan != v_nan) result ^= 8192; /* Should be true, unordered */
    
    /* Comparisons with infinity */
    if (v1 == v_inf) result ^= 16384;
    if (v1 != v_inf) result ^= 32768;
    if (v1 < v_inf)  result ^= 65536;
    if (v1 <= v_inf) result ^= 131072;
    if (v1 > v_inf)  result ^= 262144;
    if (v1 >= v_inf) result ^= 524288;
    
    /* Negative infinity comparisons */
    if (v1 == v_neg_inf) result ^= 1048576;
    if (v1 != v_neg_inf) result ^= 2097152;
    if (v1 < v_neg_inf)  result ^= 4194304;
    if (v1 <= v_neg_inf) result ^= 8388608;
    if (v1 > v_neg_inf)  result ^= 16777216;
    if (v1 >= v_neg_inf) result ^= 33554432;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v2) ? 1.0 : (v1 > v2) ? -1.0 : 0.0;
    cond_result = (v1 == v_nan) ? 99.0 : cond_result;
    cond_result = (v_nan != v_nan) ? 100.0 : cond_result;
    
    /* Use conditional move-like behavior */
    int int_result = 0;
    int_result += (v1 < v2) ? 1 : 0;
    int_result += (v1 <= v2) ? 2 : 0;
    int_result += (v1 > v2) ? 4 : 0;
    int_result += (v1 >= v2) ? 8 : 0;
    int_result += (v1 == v2) ? 16 : 0;
    int_result += (v1 != v2) ? 32 : 0;
    
    /* Unordered-specific checks */
    int_result += (v1 == v_nan) ? 64 : 0;   /* UNORDERED/UNEQ paths */
    int_result += (v1 != v_nan) ? 128 : 0;  /* UNORDERED path */
    int_result += (v1 < v_nan) ? 256 : 0;   /* UNORDERED/UNLT paths */
    int_result += (v1 > v_nan) ? 512 : 0;   /* UNORDERED/UNGT paths */
    
    result ^= int_result;
    
    return result;
}

/* Function with goto-based complex control flow */
static int fp_goto_stress(double x, double y, double nan_val) {
    volatile double a = x;
    volatile double b = y;
    volatile double nan = nan_val;
    
    int checksum = 0;
    
    /* Use goto to create non-linear control flow around FP comparisons */
    if (a < b) {
        checksum ^= 1;
        goto label1;
    }
    
    if (a == nan) {
        checksum ^= 2;
        goto label2;
    }
    
label1:
    if (b > nan) {
        checksum ^= 4;
        goto label3;
    }
    
label2:
    if (a != a) {  /* NaN check */
        checksum ^= 8;
        goto label4;
    }
    
label3:
    if (a <= b) {
        checksum ^= 16;
    }
    
label4:
    if (b >= a) {
        checksum ^= 32;
    }
    
    /* More unordered comparisons */
    if (!(a < nan)) {  /* UNGE: not less than (nlt) */
        checksum ^= 64;
    }
    
    if (!(a > nan)) {  /* UNLE: not greater than (nle) */
        checksum ^= 128;
    }
    
    if (a == b || (a != a || b != b)) {  /* UNEQ: equal or unordered */
        checksum ^= 256;
    }
    
    if (a < b || (a != a || b != b)) {  /* UNLT: less than or unordered */
        checksum ^= 512;
    }
    
    if (a > b || (a != a || b != b)) {  /* UNGT: greater than or unordered */
        checksum ^= 1024;
    }
    
    if (a <= b || (a != a || b != b)) {  /* UNLE: less than or equal or unordered */
        checksum ^= 2048;
    }
    
    if (a >= b || (a != a || b != b)) {  /* UNGE: greater than or equal or unordered */
        checksum ^= 4096;
    }
    
    if ((a < b) != (a > b) && (a == a && b == b)) {  /* LTGT: less than or greater than (but not equal, and ordered) */
        checksum ^= 8192;
    }
    
    return checksum;
}

/* Vectorized FP comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), 3.0};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Perform various vector comparisons */
    v2di cmp_eq = (v2di)(vec1 == vec2);
    v2di cmp_ne = (v2di)(vec1 != vec2);
    v2di cmp_lt = (v2di)(vec1 < vec2);
    v2di cmp_le = (v2di)(vec1 <= vec2);
    v2di cmp_gt = (v2di)(vec1 > vec2);
    v2di cmp_ge = (v2di)(vec1 >= vec2);
    
    /* Comparisons with NaN */
    v2di cmp_nan_eq = (v2di)(vec1 == vec_nan);
    v2di cmp_nan_ne = (v2di)(vec1 != vec_nan);
    v2di cmp_nan_lt = (v2di)(vec1 < vec_nan);
    
    /* Comparisons with infinity */
    v2di cmp_inf_gt = (v2di)(vec1 > vec_inf);
    v2di cmp_inf_lt = (v2di)(vec1 < vec_inf);
    
    /* Extract results to prevent optimization */
    long long *eq_ptr = (long long*)&cmp_eq;
    long long *nan_ptr = (long long*)&cmp_nan_eq;
    
    return (int)(eq_ptr[0] ^ eq_ptr[1] ^ nan_ptr[0] ^ nan_ptr[1]);
}

/* Inline assembly with condition codes */
static int inline_asm_fp(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    int result = 0;
    char cc_result;
    
    /* ucomisd with setp (parity/unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan)
        : "cc"
    );
    result ^= (cc_result << 0);
    
    /* ucomisd with seta (above: CF=0 and ZF=0) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    /* ucomisd with setb (below: CF=1) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    /* ucomisd with sete (equal: ZF=1) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    /* ucomisd with setne (not equal: ZF=0) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result ^= (cc_result << 4);
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovb %3, %0"
        : "=t"(cmov_result)
        : "u"(x), "u"(y), "t"(nan)
        : "cc"
    );
    
    return result ^ ((int)cmov_result & 0xFF);
}

/* Main test driver */
int main(void) {
    double normal1 = 1.5;
    double normal2 = 2.5;
    double zero = 0.0;
    double neg = -3.14;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int final_result = 0;
    
    printf("Starting FP comparison coverage test...\n");
    
    /* Test 1: Exhaustive scalar comparisons */
    final_result ^= fp_comparison_stress(normal1, normal2, nan_val, inf_val, neg_inf_val);
    final_result ^= fp_comparison_stress(normal1, nan_val, nan_val, inf_val, neg_inf_val);
    final_result ^= fp_comparison_stress(nan_val, nan_val, nan_val, inf_val, neg_inf_val);
    final_result ^= fp_comparison_stress(inf_val, normal1, nan_val, inf_val, neg_inf_val);
    final_result ^= fp_comparison_stress(neg_inf_val, inf_val, nan_val, inf_val, neg_inf_val);
    
    /* Test 2: Goto-based control flow */
    final_result ^= fp_goto_stress(normal1, normal2, nan_val);
    final_result ^= fp_goto_stress(nan_val, normal1, nan_val);
    final_result ^= fp_goto_stress(inf_val, neg_inf_val, nan_val);
    
    /* Test 3: Vectorized comparisons */
    final_result ^= vector_fp_comparisons();
    
    /* Test 4: Inline assembly */
    final_result ^= inline_asm_fp(normal1, normal2, nan_val);
    final_result ^= inline_asm_fp(nan_val, normal1, nan_val);
    final_result ^= inline_asm_fp(inf_val, normal1, nan_val);
    
    /* Additional unordered condition triggers */
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double vnan = nan_val;
    
    /* Direct checks for all condition code cases */
    int unordered_check = 0;
    
    /* Simulate all the switch cases from i386.cc */
    /* UNORDERED case: (a != a) || (b != b) */
    if (v1 != v1 || v2 != v2) unordered_check ^= 1;
    if (vnan != vnan || v1 != v1) unordered_check ^= 2;
    
    /* ORDERED case: (a == a) && (b == b) */
    if (v1 == v1 && v2 == v2) unordered_check ^= 4;
    
    /* UNEQ case: (a == b) || (a != a) || (b != b) */
    if (v1 == v2 || v1 != v1 || v2 != v2) unordered_check ^= 8;
    
    /* UNGE case: (a >= b) || (a != a) || (b != b) */
    if (v1 >= v2 || v1 != v1 || v2 != v2) unordered_check ^= 16;
    
    /* UNGT case: (a > b) || (a != a) || (b != b) */
    if (v1 > v2 || v1 != v1 || v2 != v2) unordered_check ^= 32;
    
    /* UNLE case: (a <= b) || (a != a) || (b != b) */
    if (v1 <= v2 || v1 != v1 || v2 != v2) unordered_check ^= 64;
    
    /* UNLT case: (a < b) || (a != a) || (b != b) */
    if (v1 < v2 || v1 != v1 || v2 != v2) unordered_check ^= 128;
    
    /* LTGT case: (a < b) || (a > b) (and both ordered) */
    if ((v1 < v2 || v1 > v2) && (v1 == v1 && v2 == v2)) unordered_check ^= 256;
    
    final_result ^= unordered_check;
    
    printf("Final checksum: %d\n", final_result);
    printf("Test completed.\n");
    
    return final_result != 0 ? 0 : 1;
}
