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
    
    /* Complex control flow with goto to prevent optimization */
    if (v1 == v2) goto label_eq;
    if (v1 != v2) goto label_neq;
    
label_eq:
    /* Ordered comparisons */
    if (v1 < v2) result |= 1;
    if (v1 <= v2) result |= 2;
    if (v1 > v2) result |= 4;
    if (v1 >= v2) result |= 8;
    
    /* Unordered comparisons with NaN */
    if (v_nan == v1) result |= 16;      /* UNORDERED/UNEQ path */
    if (v1 < v_nan) result |= 32;       /* UNORDERED/UNLT path */
    if (v_nan <= v1) result |= 64;      /* UNORDERED/UNLE path */
    if (v_nan > v_inf) result |= 128;   /* UNORDERED/UNGT path */
    if (v_neg_inf >= v_nan) result |= 256; /* UNORDERED/UNGE path */
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result |= 512;  /* Always false - UNORDERED */
    if (v_nan != v_nan) result |= 1024; /* Always true - UNORDERED/NEQ */
    
    goto label_done;
    
label_neq:
    /* More unordered comparisons */
    if (!(v_nan < v1)) result |= 2048;   /* ORDERED/UNGE (nlt) */
    if (!(v_nan <= v1)) result |= 4096;  /* ORDERED/UNGT (nle) */
    if (v1 != v1) result |= 8192;        /* UNORDERED check */
    
    /* LTGT (une) condition - ordered and not equal */
    if ((v1 < v2) || (v1 > v2)) result |= 16384;
    
label_done:
    /* Conditional moves based on FP comparisons */
    double cmov_result = (v1 < v2) ? 1.0 : 2.0;
    cmov_result = (v_nan == v1) ? 3.0 : cmov_result;
    cmov_result = (v1 != v1) ? 4.0 : cmov_result;
    cmov_result = (v1 >= v_nan) ? 5.0 : cmov_result;
    
    /* Use the result to prevent dead code elimination */
    result += (int)cmov_result;
    
    return result;
}

/* Function with inline assembly using condition codes */
static int inline_asm_fp_conds(double x, double y, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* ucomisd with setp (parity/UNORDERED) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan_val)
        : "cc"
    );
    result |= (cc_result << 0);
    
    /* ucomisd with setb (below/UNLT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 1);
    
    /* ucomisd with setbe (below or equal/UNLE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(nan_val), "x"(x)
        : "cc"
    );
    result |= (cc_result << 2);
    
    /* ucomisd with seta (above/UNGT) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(x), "x"(nan_val)
        : "cc"
    );
    result |= (cc_result << 3);
    
    /* ucomisd with setae (above or equal/UNGE) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc_result)
        : "x"(nan_val), "x"(x)
        : "cc"
    );
    result |= (cc_result << 4);
    
    /* ucomisd with setne (not equal/NEQ or UNEQ) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(x), "x"(y)
        : "cc"
    );
    result |= (cc_result << 5);
    
    return result;
}

/* Vectorized FP comparisons */
static v2di vector_fp_comparisons(v2df a, v2df b, v2df nan_vec) {
    /* Generate all possible comparison conditions */
    v2df cmp_eq = a == b;      /* EQ */
    v2df cmp_neq = a != b;     /* NEQ/UNEQ */
    v2df cmp_lt = a < b;       /* LT/UNLT */
    v2df cmp_le = a <= b;      /* LE/UNLE */
    v2df cmp_gt = a > b;       /* GT/UNGT */
    v2df cmp_ge = a >= b;      /* GE/UNGE */
    
    /* Comparisons with NaN */
    v2df cmp_nan_eq = a == nan_vec;    /* UNORDERED */
    v2df cmp_nan_neq = a != nan_vec;   /* ORDERED */
    v2df cmp_nan_lt = a < nan_vec;     /* UNORDERED/UNLT */
    v2df cmp_nan_gt = nan_vec > b;     /* UNORDERED/UNGT */
    
    /* Combine results into integer mask */
    v2di mask = (v2di)cmp_eq | (v2di)cmp_neq | (v2di)cmp_lt |
                (v2di)cmp_le | (v2di)cmp_gt | (v2di)cmp_ge |
                (v2di)cmp_nan_eq | (v2di)cmp_nan_neq |
                (v2di)cmp_nan_lt | (v2di)cmp_nan_gt;
    
    return mask;
}

/* Main test function */
int main(void) {
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double zero = 0.0;
    double neg_zero = -0.0;
    double inf = __builtin_inf();
    double neg_inf = -__builtin_inf();
    double nan = __builtin_nan("");
    
    int checksum = 0;
    
    /* Test all combinations of values */
    double test_values[] = {normal1, normal2, zero, neg_zero, inf, neg_inf, nan};
    int num_values = sizeof(test_values) / sizeof(test_values[0]);
    
    /* Exhaustive matrix of comparisons */
    for (int i = 0; i < num_values; i++) {
        for (int j = 0; j < num_values; j++) {
            checksum ^= stress_fp_comparisons(
                test_values[i], 
                test_values[j],
                nan,
                inf,
                neg_inf
            );
        }
    }
    
    /* Inline assembly tests */
    checksum += inline_asm_fp_conds(normal1, normal2, nan);
    checksum += inline_asm_fp_conds(nan, normal1, nan);
    checksum += inline_asm_fp_conds(inf, neg_inf, nan);
    
    /* Vectorized tests */
    v2df vec_a = {normal1, normal2};
    v2df vec_b = {normal2, normal1};
    v2df vec_nan = {nan, nan};
    
    v2di vec_result = vector_fp_comparisons(vec_a, vec_b, vec_nan);
    
    /* Extract results from vector */
    long long* vec_res_ptr = (long long*)&vec_result;
    checksum += (int)(vec_res_ptr[0] ^ vec_res_ptr[1]);
    
    /* Additional unordered comparison patterns */
    volatile double v_nan = nan;
    volatile double v_inf = inf;
    
    /* Direct use of all condition types in control flow */
    if (v_nan == v_nan) checksum += 1;      /* UNORDERED */
    if (v_nan != v_nan) checksum += 2;      /* ORDERED/NEQ */
    if (!(v_nan < v_inf)) checksum += 4;    /* UNGE (nlt) */
    if (!(v_nan <= v_inf)) checksum += 8;   /* UNGT (nle) */
    if (v_inf <= v_nan) checksum += 16;     /* UNLE */
    if (v_nan < v_inf) checksum += 32;      /* UNLT */
    if ((normal1 < normal2) || (normal1 > normal2)) checksum += 64; /* LTGT (une) */
    
    /* Complex conditional expression chain */
    double complex_result = 
        (v_nan == normal1) ? 1.0 :
        (normal1 != normal1) ? 2.0 :
        (v_nan >= v_inf) ? 3.0 :
        (!(v_nan < normal2)) ? 4.0 :
        (!(v_nan <= normal2)) ? 5.0 :
        (v_inf <= v_nan) ? 6.0 :
        (v_nan < v_inf) ? 7.0 :
        ((normal1 < normal2) || (normal1 > normal2)) ? 8.0 : 9.0;
    
    checksum += (int)complex_result;
    
    printf("FP condition code test checksum: %d\n", checksum);
    
    /* Prevent optimization of all computations */
    volatile int dummy = checksum;
    return dummy > 0 ? 0 : 1;
}
