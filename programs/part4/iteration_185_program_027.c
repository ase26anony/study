#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifdef __AVX__
#include <cpuid.h>
#endif

// Function to check AVX support at runtime
static int check_avx_support(void) {
#ifdef __AVX__
    unsigned int eax, ebx, ecx, edx;
    __cpuid(1, eax, ebx, ecx, edx);
    return (ecx & (1 << 28)) != 0;
#else
    return 0;
#endif
}

// SSE path using all condition codes
void test_sse_comparisons(void) {
    // Initialize vectors with various values including NaN
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_setr_ps(1.0f, 3.0f, 3.0f, INFINITY);
    __m128 c = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    
    // Results storage to prevent optimization
    __m128 results[16];
    int result_idx = 0;
    
    // Test all condition codes from uncovered block
    // Using explicit condition codes that map to the uncovered cases
    
    // UNORDERED (_CMP_UNORD_Q = 3)
    results[result_idx++] = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    
    // ORDERED (_CMP_ORD_Q = 7)
    results[result_idx++] = _mm_cmp_ps(b, c, _CMP_ORD_Q);
    
    // UNEQ (_CMP_UNEQ_UQ = 8)
    results[result_idx++] = _mm_cmp_ps(a, a, _CMP_UNEQ_UQ);
    
    // UNGE (_CMP_NGE_UQ = 9) - "nlt"
    results[result_idx++] = _mm_cmp_ps(a, b, _CMP_NGE_UQ);
    
    // UNGT (_CMP_NGT_UQ = 10) - "nle"
    results[result_idx++] = _mm_cmp_ps(b, c, _CMP_NGT_UQ);
    
    // UNLE (_CMP_ULE_UQ = 11) - "ule"
    results[result_idx++] = _mm_cmp_ps(c, a, _CMP_ULE_UQ);
    
    // UNLT (_CMP_ULT_UQ = 12) - "ult"
    results[result_idx++] = _mm_cmp_ps(a, c, _CMP_ULT_UQ);
    
    // LTGT (_CMP_NEQ_UQ = 4) - "une"
    results[result_idx++] = _mm_cmp_ps(b, b, _CMP_NEQ_UQ);
    
    // Double precision comparisons (SSE2)
    __m128d ad = _mm_setr_pd(1.0, NAN);
    __m128d bd = _mm_setr_pd(1.0, 2.0);
    
    // Test double precision with same condition codes
    results[result_idx++] = _mm_castpd_ps(_mm_cmp_pd(ad, bd, _CMP_UNORD_Q));
    results[result_idx++] = _mm_castpd_ps(_mm_cmp_pd(ad, ad, _CMP_ORD_Q));
    results[result_idx++] = _mm_castpd_ps(_mm_cmp_pd(bd, bd, _CMP_UNEQ_UQ));
    
    // Scalar comparisons (SSE)
    results[result_idx++] = _mm_castss_ps(_mm_cmp_ss(a, b, _CMP_NGE_UQ));
    results[result_idx++] = _mm_castss_ps(_mm_cmp_ss(b, c, _CMP_NGT_UQ));
    
    // Use results in control flow to prevent dead code elimination
    int final_mask = 0;
    for (int i = 0; i < result_idx; i++) {
        int mask = _mm_movemask_ps(results[i]);
        final_mask ^= mask;  // XOR to combine all masks
    }
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE Comparison Results: %0"
        : 
        : "r" (final_mask)
        : "memory"
    );
    
    printf("SSE final mask: 0x%x\n", final_mask);
}

