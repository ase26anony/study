#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Vector types for SSE/AVX comparisons */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Test functions with different comparison patterns */
__attribute__((noinline))
static int test_unordered_comparisons(float a, float b, double c, double d) {
    int sum = 0;
    
    /* Direct unordered checks using standard macros */
    sum += isunordered(a, b) ? 1 : 0;
    sum += isunordered(c, d) ? 2 : 0;
    
    /* NaN checks using self-comparison */
    sum += (a != a) ? 4 : 0;  /* true if a is NaN */
    sum += !(c == c) ? 8 : 0; /* true if c is NaN */
    
    /* Ordered checks */
    sum += isordered(a, b) ? 16 : 0;
    sum += isordered(c, d) ? 32 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_ltgt_comparisons(float a, float b, float c, float d) {
    int sum = 0;
    
    /* LTGT (less or greater, but not equal) */
    sum += islessgreater(a, b) ? 1 : 0;
    sum += islessgreater(c, d) ? 2 : 0;
    
    /* UNEQ (unordered or equal) - often generated with fast-math */
    /* Simulate by combining comparisons */
    if (!(a < b) && !(a > b) && !isunordered(a, b)) {
        sum += 4;  /* a == b (ordered equal) */
    }
    
    return sum;
}

__attribute__((noinline))
static int test_mixed_conditionals(float a, float b, double c, double d, float e, float f) {
    int sum = 0;
    
    /* Complex conditional expression mixing different comparisons */
    sum += ((a < b) ? (c != d) : (e >= f)) ? 1 : 0;
    
    /* Nested ternary with unordered checks */
    sum += (isunordered(a, b) ? (c > d) : (e < f)) ? 2 : 0;
    
    /* Chain of comparisons */
    if ((a <= b) && (c == d) && !isunordered(e, f)) {
        sum += 4;
    }
    
    /* Mixed ordered/unordered in single expression */
    sum += ((a == b) || isunordered(c, d)) ? 8 : 0;
    
    return sum;
}

__attribute__((noinline))
static int test_vector_comparisons(v4sf va, v4sf vb, v2df vc, v2df vd) {
    int sum = 0;
    
    /* Vector comparisons using GCC extensions */
    v4sf vcmp = va < vb;
    v2df vcmp2 = vc > vd;
    
    /* Extract results to prevent optimization */
    float fcmp[4];
    double dcmp[2];
    memcpy(fcmp, &vcmp, sizeof(fcmp));
    memcpy(dcmp, &vcmp2, sizeof(dcmp));
    
    for (int i = 0; i < 4; i++) {
        sum += (fcmp[i] != 0.0f) ? (1 << i) : 0;
    }
    for (int i = 0; i < 2; i++) {
        sum += (dcmp[i] != 0.0) ? (16 << i) : 0;
    }
    
    return sum;
}

#ifdef __AVX__
__attribute__((noinline))
static int test_avx_intrinsics(__m256 va, __m256 vb) {
    int sum = 0;
    
    /* AVX comparisons that generate unordered condition codes */
    __m256 cmp_unord = _mm256_cmp_ps(va, vb, _CMP_UNORD_Q);
    __m256 cmp_neq_uq = _mm256_cmp_ps(va, vb, _CMP_NEQ_UQ);
    __m256 cmp_nlt_us = _mm256_cmp_ps(va, vb, _CMP_NLT_US);
    __m256 cmp_nle_us = _mm256_cmp_ps(va, vb, _CMP_NLE_US);
    
    /* Extract results */
    float results[4][8];
    _mm256_storeu_ps(results[0], cmp_unord);
    _mm256_storeu_ps(results[1], cmp_neq_uq);
    _mm256_storeu_ps(results[2], cmp_nlt_us);
    _mm256_storeu_ps(results[3], cmp_nle_us);
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            sum += (results[i][j] != 0.0f) ? 1 : 0;
        }
    }
    
    return sum;
}
#endif

__attribute__((noinline))
static int test_fast_math_patterns(float a, float b, float c, float d) {
    int sum = 0;
    
    /* These patterns often generate UNEQ/LTGT with -ffast-math */
    
    /* Pattern 1: (a == b) that might be optimized to unordered equal */
    sum += (a == b) ? 1 : 0;
    
    /* Pattern 2: (a != b) that might become LTGT with fast-math */
    sum += (a != b) ? 2 : 0;
    
    /* Pattern 3: Combined comparisons */
    if ((a >= c) && (b <= d)) {
        sum += 4;
    }
    
    /* Pattern 4: Chained comparisons */
    sum += (a < b && b < c && c < d) ? 8 : 0;
    
    return sum;
}

int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    int total_sum = 0;
    
    /* Patterned data including NaN values */
    float float_data[] = {
        1.0f, 2.0f, 3.0f, 4.0f,
        0.0f, -0.0f, __builtin_nanf(""), INFINITY,
        5.0f, 6.0f, 7.0f, 8.0f
    };
    
    double double_data[] = {
        1.0, 2.0, __builtin_nan(""), 4.0,
        0.0, -0.0, INFINITY, -INFINITY
    };
    
    /* Vector data */
    v4sf v1 = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    v4sf v2 = {4.0f, 3.0f, 2.0f, 1.0f};
    v2df vd1 = {1.0, __builtin_nan("")};
    v2df vd2 = {__builtin_nan(""), 1.0};
    
#ifdef __AVX__
    __m256 avx1 = _mm256_setr_ps(1.0f, 2.0f, 3.0f, 4.0f,
                                 __builtin_nanf(""), 6.0f, 7.0f, 8.0f);
    __m256 avx2 = _mm256_setr_ps(8.0f, 7.0f, 6.0f, 5.0f,
                                 4.0f, 3.0f, 2.0f, 1.0f);
#endif
    
    for (volatile int i = 0; i < iterations; i++) {
        int idx = i % 8;
        
        /* Test 1: Unordered comparisons */
        total_sum += test_unordered_comparisons(
            float_data[idx], float_data[idx + 1],
            double_data[idx], double_data[idx + 1]
        );
        
        /* Test 2: LTGT comparisons */
        total_sum += test_ltgt_comparisons(
            float_data[idx], float_data[idx + 2],
            float_data[idx + 1], float_data[idx + 3]
        );
        
        /* Test 3: Mixed conditionals */
        total_sum += test_mixed_conditionals(
            float_data[idx], float_data[idx + 1],
            double_data[idx], double_data[idx + 1],
            float_data[idx + 2], float_data[idx + 3]
        );
        
        /* Test 4: Vector comparisons */
        total_sum += test_vector_comparisons(v1, v2, vd1, vd2);
        
#ifdef __AVX__
        /* Test 5: AVX intrinsics */
        total_sum += test_avx_intrinsics(avx1, avx2);
#endif
        
        /* Test 6: Fast-math patterns */
        total_sum += test_fast_math_patterns(
            float_data[idx], float_data[idx + 1],
            float_data[idx + 2], float_data[idx + 3]
        );
    }
    
    printf("Total checksum: %d\n", total_sum);
    return total_sum != 0 ? 0 : 1;
}
