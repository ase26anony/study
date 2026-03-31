#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#ifdef __GNUC__
#define FORCE_INLINE __attribute__((always_inline)) inline
#else
#define FORCE_INLINE
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
    // Using explicit condition codes to trigger the assembly output
    
    // UNORDERED: _CMP_UNORD_Q = 3
    results[result_idx++] = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    
    // ORDERED: _CMP_ORD_Q = 7
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    
    // UNEQ: _CMP_UNEQ_UQ = 12
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    
    // UNGE: _CMP_NGE_UQ = 13
    results[result_idx++] = _mm_cmp_ps(vec1, vec_inf, _CMP_NGE_UQ);
    
    // UNGT: _CMP_NGT_UQ = 14
    results[result_idx++] = _mm_cmp_ps(vec_inf, vec1, _CMP_NGT_UQ);
    
    // UNLE: _CMP_ULE_UQ = 15
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    
    // UNLT: _CMP_ULT_UQ = 16
    results[result_idx++] = _mm_cmp_ps(vec2, vec1, _CMP_ULT_UQ);
    
    // LTGT: _CMP_NEQ_UQ = 4
    results[result_idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Also test scalar versions to cover different code paths
    __m128 scalar_results[8];
    scalar_results[0] = _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q);
    scalar_results[1] = _mm_cmp_ss(vec1, vec2, _CMP_ORD_Q);
    scalar_results[2] = _mm_cmp_ss(vec1, vec2, _CMP_UNEQ_UQ);
    scalar_results[3] = _mm_cmp_ss(vec1, vec_inf, _CMP_NGE_UQ);
    
    // Combine results using arithmetic operations
    __m128 final_result = _mm_setzero_ps();
    for (int i = 0; i < result_idx; i++) {
        // Convert comparison masks to float (0.0 or 1.0)
        __m128 mask_as_float = _mm_and_ps(results[i], _mm_set1_ps(1.0f));
        final_result = _mm_add_ps(final_result, mask_as_float);
    }
    
    // Extract and return a scalar value
    float sum = 0;
    float* f = (float*)&final_result;
    for (int i = 0; i < 4; i++) {
        sum += f[i];
    }
    
    return sum;
}

// Double precision version
FORCE_INLINE double test_sse_double_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    // Test all condition codes with double precision
    __m128d results[8];
    
    results[0] = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    results[1] = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    results[4] = _mm_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    results[5] = _mm_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    results[7] = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    // Also test scalar double comparisons
    __m128d scalar_results[4];
    scalar_results[0] = _mm_cmp_sd(vec1, vec_nan, _CMP_UNORD_Q);
    scalar_results[1] = _mm_cmp_sd(vec1, vec2, _CMP_ORD_Q);
    scalar_results[2] = _mm_cmp_sd(vec1, vec2, _CMP_UNEQ_UQ);
    scalar_results[3] = _mm_cmp_sd(vec1, vec2, _CMP_NGE_UQ);
    
    // Combine results
    __m128d final_result = _mm_setzero_pd();
    for (int i = 0; i < 8; i++) {
        // Convert to double (0.0 or 1.0)
        __m128d ones = _mm_set1_pd(1.0);
        __m128d mask = _mm_and_pd(results[i], ones);
        final_result = _mm_add_pd(final_result, mask);
    }
    
    double* d = (double*)&final_result;
    return d[0] + d[1];
}

#ifdef __AVX__
// AVX version for 256-bit vectors
FORCE_INLINE float test_avx_comparisons(float a, float b, float c, float d,
                                        float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    // Test AVX comparisons with all condition codes
    __m256 results[8];
    
    results[0] = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    results[4] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Use results in blend operations to prevent dead code elimination
    __m256 blended = vec1;
    for (int i = 0; i < 8; i++) {
        // Alternate between vec1 and vec2 based on comparison results
        blended = _mm256_blendv_ps(blended, vec2, results[i]);
    }
    
    // Extract mask and use in control flow
    int mask = _mm256_movemask_ps(blended);
    float sum = 0;
    float* ptr = (float*)&blended;
    
    // Conditional operations based on comparison results
    if (mask & 0x01) sum += ptr[0];
    if (mask & 0x02) sum += ptr[1];
    if (mask & 0x04) sum += ptr[2];
    if (mask & 0x08) sum += ptr[3];
    if (mask & 0x10) sum += ptr[4];
    if (mask & 0x20) sum += ptr[5];
    if (mask & 0x40) sum += ptr[6];
    if (mask & 0x80) sum += ptr[7];
    
    return sum;
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
    
    // Complex expression with arithmetic
    __m256d sum = _mm256_setzero_pd();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to 0.0 or 1.0
        __m256d mask = _mm256_and_pd(results[i], _mm256_set1_pd(1.0));
        // Multiply by different coefficients
        __m256d coeff = _mm256_set1_pd(i + 1.0);
        sum = _mm256_add_pd(sum, _mm256_mul_pd(mask, coeff));
    }
    
    double* dptr = (double*)&sum;
    return dptr[0] + dptr[1] + dptr[2] + dptr[3];
}
#endif

