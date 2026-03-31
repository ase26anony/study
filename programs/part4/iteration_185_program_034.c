#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE inline
#endif

// Function to test SSE comparisons with all condition codes
FORCE_INLINE float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    __m128 vec_zero = _mm_setzero_ps();
    
    // Store all comparison results to prevent optimization
    __m128 results[16];
    int idx = 0;
    
    // Test all condition codes from the uncovered block
    // Using explicit condition codes that map to the uncovered cases
    
    // 1. UNORDERED (_CMP_UNORD_Q = 3)
    results[idx++] = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    
    // 2. ORDERED (_CMP_ORD_Q = 7)
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    
    // 3. UNEQ (_CMP_UNEQ_UQ = 12)
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    
    // 4. UNGE (_CMP_NGE_UQ = 13) - maps to "nlt"
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    
    // 5. UNGT (_CMP_NGT_UQ = 14) - maps to "nle"
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    
    // 6. UNLE (_CMP_ULE_UQ = 2)
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    
    // 7. UNLT (_CMP_ULT_UQ = 1)
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    
    // 8. LTGT (_CMP_NEQ_UQ = 4) - maps to "une"
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Test with NaN values specifically for UNORDERED/ORDERED
    results[idx++] = _mm_cmp_ps(vec_nan, vec1, _CMP_UNORD_Q);
    results[idx++] = _mm_cmp_ps(vec_nan, vec_nan, _CMP_ORD_Q);
    
    // Test scalar comparisons too (these also use the same condition codes)
    __m128 scalar_results[8];
    scalar_results[0] = _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q);
    scalar_results[1] = _mm_cmp_ss(vec1, vec2, _CMP_ORD_Q);
    scalar_results[2] = _mm_cmp_ss(vec1, vec_inf, _CMP_UNEQ_UQ);
    scalar_results[3] = _mm_cmp_ss(vec_inf, vec1, _CMP_NGE_UQ);
    
    // Combine results to prevent dead code elimination
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < idx; i++) {
        // Convert comparison masks to float (0.0 or 1.0)
        __m128 mask_as_float = _mm_and_ps(results[i], _mm_set1_ps(1.0f));
        sum = _mm_add_ps(sum, mask_as_float);
    }
    
    for (int i = 0; i < 4; i++) {
        __m128 mask_as_float = _mm_and_ps(scalar_results[i], _mm_set1_ps(1.0f));
        sum = _mm_add_ps(sum, mask_as_float);
    }
    
    // Horizontal sum
    __m128 shuf = _mm_movehdup_ps(sum);
    __m128 sums = _mm_add_ps(sum, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    
    float result;
    _mm_store_ss(&result, sums);
    return result;
}

// Double precision version
FORCE_INLINE double test_sse2_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    __m128d vec_inf = _mm_set1_pd(INFINITY);
    
    // Test all condition codes with double precision
    __m128d results[16];
    int idx = 0;
    
    results[idx++] = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    results[idx++] = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    results[idx++] = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    results[idx++] = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    results[idx++] = _mm_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    results[idx++] = _mm_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    results[idx++] = _mm_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    results[idx++] = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    // Scalar double comparisons
    __m128d scalar_results[4];
    scalar_results[0] = _mm_cmp_sd(vec1, vec_nan, _CMP_UNORD_Q);
    scalar_results[1] = _mm_cmp_sd(vec1, vec_inf, _CMP_ORD_Q);
    scalar_results[2] = _mm_cmp_sd(vec_nan, vec_nan, _CMP_UNEQ_UQ);
    scalar_results[3] = _mm_cmp_sd(vec_inf, vec1, _CMP_NGE_UQ);
    
    // Combine results
    __m128d sum = _mm_setzero_pd();
    for (int i = 0; i < idx; i++) {
        __m128d mask_as_double = _mm_and_pd(results[i], _mm_set1_pd(1.0));
        sum = _mm_add_pd(sum, mask_as_double);
    }
    
    for (int i = 0; i < 4; i++) {
        __m128d mask_as_double = _mm_and_pd(scalar_results[i], _mm_set1_pd(1.0));
        sum = _mm_add_pd(sum, mask_as_double);
    }
    
    // Horizontal sum
    __m128d shuf = _mm_unpackhi_pd(sum, sum);
    __m128d sums = _mm_add_sd(sum, shuf);
    
    double result;
    _mm_store_sd(&result, sums);
    return result;
}

#ifdef __AVX__
// AVX version for 256-bit vectors
FORCE_INLINE float test_avx_comparisons(float a, float b, float c, float d,
                                        float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    __m256 vec_inf = _mm256_set1_ps(INFINITY);
    
    // Test all condition codes with AVX
    __m256 results[8];
    
    results[0] = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    results[4] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Use results in blend operations to prevent optimization
    __m256 blended = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        blended = _mm256_blendv_ps(blended, vec1, results[i]);
    }
    
    // Extract mask and use in control flow
    int mask = _mm256_movemask_ps(blended);
    
    // Use the mask in arithmetic
    float result = 0.0f;
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            result += 1.0f;
        }
    }
    
    return result;
}

