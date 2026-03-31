#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Function to test SSE comparisons with all condition codes
void test_sse_comparisons(float* result) {
    // Initialize vectors with various values including NaN
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 vec3 = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    
    // Results storage
    __m128 cmp_results[8];
    
    // Test all condition codes from the uncovered block
    cmp_results[0] = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);    // UNORDERED
    cmp_results[1] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);      // ORDERED
    cmp_results[2] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);    // UNEQ
    cmp_results[3] = _mm_cmp_ps(vec1, vec3, _CMP_NGE_UQ);     // UNGE (nlt)
    cmp_results[4] = _mm_cmp_ps(vec1, vec3, _CMP_NGT_UQ);     // UNGT (nle)
    cmp_results[5] = _mm_cmp_ps(vec3, vec1, _CMP_ULE_UQ);     // UNLE (ule)
    cmp_results[6] = _mm_cmp_ps(vec3, vec1, _CMP_ULT_UQ);     // UNLT (ult)
    cmp_results[7] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);     // LTGT (une)
    
    // Use results in arithmetic to prevent dead code elimination
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        accum = _mm_add_ps(accum, mask_float);
    }
    
    // Blend operations using comparison results
    __m128 blended = _mm_blendv_ps(vec1, vec2, cmp_results[0]);  // Use UNORDERED result
    blended = _mm_blendv_ps(blended, vec3, cmp_results[1]);      // Use ORDERED result
    
    // Extract mask and use in control flow
    int mask0 = _mm_movemask_ps(cmp_results[0]);
    int mask1 = _mm_movemask_ps(cmp_results[1]);
    
    // Force conditional execution based on comparison results
    if (mask0 & 0x1) {
        accum = _mm_add_ps(accum, _mm_set1_ps(10.0f));
    }
    if (mask1 & 0x2) {
        accum = _mm_sub_ps(accum, _mm_set1_ps(5.0f));
    }
    
    // Store final result
    _mm_storeu_ps(result, accum);
}

// Double precision version
void test_sse_double_comparisons(double* result) {
    __m128d vec1 = _mm_setr_pd(1.0, NAN);
    __m128d vec2 = _mm_setr_pd(2.0, 2.0);
    __m128d vec3 = _mm_setr_pd(0.0, INFINITY);
    
    __m128d cmp_results[8];
    
    // Test with double precision comparisons
    cmp_results[0] = _mm_cmp_pd(vec1, vec2, _CMP_UNORD_Q);
    cmp_results[1] = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    cmp_results[2] = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm_cmp_pd(vec1, vec3, _CMP_NGE_UQ);
    cmp_results[4] = _mm_cmp_pd(vec1, vec3, _CMP_NGT_UQ);
    cmp_results[5] = _mm_cmp_pd(vec3, vec1, _CMP_ULE_UQ);
    cmp_results[6] = _mm_cmp_pd(vec3, vec1, _CMP_ULT_UQ);
    cmp_results[7] = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    __m128d accum = _mm_setzero_pd();
    for (int i = 0; i < 8; i++) {
        __m128d mask_double = _mm_and_pd(cmp_results[i], _mm_set1_pd(1.0));
        accum = _mm_add_pd(accum, mask_double);
    }
    
    _mm_storeu_pd(result, accum);
}

#ifdef __AVX__
// AVX versions for 256-bit vectors
void test_avx_comparisons(float* result) {
    __m256 vec1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec2 = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 5.0f, 5.0f, NAN, 8.0f);
    
    __m256 cmp_results[8];
    
    // Test AVX comparisons
    cmp_results[0] = _mm256_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    __m256 accum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        __m256 mask_float = _mm256_and_ps(cmp_results[i], _mm256_set1_ps(1.0f));
        accum = _mm256_add_ps(accum, mask_float);
    }
    
    // Complex expression with blending
    __m256 temp = _mm256_blendv_ps(vec1, vec2, cmp_results[0]);
    temp = _mm256_add_ps(temp, _mm256_mul_ps(accum, _mm256_set1_ps(0.5f)));
    
    _mm256_storeu_ps(result, temp);
}

