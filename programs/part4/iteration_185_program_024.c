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
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v_nan = _mm_set1_ps(NAN);
    __m128 v_inf = _mm_set1_ps(INFINITY);
    __m128 v_zero = _mm_setzero_ps();
    
    // Store comparison results to prevent optimization
    __m128 cmp_results[8];
    int masks[8];
    
    // Test all condition codes from the uncovered block
    cmp_results[0] = _mm_cmp_ps(v1, v_nan, _CMP_UNORD_Q);    // UNORDERED
    cmp_results[1] = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);         // ORDERED
    cmp_results[2] = _mm_cmp_ps(v1, v2, _CMP_UNEQ_UQ);       // UNEQ
    cmp_results[3] = _mm_cmp_ps(v1, v2, _CMP_NGE_UQ);        // UNGE -> nlt
    cmp_results[4] = _mm_cmp_ps(v1, v2, _CMP_NGT_UQ);        // UNGT -> nle
    cmp_results[5] = _mm_cmp_ps(v1, v2, _CMP_ULE_UQ);        // UNLE
    cmp_results[6] = _mm_cmp_ps(v1, v2, _CMP_ULT_UQ);        // UNLT
    cmp_results[7] = _mm_cmp_ps(v1, v2, _CMP_NEQ_UQ);        // LTGT -> une
    
    // Extract masks and use them to prevent dead code elimination
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm_movemask_ps(cmp_results[i]);
    }
    
    // Use comparison results in arithmetic operations
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Blend based on comparison results
        __m128 blended = _mm_blendv_ps(v1, v2, cmp_results[i]);
        sum = _mm_add_ps(sum, blended);
    }
    
    // Force assembly output with inline asm
    float result[4];
    _mm_storeu_ps(result, sum);
    
    // Use masks in conditional logic
    int final_mask = 0;
    for (int i = 0; i < 8; i++) {
        if (masks[i] != 0) {
            final_mask |= (1 << i);
        }
    }
    
    return result[0] + result[1] + result[2] + result[3] + (float)final_mask;
}

// Function to test double precision comparisons
FORCE_INLINE double test_sse2_comparisons(double a, double b) {
    __m128d v1 = _mm_set_pd(a, b);
    __m128d v2 = _mm_set_pd(b, a);
    __m128d v_nan = _mm_set1_pd(NAN);
    
    // Test double precision comparisons
    __m128d cmp_double[8];
    int masks_double[8];
    
    cmp_double[0] = _mm_cmp_pd(v1, v_nan, _CMP_UNORD_Q);     // UNORDERED
    cmp_double[1] = _mm_cmp_pd(v1, v2, _CMP_ORD_Q);          // ORDERED
    cmp_double[2] = _mm_cmp_pd(v1, v2, _CMP_UNEQ_UQ);        // UNEQ
    cmp_double[3] = _mm_cmp_pd(v1, v2, _CMP_NGE_UQ);         // UNGE -> nlt
    cmp_double[4] = _mm_cmp_pd(v1, v2, _CMP_NGT_UQ);         // UNGT -> nle
    cmp_double[5] = _mm_cmp_pd(v1, v2, _CMP_ULE_UQ);         // UNLE
    cmp_double[6] = _mm_cmp_pd(v1, v2, _CMP_ULT_UQ);         // UNLT
    cmp_double[7] = _mm_cmp_pd(v1, v2, _CMP_NEQ_UQ);         // LTGT -> une
    
    for (int i = 0; i < 8; i++) {
        masks_double[i] = _mm_movemask_pd(cmp_double[i]);
    }
    
    // Complex expression with blending
    __m128d result = _mm_setzero_pd();
    for (int i = 0; i < 8; i++) {
        __m128d temp = _mm_blendv_pd(v1, v2, cmp_double[i]);
        result = _mm_add_pd(result, temp);
    }
    
    double res[2];
    _mm_storeu_pd(res, result);
    
    return res[0] + res[1];
}

