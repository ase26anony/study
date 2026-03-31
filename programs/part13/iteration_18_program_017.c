#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent dead code elimination
__attribute__((noinline)) void use_result(int val) {
    volatile int sink = val;
    (void)sink;
}

__attribute__((noinline)) void use_vector(__m128 v) {
    volatile __m128 sink = v;
    (void)sink;
}

__attribute__((noinline)) void use_vector_d(__m128d v) {
    volatile __m128d sink = v;
    (void)sink;
}

#ifdef __AVX__
__attribute__((noinline)) void use_vector_avx(__m256 v) {
    volatile __m256 sink = v;
    (void)sink;
}

__attribute__((noinline)) void use_vector_avx_d(__m256d v) {
    volatile __m256d sink = v;
    (void)sink;
}
#endif

int main() {
    int result = 0;
    
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 2.71;
    
    // 1. UNORDERED condition
    // Using __builtin_isunordered
    if (__builtin_isunordered(f1, nan_f)) {
        result |= 1;
    }
    if (__builtin_isunordered(nan_d, d1)) {
        result |= 2;
    }
    
    // 2. ORDERED condition
    // Using __builtin_isordered
    if (__builtin_isordered(f1, f2)) {
        result |= 4;
    }
    if (__builtin_isordered(d1, d2)) {
        result |= 8;
    }
    
    // 3. UNEQ condition (unordered or equal)
    // Using direct comparison with NaN
    if (f1 != f1) {  // false for normal numbers
        result |= 16;
    }
    if (nan_f == nan_f) {  // false in IEEE 754
        result |= 32;
    }
    
    // 4. UNLT condition (unordered or less than)
    if (nan_f < f1) {  // unordered comparison
        result |= 64;
    }
    if (__builtin_isless(nan_f, f1)) {  // unordered comparison
        result |= 128;
    }
    
    // 5. UNLE condition (unordered or less than or equal)
    if (nan_f <= f1) {  // unordered comparison
        result |= 256;
    }
    if (__builtin_islessequal(nan_f, f1)) {  // unordered comparison
        result |= 512;
    }
    
    // 6. UNGT condition (unordered or greater than)
    if (nan_f > f1) {  // unordered comparison
        result |= 1024;
    }
    if (__builtin_isgreater(nan_f, f1)) {  // unordered comparison
        result |= 2048;
    }
    
    // 7. UNGE condition (unordered or greater than or equal)
    if (nan_f >= f1) {  // unordered comparison
        result |= 4096;
    }
    if (__builtin_isgreaterequal(nan_f, f1)) {  // unordered comparison
        result |= 8192;
    }
    
    // 8. LTGT condition (less than or greater than, but not equal and not unordered)
    if (__builtin_islessgreater(f1, f2)) {  // true for ordered, unequal numbers
        result |= 16384;
    }
    if (__builtin_islessgreater(d1, d2)) {
        result |= 32768;
    }
    
    // SSE vector comparisons (128-bit)
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(1.0f, 3.0f, 5.0f, nan_f);
    __m128 vec_f3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_f1, vec_f2);
    use_vector(cmp_unord);
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(vec_f1, vec_f2);
    use_vector(cmp_ord);
    
    // UNEQ vector comparison (not equal)
    __m128 cmp_uneq = _mm_cmpneq_ps(vec_f1, vec_f3);
    use_vector(cmp_uneq);
    
    // UNGE vector comparison (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_f1, vec_f3);
    use_vector(cmp_nlt);
    
    // UNGT vector comparison (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_f1, vec_f3);
    use_vector(cmp_nle);
    
    // UNLE vector comparison
    __m128 cmp_ule = _mm_cmpule_ps(vec_f1, vec_f3);
    use_vector(cmp_ule);
    
    // UNLT vector comparison
    __m128 cmp_ult = _mm_cmpult_ps(vec_f1, vec_f3);
    use_vector(cmp_ult);
    
    // Double precision SSE comparisons
    __m128d vec_d1 = _mm_set_pd(nan_d, 1.0);
    __m128d vec_d2 = _mm_set_pd(2.0, nan_d);
    __m128d vec_d3 = _mm_set_pd(1.0, 2.0);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_d1, vec_d2);
    use_vector_d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_d1, vec_d2);
    use_vector_d(cmp_ord_d);
    
    __m128d cmp_uneq_d = _mm_cmpneq_pd(vec_d1, vec_d3);
    use_vector_d(cmp_uneq_d);
    
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(vec_d1, vec_d3);
    use_vector_d(cmp_nlt_d);
    
    __m128d cmp_nle_d = _mm_cmpnle_pd(vec_d1, vec_d3);
    use_vector_d(cmp_nle_d);
    
    __m128d cmp_ule_d = _mm_cmpule_pd(vec_d1, vec_d3);
    use_vector_d(cmp_ule_d);
    
    __m128d cmp_ult_d = _mm_cmpult_pd(vec_d1, vec_d3);
    use_vector_d(cmp_ult_d);
    
