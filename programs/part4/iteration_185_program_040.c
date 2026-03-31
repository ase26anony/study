#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Function to test all SSE condition codes
void test_sse_condition_codes(void) {
    // Initialize vectors with various values including NaN
    __m128 a_ps = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b_ps = _mm_setr_ps(1.0f, 3.0f, 5.0f, NAN);
    __m128 c_ps = _mm_setr_ps(0.0f, 0.0f, INFINITY, -INFINITY);
    
    // Test all condition codes from the uncovered block
    __m128 cmp_results[8];
    
    // UNORDERED
    cmp_results[0] = _mm_cmp_ps(a_ps, b_ps, _CMP_UNORD_Q);
    
    // ORDERED
    cmp_results[1] = _mm_cmp_ps(a_ps, b_ps, _CMP_ORD_Q);
    
    // UNEQ
    cmp_results[2] = _mm_cmp_ps(a_ps, b_ps, _CMP_UNEQ_UQ);
    
    // UNGE (nlt)
    cmp_results[3] = _mm_cmp_ps(a_ps, b_ps, _CMP_NGE_UQ);
    
    // UNGT (nle)
    cmp_results[4] = _mm_cmp_ps(a_ps, b_ps, _CMP_NGT_UQ);
    
    // UNLE (ule)
    cmp_results[5] = _mm_cmp_ps(a_ps, b_ps, _CMP_ULE_UQ);
    
    // UNLT (ult)
    cmp_results[6] = _mm_cmp_ps(a_ps, b_ps, _CMP_ULT_UQ);
    
    // LTGT (une)
    cmp_results[7] = _mm_cmp_ps(a_ps, b_ps, _CMP_NEQ_UQ);
    
    // Use results in arithmetic to prevent dead code elimination
    __m128 accumulator = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_as_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        accumulator = _mm_add_ps(accumulator, mask_as_float);
    }
    
    // Extract and use result
    float result[4];
    _mm_store_ps(result, accumulator);
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results: %0" : : "x" (accumulator)
    );
}

