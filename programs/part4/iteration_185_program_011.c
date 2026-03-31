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

// Function to test SSE condition codes
float test_sse_condition_codes(void) {
    // Initialize vectors with various values including NaN
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(1.0f, 3.0f, 5.0f, NAN);
    __m128 vec3 = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f/0.0f);
    
    // Results accumulator
    __m128 results = _mm_setzero_ps();
    float final_result = 0.0f;
    
    // Test all condition codes from the uncovered block
    // Each comparison will generate assembly with condition code strings
    
    // 1. UNORDERED (_CMP_UNORD_Q)
    __m128 cmp_unord = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    results = _mm_add_ps(results, _mm_and_ps(cmp_unord, _mm_set1_ps(1.0f)));
    
    // 2. ORDERED (_CMP_ORD_Q)
    __m128 cmp_ord = _mm_cmp_ps(vec1, vec3, _CMP_ORD_Q);
    results = _mm_add_ps(results, _mm_and_ps(cmp_ord, _mm_set1_ps(2.0f)));
    
    // 3. UNEQ (_CMP_UNEQ_UQ)
    __m128 cmp_uneq = _mm_cmp_ps(vec2, vec3, _CMP_UNEQ_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_uneq, _mm_set1_ps(3.0f)));
    
    // 4. UNGE (_CMP_NGE_UQ)
    __m128 cmp_unge = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_unge, _mm_set1_ps(4.0f)));
    
    // 5. UNGT (_CMP_NGT_UQ)
    __m128 cmp_ungt = _mm_cmp_ps(vec2, vec3, _CMP_NGT_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_ungt, _mm_set1_ps(5.0f)));
    
    // 6. UNLE (_CMP_ULE_UQ)
    __m128 cmp_unle = _mm_cmp_ps(vec3, vec1, _CMP_ULE_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_unle, _mm_set1_ps(6.0f)));
    
    // 7. UNLT (_CMP_ULT_UQ)
    __m128 cmp_unlt = _mm_cmp_ps(vec1, vec3, _CMP_ULT_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_unlt, _mm_set1_ps(7.0f)));
    
    // 8. LTGT (_CMP_NEQ_UQ)
    __m128 cmp_ltgt = _mm_cmp_ps(vec2, vec1, _CMP_NEQ_UQ);
    results = _mm_add_ps(results, _mm_and_ps(cmp_ltgt, _mm_set1_ps(8.0f)));
    
    // Also test scalar comparisons which use different code paths
    __m128 scalar_cmp = _mm_cmp_ss(vec1, vec2, _CMP_UNORD_Q);
    results = _mm_add_ss(results, scalar_cmp);
    
    // Extract mask and use in control flow to prevent optimization
    int mask = _mm_movemask_ps(results);
    if (mask & 1) {
        final_result += 1.0f;
    }
    if (mask & 2) {
        final_result *= 2.0f;
    }
    if (mask & 4) {
        final_result -= 3.0f;
    }
    if (mask & 8) {
        final_result = fabs(final_result);
    }
    
    // Use inline assembly to force assembly output with vector operands
    float temp;
    _mm_store_ss(&temp, results);
    
    // This asm statement will generate assembly with condition codes
    __asm__ __volatile__ (
        "vmovss %1, %0\n\t"
        : "=x"(final_result)
        : "x"(temp)
        : "memory"
    );
    
    return final_result;
}

// Double precision version
double test_sse_double_condition_codes(void) {
    __m128d vec1 = _mm_setr_pd(1.0, NAN);
    __m128d vec2 = _mm_setr_pd(NAN, 2.0);
    __m128d vec3 = _mm_setr_pd(INFINITY, -INFINITY);
    
    __m128d results = _mm_setzero_pd();
    
    // Test double precision comparisons
    __m128d cmp_unord_d = _mm_cmp_pd(vec1, vec2, _CMP_UNORD_Q);
    results = _mm_add_pd(results, _mm_and_pd(cmp_unord_d, _mm_set1_pd(1.0)));
    
    __m128d cmp_ord_d = _mm_cmp_pd(vec1, vec3, _CMP_ORD_Q);
    results = _mm_add_pd(results, _mm_and_pd(cmp_ord_d, _mm_set1_pd(2.0)));
    
    __m128d cmp_uneq_d = _mm_cmp_pd(vec2, vec3, _CMP_UNEQ_UQ);
    results = _mm_add_pd(results, _mm_and_pd(cmp_uneq_d, _mm_set1_pd(3.0)));
    
    // Use results in control flow
    double final_result = 0.0;
    int mask = _mm_movemask_pd(results);
    
    if (mask & 1) {
        final_result = 1.0;
    }
    if (mask & 2) {
        final_result *= 2.0;
    }
    
    // Blend operation using comparison results
    __m128d blended = _mm_blendv_pd(vec1, vec2, cmp_unord_d);
    final_result += _mm_cvtsd_f64(blended);
    
    return final_result;
}

