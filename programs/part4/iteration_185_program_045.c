#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Function to test SSE comparisons with all condition codes
void test_sse_comparisons(float* result) {
    // Initialize vectors with various values including NaN
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 d = _mm_setr_ps(0.0f, 0.0f, INFINITY, INFINITY);
    
    // Perform comparisons with all condition codes from uncovered block
    __m128 cmp_results[8];
    
    // UNORDERED: _CMP_UNORD_Q
    cmp_results[0] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    
    // ORDERED: _CMP_ORD_Q  
    cmp_results[1] = _mm_cmp_ps(a, b, _CMP_ORD_Q);
    
    // UNEQ: _CMP_UNEQ_UQ
    cmp_results[2] = _mm_cmp_ps(c, d, _CMP_UNEQ_UQ);
    
    // UNGE: _CMP_NGE_UQ (maps to "nlt" in assembly)
    cmp_results[3] = _mm_cmp_ps(a, b, _CMP_NGE_UQ);
    
    // UNGT: _CMP_NGT_UQ (maps to "nle" in assembly)
    cmp_results[4] = _mm_cmp_ps(a, b, _CMP_NGT_UQ);
    
    // UNLE: _CMP_ULE_UQ
    cmp_results[5] = _mm_cmp_ps(c, d, _CMP_ULE_UQ);
    
    // UNLT: _CMP_ULT_UQ
    cmp_results[6] = _mm_cmp_ps(c, d, _CMP_ULT_UQ);
    
    // LTGT: _CMP_NEQ_UQ (maps to "une" in assembly)
    cmp_results[7] = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);
    
    // Use results in arithmetic to prevent optimization
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_as_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        sum = _mm_add_ps(sum, mask_as_float);
    }
    
    // Extract and store result
    _mm_storeu_ps(result, sum);
    
    // Force assembly generation with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results: %0 %1 %2 %3" 
        : 
        : "x" (cmp_results[0]), "x" (cmp_results[1]), 
          "x" (cmp_results[2]), "x" (cmp_results[3])
        : "memory"
    );
}

// Double precision version
void test_sse_double_comparisons(double* result) {
    __m128d a = _mm_setr_pd(1.0, NAN);
    __m128d b = _mm_setr_pd(2.0, 2.0);
    __m128d c = _mm_setr_pd(0.0, INFINITY);
    __m128d d = _mm_setr_pd(-0.0, INFINITY);
    
    // Test all condition codes with double precision
    __m128d cmp_results[8];
    
    cmp_results[0] = _mm_cmp_pd(a, b, _CMP_UNORD_Q);    // UNORDERED
    cmp_results[1] = _mm_cmp_pd(a, b, _CMP_ORD_Q);      // ORDERED
    cmp_results[2] = _mm_cmp_pd(c, d, _CMP_UNEQ_UQ);    // UNEQ
    cmp_results[3] = _mm_cmp_pd(a, b, _CMP_NGE_UQ);     // UNGE -> "nlt"
    cmp_results[4] = _mm_cmp_pd(a, b, _CMP_NGT_UQ);     // UNGT -> "nle"
    cmp_results[5] = _mm_cmp_pd(c, d, _CMP_ULE_UQ);     // UNLE
    cmp_results[6] = _mm_cmp_pd(c, d, _CMP_ULT_UQ);     // UNLT
    cmp_results[7] = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);     // LTGT -> "une"
    
    // Use results in control flow
    int mask = _mm_movemask_pd(cmp_results[0]);
    if (mask) {
        // Blend based on comparison result
        __m128d blended = _mm_blendv_pd(a, b, cmp_results[1]);
        _mm_storeu_pd(result, blended);
    }
}

#ifdef __AVX__
// AVX versions for 256-bit vectors
void test_avx_comparisons(float* result) {
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 5.0f, 5.0f, 7.0f, 9.0f);
    
    __m256 cmp_results[8];
    
    // Test all condition codes with AVX
    cmp_results[0] = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);    // UNORDERED
    cmp_results[1] = _mm256_cmp_ps(a, b, _CMP_ORD_Q);      // ORDERED
    cmp_results[2] = _mm256_cmp_ps(a, b, _CMP_UNEQ_UQ);    // UNEQ
    cmp_results[3] = _mm256_cmp_ps(a, b, _CMP_NGE_UQ);     // UNGE -> "nlt"
    cmp_results[4] = _mm256_cmp_ps(a, b, _CMP_NGT_UQ);     // UNGT -> "nle"
    cmp_results[5] = _mm256_cmp_ps(a, b, _CMP_ULE_UQ);     // UNLE
    cmp_results[6] = _mm256_cmp_ps(a, b, _CMP_ULT_UQ);     // UNLT
    cmp_results[7] = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);     // LTGT -> "une"
    
    // Complex expression with blending
    __m256 mask1 = _mm256_and_ps(cmp_results[0], cmp_results[1]);
    __m256 mask2 = _mm256_or_ps(cmp_results[2], cmp_results[3]);
    __m256 final_mask = _mm256_xor_ps(mask1, mask2);
    
    __m256 blended = _mm256_blendv_ps(a, b, final_mask);
    _mm256_storeu_ps(result, blended);
}