// Double precision version
void test_sse_double_condition_codes(void) {
    __m128d a_pd = _mm_setr_pd(1.0, NAN);
    __m128d b_pd = _mm_setr_pd(NAN, 2.0);
    __m128d c_pd = _mm_setr_pd(0.0, INFINITY);
    
    __m128d cmp_results[8];
    
    cmp_results[0] = _mm_cmp_pd(a_pd, b_pd, _CMP_UNORD_Q);
    cmp_results[1] = _mm_cmp_pd(a_pd, b_pd, _CMP_ORD_Q);
    cmp_results[2] = _mm_cmp_pd(a_pd, b_pd, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm_cmp_pd(a_pd, b_pd, _CMP_NGE_UQ);
    cmp_results[4] = _mm_cmp_pd(a_pd, b_pd, _CMP_NGT_UQ);
    cmp_results[5] = _mm_cmp_pd(a_pd, b_pd, _CMP_ULE_UQ);
    cmp_results[6] = _mm_cmp_pd(a_pd, b_pd, _CMP_ULT_UQ);
    cmp_results[7] = _mm_cmp_pd(a_pd, b_pd, _CMP_NEQ_UQ);
    
    // Use in control flow
    int mask = _mm_movemask_pd(cmp_results[0]);
    if (mask) {
        __m128d blended = _mm_blendv_pd(a_pd, b_pd, cmp_results[1]);
        __asm__ __volatile__ (
            "# Double precision blend result: %0" : : "x" (blended)
        );
    }
}

#ifdef __AVX__
// AVX versions for wider vectors
void test_avx_condition_codes(void) {
    __m256 a_ps = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 b_ps = _mm256_setr_ps(1.0f, 3.0f, 5.0f, NAN, 9.0f, 10.0f, 11.0f, 12.0f);
    
    __m256 cmp_results[8];
    
    cmp_results[0] = _mm256_cmp_ps(a_ps, b_ps, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_ps(a_ps, b_ps, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_ps(a_ps, b_ps, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_ps(a_ps, b_ps, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_ps(a_ps, b_ps, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_ps(a_ps, b_ps, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_ps(a_ps, b_ps, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_ps(a_ps, b_ps, _CMP_NEQ_UQ);
    
    // Complex expression with multiple operations
    __m256 accumulator = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert mask to float and multiply
        __m256 mask_float = _mm256_and_ps(cmp_results[i], _mm256_set1_ps(1.0f));
        __m256 scaled = _mm256_mul_ps(mask_float, _mm256_set1_ps(i + 1.0f));
        accumulator = _mm256_add_ps(accumulator, scaled);
    }
    
    // Force assembly output
    __asm__ __volatile__ (
        "# AVX accumulator: %0" : : "x" (accumulator)
    );
}

void test_avx_double_condition_codes(void) {
    __m256d a_pd = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d b_pd = _mm256_setr_pd(NAN, 2.0, 3.0, 5.0);
    
    __m256d results[4];
    results[0] = _mm256_cmp_pd(a_pd, b_pd, _CMP_UNORD_Q);
    results[1] = _mm256_cmp_pd(a_pd, b_pd, _CMP_ORD_Q);
    results[2] = _mm256_cmp_pd(a_pd, b_pd, _CMP_UNEQ_UQ);
    results[3] = _mm256_cmp_pd(a_pd, b_pd, _CMP_NGE_UQ);
    
    // Use in conditional blending
    __m256d default_val = _mm256_set1_pd(100.0);
    __m256d final_result = default_val;
    
    for (int i = 0; i < 4; i++) {
        int mask = _mm256_movemask_pd(results[i]);
        if (mask != 0) {
            __m256d blended = _mm256_blendv_pd(a_pd, b_pd, results[i]);
            final_result = _mm256_add_pd(final_result, blended);
        }
    }
    
    __asm__ __volatile__ (
        "# AVX double final: %0" : : "x" (final_result)
    );
}
#endif

// Scalar versions for completeness
void test_scalar_condition_codes(void) {
    __m128 a_ss = _mm_set_ss(NAN);
    __m128 b_ss = _mm_set_ss(1.0f);
    
    // Scalar comparisons
    __m128 s_unord = _mm_cmp_ss(a_ss, b_ss, _CMP_UNORD_Q);
    __m128 s_ord = _mm_cmp_ss(a_ss, b_ss, _CMP_ORD_Q);
    __m128 s_uneq = _mm_cmp_ss(a_ss, b_ss, _CMP_UNEQ_UQ);
    
    // Mix with vector operations
    __m128 vec = _mm_set1_ps(2.0f);
    __m128 masked = _mm_and_ps(s_unord, vec);
    
    __asm__ __volatile__ (
        "# Scalar comparison masked: %0" : : "x" (masked)
    );
}

int main(void) {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Test SSE paths
    test_sse_condition_codes();
    test_sse_double_condition_codes();
    test_scalar_condition_codes();
    
    // Test AVX paths if available
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        test_avx_condition_codes();
        test_avx_double_condition_codes();
        printf("AVX code paths tested\n");
    }
#endif
    
    printf("All condition code tests completed\n");
    
    // Compute and return a value based on comparisons
    __m128 test_a = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 test_b = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    // Use multiple condition codes in one expression
    __m128 cmp1 = _mm_cmp_ps(test_a, test_b, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ps(test_a, test_b, _CMP_ORD_Q);
    __m128 cmp3 = _mm_cmp_ps(test_a, test_b, _CMP_UNEQ_UQ);
    
    __m128 combined = _mm_add_ps(_mm_add_ps(cmp1, cmp2), cmp3);
    float result[4];
    _mm_store_ps(result, combined);
    
    return (int)(result[0] + result[1] + result[2] + result[3]);
}
