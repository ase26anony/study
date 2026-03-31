#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

// SSE comparison tests
FORCE_INLINE float test_sse_comparisons(void) {
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, INFINITY);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, INFINITY);
    __m128 c = _mm_setr_ps(0.0f, NAN, NAN, 1.0f);
    
    __m128 results[8];
    int masks[8];
    
    // Test all condition codes from uncovered block
    results[0] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);    // UNORDERED
    results[1] = _mm_cmp_ps(b, c, _CMP_ORD_Q);      // ORDERED
    results[2] = _mm_cmp_ps(a, b, _CMP_UNEQ_UQ);    // UNEQ
    results[3] = _mm_cmp_ps(a, b, _CMP_NGE_UQ);     // UNGE (nlt)
    results[4] = _mm_cmp_ps(a, b, _CMP_NGT_UQ);     // UNGT (nle)
    results[5] = _mm_cmp_ps(a, b, _CMP_ULE_UQ);     // UNLE
    results[6] = _mm_cmp_ps(a, b, _CMP_ULT_UQ);     // UNLT
    results[7] = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);     // LTGT (une)
    
    // Extract masks to prevent optimization
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm_movemask_ps(results[i]);
    }
    
    // Use results in arithmetic to create dependency chain
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Blend based on comparison results
        __m128 blended = _mm_blendv_ps(a, b, results[i]);
        accum = _mm_add_ps(accum, blended);
    }
    
    // Force assembly output with inline asm
    float sum = 0;
    __asm__ __volatile__ (
        "movaps %1, %%xmm0\n\t"
        "movaps %2, %%xmm1\n\t"
        "cmpps $3, %%xmm1, %%xmm0\n\t"  // UNORDERED
        "movaps %%xmm0, %0\n\t"
        : "=m" (results[0])
        : "m" (a), "m" (c)
        : "xmm0", "xmm1"
    );
    
    // Horizontal sum
    __m128 shuf = _mm_movehdup_ps(accum);
    __m128 sums = _mm_add_ps(accum, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    _mm_store_ss(&sum, sums);
    
    return sum + masks[0] + masks[7];
}

// Double precision SSE tests
FORCE_INLINE double test_sse_double_comparisons(void) {
    __m128d a = _mm_setr_pd(1.0, NAN);
    __m128d b = _mm_setr_pd(2.0, 2.0);
    __m128d c = _mm_setr_pd(NAN, INFINITY);
    
    __m128d results[8];
    
    // Test with double precision
    results[0] = _mm_cmp_pd(a, b, _CMP_UNORD_Q);    // UNORDERED
    results[1] = _mm_cmp_pd(b, c, _CMP_ORD_Q);      // ORDERED  
    results[2] = _mm_cmp_pd(a, b, _CMP_UNEQ_UQ);    // UNEQ
    results[3] = _mm_cmp_pd(a, b, _CMP_NGE_UQ);     // UNGE (nlt)
    results[4] = _mm_cmp_pd(a, b, _CMP_NGT_UQ);     // UNGT (nle)
    results[5] = _mm_cmp_pd(a, b, _CMP_ULE_UQ);     // UNLE
    results[6] = _mm_cmp_pd(a, b, _CMP_ULT_UQ);     // UNLT
    results[7] = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);     // LTGT (une)
    
    // Complex expression mixing comparisons
    __m128d mask1 = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
    __m128d mask2 = _mm_cmp_pd(b, c, _CMP_ORD_Q);
    __m128d blended1 = _mm_blendv_pd(a, b, mask1);
    __m128d blended2 = _mm_blendv_pd(c, a, mask2);
    
    __m128d sum = _mm_add_pd(blended1, blended2);
    sum = _mm_mul_pd(sum, _mm_sub_pd(b, a));
    
    // Extract to scalar
    double out[2];
    _mm_store_pd(out, sum);
    return out[0] + out[1];
}

#ifdef __AVX__
// AVX tests for 256-bit vectors
FORCE_INLINE float test_avx_comparisons(void) {
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, INFINITY, 0.0f, -1.0f, NAN, 3.0f);
    __m256 b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, INFINITY, 0.0f, 1.0f, NAN, 3.0f);
    
    __m256 results[8];
    
    // Test all condition codes with AVX
    results[0] = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);    // UNORDERED
    results[1] = _mm256_cmp_ps(b, a, _CMP_ORD_Q);      // ORDERED
    results[2] = _mm256_cmp_ps(a, b, _CMP_UNEQ_UQ);    // UNEQ
    results[3] = _mm256_cmp_ps(a, b, _CMP_NGE_UQ);     // UNGE (nlt)
    results[4] = _mm256_cmp_ps(a, b, _CMP_NGT_UQ);     // UNGT (nle)
    results[5] = _mm256_cmp_ps(a, b, _CMP_ULE_UQ);     // UNLE
    results[6] = _mm256_cmp_ps(a, b, _CMP_ULT_UQ);     // UNLT
    results[7] = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);     // LTGT (une)
    
    // Create complex dependency graph
    __m256 accum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Conditional blending based on each comparison
        __m256 temp = _mm256_blendv_ps(a, b, results[i]);
        accum = _mm256_add_ps(accum, temp);
        
        // Mix with arithmetic
        if (i % 2 == 0) {
            accum = _mm256_mul_ps(accum, _mm256_set1_ps(1.1f));
        }
    }
    
    // Horizontal reduction
    __m128 low = _mm256_castps256_ps128(accum);
    __m128 high = _mm256_extractf128_ps(accum, 1);
    low = _mm_add_ps(low, high);
    
    __m128 shuf = _mm_movehdup_ps(low);
    __m128 sums = _mm_add_ps(low, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    
    float result;
    _mm_store_ss(&result, sums);
    return result;
}

