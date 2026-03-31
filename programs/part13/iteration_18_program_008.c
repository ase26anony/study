#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent optimization
volatile int sink = 0;

// Function to use results and prevent dead code elimination
__attribute__((noinline)) void use_result(int val) {
    sink += val;
}

__attribute__((noinline)) void use_vector(__m128 v) {
    float f = _mm_cvtss_f32(v);
    sink += (int)f;
}

__attribute__((noinline)) void use_vector_d(__m128d v) {
    double d = _mm_cvtsd_f64(v);
    sink += (int)d;
}

#ifdef __AVX__
__attribute__((noinline)) void use_vector_avx(__m256 v) {
    __m128 lo = _mm256_castps256_ps128(v);
    float f = _mm_cvtss_f32(lo);
    sink += (int)f;
}

__attribute__((noinline)) void use_vector_avx_d(__m256d v) {
    __m128d lo = _mm256_castpd256_pd128(v);
    double d = _mm_cvtsd_f64(lo);
    sink += (int)d;
}
#endif

int main() {
    int results = 0;
    
    // Create NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    // Regular float values
    float f1 = 1.5f;
    float f2 = 2.5f;
    double d1 = 1.5;
    double d2 = 2.5;
    
    // ========== SCALAR COMPARISONS ==========
    
    // UNORDERED condition
    if (__builtin_isunordered(f1, nan_f)) {
        results |= 1;
    }
    
    // ORDERED condition
    if (__builtin_isordered(f1, f2)) {
        results |= 2;
    }
    
    // UNEQ condition (x == y, unordered)
    if (nan_f == nan_f) {  // This is false for NaN, but generates UNEQ
        results |= 4;
    }
    
    // UNGE condition (!(x < y), unordered)
    if (!(nan_f < f1)) {  // Generates UNGE (nlt)
        results |= 8;
    }
    
    // UNGT condition (!(x <= y), unordered)
    if (!(nan_f <= f1)) {  // Generates UNGT (nle)
        results |= 16;
    }
    
    // UNLE condition (x <= y, unordered)
    if (nan_f <= f1) {  // Generates UNLE (ule)
        results |= 32;
    }
    
    // UNLT condition (x < y, unordered)
    if (nan_f < f1) {  // Generates UNLT (ult)
        results |= 64;
    }
    
    // LTGT condition (x != y, ordered)
    if (__builtin_islessgreater(f1, f2)) {
        results |= 128;
    }
    
    // Double precision versions
    if (__builtin_isunordered(d1, nan_d)) results |= 256;
    if (__builtin_isordered(d1, d2)) results |= 512;
    if (nan_d == nan_d) results |= 1024;
    if (!(nan_d < d1)) results |= 2048;
    if (!(nan_d <= d1)) results |= 4096;
    if (nan_d <= d1) results |= 8192;
    if (nan_d < d1) results |= 16384;
    if (__builtin_islessgreater(d1, d2)) results |= 32768;
    
    // ========== SSE VECTOR COMPARISONS ==========
    
    // Create SSE vectors
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 vec_nan = _mm_set_ps(nan_f, nan_f, 1.0f, 2.0f);
    
    __m128d vec_d1 = _mm_set_pd(1.0, 2.0);
    __m128d vec_d2 = _mm_set_pd(2.0, 1.0);
    __m128d vec_nan_d = _mm_set_pd(nan_d, 1.0);
    
    // UNORDERED - Compare for unordered
    __m128 res_unord = _mm_cmpunord_ps(vec_f1, vec_nan);
    use_vector(res_unord);
    
    // ORDERED - Compare for ordered
    __m128 res_ord = _mm_cmpord_ps(vec_f1, vec_f2);
    use_vector(res_ord);
    
    // UNEQ - Compare for not equal
    __m128 res_uneq = _mm_cmpneq_ps(vec_f1, vec_nan);
    use_vector(res_uneq);
    
    // UNGE - Compare for not less than (nlt)
    __m128 res_unge = _mm_cmpnlt_ps(vec_nan, vec_f1);
    use_vector(res_unge);
    
    // UNGT - Compare for not less than or equal (nle)
    __m128 res_ungt = _mm_cmpnle_ps(vec_nan, vec_f1);
    use_vector(res_ungt);
    
    // UNLE - Compare for less than or equal unordered (ule)
    // Note: _mm_cmpule_ps doesn't exist, we use combination
    __m128 res_unle = _mm_or_ps(_mm_cmple_ps(vec_nan, vec_f1),
                               _mm_cmpunord_ps(vec_nan, vec_f1));
    use_vector(res_unle);
    
    // UNLT - Compare for less than unordered (ult)
    __m128 res_unlt = _mm_or_ps(_mm_cmplt_ps(vec_nan, vec_f1),
                               _mm_cmpunord_ps(vec_nan, vec_f1));
    use_vector(res_unlt);
    
    // LTGT - Compare for not equal (une) - same as UNEQ
    __m128 res_ltgt = _mm_cmpneq_ps(vec_f1, vec_f2);
    use_vector(res_ltgt);
    
    // Double precision SSE comparisons
    __m128d res_unord_d = _mm_cmpunord_pd(vec_d1, vec_nan_d);
    use_vector_d(res_unord_d);
    
    __m128d res_ord_d = _mm_cmpord_pd(vec_d1, vec_d2);
    use_vector_d(res_ord_d);
    
    __m128d res_uneq_d = _mm_cmpneq_pd(vec_d1, vec_nan_d);
    use_vector_d(res_uneq_d);
    
    __m128d res_unge_d = _mm_cmpnlt_pd(vec_nan_d, vec_d1);
    use_vector_d(res_unge_d);
    
    __m128d res_ungt_d = _mm_cmpnle_pd(vec_nan_d, vec_d1);
    use_vector_d(res_ungt_d);
    
    // ========== AVX VECTOR COMPARISONS ==========
#ifdef __AVX__
    // Create AVX vectors
    __m256 vec_f1_avx = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f,
                                      5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec_f2_avx = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f,
                                      4.0f, 3.0f, 2.0f, 1.0f);
    __m256 vec_nan_avx = _mm256_set_ps(nan_f, nan_f, nan_f, 1.0f,
                                       2.0f, 3.0f, 4.0f, 5.0f);
    
    __m256d vec_d1_avx = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    __m256d vec_d2_avx = _mm256_set_pd(4.0, 3.0, 2.0, 1.0);
    __m256d vec_nan_avx_d = _mm256_set_pd(nan_d, nan_d, 1.0, 2.0);
    
    // AVX single precision comparisons
    __m256 res_unord_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_UNORD_Q);
    use_vector_avx(res_unord_avx);
    
    __m256 res_ord_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_ORD_Q);
    use_vector_avx(res_ord_avx);
    
    __m256 res_uneq_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_NEQ_UQ);
    use_vector_avx(res_uneq_avx);
    
    __m256 res_unge_avx = _mm256_cmp_ps(vec_nan_avx, vec_f1_avx, _CMP_NLT_UQ);
    use_vector_avx(res_unge_avx);
    
    __m256 res_ungt_avx = _mm256_cmp_ps(vec_nan_avx, vec_f1_avx, _CMP_NLE_UQ);
    use_vector_avx(res_ungt_avx);
    
    __m256 res_unle_avx = _mm256_cmp_ps(vec_nan_avx, vec_f1_avx, _CMP_LE_OS);
    use_vector_avx(res_unle_avx);
    
    __m256 res_unlt_avx = _mm256_cmp_ps(vec_nan_avx, vec_f1_avx, _CMP_LT_OS);
    use_vector_avx(res_unlt_avx);
    
    __m256 res_ltgt_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_NEQ_OQ);
    use_vector_avx(res_ltgt_avx);
    
    // AVX double precision comparisons
    __m256d res_unord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_nan_avx_d, _CMP_UNORD_Q);
    use_vector_avx_d(res_unord_avx_d);
    
    __m256d res_ord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_ORD_Q);
    use_vector_avx_d(res_ord_avx_d);
    
    __m256d res_uneq_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_nan_avx_d, _CMP_NEQ_UQ);
    use_vector_avx_d(res_uneq_avx_d);
    
    __m256d res_unge_avx_d = _mm256_cmp_pd(vec_nan_avx_d, vec_d1_avx, _CMP_NLT_UQ);
    use_vector_avx_d(res_unge_avx_d);
    
    __m256d res_ungt_avx_d = _mm256_cmp_pd(vec_nan_avx_d, vec_d1_avx, _CMP_NLE_UQ);
    use_vector_avx_d(res_ungt_avx_d);
#endif
    
    // ========== LOOP WITH VARIABLE VALUES ==========
    // Prevent optimization and generate more comparison patterns
    for (int i = 0; i < 10; i++) {
        float dynamic_f = (i % 2 == 0) ? nan_f : (float)i;
        double dynamic_d = (i % 3 == 0) ? nan_d : (double)i;
        
        // Generate various condition codes in a loop
        if (__builtin_isunordered(dynamic_f, f1)) {
            results += i;
        }
        
        if (__builtin_islessgreater(dynamic_f, f2)) {
            results -= i;
        }
        
        // Ternary operator forcing condition code generation
        int cond1 = (dynamic_f != dynamic_f) ? 1 : 0;
        int cond2 = (!(dynamic_d < d1)) ? 1 : 0;
        int cond3 = (dynamic_d <= d1) ? 1 : 0;
        
        use_result(cond1 + cond2 + cond3);
    }
    
    printf("Results: %d (sink: %d)\n", results, sink);
    return 0;
}
