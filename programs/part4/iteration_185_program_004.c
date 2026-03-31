#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifdef __AVX__
#define USE_AVX 1
#else
#define USE_AVX 0
#endif

// Function to test all SSE condition codes
void test_sse_condition_codes(float* result) {
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 d = _mm_setr_ps(0.0f, 0.0f, INFINITY, INFINITY);
    
    // Test all condition codes from the uncovered block
    __m128 cmp_results[8];
    
    // UNORDERED: _CMP_UNORD_Q
    cmp_results[0] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    
    // ORDERED: _CMP_ORD_Q  
    cmp_results[1] = _mm_cmp_ps(a, b, _CMP_ORD_Q);
    
    // UNEQ: _CMP_UNEQ_UQ
    cmp_results[2] = _mm_cmp_ps(c, d, _CMP_UNEQ_UQ);
    
    // UNGE: _CMP_NGE_UQ (maps to "nlt" in assembly)
    cmp_results[3] = _mm_cmp_ps(a, _mm_set1_ps(1.5f), _CMP_NGE_UQ);
    
    // UNGT: _CMP_NGT_UQ (maps to "nle" in assembly)
    cmp_results[4] = _mm_cmp_ps(a, _mm_set1_ps(2.5f), _CMP_NGT_UQ);
    
    // UNLE: _CMP_ULE_UQ
    cmp_results[5] = _mm_cmp_ps(b, _mm_set1_ps(3.0f), _CMP_ULE_UQ);
    
    // UNLT: _CMP_ULT_UQ
    cmp_results[6] = _mm_cmp_ps(a, _mm_set1_ps(3.0f), _CMP_ULT_UQ);
    
    // LTGT: _CMP_NEQ_UQ (maps to "une" in assembly)
    cmp_results[7] = _mm_cmp_ps(c, d, _CMP_NEQ_UQ);
    
    // Use results in arithmetic to prevent dead code elimination
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_as_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        sum = _mm_add_ps(sum, mask_as_float);
    }
    
    // Blend operations using comparison results
    __m128 blended = _mm_blendv_ps(a, b, cmp_results[0]);
    blended = _mm_add_ps(blended, _mm_blendv_ps(c, d, cmp_results[1]));
    
    // Extract masks and use in control flow
    int mask0 = _mm_movemask_ps(cmp_results[0]);
    int mask1 = _mm_movemask_ps(cmp_results[1]);
    int mask2 = _mm_movemask_ps(cmp_results[2]);
    
    // Conditional computation based on comparison results
    float final_result = 0.0f;
    if (mask0 & 0x1) final_result += 1.0f;
    if (mask1 & 0x2) final_result += 2.0f;
    if (mask2 & 0x4) final_result += 3.0f;
    
    // Store to output parameter
    _mm_storeu_ps(result, _mm_add_ps(sum, blended));
    result[0] += final_result;
}

// Double precision version
void test_sse_double_condition_codes(double* result) {
    __m128d a = _mm_setr_pd(1.0, NAN);
    __m128d b = _mm_setr_pd(2.0, 2.0);
    __m128d c = _mm_setr_pd(0.0, INFINITY);
    __m128d d = _mm_setr_pd(-0.0, INFINITY);
    
    // Test with _mm_cmp_pd
    __m128d cmp_unord = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
    __m128d cmp_ord = _mm_cmp_pd(a, b, _CMP_ORD_Q);
    __m128d cmp_uneq = _mm_cmp_pd(c, d, _CMP_UNEQ_UQ);
    __m128d cmp_nge = _mm_cmp_pd(a, _mm_set1_pd(1.5), _CMP_NGE_UQ);
    
    // Use scalar comparisons too (_mm_cmp_sd)
    __m128d cmp_ngt = _mm_cmp_sd(a, _mm_set1_pd(2.5), _CMP_NGT_UQ);
    __m128d cmp_ule = _mm_cmp_pd(b, _mm_set1_pd(3.0), _CMP_ULE_UQ);
    __m128d cmp_ult = _mm_cmp_pd(a, _mm_set1_pd(3.0), _CMP_ULT_UQ);
    __m128d cmp_neq = _mm_cmp_pd(c, d, _CMP_NEQ_UQ);
    
    // Combine results
    __m128d sum = _mm_add_pd(cmp_unord, cmp_ord);
    sum = _mm_add_pd(sum, _mm_add_pd(cmp_uneq, cmp_nge));
    sum = _mm_add_pd(sum, _mm_add_pd(cmp_ngt, cmp_ule));
    sum = _mm_add_pd(sum, _mm_add_pd(cmp_ult, cmp_neq));
    
    _mm_storeu_pd(result, sum);
}

