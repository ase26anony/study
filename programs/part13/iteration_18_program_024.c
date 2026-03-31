#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

// Test function that uses various unordered comparisons
void test_unordered_comparisons(float f1, float f2, double d1, double d2) {
    int result = 0;
    
    // UNORDERED: __builtin_isunordered
    if (__builtin_isunordered(f1, f2)) {
        result |= 1;
    }
    
    // ORDERED: !__builtin_isunordered
    if (!__builtin_isunordered(d1, d2)) {
        result |= 2;
    }
    
    // UNEQ: x != x (is NaN) or comparison with NaN
    if (f1 != f1) {  // f1 is NaN
        result |= 4;
    }
    
    // UNLT: x < y where x or y is NaN
    if (f1 < f2) {  // Will be false if either is NaN, generating UNLT
        result |= 8;
    }
    
    // UNLE: x <= y where x or y is NaN
    if (d1 <= d2) {  // Will be false if either is NaN, generating UNLE
        result |= 16;
    }
    
    // UNGT: x > y where x or y is NaN
    if (f1 > f2) {  // Will be false if either is NaN, generating UNGT
        result |= 32;
    }
    
    // UNGE: x >= y where x or y is NaN
    if (d1 >= d2) {  // Will be false if either is NaN, generating UNGE
        result |= 64;
    }
    
    // LTGT: __builtin_islessgreater
    if (__builtin_islessgreater(f1, f2)) {
        result |= 128;
    }
    
    sink = result;
}

// Test with SSE intrinsics
void test_sse_unordered_comparisons(__m128 a, __m128 b, __m128d c, __m128d d) {
    __m128 res;
    __m128d resd;
    
    // UNORDERED
    res = _mm_cmpunord_ps(a, b);
    
    // ORDERED
    res = _mm_cmpord_ps(a, b);
    
    // UNEQ (unordered or equal)
    res = _mm_cmpneq_ps(a, b);
    
    // UNGE (not less than)
    res = _mm_cmpnlt_ps(a, b);
    
    // UNGT (not less than or equal)
    res = _mm_cmpnle_ps(a, b);
    
    // UNLE (unordered or less than or equal)
    res = _mm_cmpule_ps(a, b);
    
    // UNLT (unordered or less than)
    res = _mm_cmpult_ps(a, b);
    
    // Double precision versions
    // UNORDERED
    resd = _mm_cmpunord_pd(c, d);
    
    // ORDERED
    resd = _mm_cmpord_pd(c, d);
    
    // UNEQ
    resd = _mm_cmpneq_pd(c, d);
    
    // UNGE
    resd = _mm_cmpnlt_pd(c, d);
    
    // UNGT
    resd = _mm_cmpnle_pd(c, d);
    
    // UNLE
    resd = _mm_cmpule_pd(c, d);
    
    // UNLT
    resd = _mm_cmpult_pd(c, d);
    
    // Store to prevent optimization
    _mm_store_ps((float*)&sink, res);
}

// Test with AVX intrinsics (256-bit vectors)
#ifdef __AVX__
void test_avx_unordered_comparisons(__m256 a, __m256 b, __m256d c, __m256d d) {
    __m256 res;
    __m256d resd;
    
    // UNORDERED
    res = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    
    // ORDERED
    res = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
    
    // UNEQ (unordered or equal)
    res = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);
    
    // UNGE (not less than)
    res = _mm256_cmp_ps(a, b, _CMP_NLT_UQ);
    
    // UNGT (not less than or equal)
    res = _mm256_cmp_ps(a, b, _CMP_NLE_UQ);
    
    // UNLE (unordered or less than or equal)
    res = _mm256_cmp_ps(a, b, _CMP_LE_OS);
    
    // UNLT (unordered or less than)
    res = _mm256_cmp_ps(a, b, _CMP_LT_OS);
    
    // LTGT (less than or greater than, ordered)
    res = _mm256_cmp_ps(a, b, _CMP_NEQ_OQ);
    
    // Double precision versions
    // UNORDERED
    resd = _mm256_cmp_pd(c, d, _CMP_UNORD_Q);
    
    // ORDERED
    resd = _mm256_cmp_pd(c, d, _CMP_ORD_Q);
    
    // UNEQ
    resd = _mm256_cmp_pd(c, d, _CMP_NEQ_UQ);
    
    // UNGE
    resd = _mm256_cmp_pd(c, d, _CMP_NLT_UQ);
    
    // UNGT
    resd = _mm256_cmp_pd(c, d, _CMP_NLE_UQ);
    
    // UNLE
    resd = _mm256_cmp_pd(c, d, _CMP_LE_OS);
    
    // UNLT
    resd = _mm256_cmp_pd(c, d, _CMP_LT_OS);
    
    // LTGT
    resd = _mm256_cmp_pd(c, d, _CMP_NEQ_OQ);
    
    // Store to prevent optimization
    _mm256_store_ps((float*)&sink, res);
}
#endif

