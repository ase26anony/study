#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

// Test various unordered floating-point comparisons
void test_scalar_comparisons(float f1, float f2, double d1, double d2) {
    // UNORDERED condition
    if (__builtin_isunordered(f1, f2)) {
        sink = 1;
    }
    
    // ORDERED condition
    if (!__builtin_isunordered(f1, f2)) {
        sink = 2;
    }
    
    // UNEQ condition (unordered or equal)
    if (f1 != f1 || f1 == f2) {  // NaN != NaN is true, generates UNEQ
        sink = 3;
    }
    
    // UNLT condition (unordered or less than)
    if (__builtin_isless(f1, f2) || __builtin_isunordered(f1, f2)) {
        sink = 4;
    }
    
    // UNLE condition (unordered or less than or equal)
    if (__builtin_islessequal(f1, f2) || __builtin_isunordered(f1, f2)) {
        sink = 5;
    }
    
    // UNGT condition (unordered or greater than)
    if (__builtin_isgreater(f1, f2) || __builtin_isunordered(f1, f2)) {
        sink = 6;
    }
    
    // UNGE condition (unordered or greater than or equal)
    if (__builtin_isgreaterequal(f1, f2) || __builtin_isunordered(f1, f2)) {
        sink = 7;
    }
    
    // LTGT condition (less than or greater than, but not equal and not unordered)
    if (__builtin_islessgreater(f1, f2)) {
        sink = 8;
    }
    
    // Double precision versions
    if (__builtin_isunordered(d1, d2)) {
        sink = 9;
    }
    
    if (d1 != d1) {  // UNEQ
        sink = 10;
    }
}

// Test SSE vector comparisons
void test_sse_comparisons(__m128 v1, __m128 v2, __m128d d1, __m128d d2) {
    __m128 res;
    __m128d resd;
    
    // UNORDERED
    res = _mm_cmpunord_ps(v1, v2);
    resd = _mm_cmpunord_pd(d1, d2);
    
    // ORDERED
    res = _mm_cmpord_ps(v1, v2);
    resd = _mm_cmpord_pd(d1, d2);
    
    // UNEQ
    res = _mm_cmpneq_ps(v1, v2);
    resd = _mm_cmpneq_pd(d1, d2);
    
    // UNGE (not less than)
    res = _mm_cmpnlt_ps(v1, v2);
    resd = _mm_cmpnlt_pd(d1, d2);
    
    // UNGT (not less than or equal)
    res = _mm_cmpnle_ps(v1, v2);
    resd = _mm_cmpnle_pd(d1, d2);
    
    // UNLE (unordered or less than or equal)
    // Note: SSE doesn't have direct UNLE intrinsic, use combination
    __m128 unord = _mm_cmpunord_ps(v1, v2);
    __m128 le = _mm_cmple_ps(v1, v2);
    res = _mm_or_ps(unord, le);
    
    // UNLT (unordered or less than)
    __m128 lt = _mm_cmplt_ps(v1, v2);
    res = _mm_or_ps(unord, lt);
    
    // Store results to prevent dead code elimination
    float temp[4];
    _mm_store_ps(temp, res);
    sink = (int)temp[0];
}