#ifdef __AVX__
    // AVX vector comparisons (256-bit)
    __m256 vec_f1_avx = _mm256_set_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 vec_f2_avx = _mm256_set_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, nan_f, 7.0f, 8.0f);
    __m256 vec_f3_avx = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_UNORD_Q);
    use_vector_avx(cmp_unord_avx);
    
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_ORD_Q);
    use_vector_avx(cmp_ord_avx);
    
    __m256 cmp_uneq_avx = _mm256_cmp_ps(vec_f1_avx, vec_f3_avx, _CMP_NEQ_UQ);
    use_vector_avx(cmp_uneq_avx);
    
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vec_f1_avx, vec_f3_avx, _CMP_NLT_UQ);
    use_vector_avx(cmp_nlt_avx);
    
    __m256 cmp_nle_avx = _mm256_cmp_ps(vec_f1_avx, vec_f3_avx, _CMP_NLE_UQ);
    use_vector_avx(cmp_nle_avx);
    
    __m256 cmp_ule_avx = _mm256_cmp_ps(vec_f1_avx, vec_f3_avx, _CMP_LE_OS);
    use_vector_avx(cmp_ule_avx);
    
    __m256 cmp_ult_avx = _mm256_cmp_ps(vec_f1_avx, vec_f3_avx, _CMP_LT_OS);
    use_vector_avx(cmp_ult_avx);
    
    // AVX double precision
    __m256d vec_d1_avx = _mm256_set_pd(nan_d, 2.0, nan_d, 4.0);
    __m256d vec_d2_avx = _mm256_set_pd(1.0, nan_d, 3.0, nan_d);
    __m256d vec_d3_avx = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_UNORD_Q);
    use_vector_avx_d(cmp_unord_avx_d);
    
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_ORD_Q);
    use_vector_avx_d(cmp_ord_avx_d);
    
    __m256d cmp_uneq_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d3_avx, _CMP_NEQ_UQ);
    use_vector_avx_d(cmp_uneq_avx_d);
    
    __m256d cmp_nlt_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d3_avx, _CMP_NLT_UQ);
    use_vector_avx_d(cmp_nlt_avx_d);
    
    __m256d cmp_nle_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d3_avx, _CMP_NLE_UQ);
    use_vector_avx_d(cmp_nle_avx_d);
    
    __m256d cmp_ule_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d3_avx, _CMP_LE_OS);
    use_vector_avx_d(cmp_ule_avx_d);
    
    __m256d cmp_ult_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d3_avx, _CMP_LT_OS);
    use_vector_avx_d(cmp_ult_avx_d);
#endif
    
    // Loop to prevent optimization and generate more comparison patterns
    volatile float vf = 0.0f;
    for (int i = 0; i < 10; i++) {
        vf += 0.1f;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(vf, nan_f)) {
            result++;
        }
        
        if (vf != vf) {  // UNEQ pattern
            result--;
        }
        
        if (nan_f < vf) {  // UNLT pattern
            result ^= i;
        }
        
        if (__builtin_islessgreater(vf, f1)) {  // LTGT pattern
            result |= i;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
