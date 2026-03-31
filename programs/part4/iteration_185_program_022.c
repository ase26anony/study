#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#ifdef __AVX__
#include <avxintrin.h>
#endif

// Function to test SSE comparisons with all condition codes
float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    
    // Results accumulator
    __m128 results = _mm_setzero_ps();
    
    // Test all condition codes from the uncovered block
    // Each comparison uses a different condition code
    
    // 1. UNORDERED (handles NaN comparisons)
    __m128 cmp_unord = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    results = _mm_add_ps(results, _mm_and_ps(cmp_unord, _mm_set1_ps(1.0f)));
    
    // 2. ORDERED
    __m128 cmp_ord = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    results = _mm_add_ps(results, _mm_and_ps(cmp_ord, _mm_set1_ps(2.0f)));
    
    // 3. UNEQ (unordered or equal)
    __m128 cmp_uneq = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_uneq, _mm_set1_ps(3.0f)));
    
    // 4. UNGE (not less than, unordered)
    __m128 cmp_unge = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_unge, _mm_set1_ps(4.0f)));
    
    // 5. UNGT (not less than or equal, unordered)
    __m128 cmp_ungt = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_ungt, _mm_set1_ps(5.0f)));
    
    // 6. UNLE (unordered or less than or equal)
    __m128 cmp_unle = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_unle, _mm_set1_ps(6.0f)));
    
    // 7. UNLT (unordered or less than)
    __m128 cmp_unlt = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_unlt, _mm_set1_ps(7.0f)));
    
    // 8. LTGT (not equal, unordered)
    __m128 cmp_ltgt = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_ltgt, _mm_set1_ps(8.0f)));
    
    // Extract results to prevent optimization
    float res_array[4];
    _mm_storeu_ps(res_array, results);
    
    // Use inline assembly to force condition code printing
    // This ensures the output_operand function is called
    __asm__ __volatile__ (
        "# SSE comparison results\n"
        : 
        : "x" (cmp_unord), "x" (cmp_ord), "x" (cmp_uneq), 
          "x" (cmp_unge), "x" (cmp_ungt), "x" (cmp_unle),
          "x" (cmp_unlt), "x" (cmp_ltgt)
    );
    
    return res_array[0] + res_array[1] + res_array[2] + res_array[3];
}

// Double precision version
double test_sse_double_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    __m128d results = _mm_setzero_pd();
    
    // Test with double precision
    __m128d cmp_unord_d = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    __m128d cmp_ord_d = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    __m128d cmp_uneq_d = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    __m128d cmp_unge_d = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    __m128d cmp_ungt_d = _mm_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    __m128d cmp_unle_d = _mm_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    __m128d cmp_unlt_d = _mm_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    __m128d cmp_ltgt_d = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    results = _mm_add_pd(results, _mm_and_pd(cmp_unord_d, _mm_set1_pd(1.0)));
    results = _mm_add_pd(results, _mm_and_pd(cmp_ord_d, _mm_set1_pd(2.0)));
    results = _mm_add_pd(results, _mm_and_pd(cmp_uneq_d, _mm_set1_pd(3.0)));
    results = _mm_add_pd(results, _mm_and_pd(cmp_unge_d, _mm_set1_pd(4.0)));
    results = _mm_add_pd(results, _mm_and_pd(cmp_ungt_d, _mm_set1_pd(5.0)));
    results = _mm_add_pd(results, _mm_and_pd(cmp_unle_d, _mm_set1_pd(6.0)));
    results = _mm_add_pd(results, _mm_and_pd(cmp_unlt_d, _mm_set1_pd(7.0)));
    results = _mm_add_pd(results, _mm_and_pd(cmp_ltgt_d, _mm_set1_pd(8.0)));
    
    double res_array[2];
    _mm_storeu_pd(res_array, results);
    
    __asm__ __volatile__ (
        "# SSE double comparison results\n"
        : 
        : "x" (cmp_unord_d), "x" (cmp_ord_d), "x" (cmp_uneq_d)
    );
    
    return res_array[0] + res_array[1];
}

#ifdef __AVX__
// AVX version for 256-bit vectors
float test_avx_comparisons(float a, float b, float c, float d,
                          float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 results = _mm256_setzero_ps();
    
    // AVX comparisons with all condition codes
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    __m256 cmp_uneq_avx = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    __m256 cmp_unge_avx = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    __m256 cmp_ungt_avx = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    __m256 cmp_unle_avx = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    __m256 cmp_unlt_avx = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    __m256 cmp_ltgt_avx = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_unord_avx, _mm256_set1_ps(1.0f)));
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_ord_avx, _mm256_set1_ps(2.0f)));
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_uneq_avx, _mm256_set1_ps(3.0f)));
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_unge_avx, _mm256_set1_ps(4.0f)));
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_ungt_avx, _mm256_set1_ps(5.0f)));
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_unle_avx, _mm256_set1_ps(6.0f)));
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_unlt_avx, _mm256_set1_ps(7.0f)));
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_ltgt_avx, _mm256_set1_ps(8.0f)));
    
    float res_array[8];
    _mm256_storeu_ps(res_array, results);
    
    __asm__ __volatile__ (
        "# AVX comparison results\n"
        : 
        : "x" (cmp_unord_avx), "x" (cmp_ord_avx), "x" (cmp_uneq_avx)
    );
    
    float sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += res_array[i];
    }
    return sum;
}

