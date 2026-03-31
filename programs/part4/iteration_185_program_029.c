#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE static inline
#endif

// Function to test SSE comparisons with all condition codes
FORCE_INLINE float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    
    // Store all comparison results to prevent optimization
    __m128 results[16];
    int result_idx = 0;
    
    // Test all condition codes from the uncovered block
    // Using explicit condition codes that map to the uncovered cases
    
    // UNORDERED: _CMP_UNORD_Q
    results[result_idx++] = _mm_cmp_ps(vec_nan, vec1, _CMP_UNORD_Q);
    
    // ORDERED: _CMP_ORD_Q  
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    
    // UNEQ: _CMP_UNEQ_UQ
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    
    // UNGE: _CMP_NGE_UQ (maps to "nlt" in assembly)
    results[result_idx++] = _mm_cmp_ps(vec1, vec_inf, _CMP_NGE_UQ);
    
    // UNGT: _CMP_NGT_UQ (maps to "nle" in assembly)
    results[result_idx++] = _mm_cmp_ps(vec_inf, vec1, _CMP_NGT_UQ);
    
    // UNLE: _CMP_ULE_UQ
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    
    // UNLT: _CMP_ULT_UQ
    results[result_idx++] = _mm_cmp_ps(vec2, vec1, _CMP_ULT_UQ);
    
    // LTGT: _CMP_NEQ_UQ (maps to "une" in assembly)
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Also test scalar comparisons which use the same condition codes
    __m128 scalar_results[8];
    scalar_results[0] = _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q);
    scalar_results[1] = _mm_cmp_ss(vec1, vec2, _CMP_ORD_Q);
    scalar_results[2] = _mm_cmp_ss(vec1, vec2, _CMP_UNEQ_UQ);
    scalar_results[3] = _mm_cmp_ss(vec1, vec_inf, _CMP_NGE_UQ);
    
    // Use results in control flow to prevent dead code elimination
    int mask_sum = 0;
    for (int i = 0; i < result_idx; i++) {
        int mask = _mm_movemask_ps(results[i]);
        mask_sum += mask;
        
        // Use comparison result in arithmetic
        __m128 blended = _mm_blendv_ps(vec1, vec2, results[i]);
        float blended_arr[4];
        _mm_storeu_ps(blended_arr, blended);
        mask_sum += (int)blended_arr[0];
    }
    
    return (float)mask_sum;
}

// Double precision version
FORCE_INLINE double test_sse_double_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    __m128d vec_inf = _mm_set1_pd(INFINITY);
    
    __m128d results[8];
    
    // Test all condition codes with double precision
    results[0] = _mm_cmp_pd(vec_nan, vec1, _CMP_UNORD_Q);
    results[1] = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm_cmp_pd(vec1, vec_inf, _CMP_NGE_UQ);
    results[4] = _mm_cmp_pd(vec_inf, vec1, _CMP_NGT_UQ);
    results[5] = _mm_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm_cmp_pd(vec2, vec1, _CMP_ULT_UQ);
    results[7] = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    // Scalar double comparisons
    __m128d scalar_results[4];
    scalar_results[0] = _mm_cmp_sd(vec1, vec_nan, _CMP_UNORD_Q);
    scalar_results[1] = _mm_cmp_sd(vec1, vec2, _CMP_ORD_Q);
    scalar_results[2] = _mm_cmp_sd(vec1, vec_inf, _CMP_NGE_UQ);
    scalar_results[3] = _mm_cmp_sd(vec1, vec2, _CMP_UNEQ_UQ);
    
    int mask_sum = 0;
    for (int i = 0; i < 8; i++) {
        int mask = _mm_movemask_pd(results[i]);
        mask_sum += mask;
    }
    
    return (double)mask_sum;
}

