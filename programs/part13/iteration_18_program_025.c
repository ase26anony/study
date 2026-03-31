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
__attribute__((noinline)) void use_vector_256(__m256 v) {
    float f = _mm256_cvtss_f32(v);
    sink += (int)f;
}

__attribute__((noinline)) void use_vector_256_d(__m256d v) {
    double d = _mm256_cvtsd_f64(v);
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
    double normal_d = 2.71828;
    double zero_d = 0.0;
    
    // 1. UNORDERED condition (__builtin_isunordered)
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // UNORDERED
    }
    
    // 2. ORDERED condition (negation of unordered)
    if (!__builtin_isunordered(normal_f, normal_f)) {
        result |= 2;  // ORDERED
    }
    
    // 3. UNEQ condition (x != x or x == NaN)
    if (nan_f != nan_f) {
        result |= 4;  // UNEQ
    }
    
    // 4. UNLT condition (unordered or less than)
    if (nan_f < normal_f) {
        result |= 8;  // UNLT
    }
    
    // 5. UNLE condition (unordered or less than or equal)
    if (nan_f <= normal_f) {
        result |= 16;  // UNLE
    }
    
    // 6. UNGT condition (unordered or greater than)
    if (nan_f > normal_f) {
        result |= 32;  // UNGT
    }
    
    // 7. UNGE condition (unordered or greater than or equal)
    if (nan_f >= normal_f) {
        result |= 64;  // UNGE
    }
    
    // 8. LTGT condition (__builtin_islessgreater)
    if (__builtin_islessgreater(nan_f, normal_f)) {
        result |= 128;  // LTGT
    }
    
    // Repeat with double precision
    if (__builtin_isunordered(nan_d, normal_d)) result |= 256;
    if (!__builtin_isunordered(normal_d, normal_d)) result |= 512;
    if (nan_d != nan_d) result |= 1024;
    if (nan_d < normal_d) result |= 2048;
    if (nan_d <= normal_d) result |= 4096;
    if (nan_d > normal_d) result |= 8192;
    if (nan_d >= normal_d) result |= 16384;
    if (__builtin_islessgreater(nan_d, normal_d)) result |= 32768;
    
    // SSE vector comparisons (128-bit)
    __m128 vec_nan_f = _mm_set1_ps(nan_f);
    __m128 vec_normal_f = _mm_set1_ps(normal_f);
    __m128 vec_inf_f = _mm_set1_ps(inf_f);
    __m128 vec_zero_f = _mm_set1_ps(zero_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_unord);
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(vec_normal_f, vec_normal_f);
    use_vector(cmp_ord);
    
    // UNEQ vector comparison (not equal)
    __m128 cmp_uneq = _mm_cmpneq_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_uneq);
    
    // UNGE vector comparison (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_nlt);
    
    // UNGT vector comparison (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_nle);
    
    // UNLE vector comparison (unordered or less than or equal)
    __m128 cmp_ule = _mm_cmpule_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_ule);
    
    // UNLT vector comparison (unordered or less than)
    __m128 cmp_ult = _mm_cmpult_ps(vec_nan_f, vec_normal_f);
    use_vector(cmp_ult);
    
    // LTGT vector comparison (not equal, same as UNEQ for unordered)
    __m128 cmp_une = _mm_cmpneq_ps(vec_nan_f, vec_zero_f);
    use_vector(cmp_une);
    
    // Double precision SSE vector comparisons
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    __m128d vec_normal_d = _mm_set1_pd(normal_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_nan_d, vec_normal_d);
    use_vector_d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_normal_d, vec_normal_d);
    use_vector_d(cmp_ord_d);
    
    __m128d cmp_uneq_d = _mm_cmpneq_pd(vec_nan_d, vec_normal_d);
    use_vector_d(cmp_uneq_d);
    
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(vec_nan_d, vec_normal_d);
    use_vector_d(cmp_nlt_d);
    
    __m128d cmp_nle_d = _mm_cmpnle_pd(vec_nan_d, vec_normal_d);
    use_vector_d(cmp_nle_d);
    
    __m128d cmp_ule_d = _mm_cmpule_pd(vec_nan_d, vec_normal_d);
    use_vector_d(cmp_ule_d);
    
    __m128d cmp_ult_d = _mm_cmpult_pd(vec_nan_d, vec_normal_d);
    use_vector_d(cmp_ult_d);
    
    __m128d cmp_une_d = _mm_cmpneq_pd(vec_nan_d, vec_zero_d);
    use_vector_d(cmp_une_d);
    
