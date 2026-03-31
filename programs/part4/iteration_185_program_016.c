#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Function to test SSE comparisons with all condition codes
float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    
    // Results accumulator
    __m128 result = _mm_setzero_ps();
    
    // Test all condition codes from the uncovered block
    // Each comparison uses a different condition code
    
    // 1. UNORDERED (handles NaN comparisons)
    __m128 cmp_unord = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm_add_ps(result, _mm_and_ps(cmp_unord, vec1));
    
    // 2. ORDERED
    __m128 cmp_ord = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    result = _mm_add_ps(result, _mm_and_ps(cmp_ord, vec2));
    
    // 3. UNEQ (unordered or equal)
    __m128 cmp_uneq = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm_sub_ps(result, _mm_and_ps(cmp_uneq, vec1));
    
    // 4. UNGE (not less than, unordered)
    __m128 cmp_unge = _mm_cmp_ps(vec1, vec_inf, _CMP_NGE_UQ);
    result = _mm_mul_ps(result, _mm_add_ps(_mm_and_ps(cmp_unge, vec_inf), _mm_set1_ps(1.0f)));
    
    // 5. UNGT (not less than or equal, unordered)
    __m128 cmp_ungt = _mm_cmp_ps(vec_inf, vec1, _CMP_NGT_UQ);
    result = _mm_div_ps(result, _mm_add_ps(_mm_and_ps(cmp_ungt, vec2), _mm_set1_ps(1.0f)));
    
    // 6. UNLE (unordered or less than or equal)
    __m128 cmp_unle = _mm_cmp_ps(vec2, vec1, _CMP_ULE_UQ);
    result = _mm_add_ps(result, _mm_andnot_ps(cmp_unle, vec1));
    
    // 7. UNLT (unordered or less than)
    __m128 cmp_unlt = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    result = _mm_sub_ps(result, _mm_andnot_ps(cmp_unlt, vec2));
    
    // 8. LTGT (less than or greater than, unordered)
    __m128 cmp_ltgt = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_ltgt, _mm_set1_ps(2.0f)));
    
    // Extract results to prevent optimization
    float res[4];
    _mm_storeu_ps(res, result);
    
    // Use comparison masks in control flow
    int mask_unord = _mm_movemask_ps(cmp_unord);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    int mask_uneq = _mm_movemask_ps(cmp_uneq);
    
    float final_result = res[0] + res[1] + res[2] + res[3];
    
    // Control flow based on comparison results
    if (mask_unord & 1) final_result *= 1.5f;
    if (mask_ord & 2) final_result += 2.5f;
    if (mask_uneq & 4) final_result /= 3.0f;
    
    return final_result;
}

// Double precision version
double test_sse2_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    __m128d result = _mm_setzero_pd();
    
    // Test with double precision
    __m128d cmp_unord = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    __m128d cmp_ord = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    __m128d cmp_uneq = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    __m128d cmp_unge = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    
    result = _mm_add_pd(result, _mm_and_pd(cmp_unord, vec1));
    result = _mm_add_pd(result, _mm_and_pd(cmp_ord, vec2));
    result = _mm_sub_pd(result, _mm_and_pd(cmp_uneq, vec1));
    result = _mm_mul_pd(result, _mm_add_pd(_mm_and_pd(cmp_unge, vec2), _mm_set1_pd(1.0)));
    
    double res[2];
    _mm_storeu_pd(res, result);
    
    return res[0] + res[1];
}

