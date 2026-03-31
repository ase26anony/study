#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent aggressive optimization
volatile int sink = 0;

// Function to use results and prevent dead code elimination
void use_result(int r) {
    sink += r;
}

void use_vector_result(__m128 v) {
    float f[4];
    _mm_store_ps(f, v);
    sink += (int)f[0];
}

void use_vector_result_d(__m128d v) {
    double d[2];
    _mm_store_pd(d, v);
    sink += (int)d[0];
}

#ifdef __AVX__
void use_vector_result_avx(__m256 v) {
    float f[8];
    _mm256_store_ps(f, v);
    sink += (int)f[0];
}

void use_vector_result_avx_d(__m256d v) {
    double d[4];
    _mm256_store_pd(d, v);
    sink += (int)d[0];
}
#endif

int main() {
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 1.5, d2 = 2.5;
    
    int result = 0;
    
    // Test UNORDERED condition (case UNORDERED: "unord")
    // Using __builtin_isunordered
    if (__builtin_isunordered(nan_f, f1)) result |= 1;
    if (__builtin_isunordered(f1, nan_f)) result |= 2;
    if (__builtin_isunordered(nan_d, d1)) result |= 4;
    
    // Test ORDERED condition (case ORDERED: "ord")
    // Using !__builtin_isunordered
    if (!__builtin_isunordered(f1, f2)) result |= 8;
    if (!__builtin_isunordered(d1, d2)) result |= 16;
    
    // Test UNEQ condition (case UNEQ: "ueq")
    // x != x generates UNEQ when optimized
    if (nan_f != nan_f) result |= 32;
    if (nan_d != nan_d) result |= 64;
    
    // Test UNLT condition (case UNLT: "ult")
    // Compare NaN with normal number using <
    if (nan_f < f1) result |= 128;
    if (nan_d < d1) result |= 256;
    
    // Test UNLE condition (case UNLE: "ule")
    // Compare NaN with normal number using <=
    if (nan_f <= f1) result |= 512;
    if (nan_d <= d1) result |= 1024;
    
    // Test UNGT condition (case UNGT: "nle")
    // Compare normal number with NaN using >
    if (f1 > nan_f) result |= 2048;
    if (d1 > nan_d) result |= 4096;
    
    // Test UNGE condition (case UNGE: "nlt")
    // Compare normal number with NaN using >=
    if (f1 >= nan_f) result |= 8192;
    if (d1 >= nan_d) result |= 16384;
    
    // Test LTGT condition (case LTGT: "une")
    // Using __builtin_islessgreater
    if (__builtin_islessgreater(f1, f2)) result |= 32768;
    if (__builtin_islessgreater(d1, d2)) result |= 65536;
    if (__builtin_islessgreater(nan_f, f1)) result |= 131072;
    
    // SSE vector comparisons (128-bit)
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 vec_nan = _mm_set1_ps(nan_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_f1, vec_nan);
    use_vector_result(cmp_unord);
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(vec_f1, vec_f2);
    use_vector_result(cmp_ord);
    
    // UNEQ vector comparison
    __m128 cmp_neq = _mm_cmpneq_ps(vec_f1, vec_nan);
    use_vector_result(cmp_neq);
    
    // UNGE vector comparison (nlt)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_f1, vec_nan);
    use_vector_result(cmp_nlt);
    
    // UNGT vector comparison (nle)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_f1, vec_nan);
    use_vector_result(cmp_nle);
    
    // UNLE vector comparison (ule)
    __m128 cmp_ule = _mm_cmpule_ps(vec_nan, vec_f1);
    use_vector_result(cmp_ule);
    
    // UNLT vector comparison (ult)
    __m128 cmp_ult = _mm_cmpult_ps(vec_nan, vec_f1);
    use_vector_result(cmp_ult);
    
    // Double precision SSE comparisons
    __m128d vec_d1 = _mm_set_pd(1.0, 2.0);
    __m128d vec_d2 = _mm_set_pd(2.0, 1.0);
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_d1, vec_nan_d);
    use_vector_result_d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_ord_d);
    
    __m128d cmp_neq_d = _mm_cmpneq_pd(vec_d1, vec_nan_d);
    use_vector_result_d(cmp_neq_d);
    
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(vec_d1, vec_nan_d);
    use_vector_result_d(cmp_nlt_d);
    
    __m128d cmp_nle_d = _mm_cmpnle_pd(vec_d1, vec_nan_d);
    use_vector_result_d(cmp_nle_d);
    
    __m128d cmp_ule_d = _mm_cmpule_pd(vec_nan_d, vec_d1);
    use_vector_result_d(cmp_ule_d);
    
    __m128d cmp_ult_d = _mm_cmpult_pd(vec_nan_d, vec_d1);
    use_vector_result_d(cmp_ult_d);
    
#ifdef __AVX__
    // AVX vector comparisons (256-bit)
    __m256 vec_f1_avx = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec_f2_avx = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    __m256 vec_nan_avx = _mm256_set1_ps(nan_f);
    
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_UNORD_Q);
    use_vector_result_avx(cmp_unord_avx);
    
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_ORD_Q);
    use_vector_result_avx(cmp_ord_avx);
    
    __m256 cmp_neq_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_NEQ_UQ);
    use_vector_result_avx(cmp_neq_avx);
    
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_NLT_UQ);
    use_vector_result_avx(cmp_nlt_avx);
    
    __m256 cmp_nle_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_NLE_UQ);
    use_vector_result_avx(cmp_nle_avx);
    
    __m256 cmp_ule_avx = _mm256_cmp_ps(vec_nan_avx, vec_f1_avx, _CMP_LE_UQ);
    use_vector_result_avx(cmp_ule_avx);
    
    __m256 cmp_ult_avx = _mm256_cmp_ps(vec_nan_avx, vec_f1_avx, _CMP_LT_UQ);
    use_vector_result_avx(cmp_ult_avx);
    
    // AVX double precision
    __m256d vec_d1_avx = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    __m256d vec_d2_avx = _mm256_set_pd(4.0, 3.0, 2.0, 1.0);
    __m256d vec_nan_d_avx = _mm256_set1_pd(nan_d);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_nan_d_avx, _CMP_UNORD_Q);
    use_vector_result_avx_d(cmp_unord_avx_d);
    
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_ORD_Q);
    use_vector_result_avx_d(cmp_ord_avx_d);
    
    __m256d cmp_neq_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_nan_d_avx, _CMP_NEQ_UQ);
    use_vector_result_avx_d(cmp_neq_avx_d);
#endif
    
    // Loop to generate more comparison patterns
    for (int i = 0; i < 10; i++) {
        float dynamic_f = (i % 2 == 0) ? (float)i : nan_f;
        double dynamic_d = (i % 3 == 0) ? (double)i : nan_d;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(dynamic_f, f1)) result++;
        if (dynamic_d != dynamic_d) result++;
        if (dynamic_f < f1) result++;
        if (f1 > dynamic_f) result++;
        if (__builtin_islessgreater(dynamic_f, f2)) result++;
    }
    
    printf("Result: %d\n", result);
    printf("Sink: %d\n", sink);
    
    return 0;
}
