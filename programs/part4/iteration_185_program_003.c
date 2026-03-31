#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>

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
    __m128 result = _mm_setzero_ps();
    
    // Test all condition codes from the uncovered block
    // Using volatile to prevent optimization
    volatile __m128 cmp_result;
    
    // 1. UNORDERED (handles NaN comparisons)
    cmp_result = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(1.0f)));
    
    // 2. ORDERED
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(2.0f)));
    
    // 3. UNEQ (unordered or equal)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(3.0f)));
    
    // 4. UNGE (not less than, unordered)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(4.0f)));
    
    // 5. UNGT (not less than or equal, unordered)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(5.0f)));
    
    // 6. UNLE (unordered or less than or equal)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(6.0f)));
    
    // 7. UNLT (unordered or less than)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(7.0f)));
    
    // 8. LTGT (not equal, unordered)
    cmp_result = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    result = _mm_add_ps(result, _mm_and_ps(cmp_result, _mm_set1_ps(8.0f)));
    
    // Also test scalar versions to cover different code paths
    __m128 scalar_results = _mm_setzero_ps();
    
    // Scalar unordered comparison (important for NaN handling)
    scalar_results = _mm_move_ss(scalar_results, 
        _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q));
    
    // Scalar ordered comparison
    scalar_results = _mm_move_ss(scalar_results,
        _mm_cmp_ss(vec1, vec2, _CMP_ORD_Q));
    
    // Extract final result to prevent dead code elimination
    float res_array[4];
    _mm_storeu_ps(res_array, result);
    float scalar_array[4];
    _mm_storeu_ps(scalar_array, scalar_results);
    
    return res_array[0] + res_array[1] + res_array[2] + res_array[3] +
           scalar_array[0] + scalar_array[1];
}

