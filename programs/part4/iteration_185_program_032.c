#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifdef __AVX__
#define USE_AVX 1
#else
#define USE_AVX 0
#endif

// Function to test SSE comparisons with all condition codes
void test_sse_comparisons(float* result) {
    // Initialize vectors with various values including NaN
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 vec3 = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    
    // Perform comparisons with all condition codes from uncovered block
    __m128 cmp_results[8];
    
    // UNORDERED: _CMP_UNORD_Q
    cmp_results[0] = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    
    // ORDERED: _CMP_ORD_Q  
    cmp_results[1] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    
    // UNEQ: _CMP_UNEQ_UQ
    cmp_results[2] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    
    // UNGE: _CMP_NGE_UQ
    cmp_results[3] = _mm_cmp_ps(vec1, vec3, _CMP_NGE_UQ);
    
    // UNGT: _CMP_NGT_UQ
    cmp_results[4] = _mm_cmp_ps(vec2, vec3, _CMP_NGT_UQ);
    
    // UNLE: _CMP_ULE_UQ
    cmp_results[5] = _mm_cmp_ps(vec3, vec1, _CMP_ULE_UQ);
    
    // UNLT: _CMP_ULT_UQ
    cmp_results[6] = _mm_cmp_ps(vec3, vec2, _CMP_ULT_UQ);
    
    // LTGT: _CMP_NEQ_UQ
    cmp_results[7] = _mm_cmp_ps(vec1, vec3, _CMP_NEQ_UQ);
    
    // Use results in arithmetic operations to prevent dead code elimination
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        accum = _mm_add_ps(accum, mask_float);
    }
    
    // Extract and store result
    _mm_storeu_ps(result, accum);
    
    // Force assembly generation with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results stored\n"
        : 
        : "m" (*result)
        : "memory"
    );
}

// Double precision SSE comparisons
void test_sse_double_comparisons(double* result) {
    __m128d vec1 = _mm_setr_pd(1.0, NAN);
    __m128d vec2 = _mm_setr_pd(NAN, 2.0);
    __m128d vec3 = _mm_setr_pd(0.0, INFINITY);
    
    // Test a subset of condition codes with double precision
    __m128d cmp1 = _mm_cmp_pd(vec1, vec2, _CMP_UNORD_Q);   // UNORDERED
    __m128d cmp2 = _mm_cmp_pd(vec1, vec3, _CMP_ORD_Q);     // ORDERED
    __m128d cmp3 = _mm_cmp_pd(vec2, vec3, _CMP_UNEQ_UQ);   // UNEQ
    
    // Use results in control flow
    int mask1 = _mm_movemask_pd(cmp1);
    int mask2 = _mm_movemask_pd(cmp2);
    int mask3 = _mm_movemask_pd(cmp3);
    
    // Branch based on comparison results
    if (mask1 & 1) {
        result[0] = 1.0;
    } else {
        result[0] = 0.0;
    }
    
    if (mask2 & 2) {
        result[1] = 2.0;
    } else {
        result[1] = 0.0;
    }
    
    // Complex expression with blending
    __m128d blended = _mm_blendv_pd(vec1, vec2, cmp3);
    _mm_storeu_pd(&result[2], blended);
}

#if USE_AVX
// AVX comparisons for 256-bit vectors
void test_avx_comparisons(float* result) {
    __m256 avx_vec1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_vec2 = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, NAN, 8.0f);
    
    // Test all condition codes with AVX
    __m256 avx_cmp_results[8];
    
    avx_cmp_results[0] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNORD_Q);   // UNORDERED
    avx_cmp_results[1] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ORD_Q);     // ORDERED
    avx_cmp_results[2] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNEQ_UQ);   // UNEQ
    avx_cmp_results[3] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NGE_UQ);    // UNGE
    avx_cmp_results[4] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NGT_UQ);    // UNGT
    avx_cmp_results[5] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULE_UQ);    // UNLE
    avx_cmp_results[6] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULT_UQ);    // UNLT
    avx_cmp_results[7] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NEQ_UQ);    // LTGT
    
    // Complex arithmetic with comparison results
    __m256 accum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Create mask from comparison results
        __m256 mask = _mm256_and_ps(avx_cmp_results[i], _mm256_set1_ps(1.0f));
        accum = _mm256_add_ps(accum, mask);
        
        // Additional arithmetic to force decomposition
        accum = _mm256_mul_ps(accum, _mm256_set1_ps(0.5f));
    }
    
    // Extract masks and use in conditional
    for (int i = 0; i < 8; i++) {
        int mask = _mm256_movemask_ps(avx_cmp_results[i]);
        if (mask != 0) {
            accum = _mm256_add_ps(accum, _mm256_set1_ps(1.0f));
        }
    }
    
    _mm256_storeu_ps(result, accum);
    
    // Force assembly output
    __asm__ __volatile__ (
        "# AVX comparison block\n"
        : 
        : "m" (*result)
        : "memory"
    );
}

