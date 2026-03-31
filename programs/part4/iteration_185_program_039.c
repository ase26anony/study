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

/* Function to test SSE condition codes */
float test_sse_conditions(float a, float b) {
    float result = 0.0f;
    
#ifdef __SSE__
    __m128 vec_a = _mm_set1_ps(a);
    __m128 vec_b = _mm_set1_ps(b);
    __m128 cmp_result;
    int mask;
    
    /* Test all condition codes from the uncovered block */
    
    // UNORDERED - should generate "unord"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 1.0f;
    
    // ORDERED - should generate "ord"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 2.0f;
    
    // UNEQ - should generate "ueq"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 4.0f;
    
    // UNGE - should generate "nlt"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 8.0f;
    
    // UNGT - should generate "nle"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 16.0f;
    
    // UNLE - should generate "ule"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 32.0f;
    
    // UNLT - should generate "ult"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 64.0f;
    
    // LTGT - should generate "une"
    cmp_result = _mm_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 128.0f;
    
    /* Test with scalar comparisons as well */
    __m128 scalar_a = _mm_set_ss(a);
    __m128 scalar_b = _mm_set_ss(b);
    
    cmp_result = _mm_cmp_ss(scalar_a, scalar_b, _CMP_UNORD_Q);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 256.0f;
    
    cmp_result = _mm_cmp_ss(scalar_a, scalar_b, _CMP_ORD_Q);
    mask = _mm_movemask_ps(cmp_result);
    if (mask) result += 512.0f;
#endif
    
    return result;
}

/* Function to test SSE2 double precision condition codes */
double test_sse2_conditions(double a, double b) {
    double result = 0.0;
    
#ifdef __SSE2__
    __m128d vec_a = _mm_set1_pd(a);
    __m128d vec_b = _mm_set1_pd(b);
    __m128d cmp_result;
    int mask;
    
    // Test with double precision
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 1.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 2.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 4.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 8.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 16.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 32.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 64.0;
    
    cmp_result = _mm_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 128.0;
    
    /* Test scalar double comparisons */
    __m128d scalar_a = _mm_set_sd(a);
    __m128d scalar_b = _mm_set_sd(b);
    
    cmp_result = _mm_cmp_sd(scalar_a, scalar_b, _CMP_UNORD_Q);
    mask = _mm_movemask_pd(cmp_result);
    if (mask) result += 256.0;
#endif
    
    return result;
}

#ifdef __AVX__
/* Function to test AVX condition codes */
float test_avx_conditions(float a, float b) {
    __m256 vec_a = _mm256_set1_ps(a);
    __m256 vec_b = _mm256_set1_ps(b);
    __m256 cmp_result;
    int mask;
    float result = 0.0f;
    
    /* Test all AVX condition codes */
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 1.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 2.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 4.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 8.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 16.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 32.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 64.0f;
    
    cmp_result = _mm256_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm256_movemask_ps(cmp_result);
    if (mask) result += 128.0f;
    
    /* Combine with arithmetic to prevent optimization */
    __m256 blend_result = _mm256_blendv_ps(vec_a, vec_b, cmp_result);
    float* blend_arr = (float*)&blend_result;
    result += blend_arr[0] + blend_arr[4];
    
    return result;
}

/* Function to test AVX double precision condition codes */
double test_avx_double_conditions(double a, double b) {
    __m256d vec_a = _mm256_set1_pd(a);
    __m256d vec_b = _mm256_set1_pd(b);
    __m256d cmp_result;
    int mask;
    double result = 0.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 1.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 2.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 4.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NGE_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 8.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NGT_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 16.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_ULE_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 32.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_ULT_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 64.0;
    
    cmp_result = _mm256_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
    mask = _mm256_movemask_pd(cmp_result);
    if (mask) result += 128.0;
    
    return result;
}
#endif

/* Function with inline assembly to force condition code output */
void test_with_inline_asm(float a, float b) {
#ifdef __SSE__
    __m128 vec_a = _mm_set1_ps(a);
    __m128 vec_b = _mm_set1_ps(b);
    __m128 result;
    
    /* Force assembly output for various condition codes */
    asm volatile (
        "vcmpeqps %1, %0, %0\n\t"
        "vcmpltps %1, %0, %0\n\t"
        : "+x" (result)
        : "x" (vec_b)
        : "cc"
    );
    
    /* Use result to prevent dead code elimination */
    float* res_arr = (float*)&result;
    printf("Inline asm result: %f\n", res_arr[0]);
#endif
}

