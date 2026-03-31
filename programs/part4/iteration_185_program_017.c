#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#ifdef __AVX__
#include <x86intrin.h>
#endif

// Function to test SSE comparisons with all condition codes
float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    
    // Results accumulator
    __m128 result = _mm_setzero_ps();
    
    // Test all condition codes from the uncovered block
    // Each comparison produces a mask vector
    
    // 1. UNORDERED (handles NaN comparisons)
    __m128 cmp_unord = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm_add_ps(result, _mm_and_ps(cmp_unord, _mm_set1_ps(1.0f)));
    
    // 2. ORDERED
    __m128 cmp_ord = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    result = _mm_add_ps(result, _mm_and_ps(cmp_ord, _mm_set1_ps(2.0f)));
    
    // 3. UNEQ (unordered or equal)
    __m128 cmp_uneq = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_uneq, _mm_set1_ps(3.0f)));
    
    // 4. UNGE (not less than, unordered)
    __m128 cmp_unge = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_unge, _mm_set1_ps(4.0f)));
    
    // 5. UNGT (not less than or equal, unordered)
    __m128 cmp_ungt = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_ungt, _mm_set1_ps(5.0f)));
    
    // 6. UNLE (unordered or less than or equal)
    __m128 cmp_unle = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_unle, _mm_set1_ps(6.0f)));
    
    // 7. UNLT (unordered or less than)
    __m128 cmp_unlt = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_unlt, _mm_set1_ps(7.0f)));
    
    // 8. LTGT (not equal, unordered)
    __m128 cmp_ltgt = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_ltgt, _mm_set1_ps(8.0f)));
    
    // Also test scalar comparisons which use different code paths
    __m128 cmp_scalar_unord = _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q);
    __m128 cmp_scalar_ord = _mm_cmp_ss(vec1, vec2, _CMP_ORD_Q);
    
    // Extract mask and use in control flow to prevent optimization
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results: unord=%0, ord=%1\n"
        : /* no output */
        : "r" (mask_unord), "r" (mask_ord)
        : "memory"
    );
    
    // Horizontal sum of result
    __m128 shuf = _mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(result, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    
    return _mm_cvtss_f32(sums);
}

// Double precision version
double test_sse2_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    __m128d result = _mm_setzero_pd();
    
    // Test double precision comparisons
    __m128d cmp_unord = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm_add_pd(result, _mm_and_pd(cmp_unord, _mm_set1_pd(1.0)));
    
    __m128d cmp_ord = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    result = _mm_add_pd(result, _mm_and_pd(cmp_ord, _mm_set1_pd(2.0)));
    
    __m128d cmp_uneq = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_uneq, _mm_set1_pd(3.0)));
    
    // Extract masks
    int mask = _mm_movemask_pd(cmp_unord) | _mm_movemask_pd(cmp_ord);
    
    __asm__ __volatile__ (
        "# SSE2 double comparison mask: %0\n"
        : /* no output */
        : "r" (mask)
        : "memory"
    );
    
    // Horizontal sum
    __m128d shuf = _mm_shuffle_pd(result, result, 1);
    __m128d sums = _mm_add_pd(result, shuf);
    
    return _mm_cvtsd_f64(sums);
}

#ifdef __AVX__
// AVX version with 256-bit vectors
float test_avx_comparisons(float a, float b, float c, float d,
                          float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 result = _mm256_setzero_ps();
    
    // AVX comparisons
    __m256 cmp_unord = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_unord, _mm256_set1_ps(1.0f)));
    
    __m256 cmp_ord = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_ord, _mm256_set1_ps(2.0f)));
    
    __m256 cmp_uneq = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_uneq, _mm256_set1_ps(3.0f)));
    
    __m256 cmp_unge = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_unge, _mm256_set1_ps(4.0f)));
    
    // Complex expression with blending
    __m256 blended = _mm256_blendv_ps(vec1, vec2, cmp_unord);
    result = _mm256_add_ps(result, blended);
    
    // Extract mask for control flow
    int mask = _mm256_movemask_ps(cmp_unord);
    
    __asm__ __volatile__ (
        "# AVX comparison mask: %0\n"
        : /* no output */
        : "r" (mask)
        : "memory"
    );
    
    // Horizontal sum
    __m128 low = _mm256_castps256_ps128(result);
    __m128 high = _mm256_extractf128_ps(result, 1);
    __m128 sum128 = _mm_add_ps(low, high);
    
    __m128 shuf = _mm_shuffle_ps(sum128, sum128, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    
    return _mm_cvtss_f32(sums);
}
#endif

int main() {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Test data with various special values
    float test_floats[] = {1.0f, -1.0f, 0.0f, -0.0f, INFINITY, -INFINITY, NAN, 2.5f};
    double test_doubles[] = {1.0, -1.0, 0.0, -0.0, INFINITY, -INFINITY, NAN, 2.5};
    
    float sse_result = 0.0f;
    double sse2_result = 0.0;
    
    // Test multiple iterations with different data
    for (int i = 0; i < 4; i++) {
        sse_result += test_sse_comparisons(
            test_floats[i], 
            test_floats[i+1], 
            test_floats[i+2], 
            test_floats[i+3]
        );
        
        sse2_result += test_sse2_comparisons(
            test_doubles[i],
            test_doubles[i+1]
        );
    }
    
    printf("SSE result: %f\n", sse_result);
    printf("SSE2 double result: %f\n", sse2_result);
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported, testing AVX comparisons...\n");
        
        float avx_result = test_avx_comparisons(
            test_floats[0], test_floats[1], test_floats[2], test_floats[3],
            test_floats[4], test_floats[5], test_floats[6], test_floats[7]
        );
        
        printf("AVX result: %f\n", avx_result);
        
        // Test more AVX comparisons with different condition codes
        __m256 vec1 = _mm256_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
        __m256 vec2 = _mm256_setr_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
        
        // Force generation of all condition codes
        __m256 cmp_unle = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
        __m256 cmp_unlt = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
        __m256 cmp_ltgt = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
        
        // Use results in arithmetic to prevent optimization
        __m256 combined = _mm256_add_ps(cmp_unle, _mm256_add_ps(cmp_unlt, cmp_ltgt));
        float combined_sum = 0.0f;
        float* ptr = (float*)&combined;
        for (int i = 0; i < 8; i++) {
            combined_sum += ptr[i];
        }
        printf("AVX combined comparisons sum: %f\n", combined_sum);
    } else {
        printf("AVX not supported on this CPU\n");
    }
#endif
    
    // Additional test: mixed precision and width comparisons
    // This creates complex expressions that may trigger different code paths
    {
        __m128 vec_sse = _mm_setr_ps(1.0f, NAN, INFINITY, -INFINITY);
        __m128d vec_sse2 = _mm_setr_pd(1.0, NAN);
        
        // Cross-type comparisons in same function
        __m128 cmp1 = _mm_cmp_ps(vec_sse, _mm_set1_ps(0.0f), _CMP_UNORD_Q);
        __m128d cmp2 = _mm_cmp_pd(vec_sse2, _mm_set1_pd(0.0), _CMP_ORD_Q);
        
        int mask1 = _mm_movemask_ps(cmp1);
        int mask2 = _mm_movemask_pd(cmp2);
        
        // Use masks in conditional
        if ((mask1 | mask2) != 0) {
            printf("Mixed comparisons produced non-zero masks: %d, %d\n", mask1, mask2);
        }
    }
    
    return 0;
}
