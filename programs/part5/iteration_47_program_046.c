/* 
 * Program to trigger x86 condition code mnemonics in i386.cc
 * Compile with: gcc -O3 -ffast-math -mavx2 -fdump-rtl-final -o test_conds test_conds.c
 */

#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Vector types for AVX/SSE */
typedef float v4sf __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));
typedef double v2df __attribute__((vector_size(16)));
typedef double v4df __attribute__((vector_size(32)));

/* Prevent inlining to force RTL generation */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* UNORDERED: isunordered() macro */
    sum += isunordered(a, b) ? 1 : 0;
    
    /* ORDERED: !isunordered() */
    sum += !isunordered(c, d) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += (isunordered(a, b) || (a == b)) ? 4 : 0;
    
    /* UNGE: unordered or greater-or-equal */
    sum += (isunordered(a, b) || (a >= b)) ? 8 : 0;
    
    /* UNGT: unordered or greater-than */
    sum += (isunordered(a, b) || (a > b)) ? 16 : 0;
    
    /* UNLE: unordered or less-or-equal */
    sum += (isunordered(a, b) || (a <= b)) ? 32 : 0;
    
    /* UNLT: unordered or less-than */
    sum += (isunordered(a, b) || (a < b)) ? 64 : 0;
    
    /* LTGT: islessgreater() macro */
    sum += islessgreater(a, b) ? 128 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional with mixed comparisons */
    /* This should generate multiple condition codes */
    sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
    
    /* Nested ternary with different operators */
    sum += (a == b) ? ((c > d) ? 2 : 0) : ((e <= f) ? 4 : 0);
    
    /* Chain of comparisons */
    sum += ((a != b) && (c < d) && (e > f)) ? 8 : 0;
    
    /* Mixed ordered/unordered in conditional */
    sum += (isunordered(a, b) ? (c == d) : (e != f)) ? 16 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons generate condition codes */
    v4sf vcmp_eq = (va == vb);
    v4sf vcmp_neq = (va != vb);
    v4sf vcmp_lt = (va < vb);
    v4sf vcmp_gt = (va > vb);
    v4sf vcmp_le = (va <= vb);
    v4sf vcmp_ge = (va >= vb);
    
    /* Extract results to prevent elimination */
    float results[4];
    memcpy(results, &vcmp_eq, sizeof(v4sf));
    sum += (results[0] != 0.0f) ? 1 : 0;
    sum += (results[1] != 0.0f) ? 2 : 0;
    sum += (results[2] != 0.0f) ? 4 : 0;
    sum += (results[3] != 0.0f) ? 8 : 0;
    
    /* Double vector comparisons */
    v2df vcmp_d_eq = (vc == vd);
    v2df vcmp_d_neq = (vc != vd);
    
    double dresults[2];
    memcpy(dresults, &vcmp_d_eq, sizeof(v2df));
    sum += (dresults[0] != 0.0) ? 16 : 0;
    sum += (dresults[1] != 0.0) ? 32 : 0;
    
    return sum;
}

#ifdef __AVX__
__attribute__((noinline))
static int test_avx_intrinsics(__m256 av, __m256 bv) {
    int sum = 0;
    
    /* AVX intrinsics with various comparison predicates */
    __m256 cmp_unord = _mm256_cmp_ps(av, bv, _CMP_UNORD_Q);    /* UNORDERED */
    __m256 cmp_neq_uq = _mm256_cmp_ps(av, bv, _CMP_NEQ_UQ);    /* UNEQ */
    __m256 cmp_nlt_uq = _mm256_cmp_ps(av, bv, _CMP_NLT_UQ);    /* UNGE */
    __m256 cmp_nle_uq = _mm256_cmp_ps(av, bv, _CMP_NLE_UQ);    /* UNGT */
    __m256 cmp_ord_q = _mm256_cmp_ps(av, bv, _CMP_ORD_Q);      /* ORDERED */
    
    /* Extract mask bits */
    int mask_unord = _mm256_movemask_ps(cmp_unord);
    int mask_neq_uq = _mm256_movemask_ps(cmp_neq_uq);
    int mask_nlt_uq = _mm256_movemask_ps(cmp_nlt_uq);
    int mask_nle_uq = _mm256_movemask_ps(cmp_nle_uq);
    int mask_ord_q = _mm256_movemask_ps(cmp_ord_q);
    
    sum += mask_unord;
    sum += mask_neq_uq;
    sum += mask_nlt_uq;
    sum += mask_nle_uq;
    sum += mask_ord_q;
    
    return sum;
}
#endif

__attribute__((noinline))
static int test_nan_checks(float a, double b) {
    int sum = 0;
    
    /* Explicit NaN checks using IEEE 754 properties */
    sum += (a != a) ? 1 : 0;          /* true if a is NaN */
    sum += (!(b == b)) ? 2 : 0;       /* true if b is NaN */
    
    /* Mixed NaN and normal comparisons */
    sum += ((a != a) || (a > 0.0f)) ? 4 : 0;    /* UNGT-like */
    sum += ((b != b) || (b <= 1.0)) ? 8 : 0;    /* UNLE-like */
    
    return sum;
}

__attribute__((noinline))
static int test_fast_math_optimizations(float a, float b, float c, float d) {
    int sum = 0;
    
    /* With -ffast-math, these may generate UNEQ/LTGT codes */
    
    /* Equality chain that fast-math might optimize */
    sum += ((a == b) == (c == d)) ? 1 : 0;
    
    /* Inequality chain */
    sum += ((a != b) != (c != d)) ? 2 : 0;
    
    /* Mixed comparisons that fast-math can reorder */
    sum += ((a < b) && (c > d)) ? 4 : 0;
    sum += ((a <= b) || (c >= d)) ? 8 : 0;
    
    /* Complex expression that might use LTGT */
    sum += ((a < b) != (c > d)) ? 16 : 0;
    sum += ((a == b) == (c < d)) ? 32 : 0;
    
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent loop unrolling */
    volatile int iterations = (argc > 1) ? 100 : 50;
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), INFINITY,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 3.0, 4.0,
        0.0, -0.0, __builtin_nan(""), __builtin_inf(),
        5.0, 6.0, 7.0, 8.0
    };
    
    /* Initialize vectors */
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vd1 = {1.0, 2.0};
    v2df vd2 = {2.0, 1.0};
    
#ifdef __AVX__
    __m256 avx1 = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    __m256 avx2 = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
#endif
    
    for (int i = 0; i < iterations; i++) {
        /* Cycle through different data patterns */
        int idx = i % 8;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], 
            float_data[idx + 1],
            double_data[idx],
            double_data[idx + 1]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[idx],
            float_data[idx + 1],
            float_data[idx + 2],
            float_data[idx + 3],
            float_data[idx + 4],
            float_data[idx + 5]
        );
        
        /* Test 3: Vector comparisons */
        total_sum += test_vector_comparisons(v1, v2, vd1, vd2);
        
        /* Test 4: AVX intrinsics if available */
#ifdef __AVX__
        total_sum += test_avx_intrinsics(avx1, avx2);
#endif
        
        /* Test 5: NaN checks */
        total_sum += test_nan_checks(
            float_data[idx + 2],
            double_data[idx + 2]
        );
        
        /* Test 6: Fast-math optimizations */
        total_sum += test_fast_math_optimizations(
            float_data[idx],
            float_data[idx + 1],
            float_data[idx + 2],
            float_data[idx + 3]
        );
        
        /* Modify vectors slightly each iteration */
        v1[0] += 0.1f;
        v2[0] -= 0.1f;
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
