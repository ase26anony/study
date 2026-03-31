#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

// Test various unordered floating-point comparisons
void test_scalar_comparisons() {
    float nan_f = NAN;
    double nan_d = NAN;
    float inf_f = INFINITY;
    double inf_d = INFINITY;
    
    float f1 = 1.5f;
    float f2 = 2.5f;
    double d1 = 1.5;
    double d2 = 2.5;
    
    int result = 0;
    
    // UNORDERED: __builtin_isunordered
    if (__builtin_isunordered(nan_f, f1)) result |= 1;
    if (__builtin_isunordered(f1, nan_f)) result |= 2;
    if (__builtin_isunordered(nan_d, d1)) result |= 4;
    
    // ORDERED: !__builtin_isunordered
    if (!__builtin_isunordered(f1, f2)) result |= 8;
    if (!__builtin_isunordered(d1, d2)) result |= 16;
    
    // UNEQ: x != x (is NaN) or comparison with NaN
    if (nan_f != nan_f) result |= 32;           // UNEQ
    if (nan_d != nan_d) result |= 64;           // UNEQ
    if (__builtin_isunordered(f1, f2) && !__builtin_isless(f1, f2) && !__builtin_isgreater(f1, f2)) 
        result |= 128;
    
    // UNLT: < with NaN operand
    if (nan_f < f1) result |= 256;              // UNLT
    if (f1 < nan_f) result |= 512;              // UNLT
    
    // UNLE: <= with NaN operand
    if (nan_f <= f1) result |= 1024;            // UNLE
    if (f1 <= nan_f) result |= 2048;            // UNLE
    
    // UNGT: > with NaN operand
    if (nan_f > f1) result |= 4096;             // UNGT
    if (f1 > nan_f) result |= 8192;             // UNGT
    
    // UNGE: >= with NaN operand
    if (nan_f >= f1) result |= 16384;           // UNGE
    if (f1 >= nan_f) result |= 32768;           // UNGE
    
    // LTGT: __builtin_islessgreater
    if (__builtin_islessgreater(f1, f2)) result |= 65536;
    if (__builtin_islessgreater(d1, d2)) result |= 131072;
    if (__builtin_islessgreater(nan_f, f1)) result |= 262144;  // With NaN
    
    sink = result;
}

// Test SSE vector comparisons (128-bit)
void test_sse_comparisons() {
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 v2 = _mm_setr_ps(4.0f, 3.0f, 2.0f, NAN);
    __m128 v3 = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    __m128d d1 = _mm_setr_pd(1.0, NAN);
    __m128d d2 = _mm_setr_pd(NAN, 2.0);
    __m128d d3 = _mm_setr_pd(1.0, 2.0);
    
    // UNORDERED
    __m128 unord_ps = _mm_cmpunord_ps(v1, v2);
    __m128d unord_pd = _mm_cmpunord_pd(d1, d2);
    
    // ORDERED
    __m128 ord_ps = _mm_cmpord_ps(v1, v3);
    __m128d ord_pd = _mm_cmpord_pd(d1, d3);
    
    // UNEQ (not equal)
    __m128 uneq_ps = _mm_cmpneq_ps(v1, v2);
    __m128d uneq_pd = _mm_cmpneq_pd(d1, d2);
    
    // UNGE (not less than)
    __m128 unge_ps = _mm_cmpnlt_ps(v1, v2);
    __m128d unge_pd = _mm_cmpnlt_pd(d1, d2);
    
    // UNGT (not less than or equal)
    __m128 ungt_ps = _mm_cmpnle_ps(v1, v2);
    __m128d ungt_pd = _mm_cmpnle_pd(d1, d2);
    
    // UNLE (unordered or less than or equal) - use _CMP_LE_OQ
    __m128 unle_ps = _mm_cmple_ps(v1, v2);
    __m128d unle_pd = _mm_cmple_pd(d1, d2);
    
    // UNLT (unordered or less than) - use _CMP_LT_OQ
    __m128 unlt_ps = _mm_cmplt_ps(v1, v2);
    __m128d unlt_pd = _mm_cmplt_pd(d1, d2);
    
    // LTGT (not equal) - same as UNEQ for these intrinsics
    __m128 ltgt_ps = _mm_cmpneq_ps(v3, _mm_set1_ps(2.5f));
    __m128d ltgt_pd = _mm_cmpneq_pd(d3, _mm_set1_pd(1.5));
    
    // Store results to prevent optimization
    float fbuf[4];
    double dbuf[2];
    _mm_store_ps(fbuf, unord_ps);
    _mm_store_pd(dbuf, unord_pd);
    
    sink = (int)fbuf[0] + (int)dbuf[0];
}

