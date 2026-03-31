#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to test SSE comparisons with all condition codes
float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    
    // Store comparison results to prevent optimization
    __m128 results[16];
    int idx = 0;
    
    // Test all condition codes from the uncovered block
    // Using explicit condition codes that map to the uncovered cases
    
    // 1. UNORDERED - _CMP_UNORD_Q
    results[idx++] = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    
    // 2. ORDERED - _CMP_ORD_Q
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    
    // 3. UNEQ - _CMP_UNEQ_UQ
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    
    // 4. UNGE - _CMP_NGE_UQ
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    
    // 5. UNGT - _CMP_NGT_UQ
    results[idx++] = _mm_cmp_ps(vec1, vec_inf, _CMP_NGT_UQ);
    
    // 6. UNLE - _CMP_ULE_UQ
    results[idx++] = _mm_cmp_ps(vec2, vec1, _CMP_ULE_UQ);
    
    // 7. UNLT - _CMP_ULT_UQ
    results[idx++] = _mm_cmp_ps(vec1, vec_inf, _CMP_ULT_UQ);
    
    // 8. LTGT - _CMP_NEQ_UQ
    results[idx++] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Repeat with double precision
    __m128d vecd1 = _mm_set_pd(a, b);
    __m128d vecd2 = _mm_set_pd(b, a);
    __m128d vecd_nan = _mm_set1_pd(NAN);
    
    results[idx++] = _mm_cmp_pd(vecd1, vecd_nan, _CMP_UNORD_Q);
    results[idx++] = _mm_cmp_pd(vecd1, vecd2, _CMP_ORD_Q);
    results[idx++] = _mm_cmp_pd(vecd1, vecd2, _CMP_UNEQ_UQ);
    results[idx++] = _mm_cmp_pd(vecd1, vecd2, _CMP_NGE_UQ);
    results[idx++] = _mm_cmp_pd(vecd1, vecd2, _CMP_NGT_UQ);
    results[idx++] = _mm_cmp_pd(vecd1, vecd2, _CMP_ULE_UQ);
    results[idx++] = _mm_cmp_pd(vecd1, vecd2, _CMP_ULT_UQ);
    results[idx++] = _mm_cmp_pd(vecd1, vecd2, _CMP_NEQ_UQ);
    
    // Use results in arithmetic to prevent dead code elimination
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < idx; i++) {
        // Convert comparison masks to float (0.0 or 1.0)
        __m128 mask_as_float = _mm_and_ps(results[i], _mm_set1_ps(1.0f));
        sum = _mm_add_ps(sum, mask_as_float);
    }
    
    // Extract and return a scalar result
    float result_array[4];
    _mm_store_ps(result_array, sum);
    return result_array[0] + result_array[1] + result_array[2] + result_array[3];
}

// AVX version for 256-bit vectors
#ifdef __AVX__
float test_avx_comparisons(float a, float b, float c, float d) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, d, c, b, a);
    __m256 vec2 = _mm256_set_ps(d, c, b, a, a, b, c, d);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 results[8];
    
    // Test AVX comparisons with the same condition codes
    results[0] = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    results[4] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Blend operations using comparison results
    __m256 blended = vec1;
    for (int i = 0; i < 8; i++) {
        blended = _mm256_blendv_ps(blended, vec2, results[i]);
    }
    
    // Extract mask and use in control flow
    int mask = _mm256_movemask_ps(blended);
    
    // Use the mask in conditional operations
    float result = 0.0f;
    float temp_array[8];
    _mm256_storeu_ps(temp_array, blended);
    
    for (int i = 0; i < 8; i++) {
        if (mask & (1 << i)) {
            result += temp_array[i];
        } else {
            result -= temp_array[i];
        }
    }
    
    return result;
}
#endif

