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
    int result = 0;
    
    // Initialize NaN values
    float nan_f = NAN;
    float inf_f = INFINITY;
    float normal_f = 3.14f;
    float zero_f = 0.0f;
    
    double nan_d = NAN;
    double inf_d = INFINITY;
    double normal_d = 2.718281828459045;
    double zero_d = 0.0;
    
    // Generate dynamic NaN through arithmetic
    float dyn_nan_f = nan_f * 2.0f;
    double dyn_nan_d = nan_d * 2.0;
    
    // ============================================
    // 1. UNORDERED condition (unord)
    // ============================================
    
    // Using __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1 << 0;
    }
    
    if (__builtin_isunordered(normal_d, nan_d)) {
        result |= 1 << 1;
    }
    
    // Using comparison with NaN
    if (nan_f != nan_f) {  // This is UNORDERED check
        result |= 1 << 2;
    }
    
    // ============================================
    // 2. ORDERED condition (ord)
    // ============================================
    
    // Ordered comparison between normal numbers
    if (!__builtin_isunordered(normal_f, normal_f)) {
        result |= 1 << 3;
    }
    
    if (!__builtin_isunordered(zero_d, inf_d)) {
        result |= 1 << 4;
    }
    
    // ============================================
    // 3. UNEQ condition (ueq)
    // ============================================
    
    // Compare NaN with itself (unordered equal)
    if (nan_f == nan_f) {  // This is UNEQ
        result |= 1 << 5;
    }
    
    // Compare normal number with NaN
    if (normal_d == nan_d) {  // UNEQ
        result |= 1 << 6;
    }
    
    // ============================================
    // 4. UNGE condition (nlt)
    // ============================================
    
    // Greater or equal with NaN operand
    if (nan_f >= normal_f) {  // UNGE -> nlt
        result |= 1 << 7;
    }
    
    if (normal_d >= nan_d) {  // UNGE -> nlt
        result |= 1 << 8;
    }
    
    // ============================================
    // 5. UNGT condition (nle)
    // ============================================
    
    // Greater than with NaN operand
    if (nan_f > normal_f) {  // UNGT -> nle
        result |= 1 << 9;
    }
    
    if (normal_d > nan_d) {  // UNGT -> nle
        result |= 1 << 10;
    }
    
    // ============================================
    // 6. UNLE condition (ule)
    // ============================================
    
    // Less or equal with NaN operand
    if (nan_f <= normal_f) {  // UNLE -> ule
        result |= 1 << 11;
    }
    
    if (normal_d <= nan_d) {  // UNLE -> ule
        result |= 1 << 12;
    }
    
    // ============================================
    // 7. UNLT condition (ult)
    // ============================================
    
    // Less than with NaN operand
    if (nan_f < normal_f) {  // UNLT -> ult
        result |= 1 << 13;
    }
    
    if (normal_d < nan_d) {  // UNLT -> ult
        result |= 1 << 14;
    }
    
    // ============================================
    // 8. LTGT condition (une)
    // ============================================
    
    // Using __builtin_islessgreater
    if (__builtin_islessgreater(nan_f, normal_f)) {
        result |= 1 << 15;
    }
    
    if (__builtin_islessgreater(normal_d, nan_d)) {
        result |= 1 << 16;
    }
    
    // ============================================
    // SSE Intrinsics for explicit condition codes
    // ============================================
    
    __m128 vec_nan_f = _mm_set1_ps(nan_f);
    __m128 vec_normal_f = _mm_set1_ps(normal_f);
    __m128 vec_zero_f = _mm_set1_ps(zero_f);
    
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    __m128d vec_normal_d = _mm_set1_pd(normal_d);
    __m128d vec_zero_d = _mm_set1_pd(zero_d);
    
    // UNORDERED (unord)
    __m128 cmp_unord_ps = _mm_cmpunord_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_unord_ps);
    
    __m128d cmp_unord_pd = _mm_cmpunord_pd(vec_nan_d, vec_normal_d);
    use_vector_d(cmp_unord_pd);
    
    // ORDERED (ord)
    __m128 cmp_ord_ps = _mm_cmpord_ps(vec_normal_f, vec_zero_f);
    use_vector(cmp_ord_ps);
    
    __m128d cmp_ord_pd = _mm_cmpord_pd(vec_normal_d, vec_zero_d);
    use_vector_d(cmp_ord_pd);
    
    // UNEQ (ueq) - Note: _mm_cmpeq_ps with NaN gives UNEQ
    __m128 cmp_ueq_ps = _mm_cmpeq_ps(vec_nan_f, vec_nan_f);
    use_vector(cmp_ueq_ps);
    
    // UNGE (nlt)
    __m128 cmp_nlt_ps = _mm_cmpnlt_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_nlt_ps);
    
    __m128d cmp_nlt_pd = _mm_cmpnlt_pd(vec_normal_d, vec_nan_d);
    use_vector_d(cmp_nlt_pd);
    
    // UNGT (nle)
    __m128 cmp_nle_ps = _mm_cmpnle_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_nle_ps);
    
    __m128d cmp_nle_pd = _mm_cmpnle_pd(vec_normal_d, vec_nan_d);
    use_vector_d(cmp_nle_pd);
    
    // UNLE (ule) - Note: SSE uses _mm_cmple_ps for UNLE with NaN
    __m128 cmp_ule_ps = _mm_cmple_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_ule_ps);
    
    // UNLT (ult) - Note: SSE uses _mm_cmplt_ps for UNLT with NaN
    __m128 cmp_ult_ps = _mm_cmplt_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_ult_ps);
    
    // LTGT (une) - _mm_cmpneq_ps
    __m128 cmp_une_ps = _mm_cmpneq_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_une_ps);
    
    __m128d cmp_une_pd = _mm_cmpneq_pd(vec_normal_d, vec_nan_d);
    use_vector_d(cmp_une_pd);
    
    // ============================================
    // AVX Intrinsics (if available)
    // ============================================
    