// AVX double precision
double test_avx_double_comparisons(double a, double b, double c, double d) {
    __m256d vec1 = _mm256_set_pd(a, b, c, d);
    __m256d vec2 = _mm256_set_pd(d, c, b, a);
    __m256d vec_nan = _mm256_set1_pd(NAN);
    
    __m256d results = _mm256_setzero_pd();
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    __m256d cmp_uneq_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    __m256d cmp_unge_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    __m256d cmp_ungt_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    __m256d cmp_unle_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    __m256d cmp_unlt_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    __m256d cmp_ltgt_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_unord_avx_d, _mm256_set1_pd(1.0)));
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_ord_avx_d, _mm256_set1_pd(2.0)));
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_uneq_avx_d, _mm256_set1_pd(3.0)));
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_unge_avx_d, _mm256_set1_pd(4.0)));
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_ungt_avx_d, _mm256_set1_pd(5.0)));
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_unle_avx_d, _mm256_set1_pd(6.0)));
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_unlt_avx_d, _mm256_set1_pd(7.0)));
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_ltgt_avx_d, _mm256_set1_pd(8.0)));
    
    double res_array[4];
    _mm256_storeu_pd(res_array, results);
    
    __asm__ __volatile__ (
        "# AVX double comparison results\n"
        : 
        : "x" (cmp_unord_avx_d), "x" (cmp_ord_avx_d)
    );
    
    return res_array[0] + res_array[1] + res_array[2] + res_array[3];
}
#endif

// Test scalar comparisons as well
float test_scalar_comparisons(float a, float b) {
    __m128 vec1 = _mm_set_ss(a);
    __m128 vec2 = _mm_set_ss(b);
    __m128 vec_nan = _mm_set_ss(NAN);
    
    float result = 0;
    
    // Scalar comparisons
    __m128 cmp_unord_ss = _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q);
    __m128 cmp_ord_ss = _mm_cmp_ss(vec1, vec2, _CMP_ORD_Q);
    __m128 cmp_uneq_ss = _mm_cmp_ss(vec1, vec2, _CMP_UNEQ_UQ);
    __m128 cmp_unge_ss = _mm_cmp_ss(vec1, vec2, _CMP_NGE_UQ);
    __m128 cmp_ungt_ss = _mm_cmp_ss(vec1, vec2, _CMP_NGT_UQ);
    __m128 cmp_unle_ss = _mm_cmp_ss(vec1, vec2, _CMP_ULE_UQ);
    __m128 cmp_unlt_ss = _mm_cmp_ss(vec1, vec2, _CMP_ULT_UQ);
    __m128 cmp_ltgt_ss = _mm_cmp_ss(vec1, vec2, _CMP_NEQ_UQ);
    
    // Extract masks and use in control flow
    int mask_unord = _mm_movemask_ps(cmp_unord_ss);
    int mask_ord = _mm_movemask_ps(cmp_ord_ss);
    int mask_uneq = _mm_movemask_ps(cmp_uneq_ss);
    
    // Control flow based on comparison results
    if (mask_unord) result += 1.0f;
    if (mask_ord) result += 2.0f;
    if (mask_uneq) result += 3.0f;
    
    __asm__ __volatile__ (
        "# Scalar comparison results\n"
        : 
        : "x" (cmp_unord_ss), "x" (cmp_ord_ss), "x" (cmp_uneq_ss),
          "x" (cmp_unge_ss), "x" (cmp_ungt_ss), "x" (cmp_unle_ss),
          "x" (cmp_unlt_ss), "x" (cmp_ltgt_ss)
    );
    
    return result;
}

int main() {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Test with various values including NaN and INFINITY
    float result1 = test_sse_comparisons(1.0f, 2.0f, NAN, INFINITY);
    printf("SSE float result: %f\n", result1);
    
    double result2 = test_sse_double_comparisons(3.0, NAN);
    printf("SSE double result: %f\n", result2);
    
    float result3 = test_scalar_comparisons(5.0f, NAN);
    printf("Scalar float result: %f\n", result3);
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported, testing AVX comparisons...\n");
        float result4 = test_avx_comparisons(1.0f, 2.0f, 3.0f, 4.0f,
                                           5.0f, 6.0f, NAN, INFINITY);
        printf("AVX float result: %f\n", result4);
        
        double result5 = test_avx_double_comparisons(1.0, 2.0, NAN, INFINITY);
        printf("AVX double result: %f\n", result5);
    } else {
        printf("AVX not supported on this CPU\n");
    }
#endif
    
    // Complex expression mixing comparisons and arithmetic
    __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 v_nan = _mm_set1_ps(NAN);
    
    // Blend based on comparison results
    __m128 cmp1 = _mm_cmp_ps(v1, v_nan, _CMP_UNORD_Q);  // unord
    __m128 cmp2 = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);       // ord
    __m128 blended = _mm_blendv_ps(v1, v2, cmp1);
    blended = _mm_blendv_ps(blended, _mm_set1_ps(10.0f), cmp2);
    
    float blended_result[4];
    _mm_storeu_ps(blended_result, blended);
    printf("Blended result: %f %f %f %f\n", 
           blended_result[0], blended_result[1], 
           blended_result[2], blended_result[3]);
    
    return 0;
}
