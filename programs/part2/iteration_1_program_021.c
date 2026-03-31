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
    
    /* Normal vs NaN comparisons (unordered cases) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, unordered */
    if (v1 != v_nan) result ^= 128;     /* Should be true, unordered */
    if (v1 < v_nan)  result ^= 256;     /* Should be false, unordered */
    if (v1 <= v_nan) result ^= 512;     /* Should be false, unordered */
    if (v1 > v_nan)  result ^= 1024;    /* Should be false, unordered */
    if (v1 >= v_nan) result ^= 2048;    /* Should be false, unordered */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096;  /* Should be false */
    if (v_nan != v_nan) result ^= 8192;  /* Should be true */
    
    /* Infinity comparisons */
    if (v1 == v_inf) result ^= 16384;
    if (v_inf == v_neg_inf) result ^= 32768;
    if (v_inf > v_neg_inf) result ^= 65536;
    
    /* Complex conditional expressions with ?: operator */
    double cond_result = (v1 < v2) ? 1.0 : (v1 > v2) ? -1.0 : 0.0;
    result ^= (int)(cond_result * 1000);
    
    /* More complex unordered checks */
    int unordered_check = (v1 != v1) || (v2 != v2);  /* Check for NaN */
    result ^= unordered_check ? 0x10000 : 0x20000;
    
    /* Goto-based control flow to prevent optimization */
    if (v1 < v2) goto less_label;
    if (v1 > v2) goto greater_label;
    if (v1 == v2) goto equal_label;
    
    /* Unordered path */
    if (v1 != v1 || v2 != v2) goto unordered_label;
    
less_label:
    result += 0x40000;
    goto continue_label;
    
greater_label:
    result += 0x80000;
    goto continue_label;
    
equal_label:
    result += 0x100000;
    goto continue_label;
    
unordered_label:
    result += 0x200000;
    
continue_label:
    return result;
}

/* Function with vectorized comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_nan("")};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Various vector comparisons */
    v2di cmp_eq = (v2di)(vec1 == vec2);
    v2di cmp_neq = (v2di)(vec1 != vec2);
    v2di cmp_lt = (v2di)(vec1 < vec2);
    v2di cmp_le = (v2di)(vec1 <= vec2);
    v2di cmp_gt = (v2di)(vec1 > vec2);
    v2di cmp_ge = (v2di)(vec1 >= vec2);
    
    /* Unordered comparisons with NaN */
    v2di cmp_nan_eq = (v2di)(vec1 == vec_nan);
    v2di cmp_nan_neq = (v2di)(vec1 != vec_nan);
    
    /* Infinity comparisons */
    v2di cmp_inf = (v2di)(vec1 == vec_inf);
    
    /* Aggregate results */
    int result = 0;
    result ^= cmp_eq[0] ^ cmp_eq[1];
    result ^= cmp_neq[0] ^ cmp_neq[1];
    result ^= cmp_lt[0] ^ cmp_lt[1];
    result ^= cmp_le[0] ^ cmp_le[1];
    result ^= cmp_gt[0] ^ cmp_gt[1];
    result ^= cmp_ge[0] ^ cmp_ge[1];
    result ^= cmp_nan_eq[0] ^ cmp_nan_eq[1];
    result ^= cmp_nan_neq[0] ^ cmp_nan_neq[1];
    result ^= cmp_inf[0] ^ cmp_inf[1];
    
    return result;
}

/* Function with inline assembly using condition codes */
static int asm_fp_comparisons(double a, double b) {
    int result = 0;
    char setp_result, setnp_result;
    char setz_result, setnz_result;
    char setb_result, setnb_result;
    char seta_result, setna_result;
    char setbe_result, setnbe_result;
    
    /* Test UNORDERED (parity) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(setp_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= setp_result ? 1 : 0;
    
    /* Test ORDERED */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnp %0"
        : "=r"(setnp_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= setnp_result ? 2 : 0;
    
    /* Test EQUAL/UNEQ */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setz %0"
        : "=r"(setz_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= setz_result ? 4 : 0;
    
    /* Test NOT EQUAL */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setnz %0"
        : "=r"(setnz_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= setnz_result ? 8 : 0;
    
    /* Test BELOW (less than, unordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(setb_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= setb_result ? 16 : 0;
    
    /* Test ABOVE (greater than) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(seta_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= seta_result ? 32 : 0;
    
    /* Test BELOW OR EQUAL */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(setbe_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= setbe_result ? 64 : 0;
    
    return result;
}

/* Main function with exhaustive testing */
int main(void) {
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    double normal1 = 1.5;
    double normal2 = 2.5;
    double zero = 0.0;
    double neg_zero = -0.0;
    
    int total_result = 0;
    
    /* Test various combinations */
    total_result ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    total_result ^= stress_fp_comparisons(normal1, nan_val, nan_val, inf_val, neg_inf_val);
    total_result ^= stress_fp_comparisons(nan_val, normal2, nan_val, inf_val, neg_inf_val);
    total_result ^= stress_fp_comparisons(nan_val, nan_val, nan_val, inf_val, neg_inf_val);
    total_result ^= stress_fp_comparisons(inf_val, normal2, nan_val, inf_val, neg_inf_val);
    total_result ^= stress_fp_comparisons(neg_inf_val, inf_val, nan_val, inf_val, neg_inf_val);
    total_result ^= stress_fp_comparisons(zero, neg_zero, nan_val, inf_val, neg_inf_val);
    
    /* Vector comparisons */
    total_result ^= vector_fp_comparisons();
    
    /* Inline assembly comparisons */
    total_result ^= asm_fp_comparisons(normal1, normal2);
    total_result ^= asm_fp_comparisons(normal1, nan_val);
    total_result ^= asm_fp_comparisons(nan_val, normal2);
    total_result ^= asm_fp_comparisons(inf_val, normal2);
    
    /* Loop with array comparisons */
    double arr1[4] = {1.0, 2.0, nan_val, inf_val};
    double arr2[4] = {2.0, 1.0, nan_val, neg_inf_val};
    
    for (int i = 0; i < 4; i++) {
        volatile double x = arr1[i];
        volatile double y = arr2[i];
        
        /* Complex conditional chain */
        if (x < y) {
            total_result += 0x1000 * i;
        } else if (x > y) {
            total_result += 0x2000 * i;
        } else if (x == y) {
            total_result += 0x3000 * i;
        } else {
            /* Unordered case */
            total_result += 0x4000 * i;
        }
        
        /* Conditional move style */
        double cmov_result = (x < y) ? x : (x > y) ? y : (x == y) ? 0.0 : nan_val;
        total_result ^= (int)(cmov_result * 100);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result checksum: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}
