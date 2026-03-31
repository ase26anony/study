/* fp_condition_stress.c
 * Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off fp_condition_stress.c -o fp_condition_stress
 * Also test with: gcc -std=c99 -O3 -msse4.2 -ftree-vectorize -fno-trapping-math fp_condition_stress.c -o fp_condition_stress
 * For 32-bit: gcc -std=c99 -O1 -m32 -mfpmath=387 -fno-inline fp_condition_stress.c -o fp_condition_stress_32
 */

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
    
    /* 1. Ordered comparisons (normal numbers) */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* 2. Comparisons with NaN (unordered cases) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, may trigger UNORDERED */
    if (v1 != v_nan) result ^= 128;     /* Should be true, may trigger ORDERED/UNORDERED */
    if (v1 < v_nan)  result ^= 256;     /* Should be false, unordered */
    if (v_nan <= v2) result ^= 512;     /* Should be false, unordered */
    if (v_nan > v_inf) result ^= 1024;  /* Should be false, unordered */
    if (v_nan >= v_neg_inf) result ^= 2048; /* Should be false, unordered */
    
    /* 3. NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 4096;  /* Should be false, UNORDERED/UNEQ */
    if (v_nan != v_nan) result ^= 8192;  /* Should be true, UNORDERED/LTGT */
    
    /* 4. Infinity comparisons */
    if (v_inf == v_inf) result ^= 16384;
    if (v_inf <= v_neg_inf) result ^= 32768;
    if (v_neg_inf < v_inf) result ^= 65536;
    
    /* 5. Mixed comparisons */
    if (v1 == v_inf) result ^= 131072;
    if (v_neg_inf <= v2) result ^= 262144;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v_nan) ? 1.0 : 2.0;
    result ^= (int)(cond_result * 1000);
    
    cond_result = (v_nan != v_nan) ? 3.0 : 4.0;
    result ^= (int)(cond_result * 1000);
    
    cond_result = (v_inf > v_neg_inf) ? 5.0 : 6.0;
    result ^= (int)(cond_result * 1000);
    
    /* Goto-based control flow to prevent optimization */
    if (v1 != v1) { /* Always false unless v1 is NaN */
        goto nan_branch;
    }
    
normal_flow:
    /* More comparisons in normal flow */
    if (!(v1 >= v_nan)) result ^= 524288;  /* NOT GE -> LT or UNORDERED */
    if (!(v1 <= v_nan)) result ^= 1048576; /* NOT LE -> GT or UNORDERED */
    
    goto after_nan;
    
nan_branch:
    if (v2 == v2) { /* Should be true unless v2 is NaN */
        result ^= 2097152;
    }
    goto normal_flow;
    
after_nan:
    
    return result;
}

/* Function with vectorized comparisons */
static int vector_fp_comparisons(void) {
    v2df vec1 = {1.0, 2.0};
    v2df vec2 = {2.0, 1.0};
    v2df vec_nan = {__builtin_nan(""), 3.0};
    v2df vec_inf = {__builtin_inf(), -__builtin_inf()};
    
    /* Various vector comparisons generating different condition codes */
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
    v2di cmp_nan_le = (v2di)(vec1 <= vec_nan);
    
    /* NaN vs NaN */
    v2di cmp_nan_nan_eq = (v2di)(vec_nan == vec_nan);
    v2di cmp_nan_nan_ne = (v2di)(vec_nan != vec_nan);
    
    /* Infinity comparisons */
    v2di cmp_inf = (v2di)(vec_inf == vec_inf);
    v2di cmp_inf_neg = (v2di)(vec_inf > -vec_inf);
    
    /* Aggregate results */
    int result = 0;
    result ^= cmp_eq[0] ^ cmp_eq[1];
    result ^= cmp_ne[0] ^ cmp_ne[1];
    result ^= cmp_lt[0] ^ cmp_lt[1];
    result ^= cmp_le[0] ^ cmp_le[1];
    result ^= cmp_gt[0] ^ cmp_gt[1];
    result ^= cmp_ge[0] ^ cmp_ge[1];
    result ^= cmp_nan_eq[0] ^ cmp_nan_eq[1];
    result ^= cmp_nan_ne[0] ^ cmp_nan_ne[1];
    result ^= cmp_nan_lt[0] ^ cmp_nan_lt[1];
    result ^= cmp_nan_le[0] ^ cmp_nan_le[1];
    result ^= cmp_nan_nan_eq[0] ^ cmp_nan_nan_eq[1];
    result ^= cmp_nan_nan_ne[0] ^ cmp_nan_nan_ne[1];
    result ^= cmp_inf[0] ^ cmp_inf[1];
    result ^= cmp_inf_neg[0] ^ cmp_inf_neg[1];
    
    return result;
}