// Test mixed comparisons in loops
void test_mixed_comparisons_loop() {
    float f_values[] = {1.0f, 2.0f, NAN, 4.0f, 5.0f};
    double d_values[] = {1.0, NAN, 3.0, 4.0, 5.0};
    
    int sum = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            // Various unordered comparisons
            sum += __builtin_isunordered(f_values[i], f_values[j]) ? 1 : 0;
            sum += __builtin_islessgreater(f_values[i], f_values[j]) ? 1 : 0;
            
            // Direct comparisons that can generate unordered conditions
            if (f_values[i] < f_values[j]) sum++;
            if (f_values[i] <= f_values[j]) sum++;
            if (f_values[i] > f_values[j]) sum++;
            if (f_values[i] >= f_values[j]) sum++;
            
            // Double precision
            sum += __builtin_isunordered(d_values[i], d_values[j]) ? 1 : 0;
            sum += __builtin_islessgreater(d_values[i], d_values[j]) ? 1 : 0;
            
            if (d_values[i] < d_values[j]) sum++;
            if (d_values[i] <= d_values[j]) sum++;
            if (d_values[i] > d_values[j]) sum++;
            if (d_values[i] >= d_values[j]) sum++;
        }
    }
    
    sink = sum;
}

int main() {
    // Create NaN values
    float nan_f = __builtin_nanf("");
    float inf_f = __builtin_inff();
    double nan_d = __builtin_nan("");
    double inf_d = __builtin_inf();
    
    // Test scalar unordered comparisons
    test_unordered_comparisons(nan_f, 1.0f, nan_d, 2.0);
    test_unordered_comparisons(1.0f, nan_f, 2.0, nan_d);
    test_unordered_comparisons(nan_f, nan_f, nan_d, nan_d);
    test_unordered_comparisons(1.0f, 2.0f, 3.0, 4.0);
    
    // Initialize SSE vectors
    __m128 sse_f1 = _mm_set_ps(1.0f, nan_f, 3.0f, 4.0f);
    __m128 sse_f2 = _mm_set_ps(nan_f, 2.0f, 3.0f, nan_f);
    __m128d sse_d1 = _mm_set_pd(1.0, nan_d);
    __m128d sse_d2 = _mm_set_pd(nan_d, 2.0);
    
    test_sse_unordered_comparisons(sse_f1, sse_f2, sse_d1, sse_d2);
    
#ifdef __AVX__
    // Initialize AVX vectors
    __m256 avx_f1 = _mm256_set_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, nan_f, 7.0f, 8.0f);
    __m256 avx_f2 = _mm256_set_ps(nan_f, 2.0f, nan_f, nan_f, 5.0f, 6.0f, 7.0f, nan_f);
    __m256d avx_d1 = _mm256_set_pd(1.0, nan_d, 3.0, nan_d);
    __m256d avx_d2 = _mm256_set_pd(nan_d, 2.0, nan_d, 4.0);
    
    test_avx_unordered_comparisons(avx_f1, avx_f2, avx_d1, avx_d2);
#endif
    
    // Test mixed comparisons in loops
    test_mixed_comparisons_loop();
    
    // Additional explicit tests for each condition
    
    // UNORDERED explicit
    if (__builtin_isunordered(nan_f, 1.0f)) {
        sink = 1;
    }
    
    // ORDERED explicit
    if (!__builtin_isunordered(1.0f, 2.0f)) {
        sink = 2;
    }
    
    // UNEQ: x == NaN (always false, generates UNEQ)
    float x = nan_f;
    if (x == x) {  // False when x is NaN
        sink = 3;
    }
    
    // UNLT: x < y with NaN
    if (nan_f < 1.0f) {  // False, generates UNLT
        sink = 4;
    }
    
    // UNLE: x <= y with NaN
    if (1.0f <= nan_f) {  // False, generates UNLE
        sink = 5;
    }
    
    // UNGT: x > y with NaN
    if (nan_f > 1.0f) {  // False, generates UNGT
        sink = 6;
    }
    
    // UNGE: x >= y with NaN
    if (1.0f >= nan_f) {  // False, generates UNGE
        sink = 7;
    }
    
    // LTGT explicit
    if (__builtin_islessgreater(1.0f, 2.0f)) {
        sink = 8;
    }
    
    printf("Test completed successfully\n");
    return 0;
}
