#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

#ifdef __AVX__
#define USE_AVX 1
#else
#define USE_AVX 0
#endif

#ifdef __SSE__
#define USE_SSE 1
#else
#define USE_SSE 0
#endif

// Function to test all SSE condition codes
void test_sse_condition_codes(void) {
#if USE_SSE
    printf("Testing SSE condition codes...\n");
    
    // Create test vectors with various values including NaN
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(1.0f, 3.0f, NAN, 0.0f);
    __m128 vec3 = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 5.0f);
    
    // Variables to store comparison results
    __m128 cmp_results[8];
    int masks[8];
    
    // Test all condition codes from the uncovered block
    // Using explicit condition codes that map to the uncovered cases
    
    // 1. UNORDERED (_CMP_UNORD_Q)
    cmp_results[0] = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    masks[0] = _mm_movemask_ps(cmp_results[0]);
    
    // 2. ORDERED (_CMP_ORD_Q)
    cmp_results[1] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    masks[1] = _mm_movemask_ps(cmp_results[1]);
    
    // 3. UNEQ (_CMP_UNEQ_UQ)
    cmp_results[2] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    masks[2] = _mm_movemask_ps(cmp_results[2]);
    
    // 4. UNGE (_CMP_NGE_UQ) - maps to "nlt"
    cmp_results[3] = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    masks[3] = _mm_movemask_ps(cmp_results[3]);
    
    // 5. UNGT (_CMP_NGT_UQ) - maps to "nle"
    cmp_results[4] = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    masks[4] = _mm_movemask_ps(cmp_results[4]);
    
    // 6. UNLE (_CMP_ULE_UQ) - maps to "ule"
    cmp_results[5] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    masks[5] = _mm_movemask_ps(cmp_results[5]);
    
    // 7. UNLT (_CMP_ULT_UQ) - maps to "ult"
    cmp_results[6] = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    masks[6] = _mm_movemask_ps(cmp_results[6]);
    
    // 8. LTGT (_CMP_NEQ_UQ) - maps to "une"
    cmp_results[7] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    masks[7] = _mm_movemask_ps(cmp_results[7]);
    
    // Use results in arithmetic to prevent dead code elimination
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Blend based on comparison results
        __m128 blended = _mm_blendv_ps(vec1, vec3, cmp_results[i]);
        accum = _mm_add_ps(accum, blended);
    }
    
    // Extract and use result
    float result[4];
    _mm_storeu_ps(result, accum);
    printf("SSE accumulated result: %f %f %f %f\n", 
           result[0], result[1], result[2], result[3]);
    
    // Test double precision comparisons as well
    __m128d dvec1 = _mm_setr_pd(1.0, NAN);
    __m128d dvec2 = _mm_setr_pd(2.0, NAN);
    
    // Test a subset with double precision
    __m128d dresult1 = _mm_cmp_pd(dvec1, dvec2, _CMP_UNORD_Q);
    __m128d dresult2 = _mm_cmp_pd(dvec1, dvec2, _CMP_ORD_Q);
    __m128d dresult3 = _mm_cmp_pd(dvec1, dvec2, _CMP_UNEQ_UQ);
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# SSE comparison results\n"
        : 
        : "x" (dresult1), "x" (dresult2), "x" (dresult3)
        : 
    );
#endif
}

// Function to test AVX condition codes (if available)
void test_avx_condition_codes(void) {
#if USE_AVX
    printf("Testing AVX condition codes...\n");
    
    // Create 256-bit vectors
    __m256 avx_vec1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_vec2 = _mm256_setr_ps(1.0f, 3.0f, NAN, 0.0f, 9.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_vec3 = _mm256_setr_ps(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f);
    
    // Test all condition codes with AVX
    __m256 avx_results[8];
    
    avx_results[0] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNORD_Q);
    avx_results[1] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ORD_Q);
    avx_results[2] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNEQ_UQ);
    avx_results[3] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NGE_UQ);
    avx_results[4] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NGT_UQ);
    avx_results[5] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULE_UQ);
    avx_results[6] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULT_UQ);
    avx_results[7] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NEQ_UQ);
    
    // Use results in complex expression
    __m256 avx_accum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Conditional blending based on comparison
        __m256 temp = _mm256_blendv_ps(avx_vec1, avx_vec3, avx_results[i]);
        avx_accum = _mm256_add_ps(avx_accum, temp);
        
        // Also test multiplication
        avx_accum = _mm256_mul_ps(avx_accum, _mm256_set1_ps(1.0001f));
    }
    
    // Test AVX double precision
    __m256d avx_dvec1 = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d avx_dvec2 = _mm256_setr_pd(1.0, 2.0, NAN, 4.0);
    
    __m256d avx_dresult1 = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_UNORD_Q);
    __m256d avx_dresult2 = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_ORD_Q);
    __m256d avx_dresult3 = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_UNEQ_UQ);
    
    // Force assembly generation with inline asm
    __asm__ __volatile__ (
        "# AVX comparison results\n"
        : 
        : "x" (avx_dresult1), "x" (avx_dresult2), "x" (avx_dresult3)
        : 
    );
    
    // Extract and print final result
    float avx_final[8];
    _mm256_storeu_ps(avx_final, avx_accum);
    printf("AVX accumulated result: ");
    for (int i = 0; i < 8; i++) {
        printf("%f ", avx_final[i]);
    }
    printf("\n");