#ifdef __AVX__
// AVX path using 256-bit vectors
void test_avx_comparisons(void) {
    // Initialize AVX vectors
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, NAN, 8.0f);
    __m256 b = _mm256_setr_ps(1.0f, 3.0f, 3.0f, INFINITY, 5.0f, 7.0f, 7.0f, INFINITY);
    __m256 c = _mm256_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY, 0.0f, -0.0f, INFINITY, -INFINITY);
    
    // Results storage
    __m256 avx_results[8];
    int avx_idx = 0;
    
    // Test AVX comparisons with all condition codes
    avx_results[avx_idx++] = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    avx_results[avx_idx++] = _mm256_cmp_ps(b, c, _CMP_ORD_Q);
    avx_results[avx_idx++] = _mm256_cmp_ps(a, a, _CMP_UNEQ_UQ);
    avx_results[avx_idx++] = _mm256_cmp_ps(a, b, _CMP_NGE_UQ);
    avx_results[avx_idx++] = _mm256_cmp_ps(b, c, _CMP_NGT_UQ);
    avx_results[avx_idx++] = _mm256_cmp_ps(c, a, _CMP_ULE_UQ);
    avx_results[avx_idx++] = _mm256_cmp_ps(a, c, _CMP_ULT_UQ);
    avx_results[avx_idx++] = _mm256_cmp_ps(b, b, _CMP_NEQ_UQ);
    
    // AVX double precision
    __m256d ad = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d bd = _mm256_setr_pd(1.0, 2.0, 3.0, 5.0);
    
    __m256d dbl_results[4];
    dbl_results[0] = _mm256_cmp_pd(ad, bd, _CMP_UNORD_Q);
    dbl_results[1] = _mm256_cmp_pd(ad, ad, _CMP_ORD_Q);
    dbl_results[2] = _mm256_cmp_pd(bd, bd, _CMP_UNEQ_UQ);
    dbl_results[3] = _mm256_cmp_pd(ad, bd, _CMP_NGE_UQ);
    
    // Complex expression mixing comparisons and arithmetic
    __m256 blend_result = _mm256_blendv_ps(a, b, avx_results[0]);
    __m256 arith_result = _mm256_add_ps(blend_result, _mm256_mul_ps(c, avx_results[1]));
    
    // Extract masks and use in control flow
    int avx_final_mask = 0;
    for (int i = 0; i < avx_idx; i++) {
        int mask = _mm256_movemask_ps(avx_results[i]);
        avx_final_mask ^= mask;
    }
    
    for (int i = 0; i < 4; i++) {
        int mask = _mm256_movemask_pd(dbl_results[i]);
        avx_final_mask ^= mask;
    }
    
    // Force assembly output
    float arith_data[8];
    _mm256_storeu_ps(arith_data, arith_result);
    
    __asm__ __volatile__ (
        "# AVX Comparison Results: %0 %1"
        : 
        : "r" (avx_final_mask), "r" ((uintptr_t)arith_data)
        : "memory"
    );
    
    printf("AVX final mask: 0x%x\n", avx_final_mask);
}
#endif

int main(void) {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Always test SSE path
    test_sse_comparisons();
    
    // Test AVX path if supported
#ifdef __AVX__
    if (check_avx_support()) {
        test_avx_comparisons();
    } else {
        printf("AVX not supported on this processor\n");
    }
#else
    printf("AVX not compiled in\n");
#endif
    
    // Additional test with mixed operations
    __m128 v1 = _mm_setr_ps(1.0f, NAN, 3.0f, 4.0f);
    __m128 v2 = _mm_setr_ps(1.0f, 2.0f, 4.0f, 4.0f);
    
    // Create a complex dependency chain
    __m128 cmp1 = _mm_cmp_ps(v1, v2, _CMP_UNORD_Q);   // unord
    __m128 cmp2 = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);     // ord
    __m128 cmp3 = _mm_cmp_ps(v1, v2, _CMP_UNEQ_UQ);   // ueq
    
    // Use comparisons in arithmetic
    __m128 masked1 = _mm_and_ps(v1, cmp1);
    __m128 masked2 = _mm_andnot_ps(cmp2, v2);
    __m128 result = _mm_add_ps(masked1, masked2);
    
    // Extract and print to prevent optimization
    float res[4];
    _mm_storeu_ps(res, result);
    printf("Mixed ops result: %f %f %f %f\n", res[0], res[1], res[2], res[3]);
    
    return 0;
}