// AVX double precision
FORCE_INLINE double test_avx_double_comparisons(void) {
    __m256d a = _mm256_setr_pd(1.0, NAN, 3.0, INFINITY);
    __m256d b = _mm256_setr_pd(2.0, 2.0, 3.0, 1.0);
    
    // Chain comparisons with different condition codes
    __m256d cmp1 = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);   // UNORDERED
    __m256d cmp2 = _mm256_cmp_pd(a, b, _CMP_ORD_Q);     // ORDERED
    __m256d cmp3 = _mm256_cmp_pd(a, b, _CMP_UNEQ_UQ);   // UNEQ
    
    // Use comparisons in conditional operations
    __m256d sel1 = _mm256_blendv_pd(a, b, cmp1);
    __m256d sel2 = _mm256_blendv_pd(b, a, cmp2);
    __m256d sel3 = _mm256_blendv_pd(sel1, sel2, cmp3);
    
    // More comparisons in the chain
    __m256d cmp4 = _mm256_cmp_pd(sel3, a, _CMP_NGE_UQ);  // UNGE (nlt)
    __m256d cmp5 = _mm256_cmp_pd(sel3, b, _CMP_NGT_UQ);  // UNGT (nle)
    
    __m256d final = _mm256_add_pd(sel3, _mm256_set1_pd(1.0));
    final = _mm256_blendv_pd(final, a, cmp4);
    final = _mm256_blendv_pd(final, b, cmp5);
    
    // Extract result
    double out[4];
    _mm256_store_pd(out, final);
    return out[0] + out[1] + out[2] + out[3];
}
#endif

// Scalar comparisons to test _mm_cmp_ss/_sd
FORCE_INLINE float test_scalar_comparisons(void) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(NAN);
    __m128 c = _mm_set_ss(2.0f);
    
    // Scalar comparisons also use the same condition codes
    __m128 r1 = _mm_cmp_ss(a, b, _CMP_UNORD_Q);    // UNORDERED
    __m128 r2 = _mm_cmp_ss(a, c, _CMP_ORD_Q);      // ORDERED
    __m128 r3 = _mm_cmp_ss(b, c, _CMP_UNEQ_UQ);    // UNEQ
    
    // Force use in conditional
    float result = 0;
    if (_mm_cvtss_f32(r1) != 0) result += 1.0f;
    if (_mm_cvtss_f32(r2) != 0) result += 2.0f;
    if (_mm_cvtss_f32(r3) != 0) result += 3.0f;
    
    return result;
}

int main(void) {
    float sse_result = 0;
    double sse_double_result = 0;
    float scalar_result = 0;
    
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Test SSE comparisons
    for (int i = 0; i < 10; i++) {
        sse_result += test_sse_comparisons();
        sse_double_result += test_sse_double_comparisons();
        scalar_result += test_scalar_comparisons();
    }
    
#ifdef __AVX__
    float avx_result = 0;
    double avx_double_result = 0;
    
    // Test AVX comparisons if available
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported, testing AVX comparisons...\n");
        for (int i = 0; i < 10; i++) {
            avx_result += test_avx_comparisons();
            avx_double_result += test_avx_double_comparisons();
        }
        printf("AVX float result: %f\n", avx_result);
        printf("AVX double result: %f\n", avx_double_result);
    }
#endif
    
    printf("SSE float result: %f\n", sse_result);
    printf("SSE double result: %f\n", sse_double_result);
    printf("Scalar result: %f\n", scalar_result);
    
    // Force assembly generation with volatile asm
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 v2 = _mm_setr_ps(NAN, 2.0f, 3.0f, 4.0f);
    
    __asm__ __volatile__ (
        "movaps %1, %%xmm0\n\t"
        "movaps %2, %%xmm1\n\t"
        "cmpeqps %%xmm1, %%xmm0\n\t"
        "movaps %%xmm0, %0\n\t"
        : "=m" (v1)
        : "m" (v1), "m" (v2)
        : "xmm0", "xmm1"
    );
    
    return 0;
}
