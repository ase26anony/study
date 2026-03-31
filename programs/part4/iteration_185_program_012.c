#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Function to test SSE comparison condition codes
void test_sse_condition_codes(float* result) {
    // Initialize vectors with various values including NaN
    __m128 vec_a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec_b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 vec_c = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    
    // Results storage
    __m128 cmp_results[8];
    
    // Test all condition codes from uncovered block
    cmp_results[0] = _mm_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);    // UNORDERED
    cmp_results[1] = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);      // ORDERED
    cmp_results[2] = _mm_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);    // UNEQ
    cmp_results[3] = _mm_cmp_ps(vec_a, vec_c, _CMP_NGE_UQ);     // UNGE (nlt)
    cmp_results[4] = _mm_cmp_ps(vec_a, vec_c, _CMP_NGT_UQ);     // UNGT (nle)
    cmp_results[5] = _mm_cmp_ps(vec_c, vec_a, _CMP_ULE_UQ);     // UNLE (ule)
    cmp_results[6] = _mm_cmp_ps(vec_c, vec_a, _CMP_ULT_UQ);     // UNLT (ult)
    cmp_results[7] = _mm_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);     // LTGT (une)
    
    // Use results in arithmetic to prevent optimization
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        accum = _mm_add_ps(accum, mask_float);
    }
    
    // Blend operations using comparison results
    __m128 blended = _mm_blendv_ps(vec_a, vec_b, cmp_results[0]);
    blended = _mm_blendv_ps(blended, vec_c, cmp_results[1]);
    
    // Extract mask and use in control flow
    int mask0 = _mm_movemask_ps(cmp_results[0]);
    int mask1 = _mm_movemask_ps(cmp_results[1]);
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results: %0 %1"
        : 
        : "r" (mask0), "r" (mask1)
        : "memory"
    );
    
    // Store final result
    _mm_storeu_ps(result, _mm_add_ps(accum, blended));
}

// Double precision version
void test_sse_double_condition_codes(double* result) {
    __m128d vec_a = _mm_setr_pd(1.0, NAN);
    __m128d vec_b = _mm_setr_pd(NAN, 2.0);
    __m128d vec_c = _mm_setr_pd(0.0, INFINITY);
    
    // Test with double precision comparisons
    __m128d cmp_unord = _mm_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    __m128d cmp_ord = _mm_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    __m128d cmp_uneq = _mm_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    __m128d cmp_nge = _mm_cmp_pd(vec_a, vec_c, _CMP_NGE_UQ);
    
    // Use in arithmetic
    __m128d accum = _mm_add_pd(cmp_unord, cmp_ord);
    accum = _mm_add_pd(accum, cmp_uneq);
    accum = _mm_add_pd(accum, cmp_nge);
    
    // More comparisons
    __m128d cmp_ngt = _mm_cmp_pd(vec_c, vec_a, _CMP_NGT_UQ);
    __m128d cmp_ule = _mm_cmp_pd(vec_a, vec_c, _CMP_ULE_UQ);
    __m128d cmp_ult = _mm_cmp_pd(vec_c, vec_a, _CMP_ULT_UQ);
    __m128d cmp_neq = _mm_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
    
    // Complex expression to force decomposition
    __m128d result_vec = _mm_add_pd(
        _mm_mul_pd(cmp_ngt, _mm_set1_pd(2.0)),
        _mm_sub_pd(cmp_ule, cmp_ult)
    );
    result_vec = _mm_div_pd(result_vec, _mm_add_pd(cmp_neq, _mm_set1_pd(1.0)));
    
    _mm_storeu_pd(result, _mm_add_pd(accum, result_vec));
}

#ifdef __AVX__
// AVX version for 256-bit vectors
void test_avx_condition_codes(float* result) {
    __m256 vec_a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec_b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 9.0f, 10.0f, NAN, 12.0f);
    
    // Test AVX comparisons
    __m256 cmp_results[8];
    cmp_results[0] = _mm256_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    
    // Complex AVX arithmetic with comparison results
    __m256 accum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Blend based on comparison results
        __m256 temp = _mm256_blendv_ps(vec_a, vec_b, cmp_results[i]);
        accum = _mm256_add_ps(accum, temp);
        
        // Conditional multiply
        __m256 mask = _mm256_and_ps(cmp_results[i], _mm256_set1_ps(1.0f));
        accum = _mm256_add_ps(accum, _mm256_mul_ps(mask, _mm256_set1_ps(0.5f)));
    }
    
    // Extract masks for control flow
    int mask_unord = _mm256_movemask_ps(cmp_results[0]);
    int mask_ord = _mm256_movemask_ps(cmp_results[1]);
    
    // Force assembly generation
    if (mask_unord & 0x1) {
        __asm__ __volatile__ ("# AVX unord path taken" ::: "memory");
    }
    if (mask_ord & 0x2) {
        __asm__ __volatile__ ("# AVX ord path taken" ::: "memory");
    }
    
    _mm256_storeu_ps(result, accum);
}

