/* test_condition_codes.c */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Prevent aggressive optimization */
static volatile int force_volatile = 0;

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test function 1: Direct unordered comparisons using math.h macros */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* These should generate UNORDERED/ORDERED condition codes */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isordered(a, b) ? 2 : 0;
    
    /* UNEQ: unordered or equal */
    sum += !isgreater(c, d) && !isless(c, d) ? 4 : 0;
    
    /* UNGE: not less than (unordered or greater or equal) */
    sum += !isless(a, b) ? 8 : 0;
    
    /* UNGT: not less or equal (unordered or greater) */
    sum += !islessequal(a, b) ? 16 : 0;
    
    /* UNLE: unordered or less or equal */
    sum += !isgreater(a, b) ? 32 : 0;
    
    /* UNLT: unordered or less than */
    sum += !isgreaterequal(a, b) ? 64 : 0;
    
    /* LTGT: less or greater (ordered and not equal) */
    sum += islessgreater(c, d) ? 128 : 0;
    
    return sum;
}

/* Test function 2: Mixed conditional expressions */
__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, float c, float d, float e, float f) {
    int sum = 0;
    
    /* Complex ternary with different comparison types */
    for (int i = 0; i < 3; i++) {
        float t1 = a + i;
        float t2 = b - i;
        
        /* This should generate multiple condition codes */
        sum += (t1 < t2) ? (c != d) : (e >= f);
        sum += (t1 == t2) ? (c < d) : (e > f);
        sum += (t1 != t2) ? (c <= d) : (e == f);
        
        /* Nested conditionals */
        sum += ((t1 < t2) && (c > d)) || ((t1 >= t2) && (e <= f)) ? 1 : 0;
    }
    
    return sum;
}

/* Test function 3: Vectorized comparisons using GCC vector extensions */
__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Element-wise vector comparisons */
    v4sf cmp_result;
    
    /* Unordered comparison (UNORDERED) */
    cmp_result = __builtin_ia32_cmpunordps(va, vb);
    sum += ((int*)&cmp_result)[0] != 0 ? 1 : 0;
    
    /* Ordered comparison (ORDERED) */
    cmp_result = __builtin_ia32_cmpordps(va, vb);
    sum += ((int*)&cmp_result)[1] != 0 ? 2 : 0;
    
    /* Not equal unordered (UNEQ) */
    cmp_result = __builtin_ia32_cmpneqps(va, vb);
    sum += ((int*)&cmp_result)[2] != 0 ? 4 : 0;
    
    /* Not less than (UNGE) */
    cmp_result = __builtin_ia32_cmpnltps(va, vb);
    sum += ((int*)&cmp_result)[3] != 0 ? 8 : 0;
    
    /* Double vector comparisons */
    v2df cmp_dbl;
    
    /* Not less or equal (UNGT) */
    cmp_dbl = __builtin_ia32_cmpnlepd(vc, vd);
    sum += ((long long*)&cmp_dbl)[0] != 0 ? 16 : 0;
    
    /* Unordered or less or equal (UNLE) */
    cmp_dbl = __builtin_ia32_cmpngepd(vc, vd);  /* nge == unordered or less */
    sum += ((long long*)&cmp_dbl)[1] != 0 ? 32 : 0;
    
    return sum;
}

/* Test function 4: NaN checks and fast-math optimizations */
__attribute__((noinline))
static int test_nan_checks(float a, double b, float c, double d) {
    int sum = 0;
    
    /* Explicit NaN checks - should generate unordered comparisons */
    sum += (a != a) ? 1 : 0;           /* true if a is NaN */
    sum += !(b == b) ? 2 : 0;          /* true if b is NaN */
    sum += isunordered(c, c) ? 4 : 0;  /* true if c is NaN */
    
    /* Mixed with ordered comparisons */
    sum += (a == c) ? 8 : 0;
    sum += (b != d) ? 16 : 0;
    sum += (a < c) ? 32 : 0;
    sum += (b > d) ? 64 : 0;
    
    /* Complex expression that fast-math might transform */
    sum += (a <= c) && (b >= d) ? 128 : 0;
    sum += (a == c) || (b != d) ? 256 : 0;
    
    return sum;
}

