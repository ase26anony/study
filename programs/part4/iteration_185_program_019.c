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

// Function to test SSE comparisons with all condition codes
float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    
    // Store comparison results to prevent optimization
    __m128 cmp_results[8];
    int mask_results[8];
    
    // Test all condition codes from the uncovered block
    // Using explicit condition codes to trigger the printing logic
    
    // 1. UNORDERED - _CMP_UNORD_Q
    cmp_results[0] = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    mask_results[0] = _mm_movemask_ps(cmp_results[0]);
    
    // 2. ORDERED - _CMP_ORD_Q
    cmp_results[1] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    mask_results[1] = _mm_movemask_ps(cmp_results[1]);
    
    // 3. UNEQ - _CMP_UNEQ_UQ
    cmp_results[2] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    mask_results[2] = _mm_movemask_ps(cmp_results[2]);
    
    // 4. UNGE - _CMP_NGE_UQ
    cmp_results[3] = _mm_cmp_ps(vec1, vec_inf, _CMP_NGE_UQ);
    mask_results[3] = _mm_movemask_ps(cmp_results[3]);
    
    // 5. UNGT - _CMP_NGT_UQ
    cmp_results[4] = _mm_cmp_ps(vec2, vec_inf, _CMP_NGT_UQ);
    mask_results[4] = _mm_movemask_ps(cmp_results[4]);
    
    // 6. UNLE - _CMP_ULE_UQ
    cmp_results[5] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    mask_results[5] = _mm_movemask_ps(cmp_results[5]);
    
    // 7. UNLT - _CMP_ULT_UQ
    cmp_results[6] = _mm_cmp_ps(vec2, vec1, _CMP_ULT_UQ);
    mask_results[6] = _mm_movemask_ps(cmp_results[6]);
    
    // 8. LTGT - _CMP_NEQ_UQ
    cmp_results[7] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    mask_results[7] = _mm_movemask_ps(cmp_results[7]);
    
    // Use results in arithmetic to prevent dead code elimination
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Blend based on comparison results
        __m128 blended = _mm_blendv_ps(vec1, vec2, cmp_results[i]);
        sum = _mm_add_ps(sum, blended);
    }
    
    // Extract final result
    float result_array[4];
    _mm_storeu_ps(result_array, sum);
    
    // Use mask results in conditional logic
    int final_mask = 0;
    for (int i = 0; i < 8; i++) {
        if (mask_results[i] != 0) {
            final_mask |= (1 << i);
        }
    }
    
    return result_array[0] + result_array[1] + result_array[2] + result_array[3] + final_mask;
}

// Double precision version
double test_sse2_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    // Test double precision comparisons
    __m128d cmp_d_results[8];
    int mask_d_results[8];
    
    // Double precision condition codes
    cmp_d_results[0] = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    mask_d_results[0] = _mm_movemask_pd(cmp_d_results[0]);
    
    cmp_d_results[1] = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    mask_d_results[1] = _mm_movemask_pd(cmp_d_results[1]);
    
    cmp_d_results[2] = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    mask_d_results[2] = _mm_movemask_pd(cmp_d_results[2]);
    
    cmp_d_results[3] = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    mask_d_results[3] = _mm_movemask_pd(cmp_d_results[3]);
    
    cmp_d_results[4] = _mm_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    mask_d_results[4] = _mm_movemask_pd(cmp_d_results[4]);
    
    cmp_d_results[5] = _mm_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    mask_d_results[5] = _mm_movemask_pd(cmp_d_results[5]);
    
    cmp_d_results[6] = _mm_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    mask_d_results[6] = _mm_movemask_pd(cmp_d_results[6]);
    
    cmp_d_results[7] = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    mask_d_results[7] = _mm_movemask_pd(cmp_d_results[7]);
    
    // Complex expression with blending
    __m128d result = _mm_setzero_pd();
    for (int i = 0; i < 8; i++) {
        __m128d temp = _mm_blendv_pd(vec1, vec2, cmp_d_results[i]);
        result = _mm_add_pd(result, _mm_mul_pd(temp, _mm_set1_pd(i + 1)));
    }
    
    double res_array[2];
    _mm_storeu_pd(res_array, result);
    
    return res_array[0] + res_array[1];
}

