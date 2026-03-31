#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#ifdef __SSE__
#include <xmmintrin.h>
#endif

#ifdef __SSE2__
#include <emmintrin.h>
#endif

#ifdef __AVX__
#include <avxintrin.h>
#endif

/* Force assembly generation by preventing inlining */
__attribute__((noinline)) 
float test_sse_comparisons(float a, float b) {
    float result = 0.0f;
    
#ifdef __SSE__
    __m128 vec_a = _mm_set1_ps(a);
    __m128 vec_b = _mm_set1_ps(b);
    __m128 cmp_result;
    int mask;
    
    /* Test all condition codes from uncovered block */
    
    // UNORDERED (_CMP_UNORD_Q = 3)
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 1.0f;
    
    // ORDERED (_CMP_ORD_Q = 7)
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 2.0f;
    
    // UNEQ (_CMP_UNEQ_UQ = 12)
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 4.0f;
    
    // UNGE (_CMP_NGE_UQ = 13) - prints "nlt"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 8.0f;
    
    // UNGT (_CMP_NGT_UQ = 14) - prints "nle"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 16.0f;
    
    // UNLE (_CMP_ULE_UQ = 18) - prints "ule"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 32.0f;
    
    // UNLT (_CMP_ULT_UQ = 17) - prints "ult"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 64.0f;
    
    // LTGT (_CMP_NEQ_UQ = 4) - prints "une"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 128.0f;
    
    /* Also test scalar comparisons */
    __m128 scalar_cmp = _mm_cmp_ss(vec_a, vec_b, _CMP_UNORD_Q);
    result += _mm_cvtss_f32(scalar_cmp);
#endif
    
    return result;
}

__attribute__((noinline))
double test_sse2_double_comparisons(double a, double b) {
    double result = 0.0;
    
#ifdef __SSE2__
    __m128d vec_a = _mm_set1_pd(a);
    __m128d vec_b = _mm_set1_pd(b);
    __m128d cmp_result;
    int mask;
    
    /* Test double precision comparisons */
    
    // UNORDERED
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 1.0;
    
    // ORDERED
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 2.0;
    
    // UNEQ
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 4.0;
    
    // UNGE - prints "nlt"
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 8.0;
    
    // Scalar double comparison
    __m128d scalar_cmp = _mm_cmp_sd(vec_a, vec_b, _CMP_UNEQ_UQ);
    result += _mm_cvtsd_f64(scalar_cmp);
#endif
    
    return result;
}

#ifdef __AVX__
__attribute__((noinline))
float test_avx_comparisons(float a, float b, float c, float d) {
    __m256 vec1 = _mm256_set_ps(d, c, b, a, d, c, b, a);
    __m256 vec2 = _mm256_set_ps(a, b, c, d, a, b, c, d);
    __m256 cmp_result;
    float result = 0.0f;
    
    /* Test AVX comparisons with all condition codes */
    
    // UNORDERED
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    result += _mm256_cvtss_f32(_mm256_castps256_ps128(cmp_result));
    
    // ORDERED
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    result += _mm256_cvtss_f32(_mm256_castps256_ps128(cmp_result));
    
    // UNEQ
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    result += _mm256_cvtss_f32(_mm256_castps256_ps128(cmp_result));
    
    // UNGE - prints "nlt"
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    result += _mm256_cvtss_f32(_mm256_castps256_ps128(cmp_result));
    
    // UNGT - prints "nle"
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    result += _mm256_cvtss_f32(_mm256_castps256_ps128(cmp_result));
    
    // UNLE - prints "ule"
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    result += _mm256_cvtss_f32(_mm256_castps256_ps128(cmp_result));
    
    // UNLT - prints "ult"
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    result += _mm256_cvtss_f32(_mm256_castps256_ps128(cmp_result));
    
    // LTGT - prints "une"
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    result += _mm256_cvtss_f32(_mm256_castps256_ps128(cmp_result));
    
    return result;
}

__attribute__((noinline))
double test_avx_double_comparisons(double a, double b, double c, double d) {
    __m256d vec1 = _mm256_set_pd(d, c, b, a);
    __m256d vec2 = _mm256_set_pd(a, b, c, d);
    __m256d cmp_result;
    double result = 0.0;
    
    // Test AVX double precision comparisons
    cmp_result = _mm256_cmp_pd(vec1, vec2, _CMP_UNORD_Q);
    result += _mm256_cvtsd_f64(_mm256_castpd256_pd128(cmp_result));
    
    cmp_result = _mm256_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    result += _mm256_cvtsd_f64(_mm256_castpd256_pd128(cmp_result));
    
    cmp_result = _mm256_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    result += _mm256_cvtsd_f64(_mm256_castpd256_pd128(cmp_result));
    
    return result;
}
#endif