#ifdef __AVX__
    __m256 vec_nan_f_avx = _mm256_set1_ps(nan_f);
    __m256 vec_normal_f_avx = _mm256_set1_ps(normal_f);
    
    __m256d vec_nan_d_avx = _mm256_set1_pd(nan_d);
    __m256d vec_normal_d_avx = _mm256_set1_pd(normal_d);
    
    // UNORDERED
    __m256 cmp_unord_ps_avx = _mm256_cmp_ps(vec_nan_f_avx, vec_normal_f_avx, _CMP_UNORD_Q);
    use_vector_avx(cmp_unord_ps_avx);
    
    __m256d cmp_unord_pd_avx = _mm256_cmp_pd(vec_nan_d_avx, vec_normal_d_avx, _CMP_UNORD_Q);
    use_vector_avx_d(cmp_unord_pd_avx);
    
    // ORDERED
    __m256 cmp_ord_ps_avx = _mm256_cmp_ps(vec_normal_f_avx, vec_normal_f_avx, _CMP_ORD_Q);
    use_vector_avx(cmp_ord_ps_avx);
    
    // UNEQ
    __m256 cmp_ueq_ps_avx = _mm256_cmp_ps(vec_nan_f_avx, vec_nan_f_avx, _CMP_EQ_UQ);
    use_vector_avx(cmp_ueq_ps_avx);
    
    // UNGE (nlt)
    __m256 cmp_nlt_ps_avx = _mm256_cmp_ps(vec_nan_f_avx, vec_normal_f_avx, _CMP_NLT_UQ);
    use_vector_avx(cmp_nlt_ps_avx);
    
    // UNGT (nle)
    __m256 cmp_nle_ps_avx = _mm256_cmp_ps(vec_nan_f_avx, vec_normal_f_avx, _CMP_NLE_UQ);
    use_vector_avx(cmp_nle_ps_avx);
    
    // UNLE (ule)
    __m256 cmp_ule_ps_avx = _mm256_cmp_ps(vec_nan_f_avx, vec_normal_f_avx, _CMP_LE_OS);
    use_vector_avx(cmp_ule_ps_avx);
    
    // UNLT (ult)
    __m256 cmp_ult_ps_avx = _mm256_cmp_ps(vec_nan_f_avx, vec_normal_f_avx, _CMP_LT_OS);
    use_vector_avx(cmp_ult_ps_avx);
    
    // LTGT (une)
    __m256 cmp_une_ps_avx = _mm256_cmp_ps(vec_nan_f_avx, vec_normal_f_avx, _CMP_NEQ_UQ);
    use_vector_avx(cmp_une_ps_avx);
#endif
    
    // ============================================
    // Loop to prevent optimization and generate
    // more comparison instructions
    // ============================================
    
    float accum_f = 0.0f;
    double accum_d = 0.0;
    
    for (int i = 0; i < 10; i++) {
        float val_f = (float)i + 0.5f;
        double val_d = (double)i + 0.5;
        
        // Mix ordered and unordered comparisons in loop
        if (__builtin_isunordered(val_f, dyn_nan_f)) {
            accum_f += 1.0f;
        }
        
        if (val_d >= nan_d) {  // UNGE
            accum_d += 1.0;
        }
        
        if (__builtin_islessgreater(val_f, nan_f)) {  // LTGT
            accum_f += 2.0f;
        }
        
        if (val_d == nan_d) {  // UNEQ
            accum_d += 3.0;
        }
    }
    
    result += (int)accum_f + (int)accum_d;
    
    // Use all results to prevent dead code elimination
    use_result(result);
    
    printf("Result: %d (sink: %d)\n", result, sink);
    
    return 0;
}