// Force assembly output with inline assembly
FORCE_INLINE void force_asm_output(__m128 vec) {
    // Use inline assembly to force the compiler to generate
    // assembly instructions for vector operations
    __asm__ __volatile__ (
        "# Vector operand: %0"
        : 
        : "x" (vec)
        : "memory"
    );
}

int main() {
    printf("Testing x86 vector comparisons to trigger condition code printing...\n");
    
    // Initialize with various values including special cases
    float f1 = 1.0f, f2 = 2.0f, f3 = 0.0f, f4 = -1.0f;
    double d1 = 3.14, d2 = 2.71, d3 = 0.0, d4 = -3.14;
    
    float sse_result = 0;
    double sse_double_result = 0;
    
    // Test SSE comparisons multiple times with different inputs
    for (int i = 0; i < 10; i++) {
        sse_result += test_sse_comparisons(f1 + i, f2 - i, f3, f4);
        sse_double_result += test_sse_double_comparisons(d1 + i, d2 - i);
        
        // Force assembly generation
        __m128 test_vec = _mm_set_ps(f1, f2, f3, f4);
        force_asm_output(test_vec);
    }
    
    printf("SSE float result: %f\n", sse_result);
    printf("SSE double result: %f\n", sse_double_result);
    
#ifdef __AVX__
    printf("AVX supported, testing AVX comparisons...\n");
    
    float avx_result = 0;
    double avx_double_result = 0;
    
    // Test AVX comparisons
    for (int i = 0; i < 5; i++) {
        avx_result += test_avx_comparisons(
            f1 + i, f2 - i, f3, f4,
            f1 - i, f2 + i, f3 * i, f4 / (i + 1)
        );
        
        avx_double_result += test_avx_double_comparisons(
            d1 + i, d2 - i, d3, d4
        );
    }
    
    printf("AVX float result: %f\n", avx_result);
    printf("AVX double result: %f\n", avx_double_result);
#else
    printf("AVX not supported, skipping AVX tests\n");
#endif
    
    // Create a complex expression that uses all comparison types
    // to ensure they appear in the generated assembly
    __m128 vec_a = _mm_set_ps(1.0f, 2.0f, NAN, INFINITY);
    __m128 vec_b = _mm_set_ps(INFINITY, NAN, 2.0f, 1.0f);
    
    // Chain comparisons with different condition codes
    __m128 cmp1 = _mm_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);   // unord
    __m128 cmp2 = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);     // ord
    __m128 cmp3 = _mm_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);   // ueq
    __m128 cmp4 = _mm_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);    // nlt
    __m128 cmp5 = _mm_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);    // nle
    __m128 cmp6 = _mm_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);    // ule
    __m128 cmp7 = _mm_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);    // ult
    __m128 cmp8 = _mm_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);    // une
    
    // Use all results in a final computation
    __m128 final = _mm_add_ps(cmp1, cmp2);
    final = _mm_add_ps(final, cmp3);
    final = _mm_add_ps(final, cmp4);
    final = _mm_add_ps(final, cmp5);
    final = _mm_add_ps(final, cmp6);
    final = _mm_add_ps(final, cmp7);
    final = _mm_add_ps(final, cmp8);
    
    // Extract and print a result to prevent optimization
    float final_array[4];
    _mm_store_ps(final_array, final);
    printf("Final comparison sum: [%f, %f, %f, %f]\n",
           final_array[0], final_array[1], final_array[2], final_array[3]);
    
    return 0;
}
