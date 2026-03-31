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

// Function to test all SSE condition codes
void test_sse_condition_codes(void) {
    // Initialize vectors with various values including NaN
    __m128 vec_a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec_b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 vec_c = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    
    // Results storage
    __m128 results[8];
    
    // Test all condition codes from the uncovered block
    // Each comparison should generate assembly with the condition code string
    
    // 1. UNORDERED (handles NaN comparisons)
    results[0] = _mm_cmp_ps(vec_a, vec_b, _CMP_UNORD_Q);
    
    // 2. ORDERED
    results[1] = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    
    // 3. UNEQ (unordered or equal)
    results[2] = _mm_cmp_ps(vec_a, vec_b, _CMP_UNEQ_UQ);
    
    // 4. UNGE (not less than, unordered)
    results[3] = _mm_cmp_ps(vec_a, vec_b, _CMP_NGE_UQ);
    
    // 5. UNGT (not less than or equal, unordered)
    results[4] = _mm_cmp_ps(vec_a, vec_b, _CMP_NGT_UQ);
    
    // 6. UNLE (unordered or less than or equal)
    results[5] = _mm_cmp_ps(vec_a, vec_b, _CMP_ULE_UQ);
    
    // 7. UNLT (unordered or less than)
    results[6] = _mm_cmp_ps(vec_a, vec_b, _CMP_ULT_UQ);
    
    // 8. LTGT (less than or greater than, unordered)
    results[7] = _mm_cmp_ps(vec_a, vec_b, _CMP_NEQ_UQ);
    
    // Use results in control flow to prevent optimization
    int mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= _mm_movemask_ps(results[i]);
    }
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results used here"
        : 
        : "x" (results[0]), "x" (results[1]), "x" (results[2]),
          "x" (results[3]), "x" (results[4]), "x" (results[5]),
          "x" (results[6]), "x" (results[7])
    );
    
    // Complex expression mixing comparisons and arithmetic
    __m128 temp = _mm_add_ps(vec_a, vec_b);
    __m128 cmp1 = _mm_cmp_ps(temp, vec_c, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ps(vec_a, vec_b, _CMP_ORD_Q);
    __m128 blended = _mm_blendv_ps(vec_a, vec_b, cmp1);
    blended = _mm_add_ps(blended, _mm_and_ps(cmp2, vec_c));
    
    // Extract to prevent dead code elimination
    float result_store[4];
    _mm_storeu_ps(result_store, blended);
    
    printf("SSE test mask: %d\n", mask);
}

// Double precision version
void test_sse2_condition_codes(void) {
    __m128d vec_a = _mm_setr_pd(1.0, NAN);
    __m128d vec_b = _mm_setr_pd(2.0, 2.0);
    __m128d vec_c = _mm_setr_pd(0.0, INFINITY);
    
    // Test with double precision
    __m128d d_results[8];
    
    d_results[0] = _mm_cmp_pd(vec_a, vec_b, _CMP_UNORD_Q);
    d_results[1] = _mm_cmp_pd(vec_a, vec_b, _CMP_ORD_Q);
    d_results[2] = _mm_cmp_pd(vec_a, vec_b, _CMP_UNEQ_UQ);
    d_results[3] = _mm_cmp_pd(vec_a, vec_b, _CMP_NGE_UQ);
    d_results[4] = _mm_cmp_pd(vec_a, vec_b, _CMP_NGT_UQ);
    d_results[5] = _mm_cmp_pd(vec_a, vec_b, _CMP_ULE_UQ);
    d_results[6] = _mm_cmp_pd(vec_a, vec_b, _CMP_ULT_UQ);
    d_results[7] = _mm_cmp_pd(vec_a, vec_b, _CMP_NEQ_UQ);
    
    // Scalar comparisons also trigger the same code path
    __m128d scalar_cmp = _mm_cmp_sd(vec_a, vec_b, _CMP_UNORD_Q);
    
    // Complex control flow
    int mask = _mm_movemask_pd(d_results[0]) | _mm_movemask_pd(d_results[7]);
    
    // Force assembly generation
    __asm__ __volatile__ (
        "# SSE2 double precision comparisons"
        : 
        : "x" (d_results[0]), "x" (d_results[1]), "x" (scalar_cmp)
    );
    
    printf("SSE2 test mask: %d\n", mask);
}

