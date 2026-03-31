/* Compile with: gcc -std=c99 -O2 -march=x86-64 -ffp-contract=off -o fp_cond_test fp_cond_test.c */
/* Also try: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math */
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
    
    /* Complex control flow with goto to prevent optimization */
    if (v1 == v2) goto label_eq;
    if (v1 != v2) goto label_neq;
    
label_eq:
    /* Ordered equality comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 <= v2) result ^= 2;
    if (v1 >= v2) result ^= 4;
    
    /* Unordered comparisons with NaN */
    if (v_nan == v1) result ^= 8;      /* Should be false (unordered) */
    if (v1 < v_nan) result ^= 16;      /* Should be false (unordered) */
    if (v_nan != v_nan) result ^= 32;  /* NaN != NaN is true */
    
    goto label_done;
    
label_neq:
    /* Ordered inequality comparisons */
    if (v1 != v2) result ^= 64;
    if (v1 < v2) result ^= 128;
    if (v1 > v2) result ^= 256;
    if (v1 <= v2) result ^= 512;
    if (v1 >= v2) result ^= 1024;
    
    /* More unordered comparisons */
    if (!(v_nan < v1)) result ^= 2048;  /* NOT(LT) for NaN */
    if (!(v1 > v_nan)) result ^= 4096;  /* NOT(GT) for NaN */
    
label_done:
    /* Conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? 1.0 : 2.0;
    result ^= (int)cmov_result;
    
    cmov_result = (v_nan == v_nan) ? 3.0 : 4.0;  /* NaN == NaN is false */
    result ^= (int)cmov_result;
    
    cmov_result = (v1 != v_nan) ? 5.0 : 6.0;     /* v1 != NaN is true (ordered) */
    result ^= (int)cmov_result;
    
    /* Comparisons with infinity */
    if (v1 < v_inf) result ^= 8192;
    if (v1 > v_neg_inf) result ^= 16384;
    if (v_inf == v_inf) result ^= 32768;  /* Inf == Inf is true */
    
    return result;
}

/* Function with vectorized FP comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), __builtin_nan("")};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    int result = 0;
    
    /* Vector comparisons generating various condition codes */
    v2di cmp_eq = (vec1 == vec2);
    result ^= cmp_eq[0] ^ cmp_eq[1];
    
    v2di cmp_lt = (vec1 < vec2);
    result ^= cmp_lt[0] ^ cmp_lt[1];
    
    v2di cmp_le = (vec1 <= vec2);
    result ^= cmp_le[0] ^ cmp_le[1];
    
    v2di cmp_gt = (vec1 > vec2);
    result ^= cmp_gt[0] ^ cmp_gt[1];
    
    v2di cmp_ge = (vec1 >= vec2);
    result ^= cmp_ge[0] ^ cmp_ge[1];
    
    v2di cmp_neq = (vec1 != vec2);
    result ^= cmp_neq[0] ^ cmp_neq[1];
    
    /* Unordered vector comparisons */
    v2di cmp_nan_eq = (vec1 == vec_nan);
    result ^= cmp_nan_eq[0] ^ cmp_nan_eq[1];
    
    v2di cmp_nan_lt = (vec1 < vec_nan);
    result ^= cmp_nan_lt[0] ^ cmp_nan_lt[1];
    
    v2di cmp_nan_neq = (vec_nan != vec_nan);
    result ^= cmp_nan_neq[0] ^ cmp_nan_neq[1];
    
    /* Comparisons with infinity */
    v2di cmp_inf_lt = (vec1 < vec_inf);
    result ^= cmp_inf_lt[0] ^ cmp_inf_lt[1];
    
    v2di cmp_inf_gt = (vec1 > vec_inf);
    result ^= cmp_inf_gt[0] ^ cmp_inf_gt[1];
    
    return result;
}

