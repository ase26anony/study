#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

// Function to test all SSE comparison condition codes
FORCE_INLINE float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    
    // Store results to prevent optimization
    __m128 results[16];
    int result_idx = 0;
    
    // Test all condition codes from the uncovered block
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);    // UNORDERED
    results[result_idx++] = _mm_cmp_ps(vec1, vec_nan, _CMP_ORD_Q);   // ORDERED
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);    // UNEQ
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);     // UNGE
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);     // UNGT
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);     // UNLE
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);     // UNLT
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);     // LTGT
    
    // Double precision comparisons
    __m128d dvec1 = _mm_set_pd(a, b);
    __m128d dvec2 = _mm_set_pd(b, a);
    __m128d dvec_nan = _mm_set1_pd(NAN);
    
    __m128d dresults[8];
    int dresult_idx = 0;
    
    dresults[dresult_idx++] = _mm_cmp_pd(dvec1, dvec_nan, _CMP_UNORD_Q);
    dresults[dresult_idx++] = _mm_cmp_pd(dvec1, dvec2, _CMP_ORD_Q);
    dresults[dresult_idx++] = _mm_cmp_pd(dvec1, dvec2, _CMP_UNEQ_UQ);
    dresults[dresult_idx++] = _mm_cmp_pd(dvec1, dvec2, _CMP_NGE_UQ);
    dresults[dresult_idx++] = _mm_cmp_pd(dvec1, dvec2, _CMP_NGT_UQ);
    dresults[dresult_idx++] = _mm_cmp_pd(dvec1, dvec2, _CMP_ULE_UQ);
    dresults[dresult_idx++] = _mm_cmp_pd(dvec1, dvec2, _CMP_ULT_UQ);
    dresults[dresult_idx++] = _mm_cmp_pd(dvec1, dvec2, _CMP_NEQ_UQ);
    
    // Scalar comparisons (single and double)
    __m128 sscalar_results[4];
    sscalar_results[0] = _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q);
    sscalar_results[1] = _mm_cmp_ss(vec1, vec2, _CMP_ORD_Q);
    sscalar_results[2] = _mm_cmp_ss(vec1, vec2, _CMP_UNEQ_UQ);
    sscalar_results[3] = _mm_cmp_ss(vec1, vec2, _CMP_NGE_UQ);
    
    __m128d dscalar_results[4];
    dscalar_results[0] = _mm_cmp_sd(dvec1, dvec_nan, _CMP_UNORD_Q);
    dscalar_results[1] = _mm_cmp_sd(dvec1, dvec2, _CMP_ORD_Q);
    dscalar_results[2] = _mm_cmp_sd(dvec1, dvec2, _CMP_UNEQ_UQ);
    dscalar_results[3] = _mm_cmp_sd(dvec1, dvec2, _CMP_NGE_UQ);
    
    // Combine results with arithmetic to create complex expressions
    __m128 final_result = _mm_setzero_ps();
    for (int i = 0; i < result_idx; i++) {
        // Convert comparison mask to float (0.0 or 1.0)
        __m128 mask_as_float = _mm_and_ps(results[i], _mm_set1_ps(1.0f));
        final_result = _mm_add_ps(final_result, mask_as_float);
    }
    
    // Use movemask to extract comparison results for conditional branching
    int mask_sum = 0;
    for (int i = 0; i < result_idx; i++) {
        mask_sum += _mm_movemask_ps(results[i]);
    }
    
    // Force assembly output with inline asm
    __m128 asm_vec = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 asm_vec2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    
    // This inline assembly will force the compiler to generate
    // assembly with the condition code strings
    __asm__ __volatile__ (
        "vcmpps %%ymm0, %%ymm1, %%ymm2, %0\n\t"
        "vcmppd %%xmm3, %%xmm4, %%xmm5, %1\n\t"
        : 
        : "i" (_CMP_UNORD_Q), "i" (_CMP_ORD_Q)
        : "ymm0", "ymm1", "ymm2", "xmm3", "xmm4", "xmm5"
    );
    
    // Extract final scalar result
    float final_array[4];
    _mm_storeu_ps(final_array, final_result);
    
    // Use comparison results in conditional to prevent dead code elimination
    if (mask_sum > 0) {
        return final_array[0] + final_array[1] + final_array[2] + final_array[3];
    }
    
    return 0.0f;
}

#ifdef __AVX__
// AVX version for 256-bit vectors
FORCE_INLINE float test_avx_comparisons(float a, float b, float c, float d) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, d, c, b, a);
    __m256 vec2 = _mm256_set_ps(d, c, b, a, a, b, c, d);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 results[8];
    int result_idx = 0;
    
    // Test AVX comparisons
    results[result_idx++] = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    results[result_idx++] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    results[result_idx++] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    results[result_idx++] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    results[result_idx++] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    results[result_idx++] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    results[result_idx++] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    results[result_idx++] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // AVX double precision
    __m256d dvec1 = _mm256_set_pd(a, b, c, d);
    __m256d dvec2 = _mm256_set_pd(d, c, b, a);
    __m256d dvec_nan = _mm256_set1_pd(NAN);
    
    __m256d dresults[8];
    int dresult_idx = 0;
    
    dresults[dresult_idx++] = _mm256_cmp_pd(dvec1, dvec_nan, _CMP_UNORD_Q);
    dresults[dresult_idx++] = _mm256_cmp_pd(dvec1, dvec2, _CMP_ORD_Q);
    dresults[dresult_idx++] = _mm256_cmp_pd(dvec1, dvec2, _CMP_UNEQ_UQ);
    dresults[dresult_idx++] = _mm256_cmp_pd(dvec1, dvec2, _CMP_NGE_UQ);
    dresults[dresult_idx++] = _mm256_cmp_pd(dvec1, dvec2, _CMP_NGT_UQ);
    dresults[dresult_idx++] = _mm256_cmp_pd(dvec1, dvec2, _CMP_ULE_UQ);
    dresults[dresult_idx++] = _mm256_cmp_pd(dvec1, dvec2, _CMP_ULT_UQ);
    dresults[dresult_idx++] = _mm256_cmp_pd(dvec1, dvec2, _CMP_NEQ_UQ);
    
    // Blend operations using comparison results
    __m256 blended = _mm256_blendv_ps(vec1, vec2, results[0]);
    for (int i = 1; i < result_idx; i++) {
        blended = _mm256_add_ps(blended, _mm256_blendv_ps(vec1, vec2, results[i]));
    }
    
    // Extract mask and use in conditional
    int mask = _mm256_movemask_ps(results[0]);
    for (int i = 1; i < result_idx; i++) {
        mask |= _mm256_movemask_ps(results[i]);
    }
    
    float final_array[8];
    _mm256_storeu_ps(final_array, blended);
    
    if (mask != 0) {
        return final_array[0] + final_array[4];
    }
    
    return 0.0f;
}
#endif

