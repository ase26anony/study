#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Function to test all SSE comparison condition codes
void test_sse_comparisons(float* result) {
    // Initialize vectors with various values including NaN
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 vec3 = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    
    // Perform comparisons using all condition codes from uncovered block
    __m128 cmp_results[8];
    
    // UNORDERED: _CMP_UNORD_Q
    cmp_results[0] = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    
    // ORDERED: _CMP_ORD_Q
    cmp_results[1] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    
    // UNEQ: _CMP_UNEQ_UQ
    cmp_results[2] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    
    // UNGE: _CMP_NGE_UQ
    cmp_results[3] = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    
    // UNGT: _CMP_NGT_UQ
    cmp_results[4] = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    
    // UNLE: _CMP_ULE_UQ
    cmp_results[5] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    
    // UNLT: _CMP_ULT_UQ
    cmp_results[6] = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    
    // LTGT: _CMP_NEQ_UQ
    cmp_results[7] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Use results in arithmetic to prevent optimization
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_as_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        sum = _mm_add_ps(sum, mask_as_float);
    }
    
    // Extract result to prevent dead code elimination
    _mm_storeu_ps(result, sum);
}

// Double precision version
void test_sse_double_comparisons(double* result) {
    __m128d vec1 = _mm_setr_pd(1.0, NAN);
    __m128d vec2 = _mm_setr_pd(NAN, 2.0);
    __m128d vec3 = _mm_setr_pd(0.0, INFINITY);
    
    __m128d cmp_results[8];
    
    cmp_results[0] = _mm_cmp_pd(vec1, vec2, _CMP_UNORD_Q);
    cmp_results[1] = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    cmp_results[2] = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    cmp_results[4] = _mm_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    cmp_results[5] = _mm_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    cmp_results[6] = _mm_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    cmp_results[7] = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    __m128d sum = _mm_setzero_pd();
    for (int i = 0; i < 8; i++) {
        __m128d mask_as_double = _mm_and_pd(cmp_results[i], _mm_set1_pd(1.0));
        sum = _mm_add_pd(sum, mask_as_double);
    }
    
    _mm_storeu_pd(result, sum);
}

#ifdef __AVX__
// AVX versions for 256-bit vectors
void test_avx_comparisons(float* result) {
    __m256 vec1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec2 = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 9.0f, 10.0f, NAN, 12.0f);
    
    __m256 cmp_results[8];
    
    // AVX comparisons
    cmp_results[0] = _mm256_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    __m256 sum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        __m256 mask_as_float = _mm256_and_ps(cmp_results[i], _mm256_set1_ps(1.0f));
        sum = _mm256_add_ps(sum, mask_as_float);
    }
    
    _mm256_storeu_ps(result, sum);
}

