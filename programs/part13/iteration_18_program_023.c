#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

// Test various unordered floating-point comparisons
void test_scalar_comparisons() {
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    float f1 = 1.0f, f2 = 2.0f;
    double d1 = 1.0, d2 = 2.0;
    
    int results[32] = {0};
    int idx = 0;
    
    // UNORDERED: __builtin_isunordered
    results[idx++] = __builtin_isunordered(nan_f, f1);  // true
    results[idx++] = __builtin_isunordered(f1, nan_f);  // true
    results[idx++] = __builtin_isunordered(nan_f, nan_f); // true
    results[idx++] = __builtin_isunordered(f1, f2);     // false
    
    // ORDERED: !__builtin_isunordered
    results[idx++] = !__builtin_isunordered(f1, f2);    // true
    results[idx++] = !__builtin_isunordered(nan_f, f1); // false
    
    // UNEQ: x == y where at least one is NaN
    results[idx++] = (nan_f == nan_f);  // false (NaN != NaN)
    results[idx++] = (nan_f != nan_f);  // true (UNEQ condition)
    results[idx++] = (nan_d != nan_d);  // true
    
    // UNLT: x < y with NaN operand
    results[idx++] = (nan_f < f1);      // false (unordered)
    results[idx++] = (f1 < nan_f);      // false (unordered)
    
    // UNLE: x <= y with NaN operand  
    results[idx++] = (nan_f <= f1);     // false
    results[idx++] = (f1 <= nan_f);     // false
    
    // UNGT: x > y with NaN operand
    results[idx++] = (nan_f > f1);      // false
    results[idx++] = (f1 > nan_f);      // false
    
    // UNGE: x >= y with NaN operand
    results[idx++] = (nan_f >= f1);     // false
    results[idx++] = (f1 >= nan_f);     // false
    
    // LTGT: __builtin_islessgreater
    results[idx++] = __builtin_islessgreater(f1, f2);   // true (1.0 < 2.0)
    results[idx++] = __builtin_islessgreater(f2, f1);   // true (2.0 > 1.0)
    results[idx++] = __builtin_islessgreater(f1, f1);   // false (equal)
    results[idx++] = __builtin_islessgreater(nan_f, f1); // false (unordered)
    results[idx++] = __builtin_islessgreater(f1, nan_f); // false
    
    // Conditional branches to force code generation
    if (__builtin_isunordered(nan_f, f1)) sink = 1;
    if (!__builtin_isunordered(f1, f2)) sink = 2;
    if (nan_f != nan_f) sink = 3;
    if (__builtin_islessgreater(f1, f2)) sink = 4;
    
    // Ternary operators
    int r1 = (nan_f == nan_f) ? 1 : 0;
    int r2 = (nan_f != nan_f) ? 1 : 0;
    int r3 = (nan_f < f1) ? 1 : 0;
    int r4 = (f1 < nan_f) ? 1 : 0;
    int r5 = __builtin_islessgreater(f1, f2) ? 1 : 0;
    
    results[idx++] = r1 + r2 + r3 + r4 + r5;
}

// Test SSE vector comparisons
void test_sse_comparisons() {
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 v2 = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 v_nan = _mm_setr_ps(
        __builtin_nanf(""), 
        2.0f, 
        __builtin_nanf(""), 
        4.0f
    );
    
    // UNORDERED
    __m128 cmp_unord = _mm_cmpunord_ps(v_nan, v1);
    
    // ORDERED  
    __m128 cmp_ord = _mm_cmpord_ps(v1, v2);
    
    // UNEQ (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(v_nan, v1);
    
    // UNGE (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(v_nan, v1);
    
    // UNGT (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(v_nan, v1);
    
    // UNLE (unordered or less than or equal)
    __m128 cmp_ule = _mm_cmpule_ps(v_nan, v1);
    
    // UNLT (unordered or less than)
    __m128 cmp_ult = _mm_cmpult_ps(v_nan, v1);
    
    // Store results to prevent optimization
    float results[32];
    _mm_storeu_ps(results, cmp_unord);
    _mm_storeu_ps(results + 4, cmp_ord);
    _mm_storeu_ps(results + 8, cmp_neq);
    _mm_storeu_ps(results + 12, cmp_nlt);
    _mm_storeu_ps(results + 16, cmp_nle);
    _mm_storeu_ps(results + 20, cmp_ule);
    _mm_storeu_ps(results + 24, cmp_ult);
    
    // Double precision SSE
    __m128d d1 = _mm_setr_pd(1.0, 2.0);
    __m128d d2 = _mm_setr_pd(2.0, 1.0);
    __m128d d_nan = _mm_setr_pd(__builtin_nan(""), 2.0);
    
    __m128d cmp_unord_pd = _mm_cmpunord_pd(d_nan, d1);
    __m128d cmp_ord_pd = _mm_cmpord_pd(d1, d2);
    __m128d cmp_neq_pd = _mm_cmpneq_pd(d_nan, d1);
    __m128d cmp_nlt_pd = _mm_cmpnlt_pd(d_nan, d1);
    __m128d cmp_nle_pd = _mm_cmpnle_pd(d_nan, d1);
    
    _mm_storeu_pd(results + 28, cmp_unord_pd);
    _mm_storeu_pd(results + 30, cmp_ord_pd);
}