#ifdef __AVX__
// AVX version with 256-bit vectors
void test_avx_condition_codes(float* result) {
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, NAN, 8.0f);
    __m256 b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, 7.0f, NAN);
    
    // Test AVX comparisons
    __m256 cmp_unord = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    __m256 cmp_ord = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
    __m256 cmp_uneq = _mm256_cmp_ps(a, _mm256_set1_ps(3.0f), _CMP_UNEQ_UQ);
    __m256 cmp_nge = _mm256_cmp_ps(a, _mm256_set1_ps(2.0f), _CMP_NGE_UQ);
    __m256 cmp_ngt = _mm256_cmp_ps(a, _mm256_set1_ps(4.0f), _CMP_NGT_UQ);
    __m256 cmp_ule = _mm256_cmp_ps(b, _mm256_set1_ps(4.0f), _CMP_ULE_UQ);
    __m256 cmp_ult = _mm256_cmp_ps(a, _mm256_set1_ps(5.0f), _CMP_ULT_UQ);
    __m256 cmp_neq = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);
    
    // Complex expression with blending
    __m256 blended = _mm256_blendv_ps(a, b, cmp_unord);
    blended = _mm256_add_ps(blended, _mm256_blendv_ps(cmp_ord, cmp_uneq, cmp_nge));
    
    // Extract masks for control flow
    int mask_unord = _mm256_movemask_ps(cmp_unord);
    int mask_ord = _mm256_movemask_ps(cmp_ord);
    
    // Use masks in computation
    float scale = 0.0f;
    if (mask_unord) scale += 0.5f;
    if (mask_ord) scale += 1.5f;
    
    __m256 scaled = _mm256_mul_ps(blended, _mm256_set1_ps(scale));
    
    // Final combination
    __m256 final = _mm256_add_ps(_mm256_add_ps(cmp_unord, cmp_ord),
                                _mm256_add_ps(cmp_uneq, cmp_nge));
    final = _mm256_add_ps(final, _mm256_add_ps(cmp_ngt, cmp_ule));
    final = _mm256_add_ps(final, _mm256_add_ps(cmp_ult, cmp_neq));
    final = _mm256_add_ps(final, scaled);
    
    _mm256_storeu_ps(result, final);
}

// AVX double precision
void test_avx_double_condition_codes(double* result) {
    __m256d a = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d b = _mm256_setr_pd(2.0, 2.0, NAN, 4.0);
    
    __m256d cmp_unord = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
    __m256d cmp_ord = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
    __m256d cmp_uneq = _mm256_cmp_pd(a, _mm256_set1_pd(2.5), _CMP_UNEQ_UQ);
    __m256d cmp_nge = _mm256_cmp_pd(a, _mm256_set1_pd(2.0), _CMP_NGE_UQ);
    __m256d cmp_ngt = _mm256_cmp_pd(a, _mm256_set1_pd(3.5), _CMP_NGT_UQ);
    __m256d cmp_ule = _mm256_cmp_pd(b, _mm256_set1_pd(3.0), _CMP_ULE_UQ);
    __m256d cmp_ult = _mm256_cmp_pd(a, _mm256_set1_pd(4.5), _CMP_ULT_UQ);
    __m256d cmp_neq = _mm256_cmp_pd(a, b, _CMP_NEQ_UQ);
    
    __m256d sum = _mm256_add_pd(cmp_unord, cmp_ord);
    sum = _mm256_add_pd(sum, _mm256_add_pd(cmp_uneq, cmp_nge));
    sum = _mm256_add_pd(sum, _mm256_add_pd(cmp_ngt, cmp_ule));
    sum = _mm256_add_pd(sum, _mm256_add_pd(cmp_ult, cmp_neq));
    
    _mm256_storeu_pd(result, sum);
}
#endif

// Inline assembly to force condition code printing
void force_asm_output(void) {
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 result;
    
    // Use inline assembly with vector comparisons
    // This should generate assembly with condition codes
    __asm__ __volatile__ (
        "vcmpltps %[src2], %[src1], %[dst]"
        : [dst] "=x" (result)
        : [src1] "x" (v1), [src2] "x" (v2)
    );
    
    // Use the result to prevent optimization
    float f = ((float*)&result)[0];
    printf("ASM result: %f\n", f);
}

int main() {
    float sse_result[4] = {0};
    double sse_double_result[2] = {0};
    
    printf("Testing SSE condition codes...\n");
    test_sse_condition_codes(sse_result);
    test_sse_double_condition_codes(sse_double_result);
    
    printf("SSE float results: %f %f %f %f\n", 
           sse_result[0], sse_result[1], sse_result[2], sse_result[3]);
    printf("SSE double results: %f %f\n", 
           sse_double_result[0], sse_double_result[1]);
    
#ifdef __AVX__
    float avx_result[8] = {0};
    double avx_double_result[4] = {0};
    
    printf("\nTesting AVX condition codes...\n");
    test_avx_condition_codes(avx_result);
    test_avx_double_condition_codes(avx_double_result);
    
    printf("AVX float results: ");
    for (int i = 0; i < 8; i++) printf("%f ", avx_result[i]);
    printf("\nAVX double results: ");
    for (int i = 0; i < 4; i++) printf("%f ", avx_double_result[i]);
    printf("\n");
#endif
    
    force_asm_output();
    
    return 0;
}