void test_avx_double_comparisons(double* result) {
    __m256d vec1 = _mm256_setr_pd(1.0, NAN, 3.0, INFINITY);
    __m256d vec2 = _mm256_setr_pd(2.0, 2.0, 3.0, 4.0);
    
    __m256d cmp_results[8];
    
    cmp_results[0] = _mm256_cmp_pd(vec1, vec2, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    __m256d accum = _mm256_setzero_pd();
    for (int i = 0; i < 8; i++) {
        __m256d mask_double = _mm256_and_pd(cmp_results[i], _mm256_set1_pd(1.0));
        accum = _mm256_add_pd(accum, mask_double);
    }
    
    _mm256_storeu_pd(result, accum);
}
#endif

// Scalar comparisons to test _mm_cmp_ss and _mm_cmp_sd
void test_scalar_comparisons(float* f_result, double* d_result) {
    __m128 s1 = _mm_set_ss(1.0f);
    __m128 s2 = _mm_set_ss(NAN);
    __m128 s3 = _mm_set_ss(2.0f);
    
    // Test scalar single-precision comparisons
    __m128 s_cmp_unord = _mm_cmp_ss(s1, s2, _CMP_UNORD_Q);
    __m128 s_cmp_ord = _mm_cmp_ss(s1, s3, _CMP_ORD_Q);
    __m128 s_cmp_uneq = _mm_cmp_ss(s1, s3, _CMP_UNEQ_UQ);
    
    // Combine results
    __m128 s_result = _mm_add_ss(s_cmp_unord, s_cmp_ord);
    s_result = _mm_add_ss(s_result, s_cmp_uneq);
    
    *f_result = _mm_cvtss_f32(s_result);
    
    // Test scalar double-precision comparisons
    __m128d d1 = _mm_set_sd(1.0);
    __m128d d2 = _mm_set_sd(NAN);
    __m128d d3 = _mm_set_sd(2.0);
    
    __m128d d_cmp_unord = _mm_cmp_sd(d1, d2, _CMP_UNORD_Q);
    __m128d d_cmp_ord = _mm_cmp_sd(d1, d3, _CMP_ORD_Q);
    __m128d d_cmp_nlt = _mm_cmp_sd(d1, d3, _CMP_NGE_UQ);  // UNGE -> nlt
    
    __m128d d_result_vec = _mm_add_sd(d_cmp_unord, d_cmp_ord);
    d_result_vec = _mm_add_sd(d_result_vec, d_cmp_nlt);
    
    *d_result = _mm_cvtsd_f64(d_result_vec);
}

// Inline assembly to force assembly output generation
void force_asm_output(void) {
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 result;
    
    // Use inline assembly with vector comparison
    __asm__ __volatile__ (
        "vcmpps %0, %1, %2, %3\n\t"
        : "=x"(result)
        : "x"(v1), "x"(v2), "i"(_CMP_UNORD_Q)
        : 
    );
    
    // Prevent optimization
    float temp[4];
    _mm_storeu_ps(temp, result);
    printf("ASM result: %f\n", temp[0]);
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
    
    // Force assembly output
    force_asm_output();
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        float avx_results[8];
        double avx_d_results[4];
        
        test_avx_comparisons(avx_results);
        test_avx_double_comparisons(avx_d_results);
        
        // Use results to prevent dead code elimination
        printf("AVX float result[0]: %f\n", avx_results[0]);
        printf("AVX double result[0]: %f\n", avx_d_results[0]);
    }
#endif
    
    // Use all results to prevent optimization
    printf("SSE float results: %f %f %f %f\n", 
           sse_results[0], sse_results[1], sse_results[2], sse_results[3]);
    printf("SSE double results: %f %f\n", sse_d_results[0], sse_d_results[1]);
    printf("Scalar float result: %f\n", scalar_f_result);
    printf("Scalar double result: %f\n", scalar_d_result);
    
    return 0;
}
