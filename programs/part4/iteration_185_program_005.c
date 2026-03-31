#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifdef __AVX__
#include <cpuid.h>
#endif

// Function to test SSE condition codes
void test_sse_condition_codes(void) {
    // Initialize vectors with various values including NaN
    __m128 a_ps = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b_ps = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c_ps = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 d_ps = _mm_set1_ps(1.5f);
    
    __m128 pd_a = _mm_setr_pd(1.0, NAN);
    __m128 pd_b = _mm_setr_pd(NAN, 2.0);
    
    // Results storage
    __m128 results[16];
    int result_idx = 0;
    
    // Test all condition codes from uncovered block
    // Using explicit intrinsic calls that map to condition codes
    
    // 1. UNORDERED (_CMP_UNORD_Q)
    results[result_idx++] = _mm_cmp_ps(a_ps, b_ps, _CMP_UNORD_Q);
    
    // 2. ORDERED (_CMP_ORD_Q)
    results[result_idx++] = _mm_cmp_ps(a_ps, b_ps, _CMP_ORD_Q);
    
    // 3. UNEQ (_CMP_UNEQ_UQ)
    results[result_idx++] = _mm_cmp_ps(a_ps, b_ps, _CMP_UNEQ_UQ);
    
    // 4. UNGE (_CMP_NGE_UQ) - prints "nlt"
    results[result_idx++] = _mm_cmp_ps(a_ps, b_ps, _CMP_NGE_UQ);
    
    // 5. UNGT (_CMP_NGT_UQ) - prints "nle"
    results[result_idx++] = _mm_cmp_ps(a_ps, b_ps, _CMP_NGT_UQ);
    
    // 6. UNLE (_CMP_ULE_UQ) - prints "ule"
    results[result_idx++] = _mm_cmp_ps(a_ps, b_ps, _CMP_ULE_UQ);
    
    // 7. UNLT (_CMP_ULT_UQ) - prints "ult"
    results[result_idx++] = _mm_cmp_ps(a_ps, b_ps, _CMP_ULT_UQ);
    
    // 8. LTGT (_CMP_NEQ_UQ) - prints "une"
    results[result_idx++] = _mm_cmp_ps(a_ps, b_ps, _CMP_NEQ_UQ);
    
    // Test double precision variants
    results[result_idx++] = _mm_cmp_pd(pd_a, pd_b, _CMP_UNORD_Q);
    results[result_idx++] = _mm_cmp_pd(pd_a, pd_b, _CMP_ORD_Q);
    results[result_idx++] = _mm_cmp_pd(pd_a, pd_b, _CMP_UNEQ_UQ);
    results[result_idx++] = _mm_cmp_pd(pd_a, pd_b, _CMP_NGE_UQ);
    
    // Test scalar comparisons (SSE)
    __m128 scalar_result = _mm_cmp_ss(a_ps, b_ps, _CMP_UNORD_Q);
    results[result_idx++] = scalar_result;
    
    // Use results in arithmetic to prevent dead code elimination
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < result_idx; i++) {
        // Convert comparison masks to usable values
        __m128 mask_as_float = _mm_and_ps(results[i], _mm_set1_ps(1.0f));
        accum = _mm_add_ps(accum, mask_as_float);
    }
    
    // Extract and use result to prevent optimization
    float final_result[4];
    _mm_storeu_ps(final_result, accum);
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results stored\n"
        : 
        : "x" (accum)
        : "memory"
    );
}

#ifdef __AVX__
void test_avx_condition_codes(void) {
    // AVX vectors
    __m256 avx_a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 9.0f, 10.0f, NAN, 12.0f);
    
    __m256d avx_da = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d avx_db = _mm256_setr_pd(NAN, 2.0, 3.0, NAN);
    
    // Test AVX variants of all condition codes
    __m256 avx_results[8];
    
    avx_results[0] = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNORD_Q);    // UNORDERED
    avx_results[1] = _mm256_cmp_ps(avx_a, avx_b, _CMP_ORD_Q);      // ORDERED
    avx_results[2] = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNEQ_UQ);    // UNEQ
    avx_results[3] = _mm256_cmp_ps(avx_a, avx_b, _CMP_NGE_UQ);     // UNGE -> "nlt"
    avx_results[4] = _mm256_cmp_ps(avx_a, avx_b, _CMP_NGT_UQ);     // UNGT -> "nle"
    avx_results[5] = _mm256_cmp_ps(avx_a, avx_b, _CMP_ULE_UQ);     // UNLE -> "ule"
    avx_results[6] = _mm256_cmp_ps(avx_a, avx_b, _CMP_ULT_UQ);     // UNLT -> "ult"
    avx_results[7] = _mm256_cmp_ps(avx_a, avx_b, _CMP_NEQ_UQ);     // LTGT -> "une"
    
    // Test AVX double precision
    __m256d dbl_result1 = _mm256_cmp_pd(avx_da, avx_db, _CMP_UNORD_Q);
    __m256d dbl_result2 = _mm256_cmp_pd(avx_da, avx_db, _CMP_ORD_Q);
    
    // Complex expression mixing comparisons and arithmetic
    __m256 mask1 = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNORD_Q);
    __m256 mask2 = _mm256_cmp_ps(avx_a, avx_b, _CMP_ORD_Q);
    
    // Blend based on comparison results
    __m256 blended = _mm256_blendv_ps(avx_a, avx_b, mask1);
    blended = _mm256_add_ps(blended, _mm256_and_ps(mask2, _mm256_set1_ps(10.0f)));
    
    // Extract mask and use in control flow
    int mask_bits = _mm256_movemask_ps(mask1);
    if (mask_bits != 0) {
        // Use blended result
        float temp[8];
        _mm256_storeu_ps(temp, blended);
        
        // Force assembly output
        __asm__ __volatile__ (
            "# AVX comparison and blend result\n"
            : 
            : "x" (blended)
            : "memory"
        );
    }
}
#endif

int main(void) {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Always test SSE
    test_sse_condition_codes();
    
    // Test AVX if supported at compile time
#ifdef __AVX__
    test_avx_condition_codes();
    printf("AVX code path tested\n");
#else
    printf("AVX not available at compile time\n");
#endif
    
    // Runtime check for AVX (optional)
    unsigned int eax, ebx, ecx, edx;
    __cpuid(1, eax, ebx, ecx, edx);
    
    if (ecx & (1 << 28)) {  // Check OSXSAVE bit
        printf("AVX supported by CPU\n");
    }
    
    return 0;
}
