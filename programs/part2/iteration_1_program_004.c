/* Compile with: gcc -std=c99 -O2 -march=x86-64 -mtune=generic -ffp-contract=off -fno-trapping-math -o fp_cond_test fp_cond_test.c */
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
    
    /* Matrix of comparisons to trigger different condition codes */
    
    /* Normal vs Normal comparisons */
    if (v1 == v2) result ^= 1;
    if (v1 != v2) result ^= 2;
    if (v1 < v2)  result ^= 4;
    if (v1 <= v2) result ^= 8;
    if (v1 > v2)  result ^= 16;
    if (v1 >= v2) result ^= 32;
    
    /* Normal vs NaN comparisons (unordered cases) */
    if (v1 == v_nan) result ^= 64;      /* Should be false, may trigger UNORDERED */
    if (v1 != v_nan) result ^= 128;     /* Should be true, may trigger ORDERED or UNORDERED */
    if (v1 < v_nan)  result ^= 256;     /* Unordered */
    if (v1 <= v_nan) result ^= 512;     /* Unordered */
    if (v1 > v_nan)  result ^= 1024;    /* Unordered */
    if (v1 >= v_nan) result ^= 2048;    /* Unordered */
    
    /* NaN vs Normal comparisons */
    if (v_nan == v2) result ^= 4096;
    if (v_nan != v2) result ^= 8192;
    if (v_nan < v2)  result ^= 16384;
    if (v_nan <= v2) result ^= 32768;
    if (v_nan > v2)  result ^= 65536;
    if (v_nan >= v2) result ^= 131072;
    
    /* NaN vs NaN comparisons */
    if (v_nan == v_nan) result ^= 262144;  /* Should be false (NaN != NaN) */
    if (v_nan != v_nan) result ^= 524288;  /* Should be true */
    
    /* Infinity comparisons */
    if (v_inf == v_inf) result ^= 1048576;
    if (v_neg_inf < v_inf) result ^= 2097152;
    if (v_inf > v1) result ^= 4194304;
    
    /* Complex conditional expressions using ?: operator */
    double cond_result = (v1 < v_nan) ? 1.0 : 2.0;  /* Unordered comparison */
    result ^= (int)cond_result;
    
    cond_result = (v_nan == v_nan) ? 3.0 : 4.0;     /* NaN equality */
    result ^= (int)cond_result;
    
    /* Goto-based complex control flow to prevent optimization */
    if (v1 != v1) {  /* Check if v1 is NaN */
        goto nan_path;
    } else {
        goto normal_path;
    }
    
nan_path:
    result += 1000;
    if (v2 == v2) {
        goto end_comparisons;
    }
    
normal_path:
    result += 2000;
    if (v1 < v2) {
        result += 3000;
    } else if (v1 > v2) {
        result += 4000;
    } else {
        result += 5000;
    }
    
end_comparisons:
    
    /* Vectorized comparisons using GCC vector extensions */
    v2df vec_a = {a, b};
    v2df vec_b = {b, a};
    v2df vec_nan = {nan_val, nan_val};
    
    /* Compare operations that generate condition codes */
    v2di cmp_eq = (v2di)(vec_a == vec_b);
    v2di cmp_neq = (v2di)(vec_a != vec_b);
    v2di cmp_lt = (v2di)(vec_a < vec_b);
    v2di cmp_le = (v2di)(vec_a <= vec_b);
    v2di cmp_gt = (v2di)(vec_a > vec_b);
    v2di cmp_ge = (v2di)(vec_a >= vec_b);
    
    /* Unordered comparisons with NaN */
    v2di cmp_unord = (v2di)(vec_a == vec_nan);
    v2di cmp_ord = (v2di)(vec_a != vec_nan);
    
    /* Extract results from vector comparisons */
    long long *eq_ptr = (long long*)&cmp_eq;
    long long *neq_ptr = (long long*)&cmp_neq;
    long long *unord_ptr = (long long*)&cmp_unord;
    
    result ^= (int)(eq_ptr[0] & 0xFF);
    result ^= (int)(neq_ptr[0] & 0xFF);
    result ^= (int)(unord_ptr[0] & 0xFF);
    
    /* Array-based vector comparisons in a loop */
    double arr1[4] = {a, b, nan_val, inf_val};
    double arr2[4] = {b, nan_val, inf_val, neg_inf_val};
    int mask[4] = {0};
    
    for (int i = 0; i < 4; i++) {
        volatile double x = arr1[i];
        volatile double y = arr2[i];
        
        /* Various comparison types */
        mask[i] |= (x == y) ? 1 : 0;
        mask[i] |= (x != y) ? 2 : 0;
        mask[i] |= (x < y)  ? 4 : 0;
        mask[i] |= (x <= y) ? 8 : 0;
        mask[i] |= (x > y)  ? 16 : 0;
        mask[i] |= (x >= y) ? 32 : 0;
        
        /* Unordered checks */
        mask[i] |= (x != x) ? 64 : 0;  /* isNaN(x) */
        mask[i] |= (y != y) ? 128 : 0; /* isNaN(y) */
        
        result ^= mask[i];
    }
    
    return result;
}