// AVX double precision comparisons
void test_avx_double_comparisons(double* result) {
    __m256d avx_vec1 = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d avx_vec2 = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    
    // Test multiple condition codes
    __m256d cmp1 = _mm256_cmp_pd(avx_vec1, avx_vec2, _CMP_UNORD_Q);
    __m256d cmp2 = _mm256_cmp_pd(avx_vec1, avx_vec2, _CMP_ORD_Q);
    __m256d cmp3 = _mm256_cmp_pd(avx_vec1, avx_vec2, _CMP_UNEQ_UQ);
    
    // Blend based on comparison results
    __m256d blended1 = _mm256_blendv_pd(avx_vec1, avx_vec2, cmp1);
    __m256d blended2 = _mm256_blendv_pd(avx_vec1, avx_vec2, cmp2);
    
    // Final result
    __m256d final = _mm256_add_pd(blended1, blended2);
    _mm256_storeu_pd(result, final);
}
#endif

// Scalar comparisons to test _mm_cmp_ss and _mm_cmp_sd
void test_scalar_comparisons(float* f_result, double* d_result) {
    // Single precision scalar comparisons
    __m128 s1 = _mm_set_ss(1.0f);
    __m128 s2 = _mm_set_ss(NAN);
    __m128 s3 = _mm_set_ss(2.0f);
    
    __m128 s_cmp1 = _mm_cmp_ss(s1, s2, _CMP_UNORD_Q);   // UNORDERED
    __m128 s_cmp2 = _mm_cmp_ss(s1, s3, _CMP_UNEQ_UQ);   // UNEQ
    __m128 s_cmp3 = _mm_cmp_ss(s2, s3, _CMP_ORD_Q);     // ORDERED
    
    // Extract results
    *f_result = _mm_cvtss_f32(_mm_add_ss(_mm_add_ss(s_cmp1, s_cmp2), s_cmp3));
    
    // Double precision scalar comparisons
    __m128d d1 = _mm_set_sd(1.0);
    __m128d d2 = _mm_set_sd(NAN);
    __m128d d3 = _mm_set_sd(INFINITY);
    
    __m128d d_cmp1 = _mm_cmp_sd(d1, d2, _CMP_UNORD_Q);   // UNORDERED
    __m128d d_cmp2 = _mm_cmp_sd(d1, d3, _CMP_NGE_UQ);    // UNGE
    __m128d d_cmp3 = _mm_cmp_sd(d2, d3, _CMP_NEQ_UQ);    // LTGT
    
    // Use in conditional
    int d_mask = _mm_movemask_pd(_mm_or_pd(_mm_or_pd(d_cmp1, d_cmp2), d_cmp3));
    if (d_mask) {
        *d_result = 1.0;
    } else {
        *d_result = 0.0;
    }
}

int main() {
    float sse_results[4];
    double sse_d_results[4];
    float scalar_f_result;
    double scalar_d_result;
    
    // Test SSE comparisons
    test_sse_comparisons(sse_results);
    test_sse_double_comparisons(sse_d_results);
    
    // Test scalar comparisons
    test_scalar_comparisons(&scalar_f_result, &scalar_d_result);
    
#if USE_AVX
    float avx_results[8];
    double avx_d_results[4];
    
    // Test AVX comparisons if supported
    test_avx_comparisons(avx_results);
    test_avx_double_comparisons(avx_d_results);
    
    // Combine results from all tests
    float final_result = sse_results[0] + avx_results[0] + scalar_f_result;
    printf("Combined result: %f\n", final_result);
#else
    printf("AVX not available, using SSE only\n");
    printf("SSE result: %f\n", sse_results[0]);
#endif
    
    // Use results to prevent optimization
    volatile float dummy = sse_results[0] + sse_d_results[0] + scalar_f_result + scalar_d_result;
    
    return 0;
}