// Complex function that mixes comparisons with arithmetic
FORCE_INLINE float complex_vector_expression(float x, float y, float z, float w) {
    __m128 v1 = _mm_set_ps(x, y, z, w);
    __m128 v2 = _mm_set_ps(w, z, y, x);
    __m128 v_nan = _mm_set1_ps(NAN);
    __m128 v_inf = _mm_set1_ps(INFINITY);
    __m128 v_zero = _mm_setzero_ps();
    
    // Chain of comparisons and arithmetic
    __m128 cmp1 = _mm_cmp_ps(v1, v_nan, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);
    __m128 cmp3 = _mm_cmp_ps(v1, v_inf, _CMP_UNEQ_UQ);
    __m128 cmp4 = _mm_cmp_ps(v2, v_zero, _CMP_NGE_UQ);
    
    // Convert comparison masks to selection masks
    __m128 sel1 = _mm_and_ps(cmp1, v1);
    __m128 sel2 = _mm_and_ps(cmp2, v2);
    __m128 sel3 = _mm_andnot_ps(cmp3, v1);
    __m128 sel4 = _mm_and_ps(cmp4, v_inf);
    
    // Complex expression
    __m128 result = _mm_add_ps(sel1, sel2);
    result = _mm_sub_ps(result, sel3);
    result = _mm_mul_ps(result, sel4);
    
    // More comparisons in the chain
    __m128 cmp5 = _mm_cmp_ps(result, v_zero, _CMP_NGT_UQ);
    __m128 cmp6 = _mm_cmp_ps(result, v1, _CMP_ULE_UQ);
    __m128 cmp7 = _mm_cmp_ps(result, v2, _CMP_ULT_UQ);
    __m128 cmp8 = _mm_cmp_ps(v1, v2, _CMP_NEQ_UQ);
    
    // Final blending based on comparisons
    __m128 final = _mm_blendv_ps(result, v1, cmp5);
    final = _mm_blendv_ps(final, v2, cmp6);
    final = _mm_blendv_ps(final, v_nan, cmp7);
    final = _mm_blendv_ps(final, v_inf, cmp8);
    
    float res[4];
    _mm_storeu_ps(res, final);
    
    // Use all comparison masks in conditional
    int mask = _mm_movemask_ps(cmp1) | _mm_movemask_ps(cmp2) |
               _mm_movemask_ps(cmp3) | _mm_movemask_ps(cmp4) |
               _mm_movemask_ps(cmp5) | _mm_movemask_ps(cmp6) |
               _mm_movemask_ps(cmp7) | _mm_movemask_ps(cmp8);
    
    if (mask != 0) {
        return res[0] + res[1] + res[2] + res[3];
    }
    
    return 0.0f;
}

int main() {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Test with various values including special cases
    float test_values[][4] = {
        {1.0f, 2.0f, 3.0f, 4.0f},
        {0.0f, -0.0f, INFINITY, -INFINITY},
        {NAN, 5.0f, NAN, 6.0f},
        {100.0f, 200.0f, 300.0f, 400.0f}
    };
    
    float total_result = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        float a = test_values[i][0];
        float b = test_values[i][1];
        float c = test_values[i][2];
        float d = test_values[i][3];
        
        // Test SSE comparisons
        total_result += test_sse_comparisons(a, b, c, d);
        
        // Test complex expression
        total_result += complex_vector_expression(a, b, c, d);
        
        #ifdef __AVX__
        // Test AVX comparisons if available
        if (__builtin_cpu_supports("avx")) {
            total_result += test_avx_comparisons(a, b, c, d);
        }
        #endif
    }
    
    printf("Final result: %f\n", total_result);
    printf("(This value is not important - the goal is to generate assembly with condition code strings)\n");
    
    // Additional forced assembly generation
    __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 v_nan = _mm_set1_ps(NAN);
    
    // Force generation of all condition codes in assembly
    __asm__ __volatile__ (
        "# Force condition code strings in assembly\n"
        "cmpunordps %[nan], %[vec1]\n\t"
        "cmpordps %[vec2], %[vec1]\n\t"
        "cmpeq_uqps %[vec2], %[vec1]\n\t"
        "cmpnge_uqps %[vec2], %[vec1]\n\t"
        "cmpngt_uqps %[vec2], %[vec1]\n\t"
        "cmple_uqps %[vec2], %[vec1]\n\t"
        "cmplt_uqps %[vec2], %[vec1]\n\t"
        "cmpneq_uqps %[vec2], %[vec1]\n\t"
        : 
        : [vec1] "x" (v1), [vec2] "x" (v2), [nan] "x" (v_nan)
        : 
    );
    
    return 0;
}