#ifdef __AVX__
// AVX version for 256-bit vectors
float test_avx_comparisons(float a, float b, float c, float d,
                          float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 avx_cmp_results[8];
    int avx_masks[8];
    
    // AVX comparisons
    avx_cmp_results[0] = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    avx_masks[0] = _mm256_movemask_ps(avx_cmp_results[0]);
    
    avx_cmp_results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    avx_masks[1] = _mm256_movemask_ps(avx_cmp_results[1]);
    
    avx_cmp_results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    avx_masks[2] = _mm256_movemask_ps(avx_cmp_results[2]);
    
    avx_cmp_results[3] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    avx_masks[3] = _mm256_movemask_ps(avx_cmp_results[3]);
    
    avx_cmp_results[4] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    avx_masks[4] = _mm256_movemask_ps(avx_cmp_results[4]);
    
    avx_cmp_results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    avx_masks[5] = _mm256_movemask_ps(avx_cmp_results[5]);
    
    avx_cmp_results[6] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    avx_masks[6] = _mm256_movemask_ps(avx_cmp_results[6]);
    
    avx_cmp_results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    avx_masks[7] = _mm256_movemask_ps(avx_cmp_results[7]);
    
    // Complex AVX arithmetic with comparison results
    __m256 sum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Conditional selection based on comparison
        __m256 selected = _mm256_blendv_ps(vec1, vec2, avx_cmp_results[i]);
        sum = _mm256_add_ps(sum, _mm256_mul_ps(selected, _mm256_set1_ps(0.5f * i)));
    }
    
    float avx_result[8];
    _mm256_storeu_ps(avx_result, sum);
    
    float total = 0;
    for (int i = 0; i < 8; i++) {
        total += avx_result[i];
    }
    
    return total;
}
#endif

// Scalar comparisons to test _mm_cmp_ss and _mm_cmp_sd
float test_scalar_comparisons(float a, float b) {
    __m128 s1 = _mm_set_ss(a);
    __m128 s2 = _mm_set_ss(b);
    __m128 s_nan = _mm_set_ss(NAN);
    
    // Test scalar single-precision comparisons
    __m128 scalar_cmp[4];
    
    scalar_cmp[0] = _mm_cmp_ss(s1, s_nan, _CMP_UNORD_Q);
    scalar_cmp[1] = _mm_cmp_ss(s1, s2, _CMP_UNEQ_UQ);
    scalar_cmp[2] = _mm_cmp_ss(s1, s2, _CMP_NGE_UQ);
    scalar_cmp[3] = _mm_cmp_ss(s1, s2, _CMP_NEQ_UQ);
    
    // Use results in conditional computation
    float result = 0;
    for (int i = 0; i < 4; i++) {
        float cmp_val;
        _mm_store_ss(&cmp_val, scalar_cmp[i]);
        if (!isnan(cmp_val) && cmp_val != 0) {
            result += a * (i + 1);
        } else {
            result += b * (i + 1);
        }
    }
    
    return result;
}

// Inline assembly to force assembly output with condition codes
void generate_asm_with_condition_codes(float* a, float* b, float* result) {
    __m128 vec_a = _mm_loadu_ps(a);
    __m128 vec_b = _mm_loadu_ps(b);
    __m128 cmp_result;
    
    // Use inline assembly to ensure condition codes appear in assembly output
    __asm__ __volatile__ (
        "vcmpps %[cmp], %[a], %[b], %{unord%}\n\t"
        "vmovups %[cmp], %[res]"
        : [res] "=m" (*(__m128*)result)
        : [a] "x" (vec_a), [b] "x" (vec_b), [cmp] "x" (_mm_setzero_ps())
        : "memory"
    );
}

int main() {
    // Initialize test data with various values including NaN and Inf
    float test_floats[] = {1.0f, 2.0f, 0.0f, -1.0f, NAN, INFINITY, -INFINITY, 3.14f};
    double test_doubles[] = {1.0, 2.0, 0.0, -1.0, NAN, INFINITY, -INFINITY, 3.14};
    
    printf("Testing SSE comparisons...\n");
    float sse_result = 0;
    for (int i = 0; i < 4; i++) {
        sse_result += test_sse_comparisons(
            test_floats[i], 
            test_floats[i+1], 
            test_floats[i+2], 
            test_floats[i+3]
        );
    }
    printf("SSE result: %f\n", sse_result);
    
    printf("Testing SSE2 double comparisons...\n");
    double sse2_result = 0;
    for (int i = 0; i < 4; i++) {
        sse2_result += test_sse2_comparisons(
            test_doubles[i],
            test_doubles[i+1]
        );
    }
    printf("SSE2 result: %f\n", sse2_result);
    
    printf("Testing scalar comparisons...\n");
    float scalar_result = 0;
    for (int i = 0; i < 8; i += 2) {
        scalar_result += test_scalar_comparisons(
            test_floats[i],
            test_floats[i+1]
        );
    }
    printf("Scalar result: %f\n", scalar_result);
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        printf("Testing AVX comparisons...\n");
        float avx_result = test_avx_comparisons(
            test_floats[0], test_floats[1], test_floats[2], test_floats[3],
            test_floats[4], test_floats[5], test_floats[6], test_floats[7]
        );
        printf("AVX result: %f\n", avx_result);
    } else {
        printf("AVX not supported on this CPU\n");
    }
#endif
    
    // Force assembly generation with inline asm
    float asm_input[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    float asm_output[4];
    generate_asm_with_condition_codes(asm_input, asm_input, asm_output);
    
    printf("Final check value: %f\n", asm_output[0]);
    
    return 0;
}