// AVX double precision
FORCE_INLINE double test_avx_double_comparisons(double a, double b, double c, double d) {
    __m256d vec1 = _mm256_set_pd(a, b, c, d);
    __m256d vec2 = _mm256_set_pd(d, c, b, a);
    __m256d vec_nan = _mm256_set1_pd(NAN);
    
    __m256d results[8];
    
    results[0] = _mm256_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    results[1] = _mm256_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm256_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm256_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    results[4] = _mm256_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    results[5] = _mm256_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm256_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    results[7] = _mm256_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    // Complex expression with multiple operations
    __m256d sum = _mm256_setzero_pd();
    for (int i = 0; i < 8; i++) {
        // Convert comparison result to 0.0 or 1.0 and accumulate
        __m256d ones = _mm256_set1_pd(1.0);
        __m256d masked = _mm256_and_pd(results[i], ones);
        sum = _mm256_add_pd(sum, masked);
    }
    
    // Horizontal reduction
    __m128d sum_low = _mm256_extractf128_pd(sum, 0);
    __m128d sum_high = _mm256_extractf128_pd(sum, 1);
    __m128d total = _mm_add_pd(sum_low, sum_high);
    
    double result;
    _mm_store_sd(&result, total);
    return result;
}
#endif

// Force assembly output with inline assembly
FORCE_INLINE void force_asm_output(__m128 vec) {
    // This inline assembly forces the compiler to generate
    // assembly code for the vector operations
    __asm__ __volatile__ (
        "# Vector operand: %0"
        : 
        : "x" (vec)
        : 
    );
}

int main() {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Initialize test values including special cases
    float f1 = 1.5f, f2 = 2.5f, f3 = -3.0f, f4 = 0.0f;
    double d1 = 1.23456789, d2 = -9.87654321;
    
    // Test SSE single precision
    float sse_result = test_sse_comparisons(f1, f2, f3, f4);
    printf("SSE float comparison result: %f\n", sse_result);
    
    // Test SSE double precision
    double sse2_result = test_sse2_comparisons(d1, d2);
    printf("SSE2 double comparison result: %f\n", sse2_result);
    
    // Force assembly generation for some intermediate values
    __m128 test_vec = _mm_set_ps(f1, f2, f3, f4);
    force_asm_output(test_vec);
    
#ifdef __AVX__
    printf("AVX support detected, testing AVX comparisons...\n");
    
    // Test AVX single precision
    float avx_result = test_avx_comparisons(
        f1, f2, f3, f4,
        5.5f, -6.5f, 7.5f, 8.5f
    );
    printf("AVX float comparison result: %f\n", avx_result);
    
    // Test AVX double precision
    double avx_double_result = test_avx_double_comparisons(
        d1, d2, 3.1415926535, -2.7182818284
    );
    printf("AVX double comparison result: %f\n", avx_double_result);
    
    // Force AVX assembly output
    __m256 avx_vec = _mm256_set_ps(f1, f2, f3, f4, f1, f2, f3, f4);
    __asm__ __volatile__ (
        "# AVX vector operand"
        :
        : "x" (avx_vec)
        :
    );
#else
    printf("AVX not supported in this compilation\n");
#endif
    
    // Create a final result that depends on all comparisons
    // to prevent dead code elimination
    volatile float final_result = sse_result + (float)sse2_result;
#ifdef __AVX__
    final_result += avx_result + (float)avx_double_result;
#endif
    
    printf("Final aggregated result: %f\n", final_result);
    
    // Additional test: Mix comparisons with arithmetic in a loop
    // to encourage vectorization and condition code generation
    float array_a[16], array_b[16];
    for (int i = 0; i < 16; i++) {
        array_a[i] = (float)i * 0.5f;
        array_b[i] = (float)(15 - i) * 0.5f;
    }
    
    float loop_sum = 0.0f;
    for (int i = 0; i < 16; i += 4) {
        __m128 va = _mm_loadu_ps(&array_a[i]);
        __m128 vb = _mm_loadu_ps(&array_b[i]);
        
        // Use multiple condition codes in the loop
        __m128 cmp1 = _mm_cmp_ps(va, vb, _CMP_UNORD_Q);
        __m128 cmp2 = _mm_cmp_ps(va, vb, _CMP_ORD_Q);
        __m128 cmp3 = _mm_cmp_ps(va, vb, _CMP_UNEQ_UQ);
        __m128 cmp4 = _mm_cmp_ps(va, vb, _CMP_NGE_UQ);
        
        // Blend based on comparison results
        __m128 blended = _mm_blendv_ps(va, vb, cmp1);
        blended = _mm_add_ps(blended, _mm_blendv_ps(va, vb, cmp2));
        blended = _mm_add_ps(blended, _mm_blendv_ps(va, vb, cmp3));
        blended = _mm_add_ps(blended, _mm_blendv_ps(va, vb, cmp4));
        
        // Store and accumulate
        float temp[4];
        _mm_storeu_ps(temp, blended);
        loop_sum += temp[0] + temp[1] + temp[2] + temp[3];
    }
    
    printf("Loop-based comparison result: %f\n", loop_sum);
    
    return 0;
}
