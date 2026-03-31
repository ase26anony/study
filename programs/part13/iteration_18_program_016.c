#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

// Test all unordered comparison conditions
void test_scalar_comparisons(float f1, float f2, double d1, double d2) {
    // UNORDERED: __builtin_isunordered
    if (__builtin_isunordered(f1, f2)) sink = 1;
    if (__builtin_isunordered(d1, d2)) sink = 2;
    
    // ORDERED: __builtin_isordered
    if (__builtin_isordered(f1, f2)) sink = 3;
    if (__builtin_isordered(d1, d2)) sink = 4;
    
    // UNEQ: x != x or x == NaN
    if (f1 != f1) sink = 5;           // UNEQ when f1 is NaN
    if (d1 != d1) sink = 6;           // UNEQ when d1 is NaN
    if (!(f1 == f1)) sink = 7;        // Another UNEQ pattern
    
    // UNLT: x < y with NaN operand
    if (f1 < f2) sink = 8;            // UNLT when either is NaN
    if (d1 < d2) sink = 9;
    
    // UNLE: x <= y with NaN operand  
    if (f1 <= f2) sink = 10;          // UNLE when either is NaN
    if (d1 <= d2) sink = 11;
    
    // UNGT: x > y with NaN operand
    if (f1 > f2) sink = 12;           // UNGT when either is NaN
    if (d1 > d2) sink = 13;
    
    // UNGE: x >= y with NaN operand
    if (f1 >= f2) sink = 14;          // UNGE when either is NaN
    if (d1 >= d2) sink = 15;
    
    // LTGT: __builtin_islessgreater
    if (__builtin_islessgreater(f1, f2)) sink = 16;
    if (__builtin_islessgreater(d1, d2)) sink = 17;
}

// Test SSE intrinsics for explicit condition codes
void test_sse_intrinsics(__m128 a, __m128 b, __m128d c, __m128d d) {
    __m128 res;
    __m128d resd;
    
    // UNORDERED
    res = _mm_cmpunord_ps(a, b);
    _mm_store_ps((float*)&sink, res);
    
    // ORDERED
    res = _mm_cmpord_ps(a, b);
    _mm_store_ps((float*)&sink, res);
    
    // UNEQ (CMPNEQ)
    res = _mm_cmpneq_ps(a, b);
    _mm_store_ps((float*)&sink, res);
    
    // UNGE (CMPNLT)
    res = _mm_cmpnlt_ps(a, b);
    _mm_store_ps((float*)&sink, res);
    
    // UNGT (CMPNLE)
    res = _mm_cmpnle_ps(a, b);
    _mm_store_ps((float*)&sink, res);
    
    // UNLE (CMPULE) - Note: This is for UNLE specifically
    // _mm_cmpule_ps doesn't exist, so we use combination
    res = _mm_cmple_ps(a, b);  // This generates UNLE with NaN
    _mm_store_ps((float*)&sink, res);
    
    // UNLT (CMPULT) - Note: This is for UNLT specifically
    res = _mm_cmplt_ps(a, b);  // This generates UNLT with NaN
    _mm_store_ps((float*)&sink, res);
    
    // Double precision versions
    resd = _mm_cmpunord_pd(c, d);
    _mm_store_pd((double*)&sink, resd);
    
    resd = _mm_cmpord_pd(c, d);
    _mm_store_pd((double*)&sink, resd);
    
    resd = _mm_cmpneq_pd(c, d);
    _mm_store_pd((double*)&sink, resd);
    
    resd = _mm_cmpnlt_pd(c, d);
    _mm_store_pd((double*)&sink, resd);
    
    resd = _mm_cmpnle_pd(c, d);
    _mm_store_pd((double*)&sink, resd);
    
    resd = _mm_cmple_pd(c, d);  // UNLE
    _mm_store_pd((double*)&sink, resd);
    
    resd = _mm_cmplt_pd(c, d);  // UNLT
    _mm_store_pd((double*)&sink, resd);
}

