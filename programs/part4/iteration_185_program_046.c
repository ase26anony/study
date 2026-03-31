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
float test_sse_condition_codes() {
    float result = 0.0f;
    
    // Initialize vectors with various values including NaN
    __m128 vec1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2 = _mm_setr_ps(1.0f, 3.0f, 5.0f, NAN);
    __m128 vec3 = _mm_setr_ps(0.0f, INFINITY, -INFINITY, 0.0f);
    __m128 vec4 = _mm_setr_ps(0.0f, -INFINITY, INFINITY, 0.0f);
    
    // Test all condition codes from uncovered block
    __m128 cmp_results[8];
    
    // UNORDERED - _CMP_UNORD_Q
    cmp_results[0] = _mm_cmp_ps(vec1, vec2, _CMP_UNORD_Q);
    
    // ORDERED - _CMP_ORD_Q
    cmp_results[1] = _mm_cmp_ps(vec1, vec2, _CMP_ORD_Q);
    
    // UNEQ - _CMP_UNEQ_UQ
    cmp_results[2] = _mm_cmp_ps(vec1, vec2, _CMP_UNEQ_UQ);
    
    // UNGE - _CMP_NGE_UQ
    cmp_results[3] = _mm_cmp_ps(vec1, vec2, _CMP_NGE_UQ);
    
    // UNGT - _CMP_NGT_UQ
    cmp_results[4] = _mm_cmp_ps(vec1, vec2, _CMP_NGT_UQ);
    
    // UNLE - _CMP_ULE_UQ
    cmp_results[5] = _mm_cmp_ps(vec1, vec2, _CMP_ULE_UQ);
    
    // UNLT - _CMP_ULT_UQ
    cmp_results[6] = _mm_cmp_ps(vec1, vec2, _CMP_ULT_UQ);
    
    // LTGT - _CMP_NEQ_UQ
    cmp_results[7] = _mm_cmp_ps(vec1, vec2, _CMP_NEQ_UQ);
    
    // Use results in arithmetic operations to prevent dead code elimination
    __m128 accum = _mm_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Convert comparison mask to float (0.0f or 1.0f)
        __m128 mask_as_float = _mm_and_ps(cmp_results[i], _mm_set1_ps(1.0f));
        accum = _mm_add_ps(accum, mask_as_float);
    }
    
    // Extract result
    float temp[4];
    _mm_storeu_ps(temp, accum);
    result = temp[0] + temp[1] + temp[2] + temp[3];
    
    // Force assembly output with inline assembly
    __asm__ __volatile__ (
        "# SSE comparison results: %0"
        : 
        : "x" (accum)
        : "memory"
    );
    
    return result;
}

// Function to test SSE double precision condition codes
double test_sse_double_condition_codes() {
    double result = 0.0;
    
    // Initialize double precision vectors
    __m128d dvec1 = _mm_setr_pd(1.0, NAN);
    __m128d dvec2 = _mm_setr_pd(1.0, 2.0);
    __m128d dvec3 = _mm_setr_pd(INFINITY, -INFINITY);
    __m128d dvec4 = _mm_setr_pd(-INFINITY, INFINITY);
    
    // Test condition codes with double precision
    __m128d d_cmp_results[8];
    
    d_cmp_results[0] = _mm_cmp_pd(dvec1, dvec2, _CMP_UNORD_Q);    // UNORDERED
    d_cmp_results[1] = _mm_cmp_pd(dvec1, dvec2, _CMP_ORD_Q);      // ORDERED
    d_cmp_results[2] = _mm_cmp_pd(dvec1, dvec2, _CMP_UNEQ_UQ);    // UNEQ
    d_cmp_results[3] = _mm_cmp_pd(dvec3, dvec4, _CMP_NGE_UQ);     // UNGE
    d_cmp_results[4] = _mm_cmp_pd(dvec3, dvec4, _CMP_NGT_UQ);     // UNGT
    d_cmp_results[5] = _mm_cmp_pd(dvec1, dvec2, _CMP_ULE_UQ);     // UNLE
    d_cmp_results[6] = _mm_cmp_pd(dvec1, dvec2, _CMP_ULT_UQ);     // UNLT
    d_cmp_results[7] = _mm_cmp_pd(dvec3, dvec4, _CMP_NEQ_UQ);     // LTGT
    
    // Use results in control flow
    __m128d d_accum = _mm_setzero_pd();
    for (int i = 0; i < 8; i++) {
        // Blend based on comparison results
        __m128d ones = _mm_set1_pd(1.0);
        d_accum = _mm_add_pd(d_accum, 
                           _mm_and_pd(d_cmp_results[i], ones));
    }
    
    // Extract and return result
    double temp[2];
    _mm_storeu_pd(temp, d_accum);
    result = temp[0] + temp[1];
    
    return result;
}