#ifdef __AVX__
    // AVX vector comparisons (256-bit)
    __m256 vec_nan_f_256 = _mm256_set1_ps(nan_f);
    __m256 vec_normal_f_256 = _mm256_set1_ps(normal_f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_UNORD_Q);
    use_vector_256(cmp_unord_256);
    
    __m256 cmp_ord_256 = _mm256_cmp_ps(vec_normal_f_256, vec_normal_f_256, _CMP_ORD_Q);
    use_vector_256(cmp_ord_256);
    
    __m256 cmp_uneq_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_NEQ_UQ);
    use_vector_256(cmp_uneq_256);
    
    __m256 cmp_nlt_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_NLT_UQ);
    use_vector_256(cmp_nlt_256);
    
    __m256 cmp_nle_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_NLE_UQ);
    use_vector_256(cmp_nle_256);
    
    __m256 cmp_ule_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_LE_UQ);
    use_vector_256(cmp_ule_256);
    
    __m256 cmp_ult_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_LT_UQ);
    use_vector_256(cmp_ult_256);
    
    __m256 cmp_une_256 = _mm256_cmp_ps(vec_nan_f_256, vec_zero_f, _CMP_NEQ_UQ);
    use_vector_256(cmp_une_256);
    
    // AVX double precision
    __m256d vec_nan_d_256 = _mm256_set1_pd(nan_d);
    __m256d vec_normal_d_256 = _mm256_set1_pd(normal_d);
    
    __m256d cmp_unord_d_256 = _mm256_cmp_pd(vec_nan_d_256, vec_normal_d_256, _CMP_UNORD_Q);
    use_vector_256_d(cmp_unord_d_256);
    
    __m256d cmp_ord_d_256 = _mm256_cmp_pd(vec_normal_d_256, vec_normal_d_256, _CMP_ORD_Q);
    use_vector_256_d(cmp_ord_d_256);
    
    __m256d cmp_uneq_d_256 = _mm256_cmp_pd(vec_nan_d_256, vec_normal_d_256, _CMP_NEQ_UQ);
    use_vector_256_d(cmp_uneq_d_256);
    
    __m256d cmp_nlt_d_256 = _mm256_cmp_pd(vec_nan_d_256, vec_normal_d_256, _CMP_NLT_UQ);
    use_vector_256_d(cmp_nlt_d_256);
    
    __m256d cmp_nle_d_256 = _mm256_cmp_pd(vec_nan_d_256, vec_normal_d_256, _CMP_NLE_UQ);
    use_vector_256_d(cmp_nle_d_256);
    
    __m256d cmp_ule_d_256 = _mm256_cmp_pd(vec_nan_d_256, vec_normal_d_256, _CMP_LE_UQ);
    use_vector_256_d(cmp_ule_d_256);
    
    __m256d cmp_ult_d_256 = _mm256_cmp_pd(vec_nan_d_256, vec_normal_d_256, _CMP_LT_UQ);
    use_vector_256_d(cmp_ult_d_256);
    
    __m256d cmp_une_d_256 = _mm256_cmp_pd(vec_nan_d_256, _mm256_set1_pd(zero_d), _CMP_NEQ_UQ);
    use_vector_256_d(cmp_une_d_256);
#endif
    
    // Loop to prevent optimization and generate more comparison patterns
    for (int i = 0; i < 10; i++) {
        float dynamic_nan = normal_f * (i - 5);
        if (__builtin_isunordered(dynamic_nan, (float)i)) {
            result += i;
        }
        
        // Generate various comparison conditions in loop
        if (dynamic_nan != dynamic_nan) result += i * 2;
        if (dynamic_nan < i) result += i * 3;
        if (dynamic_nan <= i) result += i * 4;
        if (dynamic_nan > i) result += i * 5;
        if (dynamic_nan >= i) result += i * 6;
        if (__builtin_islessgreater(dynamic_nan, (float)i)) result += i * 7;
    }
    
    // Use ternary operators to generate conditional moves
    int r1 = (nan_f != nan_f) ? 100 : 200;
    int r2 = (__builtin_isunordered(nan_f, normal_f)) ? 300 : 400;
    int r3 = (__builtin_islessgreater(nan_f, normal_f)) ? 500 : 600;
    int r4 = (nan_f < normal_f) ? 700 : 800;
    int r5 = (nan_f <= normal_f) ? 900 : 1000;
    int r6 = (nan_f > normal_f) ? 1100 : 1200;
    int r7 = (nan_f >= normal_f) ? 1300 : 1400;
    
    result += r1 + r2 + r3 + r4 + r5 + r6 + r7;
    
    printf("Result: %d (sink: %d)\n", result, sink);
    return result != 0 ? 0 : 1;
}
