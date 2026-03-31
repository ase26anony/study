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
    
    // Store comparison results to prevent optimization
    __m128 cmp_results[8];
    
    // Test all condition codes from the uncovered block
    cmp_results[0] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);    // UNORDERED -> "unord"
    cmp_results[1] = _mm_cmp_ps(a, b, _CMP_ORD_Q);      // ORDERED -> "ord"
    cmp_results[2] = _mm_cmp_ps(a, b, _CMP_UNEQ_UQ);    // UNEQ -> "ueq"
    cmp_results[3] = _mm_cmp_ps(a, b, _CMP_NGE_UQ);     // UNGE -> "nlt"
    cmp_results[4] = _mm_cmp_ps(a, b, _CMP_NGT_UQ);     // UNGT -> "nle"
    cmp_results[5] = _mm_cmp_ps(a, b, _CMP_ULE_UQ);     // UNLE -> "ule"
    cmp_results[6] = _mm_cmp_ps(a, b, _CMP_ULT_UQ);     // UNLT -> "ult"
    cmp_results[7] = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);     // LTGT -> "une"
    
    // Also test scalar comparisons
    __m128 sse_scalar_cmp = _mm_cmp_ss(a, b, _CMP_UNORD_Q);
    
    // Use results in control flow to prevent dead code elimination
    int mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= _mm_movemask_ps(cmp_results[i]);
    }
    
    // Blend based on comparison results
    __m128 blended = _mm_blendv_ps(a, b, cmp_results[0]);
    blended = _mm_add_ps(blended, _mm_blendv_ps(c, a, cmp_results[1]));
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results stored\n"
        : 
        : "x" (cmp_results[0]), "x" (cmp_results[1]), 
          "x" (cmp_results[2]), "x" (cmp_results[3])
        : 
    );
    
    // Store final result
    _mm_storeu_ps(result, blended);
    
    // Use mask in conditional
    if (mask & 1) {
        result[0] += 1.0f;
    }
}

// Function to test AVX comparisons (256-bit)
#ifdef __AVX__
void test_avx_comparisons(float* result) {
    // Initialize AVX vectors
    __m256 avx_a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, NAN, 9.0f);
    
    // Test AVX comparisons with different condition codes
    __m256 avx_cmp_results[4];
    
    avx_cmp_results[0] = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNORD_Q);
    avx_cmp_results[1] = _mm256_cmp_ps(avx_a, avx_b, _CMP_ORD_Q);
    avx_cmp_results[2] = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNEQ_UQ);
    avx_cmp_results[3] = _mm256_cmp_ps(avx_a, avx_b, _CMP_NGE_UQ);
    
    // Blend and arithmetic operations
    __m256 avx_blended = _mm256_blendv_ps(avx_a, avx_b, avx_cmp_results[0]);
    avx_blended = _mm256_add_ps(avx_blended, _mm256_mul_ps(avx_a, avx_cmp_results[1]));
    
    // Extract masks for control flow
    int avx_masks[4];
    for (int i = 0; i < 4; i++) {
        avx_masks[i] = _mm256_movemask_ps(avx_cmp_results[i]);
    }
    
    // Force AVX assembly output
    __asm__ __volatile__ (
        "# AVX comparison operations\n"
        : 
        : "x" (avx_cmp_results[0]), "x" (avx_cmp_results[1])
        : 
    );
    
    // Store results
    _mm256_storeu_ps(result, avx_blended);
    
    // Use masks
    if (avx_masks[0] & avx_masks[1]) {
        result[0] *= 2.0f;
    }
}
#endif

// Test double precision comparisons
void test_double_comparisons(double* result) {
    __m128d dbl_a = _mm_setr_pd(1.0, NAN);
    __m128d dbl_b = _mm_setr_pd(NAN, 2.0);
    __m128d dbl_c = _mm_setr_pd(0.0, INFINITY);
    
    // Test double precision with various condition codes
    __m128d dbl_cmp[4];
    
    dbl_cmp[0] = _mm_cmp_pd(dbl_a, dbl_b, _CMP_UNORD_Q);   // UNORDERED
    dbl_cmp[1] = _mm_cmp_pd(dbl_a, dbl_b, _CMP_ORD_Q);     // ORDERED
    dbl_cmp[2] = _mm_cmp_pd(dbl_a, dbl_b, _CMP_UNEQ_UQ);   // UNEQ
    dbl_cmp[3] = _mm_cmp_pd(dbl_a, dbl_b, _CMP_NGE_UQ);    // UNGE
    
    // Test scalar double comparison
    __m128d dbl_scalar_cmp = _mm_cmp_sd(dbl_a, dbl_b, _CMP_UNORD_Q);
    
    // Complex expression with blending
    __m128d dbl_result = _mm_add_pd(
        _mm_blendv_pd(dbl_a, dbl_b, dbl_cmp[0]),
        _mm_blendv_pd(dbl_c, dbl_a, dbl_cmp[1])
    );
    
    // Force double comparison assembly
    __asm__ __volatile__ (
        "# Double precision comparisons\n"
        : 
        : "x" (dbl_cmp[0]), "x" (dbl_cmp[1])
        : 
    );
    
    _mm_storeu_pd(result, dbl_result);
}

// Main function with runtime feature detection
int main() {
    float sse_results[4] = {0};
    double dbl_results[2] = {0};
    
    printf("Testing SSE comparisons...\n");
    test_sse_comparisons(sse_results);
    
    printf("Testing double precision comparisons...\n");
    test_double_comparisons(dbl_results);
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        float avx_results[8] = {0};
        printf("Testing AVX comparisons...\n");
        test_avx_comparisons(avx_results);
        
        // Use AVX results
        float avx_sum = 0;
        for (int i = 0; i < 8; i++) {
            avx_sum += avx_results[i];
        }
        printf("AVX result sum: %f\n", avx_sum);
    }
#endif
    
    // Use results to prevent optimization
    float final_result = sse_results[0] + sse_results[1] + 
                         (float)dbl_results[0] + (float)dbl_results[1];
    
    printf("Final result: %f\n", final_result);
    printf("Test completed. Check generated assembly for condition codes.\n");
    
    return 0;
}
