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

// Function to test all SSE condition codes
void test_sse_condition_codes(void) {
    printf("Testing SSE condition codes...\n");
    
    // Initialize vectors with various values including NaN
    __m128 vec_a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec_b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 vec_c = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    
    // Results array to store comparison masks
    int results[8] = {0};
    
    // Test all condition codes from the uncovered block
    // Each comparison generates assembly with condition code strings
    
    // 1. UNORDERED (_CMP_UNORD_Q)
    __m128 cmp_unord = _mm_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    results[0] = _mm_movemask_ps(cmp_unord);
    
    // 2. ORDERED (_CMP_ORD_Q)
    __m128 cmp_ord = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    results[1] = _mm_movemask_ps(cmp_ord);
    
    // 3. UNEQ (_CMP_UNEQ_UQ)
    __m128 cmp_uneq = _mm_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    results[2] = _mm_movemask_ps(cmp_uneq);
    
    // 4. UNGE (_CMP_NGE_UQ) - "nlt" in assembly
    __m128 cmp_unge = _mm_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    results[3] = _mm_movemask_ps(cmp_unge);
    
    // 5. UNGT (_CMP_NGT_UQ) - "nle" in assembly
    __m128 cmp_ungt = _mm_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    results[4] = _mm_movemask_ps(cmp_ungt);
    
    // 6. UNLE (_CMP_ULE_UQ) - "ule" in assembly
    __m128 cmp_unle = _mm_cmp_ps(vec_a, vec_c, _CMP_ULE_UQ);
    results[5] = _mm_movemask_ps(cmp_unle);
    
    // 7. UNLT (_CMP_ULT_UQ) - "ult" in assembly
    __m128 cmp_unlt = _mm_cmp_ps(vec_a, vec_c, _CMP_ULT_UQ);
    results[6] = _mm_movemask_ps(cmp_unlt);
    
    // 8. LTGT (_CMP_NEQ_UQ) - "une" in assembly
    __m128 cmp_ltgt = _mm_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    results[7] = _mm_movemask_ps(cmp_ltgt);
    
    // Use results in control flow to prevent optimization
    int sum = 0;
    for (int i = 0; i < 8; i++) {
        if (results[i] != 0) {
            sum += results[i];
        }
    }
    
    // Complex expression with blending to force decomposition
    __m128 blended = _mm_blendv_ps(vec_a, vec_b, cmp_unord);
    blended = _mm_add_ps(blended, _mm_mul_ps(cmp_ord, vec_c));
    
    // Extract final result
    float final_result[4];
    _mm_storeu_ps(final_result, blended);
    
    printf("SSE test complete. Sum of masks: %d\n", sum);
}

// Function to test SSE2 double precision condition codes
void test_sse2_double_condition_codes(void) {
    printf("Testing SSE2 double precision condition codes...\n");
    
    __m128d vec_a = _mm_setr_pd(1.0, NAN);
    __m128d vec_b = _mm_setr_pd(NAN, 2.0);
    __m128d vec_c = _mm_setr_pd(0.0, INFINITY);
    
    // Test a subset of condition codes with doubles
    __m128d cmp_unord_d = _mm_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    __m128d cmp_ord_d = _mm_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    __m128d cmp_uneq_d = _mm_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    __m128d cmp_unge_d = _mm_cmp_pd(vec_a, vec_c, _CMP_NGE_UQ);
    
    // Use comparisons in arithmetic operations
    __m128d result = _mm_add_pd(
        _mm_and_pd(cmp_unord_d, vec_a),
        _mm_andnot_pd(cmp_ord_d, vec_b)
    );
    
    double res[2];
    _mm_storeu_pd(res, result);
    
    printf("Double test result: %f, %f\n", res[0], res[1]);
}

