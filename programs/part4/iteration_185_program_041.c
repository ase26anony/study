#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <immintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>

#ifdef __AVX__
#include <avxintrin.h>
#endif

// Function to test all SSE condition codes
void test_sse_condition_codes(void) {
    // Initialize vectors with various values including NaN
    __m128 a_ps = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b_ps = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c_ps = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    
    __m128d a_pd = _mm_setr_pd(1.0, NAN);
    __m128d b_pd = _mm_setr_pd(2.0, 2.0);
    
    // Results storage
    __m128 results_ps[8];
    __m128d results_pd[8];
    
    // Test all condition codes from the uncovered block
    // Using explicit condition codes that map to the uncovered cases
    
    // 1. UNORDERED (_CMP_UNORD_Q)
    results_ps[0] = _mm_cmp_ps(a_ps, b_ps, _CMP_UNORD_Q);
    results_pd[0] = _mm_cmp_pd(a_pd, b_pd, _CMP_UNORD_Q);
    
    // 2. ORDERED (_CMP_ORD_Q)
    results_ps[1] = _mm_cmp_ps(a_ps, b_ps, _CMP_ORD_Q);
    results_pd[1] = _mm_cmp_pd(a_pd, b_pd, _CMP_ORD_Q);
    
    // 3. UNEQ (_CMP_UNEQ_UQ)
    results_ps[2] = _mm_cmp_ps(a_ps, b_ps, _CMP_UNEQ_UQ);
    results_pd[2] = _mm_cmp_pd(a_pd, b_pd, _CMP_UNEQ_UQ);
    
    // 4. UNGE (_CMP_NGE_UQ) - maps to "nlt"
    results_ps[3] = _mm_cmp_ps(a_ps, b_ps, _CMP_NGE_UQ);
    results_pd[3] = _mm_cmp_pd(a_pd, b_pd, _CMP_NGE_UQ);
    
    // 5. UNGT (_CMP_NGT_UQ) - maps to "nle"
    results_ps[4] = _mm_cmp_ps(a_ps, b_ps, _CMP_NGT_UQ);
    results_pd[4] = _mm_cmp_pd(a_pd, b_pd, _CMP_NGT_UQ);
    
    // 6. UNLE (_CMP_ULE_UQ) - maps to "ule"
    results_ps[5] = _mm_cmp_ps(a_ps, b_ps, _CMP_ULE_UQ);
    results_pd[5] = _mm_cmp_pd(a_pd, b_pd, _CMP_ULE_UQ);
    
    // 7. UNLT (_CMP_ULT_UQ) - maps to "ult"
    results_ps[6] = _mm_cmp_ps(a_ps, b_ps, _CMP_ULT_UQ);
    results_pd[6] = _mm_cmp_pd(a_pd, b_pd, _CMP_ULT_UQ);
    
    // 8. LTGT (_CMP_NEQ_UQ) - maps to "une"
    results_ps[7] = _mm_cmp_ps(a_ps, b_ps, _CMP_NEQ_UQ);
    results_pd[7] = _mm_cmp_pd(a_pd, b_pd, _CMP_NEQ_UQ);
    
    // Use results in arithmetic operations to prevent dead code elimination
    __m128 sum_ps = _mm_setzero_ps();
    __m128d sum_pd = _mm_setzero_pd();
    
    for (int i = 0; i < 8; i++) {
        // Blend based on comparison results
        __m128 mask_ps = results_ps[i];
        __m128 blended_ps = _mm_blendv_ps(a_ps, c_ps, mask_ps);
        sum_ps = _mm_add_ps(sum_ps, blended_ps);
        
        __m128d mask_pd = results_pd[i];
        __m128d blended_pd = _mm_blendv_pd(a_pd, _mm_set1_pd(1.0), mask_pd);
        sum_pd = _mm_add_pd(sum_pd, blended_pd);
    }
    
    // Extract masks and use in conditional logic
    int final_result = 0;
    for (int i = 0; i < 8; i++) {
        int mask_ps = _mm_movemask_ps(results_ps[i]);
        int mask_pd = _mm_movemask_pd(results_pd[i]);
        
        if (mask_ps != 0 || mask_pd != 0) {
            final_result += mask_ps + mask_pd;
        }
    }
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE Condition Code Test Marker\n"
        : 
        : "r" (final_result)
        : "memory"
    );
    
    printf("SSE test result: %d\n", final_result);
}