#ifdef __AVX__
// AVX version for wider vectors
FORCE_INLINE float test_avx_comparisons(float a, float b, float c, float d,
                                        float e, float f, float g, float h) {
    __m256 v1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 v2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 v_nan = _mm256_set1_ps(NAN);
    
    __m256 cmp_avx[8];
    int masks_avx[8];
    
    // AVX comparisons
    cmp_avx[0] = _mm256_cmp_ps(v1, v_nan, _CMP_UNORD_Q);     // UNORDERED
    cmp_avx[1] = _mm256_cmp_ps(v1, v2, _CMP_ORD_Q);          // ORDERED
    cmp_avx[2] = _mm256_cmp_ps(v1, v2, _CMP_UNEQ_UQ);        // UNEQ
    cmp_avx[3] = _mm256_cmp_ps(v1, v2, _CMP_NGE_UQ);         // UNGE -> nlt
    cmp_avx[4] = _mm256_cmp_ps(v1, v2, _CMP_NGT_UQ);         // UNGT -> nle
    cmp_avx[5] = _mm256_cmp_ps(v1, v2, _CMP_ULE_UQ);         // UNLE
    cmp_avx[6] = _mm256_cmp_ps(v1, v2, _CMP_ULT_UQ);         // UNLT
    cmp_avx[7] = _mm256_cmp_ps(v1, v2, _CMP_NEQ_UQ);         // LTGT -> une
    
    for (int i = 0; i < 8; i++) {
        masks_avx[i] = _mm256_movemask_ps(cmp_avx[i]);
    }
    
    // Use comparisons in complex arithmetic
    __m256 sum_avx = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        __m256 blended = _mm256_blendv_ps(v1, v2, cmp_avx[i]);
        sum_avx = _mm256_add_ps(sum_avx, blended);
    }
    
    float result_avx[8];
    _mm256_storeu_ps(result_avx, sum_avx);
    
    float total = 0.0f;
    for (int i = 0; i < 8; i++) {
        total += result_avx[i];
    }
    
    return total;
}
#endif

// Test scalar comparisons as well
FORCE_INLINE float test_scalar_comparisons(float a, float b) {
    __m128 v1 = _mm_set_ss(a);
    __m128 v2 = _mm_set_ss(b);
    __m128 v_nan = _mm_set_ss(NAN);
    
    // Scalar comparisons
    __m128 cmp_scalar[8];
    
    cmp_scalar[0] = _mm_cmp_ss(v1, v_nan, _CMP_UNORD_Q);     // UNORDERED
    cmp_scalar[1] = _mm_cmp_ss(v1, v2, _CMP_ORD_Q);          // ORDERED
    cmp_scalar[2] = _mm_cmp_ss(v1, v2, _CMP_UNEQ_UQ);        // UNEQ
    cmp_scalar[3] = _mm_cmp_ss(v1, v2, _CMP_NGE_UQ);         // UNGE -> nlt
    cmp_scalar[4] = _mm_cmp_ss(v1, v2, _CMP_NGT_UQ);         // UNGT -> nle
    cmp_scalar[5] = _mm_cmp_ss(v1, v2, _CMP_ULE_UQ);         // UNLE
    cmp_scalar[6] = _mm_cmp_ss(v1, v2, _CMP_ULT_UQ);         // UNLT
    cmp_scalar[7] = _mm_cmp_ss(v1, v2, _CMP_NEQ_UQ);         // LTGT -> une
    
    // Force use of results
    float result = 0.0f;
    for (int i = 0; i < 8; i++) {
        float temp;
        _mm_store_ss(&temp, cmp_scalar[i]);
        result += temp;
    }
    
    return result;
}

int main() {
    // Initialize with various values including special cases
    float values[] = {1.0f, -1.0f, 0.0f, -0.0f, INFINITY, -INFINITY, NAN, 2.5f};
    double dvalues[] = {1.0, -1.0, 0.0, -0.0, INFINITY, -INFINITY, NAN, 2.5};
    
    float total_float = 0.0f;
    double total_double = 0.0;
    
    // Test multiple combinations
    for (int i = 0; i < 4; i++) {
        total_float += test_sse_comparisons(
            values[i], values[i+1], values[i+2], values[i+3]);
        
        total_double += test_sse2_comparisons(dvalues[i], dvalues[i+1]);
        
        total_float += test_scalar_comparisons(values[i], values[i+1]);
    }
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        total_float += test_avx_comparisons(
            values[0], values[1], values[2], values[3],
            values[4], values[5], values[6], values[7]);
    }
#endif
    
    // Force assembly output with volatile asm
    __asm__ __volatile__ (
        "/* Force assembly generation for vector comparisons */"
        :
        : "r" (total_float), "r" (total_double)
        : "memory"
    );
    
    // Print results to prevent optimization
    printf("Float result: %f\n", total_float);
    printf("Double result: %lf\n", total_double);
    
    return 0;
}