// Double precision version
double test_sse2_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    __m128d result = _mm_setzero_pd();
    
    // Test double precision comparisons with all condition codes
    volatile __m128d cmp_result;
    
    cmp_result = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(1.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(2.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(3.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(4.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(5.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(6.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(7.0)));
    
    cmp_result = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    result = _mm_add_pd(result, _mm_and_pd(cmp_result, _mm_set1_pd(8.0)));
    
    // Scalar double comparisons
    __m128d scalar_result = _mm_setzero_pd();
    scalar_result = _mm_move_sd(scalar_result,
        _mm_cmp_sd(vec1, vec_nan, _CMP_UNORD_Q));
    scalar_result = _mm_move_sd(scalar_result,
        _mm_cmp_sd(vec1, vec2, _CMP_ORD_Q));
    
    double res_array[2];
    _mm_storeu_pd(res_array, result);
    double scalar_array[2];
    _mm_storeu_pd(scalar_array, scalar_result);
    
    return res_array[0] + res_array[1] + scalar_array[0] + scalar_array[1];
}

#ifdef __AVX__
// AVX 256-bit version
float test_avx_comparisons(float a, float b, float c, float d,
                          float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 result = _mm256_setzero_ps();
    
    // AVX comparisons - these will generate different assembly
    volatile __m256 cmp_result;
    
    cmp_result = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(1.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(2.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(3.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(4.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(5.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(6.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(7.0f)));
    
    cmp_result = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    result = _mm256_add_ps(result, _mm256_and_ps(cmp_result, _mm256_set1_ps(8.0f)));
    
    // Use movemask to create control flow dependency
    int mask = _mm256_movemask_ps(result);
    
    float res_array[8];
    _mm256_storeu_ps(res_array, result);
    
    float sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += res_array[i];
    }
    
    // Control flow based on comparison results
    if (mask & 1) sum *= 1.1f;
    if (mask & 2) sum *= 0.9f;
    if (mask & 4) sum += 1.0f;
    if (mask & 8) sum -= 1.0f;
    
    return sum;
}
#endif

// Complex expression mixing comparisons and arithmetic
float complex_vector_expression(float a, float b, float c, float d) {
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v_nan = _mm_set1_ps(NAN);
    __m128 v_zero = _mm_setzero_ps();
    
    // Blend based on comparison results
    __m128 cmp_unord = _mm_cmp_ps(v1, v_nan, _CMP_UNORD_Q);
    __m128 blend1 = _mm_blendv_ps(v1, v2, cmp_unord);
    
    __m128 cmp_ord = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);
    __m128 blend2 = _mm_blendv_ps(blend1, v_zero, cmp_ord);
    
    __m128 cmp_uneq = _mm_cmp_ps(v1, v2, _CMP_UNEQ_UQ);
    __m128 result = _mm_add_ps(blend2, _mm_and_ps(cmp_uneq, _mm_set1_ps(10.0f)));
    
    // More comparisons in arithmetic context
    __m128 cmp_nlt = _mm_cmp_ps(v1, v2, _CMP_NGE_UQ);  // UNGE -> nlt
    result = _mm_mul_ps(result, _mm_or_ps(_mm_set1_ps(1.0f), cmp_nlt));
    
    __m128 cmp_nle = _mm_cmp_ps(v1, v2, _CMP_NGT_UQ);  // UNGT -> nle
    result = _mm_sub_ps(result, _mm_and_ps(cmp_nle, _mm_set1_ps(5.0f)));
    
    __m128 cmp_ule = _mm_cmp_ps(v1, v2, _CMP_ULE_UQ);
    __m128 cmp_ult = _mm_cmp_ps(v1, v2, _CMP_ULT_UQ);
    __m128 cmp_une = _mm_cmp_ps(v1, v2, _CMP_NEQ_UQ);  // LTGT -> une
    
    // Complex final expression using all comparison results
    __m128 final = _mm_add_ps(
        _mm_add_ps(cmp_ule, cmp_ult),
        _mm_sub_ps(cmp_une, result)
    );
    
    float res[4];
    _mm_storeu_ps(res, final);
    return res[0] + res[1] + res[2] + res[3];
}

int main() {
    // Test data including normal numbers, zeros, infinities, and NaN
    float test_floats[] = {1.0f, -2.0f, 0.0f, INFINITY, -INFINITY, NAN, 3.14f, -3.14f};
    double test_doubles[] = {1.0, -2.0, 0.0, INFINITY, -INFINITY, NAN, 3.14159, -3.14159};
    
    float float_result = 0.0f;
    double double_result = 0.0;
    
    printf("Testing SSE comparisons...\n");
    
    // Multiple test cases to ensure coverage
    for (int i = 0; i < 4; i++) {
        float_result += test_sse_comparisons(
            test_floats[i], test_floats[i+1], test_floats[i+2], test_floats[i+3]
        );
        
        double_result += test_sse2_comparisons(
            test_doubles[i], test_doubles[i+1]
        );
        
        float_result += complex_vector_expression(
            test_floats[i], test_floats[i+1], test_floats[i+2], test_floats[i+3]
        );
    }
    
#ifdef __AVX__
    printf("Testing AVX comparisons...\n");
    
    // Test AVX if available
    for (int i = 0; i < 2; i++) {
        float_result += test_avx_comparisons(
            test_floats[0], test_floats[1], test_floats[2], test_floats[3],
            test_floats[4], test_floats[5], test_floats[6], test_floats[7]
        );
    }
#endif
    
    // Force assembly output with inline assembly referencing vector operations
    __m128 asm_vec = _mm_set1_ps(float_result);
    float asm_float;
    
    // This inline assembly forces GCC to generate assembly output
    // for vector operations with condition codes
    __asm__ __volatile__ (
        "vmovups %1, %%xmm0\n\t"
        "vaddps %%xmm0, %%xmm0, %%xmm0\n\t"
        "vmovups %%xmm0, %0\n\t"
        : "=m" (asm_float)
        : "m" (asm_vec)
        : "xmm0"
    );
    
    printf("Final results: float=%f, double=%f, asm=%f\n", 
           float_result, double_result, asm_float);
    
    return 0;
}
