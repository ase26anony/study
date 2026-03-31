#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifdef __AVX__
#define USE_AVX 1
#else
#define USE_AVX 0
#endif

// Function to test SSE comparisons with all condition codes
float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    
    // Store results to prevent optimization
    __m128 results[8];
    int masks[8];
    
    // Test all condition codes from the uncovered block
    results[0] = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);    // UNORDERED
    results[1] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);         // ORDERED
    results[2] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);       // UNEQ
    results[3] = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);        // UNGE -> nlt
    results[4] = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);        // UNGT -> nle
    results[5] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);        // UNLE -> ule
    results[6] = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);        // UNLT -> ult
    results[7] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);        // LTGT -> une
    
    // Extract masks and use in control flow
    float sum = 0.0f;
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm_movemask_ps(results[i]);
        if (masks[i] != 0) {
            // Blend based on comparison result
            __m128 blended = _mm_blendv_ps(vec1, vec2, results[i]);
            float blended_arr[4];
            _mm_storeu_ps(blended_arr, blended);
            sum += blended_arr[0] + blended_arr[1] + blended_arr[2] + blended_arr[3];
        }
    }
    
    // Test scalar comparisons as well
    __m128 scalar_cmp = _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q);
    int scalar_mask = _mm_movemask_ps(scalar_cmp);
    if (scalar_mask & 1) {
        sum *= 2.0f;
    }
    
    return sum;
}

// Double precision version
double test_sse_double_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    __m128d results[8];
    int masks[8];
    
    results[0] = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    results[1] = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    results[4] = _mm_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    results[5] = _mm_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    results[7] = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm_movemask_pd(results[i]);
        if (masks[i] != 0) {
            // Complex expression to force decomposition
            __m128d temp = _mm_add_pd(vec1, vec2);
            __m128d masked = _mm_and_pd(temp, results[i]);
            double masked_arr[2];
            _mm_storeu_pd(masked_arr, masked);
            sum += masked_arr[0] * masked_arr[1];
        }
    }
    
    return sum;
}

#if USE_AVX
// AVX 256-bit versions
float test_avx_comparisons(float a, float b, float c, float d,
                          float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    __m256 results[8];
    int masks[8];
    
    // AVX comparisons - will generate different assembly patterns
    results[0] = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    results[4] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    float sum = 0.0f;
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm256_movemask_ps(results[i]);
        if (masks[i] != 0) {
            // Blend and arithmetic to create complex pattern
            __m256 blended = _mm256_blendv_ps(vec1, vec2, results[i]);
            __m256 multiplied = _mm256_mul_ps(blended, _mm256_set1_ps(2.0f));
            float result_arr[8];
            _mm256_storeu_ps(result_arr, multiplied);
            for (int j = 0; j < 8; j++) {
                sum += result_arr[j];
            }
        }
    }
    
    return sum;
}

double test_avx_double_comparisons(double a, double b, double c, double d) {
    __m256d vec1 = _mm256_set_pd(a, b, c, d);
    __m256d vec2 = _mm256_set_pd(d, c, b, a);
    __m256d vec_nan = _mm256_set1_pd(NAN);
    
    __m256d results[8];
    int masks[8];
    
    results[0] = _mm256_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);
    results[1] = _mm256_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    results[2] = _mm256_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);
    results[3] = _mm256_cmp_pd(vec1, vec2, _CMP_NGE_UQ);
    results[4] = _mm256_cmp_pd(vec1, vec2, _CMP_NGT_UQ);
    results[5] = _mm256_cmp_pd(vec1, vec2, _CMP_ULE_UQ);
    results[6] = _mm256_cmp_pd(vec1, vec2, _CMP_ULT_UQ);
    results[7] = _mm256_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        masks[i] = _mm256_movemask_pd(results[i]);
        if (masks[i] != 0) {
            // Complex expression mixing comparisons and arithmetic
            __m256d temp = _mm256_add_pd(vec1, _mm256_mul_pd(vec2, results[i]));
            double temp_arr[4];
            _mm256_storeu_pd(temp_arr, temp);
            sum += temp_arr[0] + temp_arr[1] + temp_arr[2] + temp_arr[3];
        }
    }
    
    return sum;
}
#endif

// Inline assembly to force specific condition code output
void force_asm_output(void) {
    __m128 v1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 result;
    
    // Force assembly generation for each condition code
    asm volatile (
        "vcmpps %2, %1, %0, %3\n\t"
        : "=x"(result)
        : "x"(v1), "x"(v2), "i"(_CMP_UNORD_Q)
        : "cc"
    );
    
    asm volatile (
        "vcmpps %2, %1, %0, %3\n\t"
        : "=x"(result)
        : "x"(v1), "x"(v2), "i"(_CMP_ORD_Q)
        : "cc"
    );
}

int main() {
    // Test with various values including NaN, infinity, zero
    float inf = INFINITY;
    float nan_val = NAN;
    
    printf("Testing SSE float comparisons...\n");
    float result1 = test_sse_comparisons(1.0f, 2.0f, nan_val, inf);
    printf("Result 1: %f\n", result1);
    
    printf("Testing SSE double comparisons...\n");
    double result2 = test_sse_double_comparisons(1.0, nan_val);
    printf("Result 2: %f\n", result2);
    
#if USE_AVX
    if (__builtin_cpu_supports("avx")) {
        printf("Testing AVX float comparisons...\n");
        float result3 = test_avx_comparisons(1.0f, 2.0f, 3.0f, 4.0f,
                                           5.0f, 6.0f, nan_val, inf);
        printf("Result 3: %f\n", result3);
        
        printf("Testing AVX double comparisons...\n");
        double result4 = test_avx_double_comparisons(1.0, 2.0, nan_val, inf);
        printf("Result 4: %f\n", result4);
    }
#endif
    
    // Force assembly output
    force_asm_output();
    
    // Final computation using all results to prevent dead code elimination
    float final_result = result1 + (float)result2;
#if USE_AVX
    if (__builtin_cpu_supports("avx")) {
        final_result += result3 + (float)result4;
    }
#endif
    
    printf("Final result: %f\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