// Test AVX vector comparisons if available
#ifdef __AVX__
void test_avx_comparisons() {
    __m256 v1 = _mm256_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 v2 = _mm256_setr_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    __m256 v_nan = _mm256_setr_ps(
        __builtin_nanf(""), 2.0f, __builtin_nanf(""), 4.0f,
        __builtin_nanf(""), 6.0f, __builtin_nanf(""), 8.0f
    );
    
    // Various unordered comparisons
    __m256 cmp_unord = _mm256_cmp_ps(v_nan, v1, _CMP_UNORD_Q);
    __m256 cmp_ord = _mm256_cmp_ps(v1, v2, _CMP_ORD_Q);
    __m256 cmp_neq = _mm256_cmp_ps(v_nan, v1, _CMP_NEQ_UQ);
    __m256 cmp_nlt = _mm256_cmp_ps(v_nan, v1, _CMP_NLT_UQ);
    __m256 cmp_nle = _mm256_cmp_ps(v_nan, v1, _CMP_NLE_UQ);
    
    // Double precision AVX
    __m256d d1 = _mm256_setr_pd(1.0, 2.0, 3.0, 4.0);
    __m256d d2 = _mm256_setr_pd(4.0, 3.0, 2.0, 1.0);
    __m256d d_nan = _mm256_setr_pd(
        __builtin_nan(""), 2.0, __builtin_nan(""), 4.0
    );
    
    __m256d cmp_unord_pd = _mm256_cmp_pd(d_nan, d1, _CMP_UNORD_Q);
    __m256d cmp_ord_pd = _mm256_cmp_pd(d1, d2, _CMP_ORD_Q);
    __m256d cmp_neq_pd = _mm256_cmp_pd(d_nan, d1, _CMP_NEQ_UQ);
    __m256d cmp_nlt_pd = _mm256_cmp_pd(d_nan, d1, _CMP_NLT_UQ);
    __m256d cmp_nle_pd = _mm256_cmp_pd(d_nan, d1, _CMP_NLE_UQ);
    
    // Store results
    float results[32];
    double dresults[16];
    _mm256_storeu_ps(results, cmp_unord);
    _mm256_storeu_pd(dresults, cmp_unord_pd);
}
#endif

// Loop to prevent dead code elimination
void test_loop_comparisons() {
    volatile float vals[4] = {1.0f, 2.0f, __builtin_nanf(""), 4.0f};
    volatile double dvals[4] = {1.0, 2.0, __builtin_nan(""), 4.0};
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        // Mix of ordered and unordered comparisons
        if (__builtin_isunordered(vals[i % 4], vals[(i + 1) % 4])) {
            sum += 1;
        }
        if (vals[i % 4] != vals[i % 4]) {  // UNEQ
            sum += 2;
        }
        if (__builtin_islessgreater(vals[i % 4], vals[(i + 2) % 4])) {
            sum += 3;
        }
        if (!__builtin_isunordered(dvals[i % 4], dvals[(i + 1) % 4])) {
            sum += 4;
        }
    }
    sink = sum;
}

int main() {
    printf("Testing unordered floating-point comparisons...\n");
    
    test_scalar_comparisons();
    test_sse_comparisons();
    
#ifdef __AVX__
    test_avx_comparisons();
    printf("AVX comparisons tested\n");
#endif
    
    test_loop_comparisons();
    
    printf("All unordered comparison tests completed\n");
    printf("Sink value: %d\n", sink);
    
    return 0;
}