/* Test NaN comparisons specifically */
void test_nan_comparisons() {
#ifdef __SSE__
    __m128 nan_vec = _mm_set1_ps(NAN);
    __m128 normal_vec = _mm_set1_ps(1.0f);
    __m128 zero_vec = _mm_set1_ps(0.0f);
    __m128 inf_vec = _mm_set1_ps(INFINITY);
    
    __m128 cmp1, cmp2, cmp3, cmp4;
    int mask1, mask2, mask3, mask4;
    
    /* UNORDERED comparisons with NaN */
    cmp1 = _mm_cmp_ps(nan_vec, normal_vec, _CMP_UNORD_Q);
    mask1 = _mm_movemask_ps(cmp1);
    
    cmp2 = _mm_cmp_ps(normal_vec, nan_vec, _CMP_UNORD_Q);
    mask2 = _mm_movemask_ps(cmp2);
    
    cmp3 = _mm_cmp_ps(nan_vec, nan_vec, _CMP_UNORD_Q);
    mask3 = _mm_movemask_ps(cmp3);
    
    cmp4 = _mm_cmp_ps(normal_vec, normal_vec, _CMP_ORD_Q);
    mask4 = _mm_movemask_ps(cmp4);
    
    printf("NaN comparison masks: %d %d %d %d\n", mask1, mask2, mask3, mask4);
    
    /* Test all condition codes with NaN */
    __m128 cmp_results[8];
    cmp_results[0] = _mm_cmp_ps(nan_vec, normal_vec, _CMP_UNORD_Q);
    cmp_results[1] = _mm_cmp_ps(nan_vec, normal_vec, _CMP_ORD_Q);
    cmp_results[2] = _mm_cmp_ps(nan_vec, normal_vec, _CMP_UNEQ_UQ);
    cmp_results[3] = _mm_cmp_ps(nan_vec, normal_vec, _CMP_NGE_UQ);
    cmp_results[4] = _mm_cmp_ps(nan_vec, normal_vec, _CMP_NGT_UQ);
    cmp_results[5] = _mm_cmp_ps(nan_vec, normal_vec, _CMP_ULE_UQ);
    cmp_results[6] = _mm_cmp_ps(nan_vec, normal_vec, _CMP_ULT_UQ);
    cmp_results[7] = _mm_cmp_ps(nan_vec, normal_vec, _CMP_NEQ_UQ);
    
    /* Use results in arithmetic to prevent optimization */
    __m128 sum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        sum = _mm_add_ps(sum, cmp_results[i]);
    }
    float sum_arr[4];
    _mm_store_ps(sum_arr, sum);
    printf("NaN comparison sum: %f\n", sum_arr[0]);
#endif
}

int main() {
    float f1 = 1.5f;
    float f2 = 2.5f;
    float f3 = NAN;
    float f4 = INFINITY;
    
    double d1 = 1.5;
    double d2 = 2.5;
    double d3 = NAN;
    
    printf("Testing SSE float comparisons:\n");
    float sse_result = test_sse_conditions(f1, f2);
    printf("SSE float result: %f\n", sse_result);
    
    printf("\nTesting SSE double comparisons:\n");
    double sse2_result = test_sse2_conditions(d1, d2);
    printf("SSE double result: %f\n", sse2_result);
    
    printf("\nTesting NaN comparisons:\n");
    test_nan_comparisons();
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        printf("\nTesting AVX float comparisons:\n");
        float avx_result = test_avx_conditions(f1, f2);
        printf("AVX float result: %f\n", avx_result);
        
        printf("\nTesting AVX double comparisons:\n");
        double avx_double_result = test_avx_double_conditions(d1, d2);
        printf("AVX double result: %f\n", avx_double_result);
        
        /* Test with NaN values in AVX */
        printf("\nTesting AVX with NaN:\n");
        float avx_nan_result = test_avx_conditions(f3, f1);
        printf("AVX NaN result: %f\n", avx_nan_result);
    } else {
        printf("AVX not supported on this CPU\n");
    }
#endif
    
    printf("\nTesting with inline assembly:\n");
    test_with_inline_asm(f1, f2);
    
    /* Test edge cases */
    printf("\nTesting edge cases:\n");
    float edge_result = test_sse_conditions(0.0f, -0.0f);
    printf("0 vs -0 result: %f\n", edge_result);
    
    edge_result = test_sse_conditions(f4, f1);
    printf("INF vs normal result: %f\n", edge_result);
    
    edge_result = test_sse_conditions(f3, f3);
    printf("NaN vs NaN result: %f\n", edge_result);
    
    return 0;
}