// Test scalar comparisons as well (cmpss, cmpsd)
float test_scalar_comparisons(float a, float b) {
    __m128 s1 = _mm_set_ss(a);
    __m128 s2 = _mm_set_ss(b);
    __m128 s_nan = _mm_set_ss(NAN);
    
    // Scalar comparisons with the same condition codes
    __m128 r1 = _mm_cmp_ss(s1, s_nan, _CMP_UNORD_Q);
    __m128 r2 = _mm_cmp_ss(s1, s2, _CMP_ORD_Q);
    __m128 r3 = _mm_cmp_ss(s1, s2, _CMP_UNEQ_UQ);
    __m128 r4 = _mm_cmp_ss(s1, s2, _CMP_NGE_UQ);
    __m128 r5 = _mm_cmp_ss(s1, s2, _CMP_NGT_UQ);
    __m128 r6 = _mm_cmp_ss(s1, s2, _CMP_ULE_UQ);
    __m128 r7 = _mm_cmp_ss(s1, s2, _CMP_ULT_UQ);
    __m128 r8 = _mm_cmp_ss(s1, s2, _CMP_NEQ_UQ);
    
    // Double precision scalar
    __m128d d1 = _mm_set_sd(a);
    __m128d d2 = _mm_set_sd(b);
    __m128d d_nan = _mm_set_sd(NAN);
    
    __m128d rd1 = _mm_cmp_sd(d1, d_nan, _CMP_UNORD_Q);
    __m128d rd2 = _mm_cmp_sd(d1, d2, _CMP_ORD_Q);
    __m128d rd3 = _mm_cmp_sd(d1, d2, _CMP_UNEQ_UQ);
    __m128d rd4 = _mm_cmp_sd(d1, d2, _CMP_NGE_UQ);
    __m128d rd5 = _mm_cmp_sd(d1, d2, _CMP_NGT_UQ);
    __m128d rd6 = _mm_cmp_sd(d1, d2, _CMP_ULE_UQ);
    __m128d rd7 = _mm_cmp_sd(d1, d2, _CMP_ULT_UQ);
    __m128d rd8 = _mm_cmp_sd(d1, d2, _CMP_NEQ_UQ);
    
    // Force assembly output with inline asm
    float result;
    __asm__ __volatile__ (
        "movss %1, %%xmm0\n\t"
        "movss %2, %%xmm1\n\t"
        "ucomiss %%xmm1, %%xmm0\n\t"
        "movss %%xmm0, %0\n\t"
        : "=m"(result)
        : "m"(a), "m"(b)
        : "xmm0", "xmm1"
    );
    
    return result;
}

int main() {
    // Initialize with various values including special cases
    float values[] = {1.0f, -2.0f, 0.0f, INFINITY, NAN, -INFINITY, 3.14f, -3.14f};
    
    printf("Testing SSE comparisons with all condition codes...\n");
    
    float total = 0.0f;
    for (int i = 0; i < 8; i += 4) {
        if (i + 3 < 8) {
            float result = test_sse_comparisons(
                values[i], 
                values[i+1], 
                values[i+2], 
                values[i+3]
            );
            total += result;
            printf("  Iteration %d: %f\n", i/4, result);
        }
    }
    
    printf("Testing scalar comparisons...\n");
    for (int i = 0; i < 8; i += 2) {
        float result = test_scalar_comparisons(values[i], values[i+1]);
        total += result;
    }
    
#ifdef __AVX__
    printf("Testing AVX comparisons...\n");
    for (int i = 0; i < 8; i += 4) {
        if (i + 3 < 8) {
            float result = test_avx_comparisons(
                values[i], 
                values[i+1], 
                values[i+2], 
                values[i+3]
            );
            total += result;
            printf("  AVX Iteration %d: %f\n", i/4, result);
        }
    }
#endif
    
    printf("Total: %f\n", total);
    
    // Use the result to prevent optimization
    if (total > 1000.0f) {
        printf("Unexpected large result!\n");
    }
    
    return 0;
}