#endif
}

// Test scalar comparisons as well
void test_scalar_condition_codes(void) {
#if USE_SSE
    printf("Testing scalar condition codes...\n");
    
    __m128 svec1 = _mm_set_ss(NAN);
    __m128 svec2 = _mm_set_ss(1.0f);
    __m128 svec3 = _mm_set_ss(2.0f);
    
    // Test scalar comparisons
    __m128 sresult1 = _mm_cmp_ss(svec1, svec2, _CMP_UNORD_Q);
    __m128 sresult2 = _mm_cmp_ss(svec2, svec3, _CMP_ORD_Q);
    __m128 sresult3 = _mm_cmp_ss(svec2, svec3, _CMP_UNEQ_UQ);
    __m128 sresult4 = _mm_cmp_ss(svec2, svec3, _CMP_NGE_UQ);
    
    // Mix with arithmetic
    __m128 sblend1 = _mm_blendv_ps(svec1, svec2, sresult1);
    __m128 sblend2 = _mm_blendv_ps(svec2, svec3, sresult2);
    
    __m128 saccum = _mm_add_ps(sblend1, sblend2);
    
    // Force assembly output
    __asm__ __volatile__ (
        "# Scalar comparison results\n"
        : 
        : "x" (sresult1), "x" (sresult2), "x" (sresult3), "x" (sresult4)
        : 
    );
#endif
}

// Complex function that uses comparisons in control flow
float complex_vector_operations(float* data, int size) {
#if USE_SSE
    if (size < 4) return 0.0f;
    
    __m128 accum = _mm_setzero_ps();
    __m128 nan_vec = _mm_set1_ps(NAN);
    
    for (int i = 0; i < size - 3; i += 4) {
        __m128 vec = _mm_loadu_ps(&data[i]);
        __m128 next_vec = _mm_loadu_ps(&data[(i + 1) % size]);
        
        // Multiple comparisons with different condition codes
        __m128 cmp_unord = _mm_cmp_ps(vec, nan_vec, _CMP_UNORD_Q);
        __m128 cmp_ord = _mm_cmp_ps(vec, next_vec, _CMP_ORD_Q);
        __m128 cmp_uneq = _mm_cmp_ps(vec, next_vec, _CMP_UNEQ_UQ);
        
        // Use comparisons to control computation
        __m128 masked1 = _mm_and_ps(vec, cmp_unord);
        __m128 masked2 = _mm_andnot_ps(cmp_ord, next_vec);
        __m128 blended = _mm_blendv_ps(masked1, masked2, cmp_uneq);
        
        accum = _mm_add_ps(accum, blended);
        
        // Additional comparisons
        __m128 cmp_nlt = _mm_cmp_ps(vec, next_vec, _CMP_NGE_UQ);  // nlt
        __m128 cmp_nle = _mm_cmp_ps(vec, next_vec, _CMP_NGT_UQ);  // nle
        
        // More complex expression
        __m128 temp = _mm_mul_ps(vec, _mm_set1_ps(0.5f));
        temp = _mm_blendv_ps(temp, vec, cmp_nlt);
        temp = _mm_blendv_ps(temp, next_vec, cmp_nle);
        
        accum = _mm_add_ps(accum, temp);
    }
    
    // Horizontal sum
    accum = _mm_hadd_ps(accum, accum);
    accum = _mm_hadd_ps(accum, accum);
    
    float result;
    _mm_store_ss(&result, accum);
    return result;
#else
    return 0.0f;
#endif
}

int main(void) {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Test SSE condition codes
    test_sse_condition_codes();
    
    // Test AVX condition codes if supported
    test_avx_condition_codes();
    
    // Test scalar comparisons
    test_scalar_condition_codes();
    
    // Test complex operations with control flow
    float test_data[16];
    for (int i = 0; i < 16; i++) {
        test_data[i] = (float)i * 0.5f;
        if (i % 5 == 0) test_data[i] = NAN;
    }
    
    float complex_result = complex_vector_operations(test_data, 16);
    printf("Complex operation result: %f\n", complex_result);
    
    // Final inline assembly to ensure all condition codes are emitted
    __asm__ __volatile__ (
        "# Final assembly marker\n"
        :
        :
        : 
    );
    
    return 0;
}