#ifdef __AVX__
// AVX version for 256-bit vectors
FORCE_INLINE float test_avx_comparisons(float a, float b, float c, float d) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, d, c, b, a);
    __m256 vec2 = _mm256_set_ps(d, c, b, a, a, b, c, d);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    __m256 vec_inf = _mm256_set1_ps(INFINITY);
    
    __m256 results[8];
    
    // AVX comparisons with all condition codes
    results[0] = _mm256_cmp_ps(vec_nan, vec1, _CMP_UNORD_Q);
    results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm256_cmp_ps(vec1, vec_inf, _CMP_NGE_UQ);
    results[4] = _mm256_cmp_ps(vec_inf, vec1, _CMP_NGT_UQ);
    results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm256_cmp_ps(vec2, vec1, _CMP_ULT_UQ);
    results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Complex expression mixing comparisons and arithmetic
    __m256 temp = _mm256_add_ps(vec1, vec2);
    __m256 cmp_result = _mm256_cmp_ps(temp, vec_nan, _CMP_UNORD_Q);
    __m256 blended = _mm256_blendv_ps(vec1, vec2, cmp_result);
    
    // Extract mask and use in control flow
    int mask = _mm256_movemask_ps(results[0]);
    for (int i = 1; i < 8; i++) {
        mask |= _mm256_movemask_ps(results[i]);
    }
    
    float blended_arr[8];
    _mm256_storeu_ps(blended_arr, blended);
    
    return blended_arr[0] + (float)mask;
}
#endif

// Force assembly generation with inline assembly
FORCE_INLINE void generate_assembly_output(void) {
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 v_nan = _mm_set1_ps(NAN);
    
    // Use inline assembly to force specific instruction generation
    // This ensures output_operand is called for condition codes
    __m128 result;
    
    // Generate assembly for UNORDERED
    asm volatile (
        "vcmpps %[res], %[v1], %[vnan], %[cond]\n\t"
        : [res] "=x" (result)
        : [v1] "x" (v1), [vnan] "x" (v_nan), [cond] "i" (_CMP_UNORD_Q)
    );
    
    // Generate assembly for ORDERED
    asm volatile (
        "vcmpps %[res], %[v1], %[v2], %[cond]\n\t"
        : [res] "=x" (result)
        : [v1] "x" (v1), [v2] "x" (v2), [cond] "i" (_CMP_ORD_Q)
    );
    
    // Generate assembly for UNEQ
    asm volatile (
        "vcmpps %[res], %[v1], %[v2], %[cond]\n\t"
        : [res] "=x" (result)
        : [v1] "x" (v1), [v2] "x" (v2), [cond] "i" (_CMP_UNEQ_UQ)
    );
    
    // Prevent optimization
    float dummy[4];
    _mm_storeu_ps(dummy, result);
}

int main() {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Initialize test values including special floating-point values
    float test_floats[] = {1.0f, -2.0f, 0.0f, INFINITY, -INFINITY, NAN};
    double test_doubles[] = {1.0, -2.0, 0.0, INFINITY, -INFINITY, NAN};
    
    float float_result = 0.0f;
    double double_result = 0.0;
    
    // Test multiple combinations to ensure coverage
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            float_result += test_sse_comparisons(
                test_floats[i], 
                test_floats[j],
                test_floats[(i+1)%6],
                test_floats[(j+1)%6]
            );
            
            double_result += test_sse_double_comparisons(
                test_doubles[i],
                test_doubles[j]
            );
        }
    }
    
#ifdef __AVX__
    printf("AVX supported, testing AVX comparisons...\n");
    float avx_result = 0.0f;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            avx_result += test_avx_comparisons(
                test_floats[i],
                test_floats[j],
                test_floats[(i+2)%6],
                test_floats[(j+2)%6]
            );
        }
    }
    float_result += avx_result;
#endif
    
    // Force assembly generation
    generate_assembly_output();
    
    // Use results to prevent optimization
    printf("Float result: %f\n", float_result);
    printf("Double result: %f\n", double_result);
    
    // Final check using comparison results
    if (float_result > 1000.0f || double_result > 1000.0) {
        printf("Results are large, comparisons were active\n");
    }
    
    return 0;
}