#ifdef __AVX__
// Function to test AVX condition codes
void test_avx_condition_codes(void) {
    // Initialize AVX vectors
    __m256 a_ps_avx = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 b_ps_avx = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 6.0f, 6.0f, 8.0f, 8.0f);
    __m256 c_ps_avx = _mm256_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f, 1.0f, 2.0f, 3.0f, 4.0f);
    
    __m256d a_pd_avx = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d b_pd_avx = _mm256_setr_pd(2.0, 2.0, 3.0, NAN);
    
    // Test AVX comparisons with all condition codes
    __m256 results_ps_avx[8];
    __m256d results_pd_avx[8];
    
    // 1. UNORDERED
    results_ps_avx[0] = _mm256_cmp_ps(a_ps_avx, b_ps_avx, _CMP_UNORD_Q);
    results_pd_avx[0] = _mm256_cmp_pd(a_pd_avx, b_pd_avx, _CMP_UNORD_Q);
    
    // 2. ORDERED
    results_ps_avx[1] = _mm256_cmp_ps(a_ps_avx, b_ps_avx, _CMP_ORD_Q);
    results_pd_avx[1] = _mm256_cmp_pd(a_pd_avx, b_pd_avx, _CMP_ORD_Q);
    
    // 3. UNEQ
    results_ps_avx[2] = _mm256_cmp_ps(a_ps_avx, b_ps_avx, _CMP_UNEQ_UQ);
    results_pd_avx[2] = _mm256_cmp_pd(a_pd_avx, b_pd_avx, _CMP_UNEQ_UQ);
    
    // 4. UNGE (nlt)
    results_ps_avx[3] = _mm256_cmp_ps(a_ps_avx, b_ps_avx, _CMP_NGE_UQ);
    results_pd_avx[3] = _mm256_cmp_pd(a_pd_avx, b_pd_avx, _CMP_NGE_UQ);
    
    // 5. UNGT (nle)
    results_ps_avx[4] = _mm256_cmp_ps(a_ps_avx, b_ps_avx, _CMP_NGT_UQ);
    results_pd_avx[4] = _mm256_cmp_pd(a_pd_avx, b_pd_avx, _CMP_NGT_UQ);
    
    // 6. UNLE (ule)
    results_ps_avx[5] = _mm256_cmp_ps(a_ps_avx, b_ps_avx, _CMP_ULE_UQ);
    results_pd_avx[5] = _mm256_cmp_pd(a_pd_avx, b_pd_avx, _CMP_ULE_UQ);
    
    // 7. UNLT (ult)
    results_ps_avx[6] = _mm256_cmp_ps(a_ps_avx, b_ps_avx, _CMP_ULT_UQ);
    results_pd_avx[6] = _mm256_cmp_pd(a_pd_avx, b_pd_avx, _CMP_ULT_UQ);
    
    // 8. LTGT (une)
    results_ps_avx[7] = _mm256_cmp_ps(a_ps_avx, b_ps_avx, _CMP_NEQ_UQ);
    results_pd_avx[7] = _mm256_cmp_pd(a_pd_avx, b_pd_avx, _CMP_NEQ_UQ);
    
    // Complex expression mixing comparisons and arithmetic
    __m256 accum_ps = _mm256_setzero_ps();
    __m256d accum_pd = _mm256_setzero_pd();
    
    for (int i = 0; i < 8; i++) {
        // Create dependent operations
        __m256 mask_ps = results_ps_avx[i];
        __m256 temp_ps = _mm256_mul_ps(a_ps_avx, _mm256_set1_ps(1.5f));
        __m256 blended_ps = _mm256_blendv_ps(temp_ps, c_ps_avx, mask_ps);
        accum_ps = _mm256_add_ps(accum_ps, blended_ps);
        
        __m256d mask_pd = results_pd_avx[i];
        __m256d temp_pd = _mm256_mul_pd(a_pd_avx, _mm256_set1_pd(2.0));
        __m256d blended_pd = _mm256_blendv_pd(temp_pd, _mm256_set1_pd(3.0), mask_pd);
        accum_pd = _mm256_add_pd(accum_pd, blended_pd);
    }
    
    // Extract and use results
    float final_float_result = 0.0f;
    double final_double_result = 0.0;
    
    float* accum_ps_ptr = (float*)&accum_ps;
    double* accum_pd_ptr = (double*)&accum_pd;
    
    for (int i = 0; i < 8; i++) {
        final_float_result += accum_ps_ptr[i];
    }
    
    for (int i = 0; i < 4; i++) {
        final_double_result += accum_pd_ptr[i];
    }
    
    // Force AVX assembly output
    __asm__ __volatile__ (
        "# AVX Condition Code Test Marker\n"
        : 
        : "r" (final_float_result), "r" (final_double_result)
        : "memory"
    );
    
    printf("AVX test results: float=%f, double=%f\n", final_float_result, final_double_result);
}
#endif

// Test scalar comparisons as well
void test_scalar_condition_codes(void) {
    // Scalar comparisons also use the same condition codes
    __m128 a = _mm_set_ss(1.0f);
    __m128 b = _mm_set_ss(NAN);
    __m128 c = _mm_set_ss(2.0f);
    
    // Test scalar condition codes
    __m128 result_unord = _mm_cmp_ss(a, b, _CMP_UNORD_Q);
    __m128 result_ord = _mm_cmp_ss(a, c, _CMP_ORD_Q);
    __m128 result_uneq = _mm_cmp_ss(a, c, _CMP_UNEQ_UQ);
    __m128 result_nlt = _mm_cmp_ss(c, a, _CMP_NGE_UQ);  // nlt
    __m128 result_nle = _mm_cmp_ss(c, a, _CMP_NGT_UQ);  // nle
    
    // Use results
    float results[5];
    _mm_store_ss(&results[0], result_unord);
    _mm_store_ss(&results[1], result_ord);
    _mm_store_ss(&results[2], result_uneq);
    _mm_store_ss(&results[3], result_nlt);
    _mm_store_ss(&results[4], result_nle);
    
    printf("Scalar test: %f %f %f %f %f\n", 
           results[0], results[1], results[2], results[3], results[4]);
}

int main(void) {
    printf("Testing x86 condition code printing logic...\n");
    
    // Always test SSE
    test_sse_condition_codes();
    
    // Test scalar comparisons
    test_scalar_condition_codes();
    
    // Test AVX if available
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        test_avx_condition_codes();
    } else {
        printf("AVX not supported on this CPU\n");
    }
#else
    printf("AVX not enabled at compile time\n");
#endif
    
    return 0;
}