#ifdef __AVX__
// AVX 256-bit vector version
float test_avx_condition_codes(void) {
    __m256 vec1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec2 = _mm256_setr_ps(1.0f, 3.0f, 5.0f, NAN, 9.0f, 10.0f, 11.0f, 12.0f);
    __m256 vec3 = _mm256_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f/0.0f, 
                                  13.0f, 14.0f, 15.0f, 16.0f);
    
    __m256 results = _mm256_setzero_ps();
    
    // Test AVX comparisons with all condition codes
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_unord_avx, _mm256_set1_ps(1.0f)));
    
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec1, vec3, _CMP_ORD_Q);
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_ord_avx, _mm256_set1_ps(2.0f)));
    
    __m256 cmp_uneq_avx = _mm256_cmp_ps(vec2, vec3, _CMP_UNEQ_UQ);
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_uneq_avx, _mm256_set1_ps(3.0f)));
    
    __m256 cmp_unge_avx = _mm256_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_unge_avx, _mm256_set1_ps(4.0f)));
    
    __m256 cmp_ungt_avx = _mm256_cmp_ps(vec2, vec3, _CMP_NGT_UQ);
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_ungt_avx, _mm256_set1_ps(5.0f)));
    
    __m256 cmp_unle_avx = _mm256_cmp_ps(vec3, vec1, _CMP_ULE_UQ);
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_unle_avx, _mm256_set1_ps(6.0f)));
    
    __m256 cmp_unlt_avx = _mm256_cmp_ps(vec1, vec3, _CMP_ULT_UQ);
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_unlt_avx, _mm256_set1_ps(7.0f)));
    
    __m256 cmp_ltgt_avx = _mm256_cmp_ps(vec2, vec1, _CMP_NEQ_UQ);
    results = _mm256_add_ps(results, _mm256_and_ps(cmp_ltgt_avx, _mm256_set1_ps(8.0f)));
    
    // Extract results and use in computation
    float final_result = 0.0f;
    float temp[8];
    _mm256_storeu_ps(temp, results);
    
    for (int i = 0; i < 8; i++) {
        if (!isnan(temp[i])) {
            final_result += temp[i];
        }
    }
    
    // Complex expression with blending
    __m256 blended = _mm256_blendv_ps(vec1, vec2, cmp_unord_avx);
    blended = _mm256_add_ps(blended, _mm256_mul_ps(cmp_ord_avx, vec3));
    
    // Force assembly generation with inline asm
    float asm_result;
    __asm__ __volatile__ (
        "vmovss %%xmm0, %0\n\t"
        : "=m"(asm_result)
        :
        : "memory"
    );
    
    return final_result + asm_result;
}

// AVX double precision
double test_avx_double_condition_codes(void) {
    __m256d vec1 = _mm256_setr_pd(1.0, NAN, 3.0, 4.0);
    __m256d vec2 = _mm256_setr_pd(NAN, 2.0, 5.0, 6.0);
    
    __m256d results = _mm256_setzero_pd();
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_UNORD_Q);
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_unord_avx_d, _mm256_set1_pd(1.0)));
    
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec1, vec2, _CMP_ORD_Q);
    results = _mm256_add_pd(results, _mm256_and_pd(cmp_ord_avx_d, _mm256_set1_pd(2.0)));
    
    // Use mask for control flow
    int mask = _mm256_movemask_pd(results);
    double final_result = 0.0;
    
    if (mask & 1) final_result += 1.0;
    if (mask & 2) final_result += 2.0;
    if (mask & 4) final_result += 4.0;
    if (mask & 8) final_result += 8.0;
    
    return final_result;
}
#endif

// Mixed operations to create complex expressions
float test_mixed_operations(void) {
    __m128 a = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 b = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 c = _mm_setr_ps(NAN, INFINITY, -INFINITY, 0.0f);
    
    // Chain comparisons with arithmetic
    __m128 cmp1 = _mm_cmp_ps(a, b, _CMP_UNORD_Q);
    __m128 cmp2 = _mm_cmp_ps(b, c, _CMP_ORD_Q);
    __m128 cmp3 = _mm_cmp_ps(a, c, _CMP_UNEQ_UQ);
    
    // Complex expression that compiler needs to decompose
    __m128 result = _mm_add_ps(
        _mm_mul_ps(cmp1, a),
        _mm_sub_ps(
            _mm_div_ps(cmp2, b),
            _mm_and_ps(cmp3, c)
        )
    );
    
    // Extract and return
    float temp[4];
    _mm_storeu_ps(temp, result);
    return temp[0] + temp[1] + temp[2] + temp[3];
}

int main(void) {
    printf("Testing SSE condition codes...\n");
    float sse_result = test_sse_condition_codes();
    printf("SSE result: %f\n", sse_result);
    
    printf("Testing SSE double precision condition codes...\n");
    double sse_double_result = test_sse_double_condition_codes();
    printf("SSE double result: %f\n", sse_double_result);
    
    printf("Testing mixed operations...\n");
    float mixed_result = test_mixed_operations();
    printf("Mixed result: %f\n", mixed_result);
    
#ifdef __AVX__
    if (__builtin_cpu_supports("avx")) {
        printf("Testing AVX condition codes...\n");
        float avx_result = test_avx_condition_codes();
        printf("AVX result: %f\n", avx_result);
        
        printf("Testing AVX double precision condition codes...\n");
        double avx_double_result = test_avx_double_condition_codes();
        printf("AVX double result: %f\n", avx_double_result);
    } else {
        printf("AVX not supported on this CPU\n");
    }
#else
    printf("AVX not enabled in compilation\n");
#endif
    
    return 0;
}