// Test AVX vector comparisons
#ifdef __AVX__
void test_avx_comparisons(__m256 v1, __m256 v2, __m256d d1, __m256d d2) {
    __m256 res;
    __m256d resd;
    
    // UNORDERED
    res = _mm256_cmp_ps(v1, v2, _CMP_UNORD_Q);
    resd = _mm256_cmp_pd(d1, d2, _CMP_UNORD_Q);
    
    // ORDERED
    res = _mm256_cmp_ps(v1, v2, _CMP_ORD_Q);
    resd = _mm256_cmp_pd(d1, d2, _CMP_ORD_Q);
    
    // UNEQ
    res = _mm256_cmp_ps(v1, v2, _CMP_NEQ_UQ);
    resd = _mm256_cmp_pd(d1, d2, _CMP_NEQ_UQ);
    
    // UNGE
    res = _mm256_cmp_ps(v1, v2, _CMP_NLT_UQ);
    resd = _mm256_cmp_pd(d1, d2, _CMP_NLT_UQ);
    
    // UNGT
    res = _mm256_cmp_ps(v1, v2, _CMP_NLE_UQ);
    resd = _mm256_cmp_pd(d1, d2, _CMP_NLE_UQ);
    
    // UNLE
    res = _mm256_cmp_ps(v1, v2, _CMP_LE_OS);
    resd = _mm256_cmp_pd(d1, d2, _CMP_LE_OS);
    
    // UNLT
    res = _mm256_cmp_ps(v1, v2, _CMP_LT_OS);
    resd = _mm256_cmp_pd(d1, d2, _CMP_LT_OS);
    
    // LTGT
    res = _mm256_cmp_ps(v1, v2, _CMP_NEQ_OQ);
    resd = _mm256_cmp_pd(d1, d2, _CMP_NEQ_OQ);
    
    // Store results
    float temp[8];
    _mm256_store_ps(temp, res);
    sink = (int)temp[0];
}
#endif

// Test mixed comparisons in loops
void test_loop_comparisons(int iterations) {
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    for (int i = 0; i < iterations; i++) {
        float f = (float)i;
        double d = (double)i;
        
        // Test with NaN on left
        if (__builtin_isunordered(nan_f, f)) {
            sink++;
        }
        
        // Test with NaN on right
        if (__builtin_isunordered(f, nan_f)) {
            sink++;
        }
        
        // Test NaN vs NaN
        if (__builtin_isunordered(nan_f, nan_f)) {
            sink++;
        }
        
        // Test infinity comparisons
        if (__builtin_isless(f, inf_f)) {
            sink++;
        }
        
        // Generate UNEQ through NaN propagation
        float nan_prop = nan_f * 2.0f;
        if (nan_prop != nan_prop) {  // Always true for NaN
            sink++;
        }
        
        // Double precision in loop
        if (__builtin_isunordered(nan_d, d)) {
            sink++;
        }
        
        // LTGT with normal numbers
        if (__builtin_islessgreater(f, f + 1.0f)) {
            sink++;
        }
    }
}

int main() {
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float normal_f = 3.14159f;
    double normal_d = 2.71828;
    
    // Test scalar comparisons with various operand combinations
    test_scalar_comparisons(nan_f, normal_f, nan_d, normal_d);
    test_scalar_comparisons(normal_f, nan_f, normal_d, nan_d);
    test_scalar_comparisons(nan_f, nan_f, nan_d, nan_d);
    test_scalar_comparisons(normal_f, normal_f * 2, normal_d, normal_d * 2);
    
    // Initialize SSE vectors
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 v2 = _mm_setr_ps(nan_f, 2.0f, 3.0f, nan_f);
    __m128d d1 = _mm_setr_pd(1.0, nan_d);
    __m128d d2 = _mm_setr_pd(nan_d, 2.0);
    
    test_sse_comparisons(v1, v2, d1, d2);
    
    // Test AVX if available
    #ifdef __AVX__
    __m256 v256_1 = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 v256_2 = _mm256_setr_ps(nan_f, 2.0f, nan_f, 4.0f, 5.0f, nan_f, 7.0f, 8.0f);
    __m256d d256_1 = _mm256_setr_pd(1.0, nan_d, 3.0, nan_d);
    __m256d d256_2 = _mm256_setr_pd(nan_d, 2.0, nan_d, 4.0);
    
    test_avx_comparisons(v256_1, v256_2, d256_1, d256_2);
    #endif
    
    // Test in loops to ensure code generation
    test_loop_comparisons(10);
    
    // Use results to prevent dead code elimination
    printf("Result: %d\n", sink);
    
    return 0;
}