// Test AVX vector comparisons (256-bit)
#ifdef __AVX__
void test_avx_comparisons() {
    __m256 v1 = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, NAN, 7.0f, 8.0f);
    __m256 v2 = _mm256_setr_ps(8.0f, 7.0f, 6.0f, NAN, 4.0f, 3.0f, 2.0f, NAN);
    __m256 v3 = _mm256_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    
    __m256d d1 = _mm256_setr_pd(1.0, NAN, 3.0, NAN);
    __m256d d2 = _mm256_setr_pd(NAN, 2.0, NAN, 4.0);
    __m256d d3 = _mm256_setr_pd(1.0, 2.0, 3.0, 4.0);
    
    // UNORDERED
    __m256 unord_ps = _mm256_cmp_ps(v1, v2, _CMP_UNORD_Q);
    __m256d unord_pd = _mm256_cmp_pd(d1, d2, _CMP_UNORD_Q);
    
    // ORDERED
    __m256 ord_ps = _mm256_cmp_ps(v1, v3, _CMP_ORD_Q);
    __m256d ord_pd = _mm256_cmp_pd(d1, d3, _CMP_ORD_Q);
    
    // UNEQ
    __m256 uneq_ps = _mm256_cmp_ps(v1, v2, _CMP_NEQ_UQ);
    __m256d uneq_pd = _mm256_cmp_pd(d1, d2, _CMP_NEQ_UQ);
    
    // UNGE
    __m256 unge_ps = _mm256_cmp_ps(v1, v2, _CMP_NLT_UQ);
    __m256d unge_pd = _mm256_cmp_pd(d1, d2, _CMP_NLT_UQ);
    
    // UNGT
    __m256 ungt_ps = _mm256_cmp_ps(v1, v2, _CMP_NLE_UQ);
    __m256d ungt_pd = _mm256_cmp_pd(d1, d2, _CMP_NLE_UQ);
    
    // UNLE
    __m256 unle_ps = _mm256_cmp_ps(v1, v2, _CMP_LE_OS);
    __m256d unle_pd = _mm256_cmp_pd(d1, d2, _CMP_LE_OS);
    
    // UNLT
    __m256 unlt_ps = _mm256_cmp_ps(v1, v2, _CMP_LT_OS);
    __m256d unlt_pd = _mm256_cmp_pd(d1, d2, _CMP_LT_OS);
    
    // LTGT
    __m256 ltgt_ps = _mm256_cmp_ps(v3, _mm256_set1_ps(2.5f), _CMP_NEQ_OQ);
    __m256d ltgt_pd = _mm256_cmp_pd(d3, _mm256_set1_pd(1.5), _CMP_NEQ_OQ);
    
    // Store results
    float fbuf[8];
    double dbuf[4];
    _mm256_store_ps(fbuf, unord_ps);
    _mm256_store_pd(dbuf, unord_pd);
    
    sink = (int)fbuf[0] + (int)dbuf[0];
}
#endif

// Test mixed comparisons in loops to generate conditional jumps
void test_conditional_branches() {
    float values[] = {1.0f, 2.0f, NAN, 4.0f, 5.0f};
    double dvalues[] = {1.0, NAN, 3.0, 4.0, NAN};
    
    int count_unordered = 0;
    int count_ordered = 0;
    int count_uneq = 0;
    int count_unlt = 0;
    int count_unle = 0;
    int count_ungt = 0;
    int count_unge = 0;
    int count_ltgt = 0;
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            // UNORDERED
            if (__builtin_isunordered(values[i], values[j])) {
                count_unordered++;
            }
            
            // ORDERED
            if (!__builtin_isunordered(values[i], values[j])) {
                count_ordered++;
            }
            
            // UNEQ (NaN != NaN is true)
            if (values[i] != values[i]) {
                count_uneq++;
            }
            
            // UNLT
            if (values[i] < values[j]) {
                count_unlt++;
            }
            
            // UNLE
            if (values[i] <= values[j]) {
                count_unle++;
            }
            
            // UNGT
            if (values[i] > values[j]) {
                count_ungt++;
            }
            
            // UNGE
            if (values[i] >= values[j]) {
                count_unge++;
            }
            
            // LTGT
            if (__builtin_islessgreater(dvalues[i], dvalues[j])) {
                count_ltgt++;
            }
        }
    }
    
    sink = count_unordered + count_ordered + count_uneq + count_unlt + 
           count_unle + count_ungt + count_unge + count_ltgt;
}

int main() {
    printf("Testing floating-point unordered comparisons...\n");
    
    // Test scalar comparisons
    test_scalar_comparisons();
    
    // Test SSE vector comparisons
    test_sse_comparisons();
    
    // Test AVX vector comparisons if available
    #ifdef __AVX__
    test_avx_comparisons();
    #endif
    
    // Test conditional branches with loops
    test_conditional_branches();
    
    printf("All tests completed (sink = %d)\n", sink);
    
    return 0;
}