void test_avx_double_comparisons(double* result) {
    __m256d a = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d b = _mm256_setr_pd(2.0, 2.0, 3.0, NAN);
    
    __m256d cmp_results[4];
    
    // Test subset with AVX double precision
    cmp_results[0] = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);    // UNORDERED
    cmp_results[1] = _mm256_cmp_pd(a, b, _CMP_ORD_Q);      // ORDERED
    cmp_results[2] = _mm256_cmp_pd(a, b, _CMP_UNEQ_UQ);    // UNEQ
    cmp_results[3] = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);     // LTGT -> "une"
    
    // Use in arithmetic operations
    __m256d sum = _mm256_setzero_pd();
    for (int i = 0; i < 4; i++) {
        // Convert comparison to 0.0 or 1.0
        __m256d ones = _mm256_set1_pd(1.0);
        __m256d mask = _mm256_and_pd(cmp_results[i], ones);
        sum = _mm256_add_pd(sum, mask);
    }
    
    _mm256_storeu_pd(result, sum);
}
#endif

// Scalar comparisons to test _mm_cmp_ss and _mm_cmp_sd
void test_scalar_comparisons(float* f_result, double* d_result) {
    // Single precision scalar comparisons
    __m128 a_ss = _mm_set_ss(1.0f);
    __m128 b_ss = _mm_set_ss(NAN);
    
    __m128 cmp_ss[4];
    cmp_ss[0] = _mm_cmp_ss(a_ss, b_ss, _CMP_UNORD_Q);    // UNORDERED
    cmp_ss[1] = _mm_cmp_ss(a_ss, b_ss, _CMP_ORD_Q);      // ORDERED
    cmp_ss[2] = _mm_cmp_ss(a_ss, b_ss, _CMP_UNEQ_UQ);    // UNEQ
    cmp_ss[3] = _mm_cmp_ss(a_ss, b_ss, _CMP_NEQ_UQ);     // LTGT -> "une"
    
    // Double precision scalar comparisons
    __m128d a_sd = _mm_set_sd(1.0);
    __m128d b_sd = _mm_set_sd(NAN);
    
    __m128d cmp_sd[4];
    cmp_sd[0] = _mm_cmp_sd(a_sd, b_sd, _CMP_UNORD_Q);    // UNORDERED
    cmp_sd[1] = _mm_cmp_sd(a_sd, b_sd, _CMP_ORD_Q);      // ORDERED
    cmp_sd[2] = _mm_cmp_sd(a_sd, b_sd, _CMP_UNEQ_UQ);    // UNEQ
    cmp_sd[3] = _mm_cmp_sd(a_sd, b_sd, _CMP_NEQ_UQ);     // LTGT -> "une"
    
    // Extract results
    *f_result = _mm_cvtss_f32(cmp_ss[0]) + _mm_cvtss_f32(cmp_ss[1]);
    *d_result = _mm_cvtsd_f64(cmp_sd[0]) + _mm_cvtsd_f64(cmp_sd[1]);
}

int main() {
    float sse_results[4];
    double sse_d_results[2];
    float scalar_f_result;
    double scalar_d_result;
    
    // Test all comparison types
    test_sse_comparisons(sse_results);
    test_sse_double_comparisons(sse_d_results);
    test_scalar_comparisons(&scalar_f_result, &scalar_d_result);
    
#ifdef __AVX__
    float avx_results[8];
    double avx_d_results[4];
    
    test_avx_comparisons(avx_results);
    test_avx_double_comparisons(avx_d_results);
    
    // Compute checksum from all results
    float checksum = 0.0f;
    for (int i = 0; i < 8; i++) checksum += avx_results[i];
    for (int i = 0; i < 4; i++) checksum += (float)avx_d_results[i];
    
    printf("AVX checksum: %f\n", checksum);
#endif
    
    // Use results to prevent optimization
    printf("SSE results: %f %f %f %f\n", 
           sse_results[0], sse_results[1], sse_results[2], sse_results[3]);
    printf("SSE double results: %f %f\n", sse_d_results[0], sse_d_results[1]);
    printf("Scalar results: %f %f\n", scalar_f_result, scalar_d_result);
    
    return 0;
}