#ifdef __AVX__
void test_avx_condition_codes(void) {
    // AVX 256-bit vectors
    __m256 avx_a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 9.0f, 10.0f, 11.0f, 12.0f);
    
    __m256 avx_results[8];
    
    // Test all condition codes with AVX
    avx_results[0] = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNORD_Q);
    avx_results[1] = _mm256_cmp_ps(avx_a, avx_b, _CMP_ORD_Q);
    avx_results[2] = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNEQ_UQ);
    avx_results[3] = _mm256_cmp_ps(avx_a, avx_b, _CMP_NGE_UQ);
    avx_results[4] = _mm256_cmp_ps(avx_a, avx_b, _CMP_NGT_UQ);
    avx_results[5] = _mm256_cmp_ps(avx_a, avx_b, _CMP_ULE_UQ);
    avx_results[6] = _mm256_cmp_ps(avx_a, avx_b, _CMP_ULT_UQ);
    avx_results[7] = _mm256_cmp_ps(avx_a, avx_b, _CMP_NEQ_UQ);
    
    // AVX double precision
    __m256d avx_da = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d avx_db = _mm256_setr_pd(2.0, 2.0, NAN, 5.0);
    
    __m256d avx_dresult = _mm256_cmp_pd(avx_da, avx_db, _CMP_UNORD_Q);
    
    // Use in complex expression
    __m256 blended = _mm256_blendv_ps(avx_a, avx_b, avx_results[0]);
    blended = _mm256_add_ps(blended, _mm256_and_ps(avx_results[1], avx_b));
    
    // Extract mask for control flow
    int mask = _mm256_movemask_ps(avx_results[0]);
    
    __asm__ __volatile__ (
        "# AVX 256-bit comparisons"
        : 
        : "x" (avx_results[0]), "x" (avx_results[7]), "x" (avx_dresult)
    );
    
    printf("AVX test mask: %d\n", mask);
}
#endif

// Function that mixes all types of comparisons in a complex expression
float complex_expression_test(float *input, int n) {
    float sum = 0.0f;
    
    #ifdef __SSE__
    // Process in vectors if possible
    for (int i = 0; i < n - 3; i += 4) {
        __m128 v1 = _mm_loadu_ps(&input[i]);
        __m128 v2 = _mm_loadu_ps(&input[i + 4 < n ? i + 4 : 0]);
        
        // Multiple comparisons in one expression
        __m128 cmp_unord = _mm_cmp_ps(v1, v2, _CMP_UNORD_Q);
        __m128 cmp_ord = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);
        __m128 cmp_uneq = _mm_cmp_ps(v1, v2, _CMP_UNEQ_UQ);
        
        // Blend based on comparison results
        __m128 blended1 = _mm_blendv_ps(v1, v2, cmp_unord);
        __m128 blended2 = _mm_blendv_ps(v1, v2, cmp_ord);
        __m128 result = _mm_add_ps(blended1, blended2);
        result = _mm_add_ps(result, _mm_and_ps(cmp_uneq, v1));
        
        // Horizontal sum
        __m128 shuf = _mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 3, 0, 1));
        __m128 sums = _mm_add_ps(result, shuf);
        shuf = _mm_movehl_ps(shuf, sums);
        sums = _mm_add_ss(sums, shuf);
        
        sum += _mm_cvtss_f32(sums);
    }
    #endif
    
    return sum;
}

int main(void) {
    printf("Testing x86 condition code generation for uncovered GCC block\n");
    
    // Initialize test data with NaN values
    float test_data[16];
    for (int i = 0; i < 16; i++) {
        test_data[i] = (float)i * 1.5f;
        if (i % 5 == 0) test_data[i] = NAN;
        if (i % 7 == 0) test_data[i] = INFINITY;
    }
    
    // Run all tests
    test_sse_condition_codes();
    test_sse2_condition_codes();
    
    #ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        test_avx_condition_codes();
    }
    #endif
    
    // Complex expression test
    float result = complex_expression_test(test_data, 16);
    printf("Complex expression result: %f\n", result);
    
    // Additional test with volatile to force assembly output
    volatile __m128 force_asm = _mm_setzero_ps();
    __m128 v1 = _mm_setr_ps(1.0f, NAN, 3.0f, 4.0f);
    __m128 v2 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    
    // This should generate assembly with "unord" condition code
    __m128 cmp_result = _mm_cmp_ps(v1, v2, _CMP_UNORD_Q);
    
    // And this with "ord"
    __m128 cmp_result2 = _mm_cmp_ps(v1, v2, _CMP_ORD_Q);
    
    // Force use in inline assembly
    __asm__ __volatile__ (
        "# Final assembly forcing condition codes"
        : "=x" (force_asm)
        : "0" (cmp_result), "x" (cmp_result2)
    );
    
    return 0;
}