/* Function with inline assembly to explicitly use condition codes */
static int inline_asm_fp_conds(double a, double b, double nan_val) {
    int result = 0;
    uint8_t cc_result;
    
    /* Inline assembly that uses FP comparison condition codes */
    
    /* UNORDERED/ORDERED test (parity flag) */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setp %0"
        : "=r"(cc_result)
        : "x"(a), "x"(nan_val)
        : "cc"
    );
    result ^= cc_result;
    
    /* UNEQ/UNLT/UNLE/etc. tests using different condition codes */
    asm volatile (
        "ucomisd %2, %1\n\t"
        "seta %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 1);
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "setb %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 2);
    
    asm volatile (
        "ucomisd %2, %1\n\t"
        "sete %0"
        : "=r"(cc_result)
        : "x"(a), "x"(b)
        : "cc"
    );
    result ^= (cc_result << 3);
    
    /* Conditional move based on FP comparison */
    double cmov_result;
    asm volatile (
        "ucomisd %2, %1\n\t"
        "fcmovbe %3, %0"
        : "=t"(cmov_result)
        : "u"(a), "u"(b), "t"(3.14159)
        : "cc"
    );
    result ^= (int)cmov_result;
    
    return result;
}

int main(void) {
    /* Initialize FP values */
    double normal1 = 3.14159;
    double normal2 = 2.71828;
    double nan_val = __builtin_nan("");
    double inf_val = __builtin_inf();
    double neg_inf_val = -__builtin_inf();
    
    int checksum = 0;
    
    /* Test various combinations of values */
    checksum ^= stress_fp_comparisons(normal1, normal2, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(normal1, nan_val, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(nan_val, normal2, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(nan_val, nan_val, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(inf_val, normal1, nan_val, inf_val, neg_inf_val);
    checksum ^= stress_fp_comparisons(neg_inf_val, inf_val, nan_val, inf_val, neg_inf_val);
    
    /* Test with inline assembly */
    checksum ^= inline_asm_fp_conds(normal1, normal2, nan_val);
    checksum ^= inline_asm_fp_conds(normal1, nan_val, nan_val);
    checksum ^= inline_asm_fp_conds(nan_val, normal2, nan_val);
    
    /* Additional tests with volatile variables in loops */
    volatile double v_vals[6] = {normal1, normal2, nan_val, inf_val, neg_inf_val, 0.0};
    
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            /* Exhaustive comparison matrix */
            if (v_vals[i] == v_vals[j]) checksum += 1;
            if (v_vals[i] != v_vals[j]) checksum += 2;
            if (v_vals[i] < v_vals[j])  checksum += 3;
            if (v_vals[i] <= v_vals[j]) checksum += 4;
            if (v_vals[i] > v_vals[j])  checksum += 5;
            if (v_vals[i] >= v_vals[j]) checksum += 6;
            
            /* Complex conditional */
            double tmp = (v_vals[i] < v_vals[j]) ? v_vals[i] : v_vals[j];
            checksum ^= (int)tmp;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    
    return checksum & 0xFF;
}
