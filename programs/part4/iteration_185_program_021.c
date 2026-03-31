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
    printf("Testing SSE condition codes...\n");
    
    // Initialize vectors with various values including NaN
    __m128 a = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 b = _mm_setr_ps(2.0f, 2.0f, 3.0f, NAN);
    __m128 c = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    __m128 d = _mm_setr_ps(0.0f, 0.0f, INFINITY, INFINITY);
    
    // Results array to prevent optimization
    volatile int results[8] = {0};
    int result_idx = 0;
    
    // Test each condition code from the uncovered block
    // Using volatile to force actual computation
    volatile __m128 cmp_result;
    
    // 1. UNORDERED (_CMP_UNORD_Q)
    cmp_result = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    results[result_idx++] = _mm_movemask_ps(cmp_result);
    
    // 2. ORDERED (_CMP_ORD_Q)
    cmp_result = _mm_cmp_ps(a, b, _CMP_ORD_Q);
    results[result_idx++] = _mm_movemask_ps(cmp_result);
    
    // 3. UNEQ (_CMP_UNEQ_UQ)
    cmp_result = _mm_cmp_ps(a, b, _CMP_UNEQ_UQ);
    results[result_idx++] = _mm_movemask_ps(cmp_result);
    
    // 4. UNGE (_CMP_NGE_UQ) - maps to "nlt"
    cmp_result = _mm_cmp_ps(a, b, _CMP_NGE_UQ);
    results[result_idx++] = _mm_movemask_ps(cmp_result);
    
    // 5. UNGT (_CMP_NGT_UQ) - maps to "nle"
    cmp_result = _mm_cmp_ps(c, d, _CMP_NGT_UQ);
    results[result_idx++] = _mm_movemask_ps(cmp_result);
    
    // 6. UNLE (_CMP_ULE_UQ) - maps to "ule"
    cmp_result = _mm_cmp_ps(c, d, _CMP_ULE_UQ);
    results[result_idx++] = _mm_movemask_ps(cmp_result);
    
    // 7. UNLT (_CMP_ULT_UQ) - maps to "ult"
    cmp_result = _mm_cmp_ps(c, d, _CMP_ULT_UQ);
    results[result_idx++] = _mm_movemask_ps(cmp_result);
    
    // 8. LTGT (_CMP_NEQ_UQ) - maps to "une"
    cmp_result = _mm_cmp_ps(a, b, _CMP_NEQ_UQ);
    results[result_idx++] = _mm_movemask_ps(cmp_result);
    
    // Use results in conditional to prevent dead code elimination
    int final_result = 0;
    for (int i = 0; i < 8; i++) {
        if (results[i] != 0) {
            final_result |= (1 << i);
        }
    }
    printf("SSE final mask result: %d\n", final_result);
}

// Function to test double precision SSE condition codes
void test_sse_double_condition_codes(void) {
    printf("Testing SSE double precision condition codes...\n");
    
    // Initialize double precision vectors
    __m128d a = _mm_setr_pd(1.0, NAN);
    __m128d b = _mm_setr_pd(2.0, 2.0);
    __m128d c = _mm_setr_pd(0.0, INFINITY);
    __m128d d = _mm_setr_pd(0.0, INFINITY);
    
    volatile int results[8] = {0};
    int result_idx = 0;
    volatile __m128d cmp_result;
    
    // Test all condition codes with double precision
    cmp_result = _mm_cmp_pd(a, b, _CMP_UNORD_Q);
    results[result_idx++] = _mm_movemask_pd(cmp_result);
    
    cmp_result = _mm_cmp_pd(a, b, _CMP_ORD_Q);
    results[result_idx++] = _mm_movemask_pd(cmp_result);
    
    cmp_result = _mm_cmp_pd(a, b, _CMP_UNEQ_UQ);
    results[result_idx++] = _mm_movemask_pd(cmp_result);
    
    cmp_result = _mm_cmp_pd(a, b, _CMP_NGE_UQ);
    results[result_idx++] = _mm_movemask_pd(cmp_result);
    
    cmp_result = _mm_cmp_pd(c, d, _CMP_NGT_UQ);
    results[result_idx++] = _mm_movemask_pd(cmp_result);
    
    cmp_result = _mm_cmp_pd(c, d, _CMP_ULE_UQ);
    results[result_idx++] = _mm_movemask_pd(cmp_result);
    
    cmp_result = _mm_cmp_pd(c, d, _CMP_ULT_UQ);
    results[result_idx++] = _mm_movemask_pd(cmp_result);
    
    cmp_result = _mm_cmp_pd(a, b, _CMP_NEQ_UQ);
    results[result_idx++] = _mm_movemask_pd(cmp_result);
    
    // Complex expression mixing comparison with arithmetic
    __m128d e = _mm_set1_pd(3.0);
    __m128d f = _mm_set1_pd(1.5);
    __m128d cmp = _mm_cmp_pd(e, f, _CMP_UNORD_Q);
    __m128d blended = _mm_blendv_pd(e, f, cmp);
    results[0] = _mm_movemask_pd(blended);
    
    printf("SSE double test completed\n");
}