#ifdef __AVX__
// Function to test AVX condition codes
void test_avx_condition_codes(void) {
    printf("Testing AVX condition codes...\n");
    
    // AVX 256-bit vectors
    __m256 vec_a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec_b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, NAN, 9.0f);
    
    // Test AVX versions of condition codes
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    __m256 cmp_uneq_avx = _mm256_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    __m256 cmp_unge_avx = _mm256_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    __m256 cmp_ungt_avx = _mm256_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    __m256 cmp_unle_avx = _mm256_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);
    __m256 cmp_unlt_avx = _mm256_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);
    __m256 cmp_ltgt_avx = _mm256_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    
    // Complex expression with AVX blending
    __m256 blended_avx = _mm256_blendv_ps(vec_a, vec_b, cmp_unord_avx);
    blended_avx = _mm256_add_ps(blended_avx, _mm256_mul_ps(cmp_ord_avx, vec_a));
    
    // Extract mask and use in control flow
    int mask_unord = _mm256_movemask_ps(cmp_unord_avx);
    int mask_ord = _mm256_movemask_ps(cmp_ord_avx);
    
    float avx_result[8];
    _mm256_storeu_ps(avx_result, blended_avx);
    
    printf("AVX test complete. Masks: unord=0x%02x, ord=0x%02x\n", 
           mask_unord, mask_ord);
}

// Test AVX double precision
void test_avx_double_condition_codes(void) {
    printf("Testing AVX double precision condition codes...\n");
    
    __m256d vec_a = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d vec_b = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    __m256d cmp_uneq_avx_d = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    
    double avx_d_result[4];
    _mm256_storeu_pd(avx_d_result, _mm256_add_pd(
        _mm256_and_pd(cmp_unord_avx_d, vec_a),
        _mm256_and_pd(cmp_ord_avx_d, vec_b)
    ));
    
    printf("AVX double result: %f, %f, %f, %f\n", 
           avx_d_result[0], avx_d_result[1], 
           avx_d_result[2], avx_d_result[3]);
}
#endif

// Test scalar comparisons (SSE scalar)
void test_scalar_condition_codes(void) {
    printf("Testing scalar condition codes...\n");
    
    __m128 vec_a = _mm_set_ss(NAN);
    __m128 vec_b = _mm_set_ss(1.0f);
    
    // Scalar comparisons also generate condition codes
    __m128 cmp_unord_ss = _mm_cmp_ss(vec_a, vec_b, _CMP_UNORD_Q);
    __m128 cmp_ord_ss = _mm_cmp_ss(vec_a, vec_b, _CMP_ORD_Q);
    __m128 cmp_uneq_ss = _mm_cmp_ss(vec_a, vec_b, _CMP_UNEQ_UQ);
    
    float result[4];
    _mm_storeu_ps(result, _mm_add_ss(cmp_unord_ss, cmp_ord_ss));
    
    printf("Scalar test result: %f\n", result[0]);
}

// Main function with runtime feature detection
int main(void) {
    printf("Starting condition code coverage test...\n\n");
    
    // Always test SSE if available
    #ifdef __SSE__
    test_sse_condition_codes();
    printf("\n");
    
    test_sse2_double_condition_codes();
    printf("\n");
    
    test_scalar_condition_codes();
    printf("\n");
    #else
    printf("SSE not available on this platform\n");
    #endif
    
    // Test AVX if available
    #ifdef __AVX__
    test_avx_condition_codes();
    printf("\n");
    
    test_avx_double_condition_codes();
    printf("\n");
    #else
    printf("AVX not available on this platform\n");
    #endif
    
    // Force assembly generation for all condition codes
    // by using inline assembly with vector operands
    #ifdef __SSE__
    __m128 asm_vec = _mm_set1_ps(1.0f);
    int asm_result;
    
    // This inline assembly forces GCC to generate the condition code strings
    // in the assembly output
    __asm__ __volatile__ (
        "vcmpps %%xmm0, %%xmm1, %%xmm2, %{unord%}\n\t"
        "vcmpps %%xmm0, %%xmm1, %%xmm3, %{ord%}\n\t"
        "vmovmskps %%xmm2, %0"
        : "=r" (asm_result)
        : 
        : "xmm0", "xmm1", "xmm2", "xmm3"
    );
    
    printf("Inline assembly test completed. Result: %d\n", asm_result);
    #endif
    
    printf("\nAll tests completed. Check generated assembly for condition code strings:\n");
    printf("  unord, ord, ueq, nlt, nle, ule, ult, une\n");
    
    return 0;
}