/* Complex expression mixing comparisons with arithmetic */
__attribute__((noinline))
float complex_vector_expression(float a, float b, float c, float d) {
#ifdef __SSE__
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v3 = _mm_set_ps(b, a, d, c);
    
    /* Blend based on comparison results */
    __m128 cmp1 = _mm_cmp_ps(v1, v2, _CMP_UNORD_Q);
    __m128 blended1 = _mm_blendv_ps(v1, v2, cmp1);
    
    __m128 cmp2 = _mm_cmp_ps(v2, v3, _CMP_ORD_Q);
    __m128 blended2 = _mm_blendv_ps(v2, v3, cmp2);
    
    __m128 cmp3 = _mm_cmp_ps(blended1, blended2, _CMP_UNEQ_UQ);
    __m128 result_vec = _mm_add_ps(_mm_mul_ps(blended1, blended2), 
                                   _mm_and_ps(cmp3, _mm_set1_ps(1.0f)));
    
    /* Extract result */
    float result = 0.0f;
    float temp[4];
    _mm_storeu_ps(temp, result_vec);
    for (int i = 0; i < 4; i++) {
        result += temp[i];
    }
    
    return result;
#else
    return a + b + c + d;
#endif
}

/* Test with NaN values */
__attribute__((noinline))
float test_nan_comparisons() {
    float result = 0.0f;
    
#ifdef __SSE__
    /* Create vectors with NaN values */
    __m128 nan_vec = _mm_set1_ps(NAN);
    __m128 normal_vec = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 inf_vec = _mm_set1_ps(INFINITY);
    
    /* UNORDERED comparisons with NaN */
    __m128 cmp1 = _mm_cmp_ps(nan_vec, normal_vec, _CMP_UNORD_Q);
    int mask1 = _mm_movemask_ps(cmp1);
    if (mask1 == 0xF) result += 1.0f;  // All comparisons unordered with NaN
    
    /* ORDERED comparisons - NaN should be false */
    __m128 cmp2 = _mm_cmp_ps(normal_vec, normal_vec, _CMP_ORD_Q);
    int mask2 = _mm_movemask_ps(cmp2);
    if (mask2 == 0xF) result += 2.0f;  // All comparisons ordered
    
    /* Mixed NaN and normal values */
    __m128 mixed1 = _mm_set_ps(NAN, 1.0f, NAN, 2.0f);
    __m128 mixed2 = _mm_set_ps(1.0f, NAN, 2.0f, NAN);
    
    __m128 cmp3 = _mm_cmp_ps(mixed1, mixed2, _CMP_UNEQ_UQ);
    int mask3 = _mm_movemask_ps(cmp3);
    result += mask3 * 0.1f;
    
    /* Infinity comparisons */
    __m128 cmp4 = _mm_cmp_ps(inf_vec, normal_vec, _CMP_NGE_UQ);
    int mask4 = _mm_movemask_ps(cmp4);
    result += mask4 * 0.01f;
#endif
    
    return result;
}

int main() {
    printf("Testing x86 vector comparison condition codes...\n");
    
    float final_result = 0.0f;
    double double_result = 0.0;
    
    /* Test with various values including special cases */
    float test_values[] = {1.0f, -1.0f, 0.0f, -0.0f, 100.0f, -100.0f};
    double test_doubles[] = {1.0, -1.0, 0.0, -0.0, 1e10, -1e10};
    
    /* Test SSE comparisons */
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            final_result += test_sse_comparisons(test_values[i], test_values[j]);
            double_result += test_sse2_double_comparisons(test_doubles[i], test_doubles[j]);
        }
    }
    
    /* Test complex expressions */
    final_result += complex_vector_expression(1.0f, 2.0f, 3.0f, 4.0f);
    final_result += complex_vector_expression(-1.0f, 0.0f, 1.0f, NAN);
    
    /* Test NaN comparisons */
    final_result += test_nan_comparisons();
    
#ifdef __AVX__
    /* Test AVX if available */
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported, testing AVX comparisons...\n");
        
        final_result += test_avx_comparisons(1.0f, 2.0f, 3.0f, 4.0f);
        final_result += test_avx_comparisons(NAN, INFINITY, -INFINITY, 0.0f);
        
        double_result += test_avx_double_comparisons(1.0, 2.0, 3.0, 4.0);
        double_result += test_avx_double_comparisons(NAN, INFINITY, -INFINITY, 0.0);
    } else {
        printf("AVX not supported, skipping AVX tests...\n");
    }
#endif
    
    /* Use inline assembly to force condition code printing */
    __asm__ __volatile__ (
        "# Force assembly generation with vector comparisons\n"
        :
        :
        : "memory"
    );
    
    printf("Final float result: %f\n", final_result);
    printf("Final double result: %lf\n", double_result);
    
    /* Prevent dead code elimination */
    volatile float dummy = final_result;
    (void)dummy;
    
    return 0;
}
