#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Function to test SSE comparison condition codes
void test_sse_comparisons(float* result) {
    // Initialize vectors with various values including NaN
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    
    // Test all condition codes from the uncovered block
    __m128 cmp_results[8];
    
    // UNORDERED: _CMP_UNORD_Q
    cmp_results[0] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    
    // ORDERED: _CMP_ORD_Q
    cmp_results[1] = _mm_cmp_ps(a, b, _CMP_ORD_Q);
    
    // UNEQ: _CMP_UNEQ_UQ
    cmp_results[2] = _mm_cmp_ps(a, b, _CMP_UNEQ_UQ);
    
    // UNGE: _CMP_NGE_UQ (maps to nlt)
    cmp_results[3] = _mm_cmp_ps(a, b, _CMP_NGE_UQ);
    
    // UNGT: _CMP_NGT_UQ (maps to nle)
    cmp_results[4] = _mm_cmp_ps(a, b, _CMP_NGT_UQ);
    
    // UNLE: _CMP_ULE_UQ
    cmp_results[5] = _mm_cmp_ps(a, b, _CMP_ULE_UQ);
    
    // UNLT: _CMP_ULT_UQ
    cmp_results[6] = _mm_cmp_ps(a, b, _CMP_ULT_UQ);
    
    // LTGT: _CMP_NEQ_UQ (maps to une)
    cmp_results[7] = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);
    
    // Use results in arithmetic to prevent optimization
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        sum = _mm_add_ps(sum, mask_float);
    }
    
    // Blend operations using comparison results
    __m128 blended = _mm_blendv_ps(a, b, cmp_results[0]);  // Using UNORDERED result
    blended = _mm_add_ps(blended, _mm_blendv_ps(b, c, cmp_results[1]));  // Using ORDERED result
    
    // Extract masks and use in control flow
    int mask0 = _mm_movemask_ps(cmp_results[0]);  // UNORDERED mask
    int mask1 = _mm_movemask_ps(cmp_results[1]);  // ORDERED mask
    
    // Control flow dependent on comparison results
    if (mask0 & 0x1) {
        blended = _mm_mul_ps(blended, _mm_set1_ps(2.0f));
    }
    if (mask1 & 0x2) {
        blended = _mm_add_ps(blended, sum);
    }
    
    // Store final result
    _mm_storeu_ps(result, blended);
}

// Function to test AVX comparison condition codes
#ifdef __AVX__
void test_avx_comparisons(float* result) {
    // Initialize AVX vectors
    __m256 a256 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 b256 = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, NAN, 8.0f);
    
    // Test condition codes with AVX
    __m256 cmp_results256[8];
    
    cmp_results256[0] = _mm256_cmp_ps(a256, b256, _CMP_UNORD_Q);
    cmp_results256[1] = _mm256_cmp_ps(a256, b256, _CMP_ORD_Q);
    cmp_results256[2] = _mm256_cmp_ps(a256, b256, _CMP_UNEQ_UQ);
    cmp_results256[3] = _mm256_cmp_ps(a256, b256, _CMP_NGE_UQ);
    cmp_results256[4] = _mm256_cmp_ps(a256, b256, _CMP_NGT_UQ);
    cmp_results256[5] = _mm256_cmp_ps(a256, b256, _CMP_ULE_UQ);
    cmp_results256[6] = _mm256_cmp_ps(a256, b256, _CMP_ULT_UQ);
    cmp_results256[7] = _mm256_cmp_ps(a256, b256, _CMP_NEQ_UQ);
    
    // Complex expression mixing comparisons and arithmetic
    __m256 sum256 = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        __m256 mask_float = _mm256_and_ps(cmp_results256[i], _mm256_set1_ps(1.0f));
        sum256 = _mm256_add_ps(sum256, mask_float);
        
        // Alternate between add and mul based on iteration
        if (i % 2 == 0) {
            sum256 = _mm256_add_ps(sum256, _mm256_set1_ps(0.5f));
        } else {
            sum256 = _mm256_mul_ps(sum256, _mm256_set1_ps(1.5f));
        }
    }
    
    // Use inline assembly to force assembly output with vector operands
    __m256 final_result;
    asm volatile (
        "vmulps %1, %2, %0\n\t"
        : "=x"(final_result)
        : "x"(sum256), "x"(a256)
        : 
    );
    
    _mm256_storeu_ps(result, final_result);
}
#endif

// Test double precision comparisons
void test_double_comparisons(double* result) {
    __m128d a_d = _mm_setr_pd(1.0, NAN);
    __m128d b_d = _mm_setr_pd(2.0, 2.0);
    
    // Test with double precision
    __m128d cmp_d[8];
    
    cmp_d[0] = _mm_cmp_pd(a_d, b_d, _CMP_UNORD_Q);
    cmp_d[1] = _mm_cmp_pd(a_d, b_d, _CMP_ORD_Q);
    cmp_d[2] = _mm_cmp_pd(a_d, b_d, _CMP_UNEQ_UQ);
    cmp_d[3] = _mm_cmp_pd(a_d, b_d, _CMP_NGE_UQ);
    cmp_d[4] = _mm_cmp_pd(a_d, b_d, _CMP_NGT_UQ);
    cmp_d[5] = _mm_cmp_pd(a_d, b_d, _CMP_ULE_UQ);
    cmp_d[6] = _mm_cmp_pd(a_d, b_d, _CMP_ULT_UQ);
    cmp_d[7] = _mm_cmp_pd(a_d, b_d, _CMP_NEQ_UQ);
    
    // Use results in computation
    __m128d sum_d = _mm_setzero_pd();
    for (int i = 0; i < 8; i++) {
        // Convert mask to double
        __m128d ones = _mm_set1_pd(1.0);
        __m128d mask_double = _mm_and_pd(cmp_d[i], ones);
        sum_d = _mm_add_pd(sum_d, mask_double);
    }
    
    // Extract mask for control flow
    int mask_d = _mm_movemask_pd(cmp_d[0]);
    if (mask_d) {
        sum_d = _mm_mul_pd(sum_d, _mm_set1_pd(3.0));
    }
    
    _mm_storeu_pd(result, sum_d);
}

int main() {
    float sse_result[4];
    float avx_result[8];
    double double_result[2];
    
    // Test SSE comparisons
    test_sse_comparisons(sse_result);
    
    // Test AVX comparisons if supported
    #ifdef __AVX__
    test_avx_comparisons(avx_result);
    
    // Mix SSE and AVX results
    float mixed = 0.0f;
    for (int i = 0; i < 4; i++) {
        mixed += sse_result[i];
    }
    for (int i = 0; i < 8; i++) {
        mixed += avx_result[i];
    }
    printf("Mixed SSE/AVX result: %f\n", mixed);
    #endif
    
    // Test double precision comparisons
    test_double_comparisons(double_result);
    
    // Final computation using all results to prevent optimization
    float final_result = sse_result[0] + sse_result[1] + sse_result[2] + sse_result[3];
    final_result += (float)(double_result[0] + double_result[1]);
    
    #ifdef __AVX__
    for (int i = 0; i < 8; i++) {
        final_result += avx_result[i];
    }
    #endif
    
    printf("Final result: %f\n", final_result);
    
    return (final_result > 0.0f) ? 0 : 1;
}