#ifdef __AVX__
// Function to test AVX condition codes
float test_avx_condition_codes() {
    float result = 0.0f;
    
    // Initialize AVX vectors
    __m256 avx_vec1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, NAN, 8.0f);
    __m256 avx_vec2 = _mm256_setr_ps(1.0f, 3.0f, 5.0f, NAN, 1.0f, 7.0f, 8.0f, NAN);
    
    // Test AVX comparisons with all condition codes
    __m256 avx_cmp_results[8];
    
    avx_cmp_results[0] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNORD_Q);   // UNORDERED
    avx_cmp_results[1] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ORD_Q);     // ORDERED
    avx_cmp_results[2] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_UNEQ_UQ);   // UNEQ
    avx_cmp_results[3] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NGE_UQ);    // UNGE
    avx_cmp_results[4] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NGT_UQ);    // UNGT
    avx_cmp_results[5] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULE_UQ);    // UNLE
    avx_cmp_results[6] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_ULT_UQ);    // UNLT
    avx_cmp_results[7] = _mm256_cmp_ps(avx_vec1, avx_vec2, _CMP_NEQ_UQ);    // LTGT
    
    // Complex expression combining comparisons and arithmetic
    __m256 avx_accum = _mm256_setzero_ps();
    for (int i = 0; i < 8; i++) {
        // Create dependent operations
        __m256 mask = avx_cmp_results[i];
        __m256 masked_val = _mm256_and_ps(mask, _mm256_set1_ps(1.0f));
        
        // Blend with different vectors based on iteration
        __m256 blend_vec = _mm256_set1_ps((float)i);
        avx_accum = _mm256_add_ps(avx_accum, 
                                _mm256_blendv_ps(masked_val, blend_vec, mask));
    }
    
    // Extract mask and use in conditional
    int mask = _mm256_movemask_ps(avx_accum);
    if (mask != 0) {
        float temp[8];
        _mm256_storeu_ps(temp, avx_accum);
        for (int i = 0; i < 8; i++) {
            result += temp[i];
        }
    }
    
    // Force assembly output
    __asm__ __volatile__ (
        "# AVX comparison results processed"
        : 
        : "x" (avx_accum)
        : "memory"
    );
    
    return result;
}

// Test AVX double precision
double test_avx_double_condition_codes() {
    double result = 0.0;
    
    __m256d avx_dvec1 = _mm256_setr_pd(1.0, NAN, INFINITY, -INFINITY);
    __m256d avx_dvec2 = _mm256_setr_pd(1.0, 2.0, -INFINITY, INFINITY);
    
    __m256d avx_dcmp[8];
    
    avx_dcmp[0] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_UNORD_Q);
    avx_dcmp[1] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_ORD_Q);
    avx_dcmp[2] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_UNEQ_UQ);
    avx_dcmp[3] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_NGE_UQ);
    avx_dcmp[4] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_NGT_UQ);
    avx_dcmp[5] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_ULE_UQ);
    avx_dcmp[6] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_ULT_UQ);
    avx_dcmp[7] = _mm256_cmp_pd(avx_dvec1, avx_dvec2, _CMP_NEQ_UQ);
    
    __m256d d_accum = _mm256_setzero_pd();
    for (int i = 0; i < 8; i++) {
        d_accum = _mm256_add_pd(d_accum, 
                              _mm256_and_pd(avx_dcmp[i], _mm256_set1_pd(1.0)));
    }
    
    double temp[4];
    _mm256_storeu_pd(temp, d_accum);
    result = temp[0] + temp[1] + temp[2] + temp[3];
    
    return result;
}
#endif

// Test scalar comparisons (SSE scalar)
float test_scalar_condition_codes() {
    float result = 0.0f;
    
    __m128 svec1 = _mm_set_ss(NAN);
    __m128 svec2 = _mm_set_ss(1.0f);
    __m128 svec3 = _mm_set_ss(INFINITY);
    __m128 svec4 = _mm_set_ss(-INFINITY);
    
    // Test scalar comparisons
    __m128 s_cmp_results[4];
    
    s_cmp_results[0] = _mm_cmp_ss(svec1, svec2, _CMP_UNORD_Q);   // UNORDERED
    s_cmp_results[1] = _mm_cmp_ss(svec1, svec2, _CMP_ORD_Q);     // ORDERED
    s_cmp_results[2] = _mm_cmp_ss(svec3, svec4, _CMP_UNEQ_UQ);   // UNEQ
    s_cmp_results[3] = _mm_cmp_ss(svec3, svec4, _CMP_NEQ_UQ);    // LTGT
    
    // Use in conditional computation
    for (int i = 0; i < 4; i++) {
        float cmp_val;
        _mm_store_ss(&cmp_val, s_cmp_results[i]);
        if (cmp_val != 0) {
            result += 1.0f;
        }
    }
    
    return result;
}

int main() {
    printf("Testing x86 vector comparison condition codes...\n");
    
    // Test SSE single precision
    float sse_result = test_sse_condition_codes();
    printf("SSE float comparisons result: %f\n", sse_result);
    
    // Test SSE double precision
    double sse_double_result = test_sse_double_condition_codes();
    printf("SSE double comparisons result: %f\n", sse_double_result);
    
    // Test scalar comparisons
    float scalar_result = test_scalar_condition_codes();
    printf("Scalar comparisons result: %f\n", scalar_result);
    
#ifdef __AVX__
    printf("AVX support detected, testing AVX comparisons...\n");
    
    // Test AVX single precision
    float avx_result = test_avx_condition_codes();
    printf("AVX float comparisons result: %f\n", avx_result);
    
    // Test AVX double precision
    double avx_double_result = test_avx_double_condition_codes();
    printf("AVX double comparisons result: %f\n", avx_double_result);
#else
    printf("AVX not supported, skipping AVX tests\n");
#endif
    
    // Combine all results to prevent optimization
    float final_result = sse_result + scalar_result;
#ifdef __AVX__
    final_result += avx_result;
#endif
    
    printf("Final combined result: %f\n", final_result);
    
    return (final_result > 0.0f) ? 0 : 1;
}