#ifdef __AVX__
// Function to test AVX condition codes
void test_avx_condition_codes(void) {
    printf("Testing AVX condition codes...\n");
    
    // Initialize AVX vectors
    __m256 a = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 b = _mm256_setr_ps(2.0f, 2.0f, 3.0f, NAN, 4.0f, 7.0f, 7.0f, 9.0f);
    
    volatile int results[8] = {0};
    int result_idx = 0;
    volatile __m256 cmp_result;
    
    // Test all condition codes with AVX
    cmp_result = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    results[result_idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
    results[result_idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_UNEQ_UQ);
    results[result_idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_NGE_UQ);
    results[result_idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_NGT_UQ);
    results[result_idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_ULE_UQ);
    results[result_idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_ULT_UQ);
    results[result_idx++] = _mm256_movemask_ps(cmp_result);
    
    cmp_result = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);
    results[result_idx++] = _mm256_movemask_ps(cmp_result);
    
    // Complex AVX expression with blending
    __m256 ones = _mm256_set1_ps(1.0f);
    __m256 zeros = _mm256_set1_ps(0.0f);
    __m256 cmp_mask = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    __m256 blended = _mm256_blendv_ps(ones, zeros, cmp_mask);
    
    // Extract and use result
    float blended_array[8];
    _mm256_storeu_ps(blended_array, blended);
    float sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += blended_array[i];
    }
    printf("AVX blended sum: %f\n", sum);
}

// Test AVX double precision
void test_avx_double_condition_codes(void) {
    printf("Testing AVX double precision condition codes...\n");
    
    __m256d a = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d b = _mm256_setr_pd(2.0, 2.0, 3.0, NAN);
    
    volatile int results[4] = {0};
    volatile __m256d cmp_result;
    
    // Mix different condition codes in a complex expression
    __m256d cmp1 = _mm256_cmp_pd(a, b, _CMP_UNORD_Q);
    __m256d cmp2 = _mm256_cmp_pd(a, b, _CMP_ORD_Q);
    __m256d cmp3 = _mm256_cmp_pd(a, b, _CMP_UNEQ_UQ);
    
    // Combine comparisons with arithmetic
    __m256d add_result = _mm256_add_pd(a, b);
    __m256d masked1 = _mm256_and_pd(add_result, cmp1);
    __m256d masked2 = _mm256_andnot_pd(cmp2, add_result);
    __m256d final = _mm256_or_pd(masked1, masked2);
    
    // Force the value to be used
    double final_array[4];
    _mm256_storeu_pd(final_array, final);
    printf("AVX double result: %f, %f, %f, %f\n", 
           final_array[0], final_array[1], final_array[2], final_array[3]);
}
#endif

// Test scalar comparisons (SSE scalar)
void test_scalar_condition_codes(void) {
    printf("Testing scalar condition codes...\n");
    
    volatile __m128 a = _mm_set_ss(1.0f);
    volatile __m128 b = _mm_set_ss(NAN);
    volatile __m128 c = _mm_set_ss(2.0f);
    
    volatile int results[4] = {0};
    
    // Scalar comparisons also use the same condition codes
    __m128 cmp1 = _mm_cmp_ss(a, b, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ss(a, c, _CMP_ORD_Q);
    __m128 cmp3 = _mm_cmp_ss(b, c, _CMP_UNEQ_UQ);
    
    results[0] = _mm_extract_ps(cmp1, 0);
    results[1] = _mm_extract_ps(cmp2, 0);
    results[2] = _mm_extract_ps(cmp3, 0);
    
    printf("Scalar comparison results: %d, %d, %d\n", 
           results[0], results[1], results[2]);
}

// Main function with runtime feature detection
int main(void) {
    printf("Starting condition code coverage test...\n");
    
    // Always test SSE (baseline)
    test_sse_condition_codes();
    test_sse_double_condition_codes();
    test_scalar_condition_codes();
    
    // Test AVX if supported at compile time
#ifdef __AVX__
    printf("AVX support detected at compile time\n");
    test_avx_condition_codes();
    test_avx_double_condition_codes();
#else
    printf("AVX not supported at compile time\n");
#endif
    
    // Runtime detection (optional)
    if (__builtin_cpu_supports("avx")) {
        printf("AVX supported at runtime\n");
        // Could call AVX functions here with proper runtime dispatch
    }
    
    printf("Condition code test completed\n");
    return 0;
}