#ifdef __AVX__
// AVX version for 256-bit vectors
float test_avx_comparisons(float a, float b, float c, float d,
                          float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 result = _mm256_setzero_ps();
    
    // AVX comparisons
    __m256 cmp_unord = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    __m256 cmp_ord = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    __m256 cmp_uneq = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    __m256 cmp_unge = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    __m256 cmp_ungt = _mm256_cmp_ps(vec2, vec1, _CMP_NGT_UQ);
    __m256 cmp_unle = _mm256_cmp_ps(vec2, vec1, _CMP_ULE_UQ);
    __m256 cmp_unlt = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    __m256 cmp_ltgt = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Complex expression using all comparisons
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_unord, vec1));
    result = _mm256_sub_ps(result, _mm256_and_ps(cmp_ord, vec2));
    result = _mm256_mul_ps(result, _mm256_add_ps(_mm256_and_ps(cmp_uneq, vec1), _mm256_set1_ps(1.0f)));
    result = _mm256_div_ps(result, _mm256_add_ps(_mm256_and_ps(cmp_unge, vec2), _mm256_set1_ps(1.0f)));
    result = _mm256_add_ps(result, _mm256_andnot_ps(cmp_ungt, vec1));
    result = _mm256_sub_ps(result, _mm256_andnot_ps(cmp_unle, vec2));
    result = _mm256_mul_ps(result, _mm256_add_ps(_mm256_and_ps(cmp_unlt, vec1), _mm256_set1_ps(1.0f)));
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_ltgt, _mm256_set1_ps(3.0f)));
    
    // Blend operations based on comparison results
    __m256 blended = _mm256_blendv_ps(vec1, vec2, cmp_unord);
    result = _mm256_add_ps(result, blended);
    
    float res[8];
    _mm256_storeu_ps(res, result);
    
    // Extract masks for control flow
    int mask = _mm256_movemask_ps(cmp_unord) | _mm256_movemask_ps(cmp_ord);
    
    float final = 0;
    for (int i = 0; i < 8; i++) {
        final += res[i];
        if (mask & (1 << i)) {
            final *= 1.1f;
        }
    }
    
    return final;
}
#endif

// Scalar comparisons to test _mm_cmp_ss and _mm_cmp_sd
float test_scalar_comparisons(float a, float b) {
    __m128 s1 = _mm_set_ss(a);
    __m128 s2 = _mm_set_ss(b);
    __m128 s_nan = _mm_set_ss(NAN);
    
    // Scalar comparisons with different condition codes
    __m128 cmp1 = _mm_cmp_ss(s1, s_nan, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ss(s1, s2, _CMP_ORD_Q);
    __m128 cmp3 = _mm_cmp_ss(s1, s2, _CMP_UNEQ_UQ);
    __m128 cmp4 = _mm_cmp_ss(s2, s1, _CMP_NGE_UQ);
    
    // Combine results
    __m128 result = _mm_add_ss(cmp1, cmp2);
    result = _mm_sub_ss(result, cmp3);
    result = _mm_mul_ss(result, cmp4);
    
    float res;
    _mm_store_ss(&res, result);
    
    return res;
}

int main() {
    // Test data including normal numbers, zeros, infinities, and NaN
    float test_floats[] = {1.0f, 2.0f, 0.0f, -1.0f, INFINITY, -INFINITY, NAN, 3.14f};
    double test_doubles[] = {1.0, 2.0, 0.0, -1.0, INFINITY, -INFINITY, NAN, 3.14};
    
    float float_result = 0;
    double double_result = 0;
    
    // Test SSE comparisons
    for (int i = 0; i < 4; i++) {
        float_result += test_sse_comparisons(
            test_floats[i], test_floats[i+1], test_floats[i+2], test_floats[i+3]
        );
        
        double_result += test_sse2_comparisons(
            test_doubles[i], test_doubles[i+1]
        );
        
        float_result += test_scalar_comparisons(
            test_floats[i], test_floats[i+1]
        );
    }
    
#ifdef __AVX__
    // Test AVX comparisons if available
    float avx_result = test_avx_comparisons(
        test_floats[0], test_floats[1], test_floats[2], test_floats[3],
        test_floats[4], test_floats[5], test_floats[6], test_floats[7]
    );
    float_result += avx_result;
#endif
    
    // Force assembly output with inline assembly
    __m128 vec = _mm_set1_ps(float_result);
    __m128 cmp_test;
    
    // Generate assembly for each condition code
    asm volatile (
        "vcmpeqps %1, %0, %0\n\t"
        "vcmpltps %1, %0, %0\n\t"
        : "=x"(cmp_test)
        : "x"(vec)
        : "cc"
    );
    
    // Print results to prevent optimization
    printf("Float result: %f\n", float_result);
    printf("Double result: %f\n", double_result);
    
    return 0;
}
