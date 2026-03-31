#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Function to test SSE comparisons with all condition codes
float test_sse_comparisons(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_inf = _mm_set1_ps(INFINITY);
    
    // Test all condition codes from the uncovered block
    __m128 cmp_results[8];
    
    // 1. UNORDERED (_CMP_UNORD_Q)
    cmp_results[0] = _mm_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    
    // 2. ORDERED (_CMP_ORD_Q)
    cmp_results[1] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    
    // 3. UNEQ (_CMP_UNEQ_UQ)
    cmp_results[2] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    
    // 4. UNGE (_CMP_NGE_UQ) - nlt
    cmp_results[3] = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    
    // 5. UNGT (_CMP_NGT_UQ) - nle
    cmp_results[4] = _mm_cmp_ps(vec1, vec_inf, _CMP_NGT_UQ);
    
    // 6. UNLE (_CMP_ULE_UQ) - ule
    cmp_results[5] = _mm_cmp_ps(vec2, vec1, _CMP_ULE_UQ);
    
    // 7. UNLT (_CMP_ULT_UQ) - ult
    cmp_results[6] = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    
    // 8. LTGT (_CMP_NEQ_UQ) - une
    cmp_results[7] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Combine results using arithmetic operations
    __m128 result = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison masks to float (0.0 or 1.0)
        __m128 mask_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        result = _mm_add_ps(result, mask_float);
    }
    
    // Extract and return a scalar value
    float res_arr[4];
    _mm_store_ps(res_arr, result);
    return res_arr[0] + res_arr[1] + res_arr[2] + res_arr[3];
}

// Function to test double precision comparisons
double test_sse2_comparisons(double a, double b) {
    __m128d vec1 = _mm_set_pd(a, b);
    __m128d vec2 = _mm_set_pd(b, a);
    __m128d vec_nan = _mm_set1_pd(NAN);
    
    // Test a subset of condition codes with doubles
    __m128d cmp_results[4];
    
    cmp_results[0] = _mm_cmp_pd(vec1, vec_nan, _CMP_UNORD_Q);    // UNORDERED
    cmp_results[1] = _mm_cmp_pd(vec1, vec2, _CMP_ORD_Q);         // ORDERED
    cmp_results[2] = _mm_cmp_pd(vec1, vec2, _CMP_UNEQ_UQ);       // UNEQ
    cmp_results[3] = _mm_cmp_pd(vec1, vec2, _CMP_NEQ_UQ);        // LTGT
    
    // Use results in conditional blending
    __m128d blended = _mm_blendv_pd(vec1, vec2, cmp_results[0]);
    blended = _mm_add_pd(blended, _mm_and_pd(cmp_results[1], _mm_set1_pd(2.0)));
    
    double res_arr[2];
    _mm_store_pd(res_arr, blended);
    return res_arr[0] * res_arr[1];
}

#ifdef __AVX__
// AVX version for wider vectors
float test_avx_comparisons(float a, float b, float c, float d,
                          float e, float f, float g, float h) {
    __m256 vec1 = _mm256_set_ps(a, b, c, d, e, f, g, h);
    __m256 vec2 = _mm256_set_ps(h, g, f, e, d, c, b, a);
    __m256 vec_nan = _mm256_set1_ps(NAN);
    
    // Test all condition codes with AVX
    __m256 cmp_results[8];
    
    cmp_results[0] = _mm256_cmp_ps(vec1, vec_nan, _CMP_UNORD_Q);
    cmp_results[1] = _mm256_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    cmp_results[2] = _mm256_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    cmp_results[4] = _mm256_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    cmp_results[5] = _mm256_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    cmp_results[6] = _mm256_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    cmp_results[7] = _mm256_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Complex expression to prevent optimization
    __m256 result = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        __m256 mask = _mm256_and_ps(cmp_results[i], _mm256_set1_ps(1.0f));
        result = _mm256_add_ps(result, _mm256_mul_ps(mask, _mm256_set1_ps(i + 1.0f)));
    }
    
    // Horizontal sum
    __m128 sum128 = _mm_add_ps(_mm256_extractf128_ps(result, 0),
                              _mm256_extractf128_ps(result, 1));
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    
    float final_result;
    _mm_store_ss(&final_result, sum128);
    return final_result;
}
#endif

// Test scalar comparisons as well
float test_scalar_comparisons(float a, float b) {
    __m128 vec1 = _mm_set_ss(a);
    __m128 vec2 = _mm_set_ss(b);
    __m128 vec_nan = _mm_set_ss(NAN);
    
    // Scalar comparisons also use the same condition codes
    __m128 cmp_unord = _mm_cmp_ss(vec1, vec_nan, _CMP_UNORD_Q);
    __m128 cmp_ord = _mm_cmp_ss(vec1, vec2, _CMP_ORD_Q);
    __m128 cmp_uneq = _mm_cmp_ss(vec1, vec2, _CMP_UNEQ_UQ);
    __m128 cmp_ltgt = _mm_cmp_ss(vec1, vec2, _CMP_NEQ_UQ);
    
    // Force assembly output with inline asm
    float result = 0.0f;
    __asm__ __volatile__ (
        "movaps %1, %%xmm0\n\t"
        "movaps %2, %%xmm1\n\t"
        "addss %%xmm1, %%xmm0\n\t"
        "movss %%xmm0, %0"
        : "=m"(result)
        : "m"(cmp_unord), "m"(cmp_ord)
        : "xmm0", "xmm1"
    );
    
    return result + *((float*)&cmp_uneq) + *((float*)&cmp_ltgt);
}

int main() {
    // Initialize with various values including special cases
    float values[] = {1.0f, -1.0f, 0.0f, INFINITY, -INFINITY, NAN, 2.5f, -2.5f};
    double dvalues[] = {3.14, -3.14, 0.0, NAN, INFINITY};
    
    float total = 0.0f;
    
    // Test SSE comparisons
    for (int i = 0; i < 4; i++) {
        total += test_sse_comparisons(values[i], values[i+1], values[i+2], values[i+3]);
    }
    
    // Test SSE2 double comparisons
    double dtotal = 0.0;
    for (int i = 0; i < 3; i++) {
        dtotal += test_sse2_comparisons(dvalues[i], dvalues[i+1]);
    }
    
    // Test scalar comparisons
    total += test_scalar_comparisons(values[0], values[1]);
    total += test_scalar_comparisons(values[2], NAN);
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        // Test AVX comparisons if supported
        float avx_result = test_avx_comparisons(
            values[0], values[1], values[2], values[3],
            values[4], values[5], values[6], values[7]
        );
        total += avx_result;
        printf("AVX result: %f\n", avx_result);
    }
#endif
    
    // Use results in conditional to prevent dead code elimination
    if (total > 100.0f || dtotal > 50.0) {
        printf("Results: %f (float), %lf (double)\n", total, dtotal);
    } else {
        printf("Accumulated: %f\n", total + (float)dtotal);
    }
    
    // Additional test: extract comparison masks and branch
    __m128 test_vec = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 cmp_mask = _mm_cmp_ps(test_vec, _mm_set1_ps(2.5f), _CMP_NGE_UQ);  // UNGE -> nlt
    int mask = _mm_movemask_ps(cmp_mask);
    
    if (mask & 1) total *= 1.1f;
    if (mask & 2) total *= 1.2f;
    if (mask & 4) total *= 1.3f;
    if (mask & 8) total *= 1.4f;
    
    printf("Final result: %f\n", total);
    
    return 0;
}