/* Test function 5: AVX intrinsics for advanced condition codes */
#ifdef __AVX__
__attribute__((noinline))
static int test_avx_comparisons(__m256 va, __m256 vb, __m256d vc, __m256d vd) {
    int sum = 0;
    
    /* AVX comparison with UNORD_Q predicate */
    __m256 cmp_unord = _mm256_cmp_ps(va, vb, _CMP_UNORD_Q);
    sum += _mm256_movemask_ps(cmp_unord) & 1;
    
    /* AVX comparison with NEQ_UQ predicate */
    __m256 cmp_neq_uq = _mm256_cmp_ps(va, vb, _CMP_NEQ_UQ);
    sum += (_mm256_movemask_ps(cmp_neq_uq) & 2) << 1;
    
    /* AVX comparison with NLT_US predicate (UNGE) */
    __m256 cmp_nlt_us = _mm256_cmp_ps(va, vb, _CMP_NLT_US);
    sum += (_mm256_movemask_ps(cmp_nlt_us) & 4) << 2;
    
    /* AVX double comparison with NLE_US predicate (UNGT) */
    __m256d cmp_nle_us = _mm256_cmp_pd(vc, vd, _CMP_NLE_US);
    sum += (_mm256_movemask_pd(cmp_nle_us) & 1) << 3;
    
    /* AVX comparison with UNORD_S predicate */
    __m256 cmp_unord_s = _mm256_cmp_ps(va, vb, _CMP_UNORD_S);
    sum += (_mm256_movemask_ps(cmp_unord_s) & 8) << 4;
    
    return sum;
}
#endif

int main(int argc, char *argv[]) {
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 0.0f, -0.0f,
        __builtin_nanf(""), 3.0f,
        __builtin_inff(), -__builtin_inff(),
        4.0f, 5.0f
    };
    
    double double_data[] = {
        1.0, 2.0, 0.0, -0.0,
        __builtin_nan(""), 3.0,
        __builtin_inf(), -__builtin_inf(),
        4.0, 5.0
    };
    
    /* Vector data */
    v4sf vec_float1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_float2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vec_double1 = {1.0, 2.0};
    v2df vec_double2 = {2.0, 1.0};
    
    /* Use argc to prevent excessive loop unrolling */
    int iterations = (argc > 1) ? (argc % 5) + 1 : 3;
    
    for (int i = 0; i < iterations; i++) {
        int idx = i % 10;
        int idx2 = (i + 1) % 10;
        int idx3 = (i + 2) % 10;
        int idx4 = (i + 3) % 10;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx2],
            double_data[idx3], double_data[idx4]
        );
        
        /* Test 2: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[idx], float_data[idx2],
            float_data[idx3], float_data[idx4],
            float_data[(idx + 4) % 10], float_data[(idx + 5) % 10]
        );
        
        /* Test 3: Vector comparisons */
        total_sum += test_vector_comparisons(
            vec_float1, vec_float2,
            vec_double1, vec_double2
        );
        
        /* Test 4: NaN checks */
        total_sum += test_nan_checks(
            float_data[idx], double_data[idx2],
            float_data[idx3], double_data[idx4]
        );
        
        #ifdef __AVX__
        /* Test 5: AVX comparisons */
        __m256 avx_float1 = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
        __m256 avx_float2 = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
        __m256d avx_double1 = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
        __m256d avx_double2 = _mm256_set_pd(4.0, 3.0, 2.0, 1.0);
        
        total_sum += test_avx_comparisons(avx_float1, avx_float2, avx_double1, avx_double2);
        #endif
        
        /* Modify data slightly each iteration */
        float_data[idx] += 0.1f;
        double_data[idx2] -= 0.1;
    }
    
    /* Add volatile dependency to prevent dead code elimination */
    total_sum += force_volatile;
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