/* Function with inline assembly using FP condition codes */
static int asm_fp_condition_codes(double a, double b, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setp %[res]\n\t"
        : [res]"=r"(cc_result)
        : [a]"x"(a), [b]"x"(b)
        : "cc"
    );
    result ^= cc_result;
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "sete %[res]\n\t"
        : [res]"=r"(cc_result)
        : [a]"x"(a), [b]"x"(b)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setb %[res]\n\t"
        : [res]"=r"(cc_result)
        : [a]"x"(a), [b]"x"(b)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setbe %[res]\n\t"
        : [res]"=r"(cc_result)
        : [a]"x"(a), [b]"x"(b)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "seta %[res]\n\t"
        : [res]"=r"(cc_result)
        : [a]"x"(a), [b]"x"(b)
        : "cc"
    );
    result ^= (cc_result << 4);
    
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setae %[res]\n\t"
        : [res]"=r"(cc_result)
        : [a]"x"(a), [b]"x"(b)
        : "cc"
    );
    result ^= (cc_result << 5);
    
    /* Test with NaN */
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "setnp %[res]\n\t"
        : [res]"=r"(cc_result)
        : [a]"x"(nan_val), [b]"x"(b)
        : "cc"
    );
    result ^= (cc_result << 6);
    
    /* Conditional move based on FP comparison */
    double cmov_dest;
    asm volatile (
        "ucomisd %[b], %[a]\n\t"
        "fcmovb %[src], %[dest]\n\t"
        : [dest]"=t"(cmov_dest)
        : [a]"t"(a), [b]"t"(b), [src]"t"(3.14159)
        : "cc"
    );
    result ^= (int)cmov_dest;
    
    return result;
}

/* Complex control flow with FP comparisons */
static int complex_fp_control_flow(double a, double b, double nan_val) {
    volatile double x = a;
    volatile double y = b;
    volatile double nan = nan_val;
    
    int result = 0;
    
    /* Nested if-else chains with FP comparisons */
    if (x < y) {
        result ^= 1;
        if (x != y) {
            result ^= 2;
            if (x <= y) {
                result ^= 4;
                goto label1;
            } else {
                goto label2;
            }
        }
    } else if (x > y) {
        result ^= 8;
        if (x >= y) {
            result ^= 16;
label1:
            if (x == y) {
                result ^= 32;
            }
        }
    } else {
        result ^= 64;
    }
    
label2:
    /* Unordered comparison branches */
    if (nan == x) {
        result ^= 128;
    } else if (!(nan < x)) {
        result ^= 256;
    } else if (!(x > nan)) {
        result ^= 512;
    }
    
    /* Switch-like behavior using goto */
    if (x != x) {  /* x is NaN */
        goto unordered_case;
    } else if (x < y) {
        goto lt_case;
    } else if (x > y) {
        goto gt_case;
    } else {
        goto eq_case;
    }
    
unordered_case:
    result ^= 1024;
    if (nan != nan) result ^= 2048;
    goto end;
    
lt_case:
    result ^= 4096;
    if (x <= y) result ^= 8192;
    goto end;
    
gt_case:
    result ^= 16384;
    if (x >= y) result ^= 32768;
    goto end;
    
eq_case:
    result ^= 65536;
    if (x == y) result ^= 131072;
    
end:
    return result;
}

int main(void) {
    /* Initialize FP values */
    double normal1 = 1.5;
    double normal2 = 2.5;
    double zero = 0.0;
    double neg = -3.14;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int checksum = 0;
    
    /* Test all combinations of value pairs */
    double test_values[] = {normal1, normal2, zero, neg, nan_val, inf_val, neg_inf_val};
    const int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            checksum ^= stress_fp_comparisons(test_values[i], test_values[j], 
                                            nan_val, inf_val, neg_inf_val);
        }
    }
    
    /* Vectorized comparisons */
    checksum ^= vector_fp_comparisons();
    
    /* Inline assembly with condition codes */
    checksum ^= asm_fp_condition_codes(normal1, normal2, nan_val);
    checksum ^= asm_fp_condition_codes(normal1, nan_val, nan_val);
    checksum ^= asm_fp_condition_codes(inf_val, normal2, nan_val);
    
    /* Complex control flow */
    checksum ^= complex_fp_control_flow(normal1, normal2, nan_val);
    checksum ^= complex_fp_control_flow(nan_val, normal1, nan_val);
    checksum ^= complex_fp_control_flow(inf_val, neg_inf_val, nan_val);
    
    /* Prevent dead code elimination */
    volatile int* volatile_ptr = &checksum;
    *volatile_ptr = checksum;
    
    printf("FP comparison condition code test completed. Checksum: %d\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