// AVX double precision
void test_avx_double_condition_codes(double* result) {
    __m256d vec_a = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d vec_b = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    
    __m256d cmp_unord = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    __m256d cmp_ord = _mm256_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    __m256d cmp_uneq = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    __m256d cmp_nge = _mm256_cmp_pd(vec_a, vec_b, _CMP_NGE_UQ);
    __m256d cmp_ngt = _mm256_cmp_pd(vec_a, vec_b, _CMP_NGT_UQ);
    __m256d cmp_ule = _mm256_cmp_pd(vec_a, vec_b, _CMP_ULE_UQ);
    __m256d cmp_ult = _mm256_cmp_pd(vec_a, vec_b, _CMP_ULT_UQ);
    __m256d cmp_neq = _mm256_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
    
    // Complex expression forcing instruction selection
    __m256d result_vec = _mm256_add_pd(
        _mm256_mul_pd(cmp_unord, _mm256_set1_pd(0.25)),
        _mm256_sub_pd(cmp_ord, cmp_uneq)
    );
    result_vec = _mm256_add_pd(result_vec, 
        _mm256_mul_pd(_mm256_add_pd(cmp_nge, cmp_ngt), _mm256_set1_pd(0.5)));
    result_vec = _mm256_div_pd(result_vec,
        _mm256_add_pd(_mm256_add_pd(cmp_ule, cmp_ult), _mm256_set1_pd(1.0)));
    
    _mm256_storeu_pd(result, _mm256_add_pd(result_vec, cmp_neq));
}
#endif

// Scalar comparisons to test _CMP_* codes
void test_scalar_condition_codes(float* result) {
    // Use scalar comparison intrinsics
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(NAN);
    __m128 c = _mm_set_ss(2.0f);
    
    __m128 cmp1 = _mm_cmp_ss(a, b, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ss(a, c, _CMP_UNEQ_UQ);
    __m128 cmp3 = _mm_cmp_ss(b, a, _CMP_NGE_UQ);
    __m128 cmp4 = _mm_cmp_ss(c, a, _CMP_ULE_UQ);
    
    // Mix with vector operations
    __m128 vec_a = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_b = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    __m128 vcmp1 = _mm_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    __m128 vcmp2 = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    
    // Blend scalar and vector results
    __m128 blended = _mm_blendv_ps(
        _mm_shuffle_ps(cmp1, cmp2, _MM_SHUFFLE(0, 0, 0, 0)),
        vcmp1,
        vcmp2
    );
    
    _mm_storeu_ps(result, blended);
}

int main() {
    float sse_result[4];
    double sse_double_result[2];
    float scalar_result[4];
    
    // Test SSE paths
    test_sse_condition_codes(sse_result);
    test_sse_double_condition_codes(sse_double_result);
    test_scalar_condition_codes(scalar_result);
    
    #ifdef __AVX__
    float avx_result[8];
    double avx_double_result[4];
    
    // Test AVX paths if available
    test_avx_condition_codes(avx_result);
    test_avx_double_condition_codes(avx_double_result);
    
    // Use results to prevent dead code elimination
    float sum = 0.0f;
    for (int i = 0; i < 8; i++) sum += avx_result[i];
    for (int i = 0; i < 4; i++) sum += (float)avx_double_result[i];
    printf("AVX sum: %f\n", sum);
    #endif
    
    // Use all results to prevent optimization
    float final_result = 0.0f;
    for (int i = 0; i < 4; i++) {
        final_result += sse_result[i];
        final_result += scalar_result[i];
    }
    for (int i = 0; i < 2; i++) {
        final_result += (float)sse_double_result[i];
    }
    
    printf("Final result: %f\n", final_result);
    
    // Additional test with runtime feature detection
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        __asm__ __volatile__ (
            "# AVX supported, generating vector comparisons"
            :
            :
            : "memory"
        );
        
        // One more AVX comparison with all condition codes
        __m256 test_a = _mm256_set1_ps(1.0f);
        __m256 test_b = _mm256_set1_ps(2.0f);
        
        // Generate assembly for all condition codes
        __m256 r1 = _mm256_cmp_ps(test_a, test_b, _CMP_UNORD_Q);
        __m256 r2 = _mm256_cmp_ps(test_a, test_b, _CMP_ORD_Q);
        __m256 r3 = _mm256_cmp_ps(test_a, test_b, _CMP_UNEQ_UQ);
        __m256 r4 = _mm256_cmp_ps(test_a, test_b, _CMP_NGE_UQ);
        __m256 r5 = _mm256_cmp_ps(test_a, test_b, _CMP_NGT_UQ);
        __m256 r6 = _mm256_cmp_ps(test_a, test_b, _CMP_ULE_UQ);
        __m256 r7 = _mm256_cmp_ps(test_a, test_b, _CMP_ULT_UQ);
        __m256 r8 = _mm256_cmp_ps(test_a, test_b, _CMP_NEQ_UQ);
        
        // Force use of results
        __m256 sum_vec = _mm256_add_ps(r1, r2);
        sum_vec = _mm256_add_ps(sum_vec, r3);
        sum_vec = _mm256_add_ps(sum_vec, r4);
        sum_vec = _mm256_add_ps(sum_vec, r5);
        sum_vec = _mm256_add_ps(sum_vec, r6);
        sum_vec = _mm256_add_ps(sum_vec, r7);
        sum_vec = _mm256_add_ps(sum_vec, r8);
        
        float temp[8];
        _mm256_storeu_ps(temp, sum_vec);
        printf("AVX comparison sum: %f\n", temp[0]);
    }
    #endif
    
    return 0;
}