/* Function with inline assembly using condition codes */
static int asm_fp_condition_codes(double a, double b, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that reads FP condition codes */
    
    /* UNORDERED/ORDERED test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result ^= (cc_result << 0);
    
    /* UNEQ/NEQ test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    /* UNLT/LT test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    /* UNLE/LE test */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setbe %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    /* UNGT/GT test (using not-less-or-equal) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 4);
    
    /* UNGE/GE test (using not-less) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setae %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 5);
    
    /* LTGT test (using not-equal and ordered) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setne %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 6);
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "cmova %3, %0"
        : "=r"(cmov_result)
        : "x"(a), "x"(b), "r"(1.0), "0"(0.0)
        : "cc"
    );
    result ^= (int)(cmov_result * 1000);
    
    return result;
}

/* Loop-based comparisons to prevent constant folding */
static int loop_fp_comparisons(double init_val, double nan_val) {
    volatile double accum = 0.0;
    double vals[8];
    
    /* Initialize array with mixed values */
    for (int i = 0; i < 8; i++) {
        vals[i] = init_val + i;
    }
    vals[3] = nan_val;
    vals[6] = __builtin_inf();
    
    int result = 0;
    
    /* Nested loops with FP comparisons */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            volatile double a = vals[i];
            volatile double b = vals[j];
            
            /* Exhaustive comparison matrix */
            if (a == b) result ^= (i * 8 + j + 1);
            if (a != b) result ^= (i * 8 + j + 2);
            if (a < b)  result ^= (i * 8 + j + 4);
            if (a <= b) result ^= (i * 8 + j + 8);
            if (a > b)  result ^= (i * 8 + j + 16);
            if (a >= b) result ^= (i * 8 + j + 32);
            
            /* Conditional expression accumulation */
            accum += (a == b) ? 1.0 : 
                    (a < b)  ? 2.0 :
                    (a > b)  ? 3.0 : 4.0;
        }
    }
    
    result ^= (int)(accum * 1000);
    return result;
}

int main(void) {
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int final_result = 0;
    
    /* Stress scalar FP comparisons */
    final_result ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    final_result ^= stress_fp_comparisons(normal2, normal1, nan_val, inf_val, neg_inf_val);
    final_result ^= stress_fp_comparisons(inf_val, neg_inf_val, nan_val, inf_val, neg_inf_val);
    
    /* Vectorized comparisons */
    final_result ^= vector_fp_comparisons();
    
    /* Inline assembly with condition codes */
    final_result ^= asm_fp_condition_codes(normal1, normal2, nan_val);
    final_result ^= asm_fp_condition_codes(nan_val, normal1, nan_val);
    final_result ^= asm_fp_condition_codes(inf_val, neg_inf_val, nan_val);
    
    /* Loop-based comparisons */
    final_result ^= loop_fp_comparisons(normal1, nan_val);
    final_result ^= loop_fp_comparisons(inf_val, nan_val);
    
    /* Additional unordered comparison patterns */
    {
        volatile double v = 1.0;
        volatile double n = nan_val;
        
        /* Direct unordered pattern tests */
        int r = 0;
        r |= (v == n) ? 0x1 : 0;      /* UNORDERED/UNEQ */
        r |= (v != n) ? 0x2 : 0;      /* ORDERED/LTGT */
        r |= (v < n)  ? 0x4 : 0;      /* UNORDERED/UNLT */
        r |= (v <= n) ? 0x8 : 0;      /* UNORDERED/UNLE */
        r |= (v > n)  ? 0x10 : 0;     /* UNORDERED/UNGT */
        r |= (v >= n) ? 0x20 : 0;     /* UNORDERED/UNGE */
        r |= (n == n) ? 0x40 : 0;     /* UNORDERED/UNEQ */
        r |= (n != n) ? 0x80 : 0;     /* UNORDERED/LTGT */
        
        final_result ^= r;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
