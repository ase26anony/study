#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent optimization
volatile int sink;

// Function to force conditional jumps
__attribute__((noinline)) 
void use_condition(int cond) {
    sink = cond;
}

int main() {
    // Initialize NaN values
    float nan_f = NAN;
    double nan_d = NAN;
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    int result = 0;
    
    // ============================================
    // 1. Scalar unordered comparisons using builtins
    // ============================================
    
    // UNORDERED - __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1 << 0;
    }
    
    // ORDERED - __builtin_isordered
    if (__builtin_isordered(normal_f, normal_d)) {
        result |= 1 << 1;
    }
    
    // UNEQ - (x != x) or (x == NaN)
    if (nan_f != nan_f) {  // Always true for NaN
        result |= 1 << 2;
    }
    
    // UNLT - __builtin_isless with NaN operand
    if (__builtin_isless(nan_f, normal_f)) {
        result |= 1 << 3;
    }
    
    // UNLE - __builtin_islessequal with NaN operand  
    if (__builtin_islessequal(nan_f, normal_f)) {
        result |= 1 << 4;
    }
    
    // UNGT - __builtin_isgreater with NaN operand
    if (__builtin_isgreater(nan_f, normal_f)) {
        result |= 1 << 5;
    }
    
    // UNGE - __builtin_isgreaterequal with NaN operand
    if (__builtin_isgreaterequal(nan_f, normal_f)) {
        result |= 1 << 6;
    }
    
    // LTGT - __builtin_islessgreater
    if (__builtin_islessgreater(normal_f, normal_d)) {
        result |= 1 << 7;
    }
    
    // ============================================
    // 2. SSE vector comparisons (128-bit)
    // ============================================
    
    __m128 vec_nan_f = _mm_set1_ps(NAN);
    __m128 vec_normal_f = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128i cmp_result;
    
    // UNORDERED - _mm_cmpunord_ps
    cmp_result = _mm_castps_si128(_mm_cmpunord_ps(vec_nan_f, vec_normal_f));
    result += _mm_extract_epi32(cmp_result, 0) & 1;
    
    // ORDERED - _mm_cmpord_ps
    cmp_result = _mm_castps_si128(_mm_cmpord_ps(vec_normal_f, vec_normal_f));
    result += _mm_extract_epi32(cmp_result, 1) & 1;
    
    // UNEQ - _mm_cmpneq_ps (unordered equal)
    cmp_result = _mm_castps_si128(_mm_cmpneq_ps(vec_nan_f, vec_nan_f));
    result += _mm_extract_epi32(cmp_result, 2) & 1;
    
    // UNGE - _mm_cmpnlt_ps (not less than)
    cmp_result = _mm_castps_si128(_mm_cmpnlt_ps(vec_nan_f, vec_normal_f));
    result += _mm_extract_epi32(cmp_result, 3) & 1;
    
    // UNGT - _mm_cmpnle_ps (not less or equal)
    cmp_result = _mm_castps_si128(_mm_cmpnle_ps(vec_nan_f, vec_normal_f));
    result += _mm_extract_epi32(cmp_result, 0) & 1;
    
    // UNLE - _mm_cmpule_ps (unordered less or equal)
    // Note: GCC may generate different patterns for this
    __m128 vec_small_f = _mm_set1_ps(0.5f);
    cmp_result = _mm_castps_si128(_mm_cmple_ps(vec_nan_f, vec_small_f));
    result += _mm_extract_epi32(cmp_result, 1) & 1;
    
    // UNLT - _mm_cmpult_ps (unordered less than)
    cmp_result = _mm_castps_si128(_mm_cmplt_ps(vec_nan_f, vec_normal_f));
    result += _mm_extract_epi32(cmp_result, 2) & 1;
    
    // ============================================
    // 3. SSE double precision comparisons
    // ============================================
    
    __m128d vec_nan_d = _mm_set1_pd(NAN);
    __m128d vec_normal_d = _mm_setr_pd(1.0, 2.0);
    
    // UNORDERED - _mm_cmpunord_pd
    __m128i cmp_result_d = _mm_castpd_si128(_mm_cmpunord_pd(vec_nan_d, vec_normal_d));
    result += _mm_extract_epi32(cmp_result_d, 0) & 1;
    
    // ORDERED - _mm_cmpord_pd
    cmp_result_d = _mm_castpd_si128(_mm_cmpord_pd(vec_normal_d, vec_normal_d));
    result += _mm_extract_epi32(cmp_result_d, 1) & 1;
    
    // LTGT - _mm_cmpneq_pd (not equal)
    cmp_result_d = _mm_castpd_si128(_mm_cmpneq_pd(vec_normal_d, _mm_set1_pd(3.0)));
    result += _mm_extract_epi32(cmp_result_d, 0) & 1;
    
    // ============================================
    // 4. AVX vector comparisons (256-bit) if available
    // ============================================
    
    #ifdef __AVX__
    __m256 vec_nan_f_256 = _mm256_set1_ps(NAN);
    __m256 vec_normal_f_256 = _mm256_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    
    // UNORDERED - _mm256_cmpunord_ps
    __m256 cmp_256 = _mm256_cmpunord_ps(vec_nan_f_256, vec_normal_f_256);
    float* cmp_ptr = (float*)&cmp_256;
    result += (cmp_ptr[0] != 0.0f) ? 1 : 0;
    
    // ORDERED - _mm256_cmpord_ps
    cmp_256 = _mm256_cmpord_ps(vec_normal_f_256, vec_normal_f_256);
    cmp_ptr = (float*)&cmp_256;
    result += (cmp_ptr[1] != 0.0f) ? 1 : 0;
    
    // UNGE - _mm256_cmpnlt_ps
    cmp_256 = _mm256_cmpnlt_ps(vec_nan_f_256, vec_normal_f_256);
    cmp_ptr = (float*)&cmp_256;
    result += (cmp_ptr[2] != 0.0f) ? 1 : 0;
    #endif
    
    // ============================================
    // 5. Mixed operations in loops to prevent dead code elimination
    // ============================================
    
    float f1 = 1.0f;
    float f2 = 2.0f;
    double d1 = 1.0;
    double d2 = 2.0;
    
    for (int i = 0; i < 10; i++) {
        // Vary the values slightly
        f1 += 0.1f * i;
        f2 -= 0.05f * i;
        d1 += 0.2 * i;
        d2 -= 0.1 * i;
        
        // Generate various condition codes
        if (__builtin_isunordered(f1, f2)) {
            result++;
        }
        
        if (__builtin_islessgreater(d1, d2)) {
            result++;
        }
        
        // Create NaN through arithmetic
        float maybe_nan = (i == 5) ? 0.0f / 0.0f : f1;
        if (maybe_nan != maybe_nan) {  // UNEQ
            result++;
        }
        
        // Force conditional jumps
        use_condition(__builtin_isless(f1, f2));
        use_condition(__builtin_isgreater(d1, d2));
    }
    
    printf("Result: %d\n", result);
    return result != 0;
}
