#include <immintrin.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

// Function to test SSE comparisons
void test_sse_comparisons(float* result) {
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    
    // Test all condition codes from the uncovered block
    __m128 cmp_results[8];
    
    // UNORDERED - _CMP_UNORD_Q
    cmp_results[0] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    
    // ORDERED - _CMP_ORD_Q  
    cmp_results[1] = _mm_cmp_ps(a, b, _CMP_ORD_Q);
    
    // UNEQ - _CMP_UNEQ_UQ
    cmp_results[2] = _mm_cmp_ps(a, b, _CMP_UNEQ_UQ);
    
    // UNGE - _CMP_NGE_UQ
    cmp_results[3] = _mm_cmp_ps(a, b, _CMP_NGE_UQ);
    
    // UNGT - _CMP_NGT_UQ
    cmp_results[4] = _mm_cmp_ps(a, b, _CMP_NGT_UQ);
    
    // UNLE - _CMP_ULE_UQ
    cmp_results[5] = _mm_cmp_ps(a, c, _CMP_ULE_UQ);
    
    // UNLT - _CMP_ULT_UQ
    cmp_results[6] = _mm_cmp_ps(a, c, _CMP_ULT_UQ);
    
    // LTGT - _CMP_NEQ_UQ
    cmp_results[7] = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);
    
    // Use results in arithmetic to prevent optimization
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float for arithmetic
        __m128 mask_as_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        sum = _mm_add_ps(sum, mask_as_float);
    }
    
    // Extract and store result
    _mm_storeu_ps(result, sum);
}

// Function to test double precision SSE comparisons
void test_sse_double_comparisons(double* result) {
    __m128d a = _mm_setr_pd(1.0, NAN);
    __m128d b = _mm_setr_pd(2.0, 2.0);
    __m128d c = _mm_setr_pd(0.0, INFINITY);
    
    __m128d cmp_results[8];
    
    // Test with double precision
    cmp_results[0] = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
    cmp_results[1] = _mm_cmp_pd(a, b, _CMP_ORD_Q);
    cmp_results[2] = _mm_cmp_pd(a, b, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm_cmp_pd(a, b, _CMP_NGE_UQ);
    cmp_results[4] = _mm_cmp_pd(a, b, _CMP_NGT_UQ);
    cmp_results[5] = _mm_cmp_pd(a, c, _CMP_ULE_UQ);
    cmp_results[6] = _mm_cmp_pd(a, c, _CMP_ULT_UQ);
    cmp_results[7] = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);
    
    // Use results in control flow
    __m128d sum = _mm_setzero_pd();
    for (int i = 0; i < 8; i++) {
        // Extract mask and use in conditional operations
        int mask = _mm_movemask_pd(cmp_results[i]);
        if (mask & 1) {
            sum = _mm_add_pd(sum, _mm_set1_pd(1.0));
        }
        if (mask & 2) {
            sum = _mm_add_pd(sum, _mm_set1_pd(2.0));
        }
    }
    
    _mm_storeu_pd(result, sum);
}

#ifdef __AVX__
// AVX versions for 256-bit vectors
void test_avx_comparisons(float* result) {
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, 8.0f, 8.0f);
    
    __m256 cmp_results[8];
    
    // Test AVX comparisons
    cmp_results[0] = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_ps(a, b, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_ps(a, b, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_ps(a, b, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_ps(a, b, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_ps(a, b, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);
    
    // Complex expression with blending
    __m256 sum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Blend based on comparison results
        __m256 blend_val = _mm256_set1_ps((float)(i + 1));
        sum = _mm256_add_ps(sum, _mm256_blendv_ps(_mm256_setzero_ps(), 
                                                  blend_val, 
                                                  cmp_results[i]));
    }
    
    _mm256_storeu_ps(result, sum);
}

void test_avx_double_comparisons(double* result) {
    __m256d a = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d b = _mm256_setr_pd(2.0, 2.0, 3.0, NAN);
    
    __m256d cmp_results[8];
    
    cmp_results[0] = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_pd(a, b, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_pd(a, b, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_pd(a, b, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_pd(a, b, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_pd(a, b, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
    
    // Use movemask for control flow
    __m256d sum = _mm256_setzero_pd();
    for (int i = 0; i < 8; i++) {
        int mask = _mm256_movemask_pd(cmp_results[i]);
        __m256d add_val = _mm256_set1_pd(0.0);
        
        // Conditional addition based on mask bits
        if (mask & 1) add_val = _mm256_add_pd(add_val, _mm256_set1_pd(1.0));
        if (mask & 2) add_val = _mm256_add_pd(add_val, _mm256_set1_pd(2.0));
        if (mask & 4) add_val = _mm256_add_pd(add_val, _mm256_set1_pd(4.0));
        if (mask & 8) add_val = _mm256_add_pd(add_val, _mm256_set1_pd(8.0));
        
        sum = _mm256_add_pd(sum, add_val);
    }
    
    _mm256_storeu_pd(result, sum);
}
#endif

// Test scalar comparisons as well
void test_scalar_comparisons(float* result) {
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(NAN);
    __m128 c = _mm_set_ss(2.0f);
    
    // Scalar comparisons
    __m128 cmp1 = _mm_cmp_ss(a, b, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ss(a, c, _CMP_ORD_Q);
    __m128 cmp3 = _mm_cmp_ss(b, c, _CMP_UNEQ_UQ);
    __m128 cmp4 = _mm_cmp_ss(a, c, _CMP_NGE_UQ);
    
    // Combine results
    __m128 sum = _mm_add_ss(_mm_add_ss(cmp1, cmp2), 
                           _mm_add_ss(cmp3, cmp4));
    
    *result = _mm_cvtss_f32(sum);
}

int main() {
    float sse_result[4];
    double sse_double_result[2];
    float scalar_result;
    
    // Test SSE comparisons
    test_sse_comparisons(sse_result);
    
    // Test double precision SSE
    test_sse_double_comparisons(sse_double_result);
    
    // Test scalar comparisons
    test_scalar_comparisons(&scalar_result);
    
#ifdef __AVX__
    float avx_result[8];
    double avx_double_result[4];
    
    // Test AVX comparisons if available
    test_avx_comparisons(avx_result);
    test_avx_double_comparisons(avx_double_result);
    
    // Print AVX results to prevent optimization
    printf("AVX float results: ");
    for (int i = 0; i < 8; i++) {
        printf("%f ", avx_result[i]);
    }
    printf("\n");
    
    printf("AVX double results: ");
    for (int i = 0; i < 4; i++) {
        printf("%f ", avx_double_result[i]);
    }
    printf("\n");
#endif
    
    // Print results to prevent dead code elimination
    printf("SSE float results: %f %f %f %f\n", 
           sse_result[0], sse_result[1], sse_result[2], sse_result[3]);
    printf("SSE double results: %f %f\n", 
           sse_double_result[0], sse_double_result[1]);
    printf("Scalar result: %f\n", scalar_result);
    
    return 0;
}