void test_avx_double_comparisons(double* result) {
    __m256d vec1 = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d vec2 = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    
    __m256d cmp_results[8];
    
    cmp_results[0] = _mm256_cmp_pd(vec1, vec2, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    __m256d sum = _mm256_setzero_pd();
    for (int i = 0; i < 8; i++) {
        __m256d mask_as_double = _mm256_and_pd(cmp_results[i], _mm256_set1_pd(1.0));
        sum = _mm256_add_pd(sum, mask_as_double);
    }
    
    _mm256_storeu_pd(result, sum);
}
#endif

// Scalar comparisons to test _mm_cmp_ss and _mm_cmp_sd
void test_scalar_comparisons(float* f_result, double* d_result) {
    // Scalar float comparisons
    __m128 s1 = _mm_set_ss(1.0f);
    __m128 s2 = _mm_set_ss(NAN);
    
    __m128 s_cmp_results[8];
    s_cmp_results[0] = _mm_cmp_ss(s1, s2, _CMP_UNORD_Q);
    s_cmp_results[1] = _mm_cmp_ss(s1, s2, _CMP_ORD_Q);
    s_cmp_results[2] = _mm_cmp_ss(s1, s2, _CMP_UNEQ_UQ);
    s_cmp_results[3] = _mm_cmp_ss(s1, s2, _CMP_NGE_UQ);
    s_cmp_results[4] = _mm_cmp_ss(s1, s2, _CMP_NGT_UQ);
    s_cmp_results[5] = _mm_cmp_ss(s1, s2, _CMP_ULE_UQ);
    s_cmp_results[6] = _mm_cmp_ss(s1, s2, _CMP_ULT_UQ);
    s_cmp_results[7] = _mm_cmp_ss(s1, s2, _CMP_NEQ_UQ);
    
    // Scalar double comparisons
    __m128d d1 = _mm_set_sd(1.0);
    __m128d d2 = _mm_set_sd(NAN);
    
    __m128d d_cmp_results[8];
    d_cmp_results[0] = _mm_cmp_sd(d1, d2, _CMP_UNORD_Q);
    d_cmp_results[1] = _mm_cmp_sd(d1, d2, _CMP_ORD_Q);
    d_cmp_results[2] = _mm_cmp_sd(d1, d2, _CMP_UNEQ_UQ);
    d_cmp_results[3] = _mm_cmp_sd(d1, d2, _CMP_NGE_UQ);
    d_cmp_results[4] = _mm_cmp_sd(d1, d2, _CMP_NGT_UQ);
    d_cmp_results[5] = _mm_cmp_sd(d1, d2, _CMP_ULE_UQ);
    d_cmp_results[6] = _mm_cmp_sd(d1, d2, _CMP_ULT_UQ);
    d_cmp_results[7] = _mm_cmp_sd(d1, d2, _CMP_NEQ_UQ);
    
    // Use results to prevent optimization
    float f_sum = 0;
    double d_sum = 0;
    
    for (int i = 0; i < 8; i++) {
        f_sum += _mm_cvtss_f32(s_cmp_results[i]);
        d_sum += _mm_cvtsd_f64(d_cmp_results[i]);
    }
    
    *f_result = f_sum;
    *d_result = d_sum;
}

// Complex expression mixing comparisons and arithmetic
__m128 complex_vector_expression(__m128 a, __m128 b, __m128 c) {
    // Compare with UNORDERED
    __m128 cmp_unord = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    // Compare with ORDERED
    __m128 cmp_ord = _mm_cmp_ps(b, c, _CMP_ORD_Q);
    // Compare with UNEQ
    __m128 cmp_uneq = _mm_cmp_ps(a, c, _CMP_UNEQ_UQ);
    
    // Blend results based on comparisons
    __m128 result = _mm_blendv_ps(a, b, cmp_unord);
    result = _mm_blendv_ps(result, c, cmp_ord);
    result = _mm_add_ps(result, _mm_and_ps(cmp_uneq, _mm_set1_ps(1.0f)));
    
    return result;
}

int main() {
    float sse_results[4];
    double sse_d_results[2];
    float scalar_f_result;
    double scalar_d_result;
    
    // Test SSE comparisons
    test_sse_comparisons(sse_results);
    test_sse_double_comparisons(sse_d_results);
    
    // Test scalar comparisons
    test_scalar_comparisons(&scalar_f_result, &scalar_d_result);
    
#ifdef __AVX__
    float avx_results[8];
    double avx_d_results[4];
    
    // Test AVX comparisons if available
    test_avx_comparisons(avx_results);
    test_avx_double_comparisons(avx_d_results);
    
    // Print some results to prevent optimization
    printf("AVX float results: %f %f %f %f\n", 
           avx_results[0], avx_results[1], avx_results[2], avx_results[3]);
    printf("AVX double results: %f %f\n", avx_d_results[0], avx_d_results[1]);
#endif
    
    // Test complex expression
    __m128 vec_a = _mm_setr_ps(1.0f, NAN, 3.0f, 4.0f);
    __m128 vec_b = _mm_setr_ps(5.0f, 6.0f, NAN, 8.0f);
    __m128 vec_c = _mm_setr_ps(9.0f, 10.0f, 11.0f, NAN);
    __m128 complex_result = complex_vector_expression(vec_a, vec_b, vec_c);
    
    // Extract movemask for conditional logic
    __m128 cmp_nlt = _mm_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);  // UNGE -> nlt
    __m128 cmp_nle = _mm_cmp_ps(vec_b, vec_c, _CMP_NGT_UQ);  // UNGT -> nle
    __m128 cmp_ult = _mm_cmp_ps(vec_a, vec_c, _CMP_ULT_UQ);  // UNLT -> ult
    
    int mask_nlt = _mm_movemask_ps(cmp_nlt);
    int mask_nle = _mm_movemask_ps(cmp_nle);
    int mask_ult = _mm_movemask_ps(cmp_ult);
    
    // Use masks in conditional logic
    float final_result = 0.0f;
    if (mask_nlt & 1) final_result += 1.0f;
    if (mask_nle & 2) final_result += 2.0f;
    if (mask_ult & 4) final_result += 3.0f;
    
    // Force assembly output with inline asm
    __m128 asm_vec = _mm_set1_ps(1.0f);
    __m128 asm_cmp;
    
    // This inline assembly will force the compiler to generate
    // assembly with condition code strings
    __asm__ __volatile__ (
        "vcmpps %0, %1, %2, %3\n\t"
        : "=x"(asm_cmp)
        : "x"(asm_vec), "x"(asm_vec), "i"(_CMP_UNORD_Q)
    );
    
    printf("Final result: %f\n", final_result);
    printf("SSE results: %f %f %f %f\n", 
           sse_results[0], sse_results[1], sse_results[2], sse_results[3]);
    
    return 0;
}