// Test AVX intrinsics for 256-bit vectors
#ifdef __AVX__
void test_avx_intrinsics(__m256 a, __m256 b, __m256d c, __m256d d) {
    __m256 res;
    __m256d resd;
    
    // UNORDERED
    res = _mm256_cmp_ps(a, b, _CMP_UNORD_Q);
    _mm256_store_ps((float*)&sink, res);
    
    // ORDERED
    res = _mm256_cmp_ps(a, b, _CMP_ORD_Q);
    _mm256_store_ps((float*)&sink, res);
    
    // UNEQ
    res = _mm256_cmp_ps(a, b, _CMP_NEQ_UQ);
    _mm256_store_ps((float*)&sink, res);
    
    // UNGE
    res = _mm256_cmp_ps(a, b, _CMP_NLT_UQ);
    _mm256_store_ps((float*)&sink, res);
    
    // UNGT
    res = _mm256_cmp_ps(a, b, _CMP_NLE_UQ);
    _mm256_store_ps((float*)&sink, res);
    
    // UNLE
    res = _mm256_cmp_ps(a, b, _CMP_LE_OS);
    _mm256_store_ps((float*)&sink, res);
    
    // UNLT
    res = _mm256_cmp_ps(a, b, _CMP_LT_OS);
    _mm256_store_ps((float*)&sink, res);
    
    // LTGT (UNE)
    res = _mm256_cmp_ps(a, b, _CMP_NEQ_OQ);
    _mm256_store_ps((float*)&sink, res);
    
    // Double precision versions
    resd = _mm256_cmp_pd(c, d, _CMP_UNORD_Q);
    _mm256_store_pd((double*)&sink, resd);
    
    resd = _mm256_cmp_pd(c, d, _CMP_ORD_Q);
    _mm256_store_pd((double*)&sink, resd);
    
    resd = _mm256_cmp_pd(c, d, _CMP_NEQ_UQ);
    _mm256_store_pd((double*)&sink, resd);
    
    resd = _mm256_cmp_pd(c, d, _CMP_NLT_UQ);
    _mm256_store_pd((double*)&sink, resd);
    
    resd = _mm256_cmp_pd(c, d, _CMP_NLE_UQ);
    _mm256_store_pd((double*)&sink, resd);
    
    resd = _mm256_cmp_pd(c, d, _CMP_LE_OS);
    _mm256_store_pd((double*)&sink, resd);
    
    resd = _mm256_cmp_pd(c, d, _CMP_LT_OS);
    _mm256_store_pd((double*)&sink, resd);
    
    resd = _mm256_cmp_pd(c, d, _CMP_NEQ_OQ);
    _mm256_store_pd((double*)&sink, resd);
}
#endif

// Test mixed comparisons in loops to prevent optimization
int test_loop_comparisons() {
    float f1, f2;
    double d1, d2;
    int sum = 0;
    
    // Create NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    
    // Test with different combinations
    for (int i = 0; i < 10; i++) {
        f1 = (i % 2 == 0) ? (float)i : nan_f;
        f2 = (i % 3 == 0) ? (float)(i * 2) : nan_f;
        d1 = (i % 2 == 0) ? (double)i : nan_d;
        d2 = (i % 3 == 0) ? (double)(i * 2) : nan_d;
        
        // UNORDERED
        sum += __builtin_isunordered(f1, f2);
        sum += __builtin_isunordered(d1, d2);
        
        // ORDERED
        sum += __builtin_isordered(f1, f2);
        sum += __builtin_isordered(d1, d2);
        
        // UNEQ
        sum += (f1 != f1) ? 1 : 0;
        sum += (d1 != d1) ? 1 : 0;
        
        // UNLT, UNLE, UNGT, UNGE with NaN operands
        sum += (f1 < f2) ? 1 : 0;
        sum += (f1 <= f2) ? 1 : 0;
        sum += (f1 > f2) ? 1 : 0;
        sum += (f1 >= f2) ? 1 : 0;
        
        // LTGT
        sum += __builtin_islessgreater(f1, f2);
        sum += __builtin_islessgreater(d1, d2);
    }
    
    return sum;
}

int main() {
    // Initialize with NaN and normal values
    float nan_f = __builtin_nanf("");
    float normal_f = 3.14f;
    double nan_d = __builtin_nan("");
    double normal_d = 2.71828;
    
    // Test scalar comparisons
    test_scalar_comparisons(nan_f, normal_f, nan_d, normal_d);
    test_scalar_comparisons(normal_f, nan_f, normal_d, nan_d);
    test_scalar_comparisons(nan_f, nan_f, nan_d, nan_d);
    test_scalar_comparisons(normal_f, normal_f, normal_d, normal_d);
    
    // Initialize SSE vectors
    __m128 vec_f1 = _mm_setr_ps(1.0f, nan_f, 3.0f, 4.0f);
    __m128 vec_f2 = _mm_setr_ps(nan_f, 2.0f, 3.0f, nan_f);
    __m128d vec_d1 = _mm_setr_pd(1.0, nan_d);
    __m128d vec_d2 = _mm_setr_pd(nan_d, 2.0);
    
    // Test SSE intrinsics
    test_sse_intrinsics(vec_f1, vec_f2, vec_d1, vec_d2);
    
#ifdef __AVX__
    // Initialize AVX vectors
    __m256 vec_f1_avx = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, nan_f, 7.0f, 8.0f);
    __m256 vec_f2_avx = _mm256_setr_ps(nan_f, 2.0f, 3.0f, nan_f, nan_f, 6.0f, 7.0f, nan_f);
    __m256d vec_d1_avx = _mm256_setr_pd(1.0, nan_d, 3.0, 4.0);
    __m256d vec_d2_avx = _mm256_setr_pd(nan_d, 2.0, nan_d, 4.0);
    
    // Test AVX intrinsics
    test_avx_intrinsics(vec_f1_avx, vec_f2_avx, vec_d1_avx, vec_d2_avx);
#endif
    
    // Test loop comparisons
    int result = test_loop_comparisons();
    
    printf("Result: %d\n", result);
    printf("Test completed\n");
    
    return 0;
}
