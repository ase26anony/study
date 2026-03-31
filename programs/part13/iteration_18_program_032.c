#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent optimization
volatile int sink = 0;

// Function to use results and prevent dead code elimination
void use_result(int val) {
    sink += val;
}

void use_vector(__m128 v) {
    float f[4];
    _mm_storeu_ps(f, v);
    sink += (int)f[0];
}

void use_vector_d(__m128d v) {
    double d[2];
    _mm_storeu_pd(d, v);
    sink += (int)d[0];
}

void use_vector_avx(__m256 v) {
    float f[8];
    _mm256_storeu_ps(f, v);
    sink += (int)f[0];
}

void use_vector_avx_d(__m256d v) {
    double d[4];
    _mm256_storeu_pd(d, v);
    sink += (int)d[0];
}

int main() {
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    float f1 = 1.0f, f2 = 2.0f;
    double d1 = 1.0, d2 = 2.0;
    
    int result = 0;
    
    // 1. UNORDERED condition - using __builtin_isunordered
    if (__builtin_isunordered(nan_f, f1)) {
        result |= 1;  // Should always be true
    }
    
    if (__builtin_isunordered(d1, nan_d)) {
        result |= 2;  // Should always be true
    }
    
    // 2. ORDERED condition - using ordered comparison with non-NaN values
    if (!__builtin_isunordered(f1, f2)) {
        result |= 4;  // Should be true for ordered values
    }
    
    // 3. UNEQ condition (unordered or equal)
    // Using direct comparison with NaN
    if (nan_f != nan_f) {  // NaN != NaN is true (unordered)
        result |= 8;
    }
    
    // Using __builtin_islessequal with NaN
    if (!__builtin_islessequal(f1, nan_f)) {
        result |= 16;  // UNEQ when comparing with NaN
    }
    
    // 4. UNGE condition (unordered or greater or equal)
    // Using >= with NaN operand
    if (!(f1 >= nan_f)) {  // This generates UNGE (nlt)
        result |= 32;
    }
    
    if (!(d1 >= nan_d)) {
        result |= 64;
    }
    
    // 5. UNGT condition (unordered or greater)
    // Using > with NaN operand
    if (!(f1 > nan_f)) {  // This generates UNGT (nle)
        result |= 128;
    }
    
    if (!(d1 > nan_d)) {
        result |= 256;
    }
    
    // 6. UNLE condition (unordered or less or equal)
    // Using <= with NaN operand
    if (nan_f <= f1) {  // This generates UNLE (ule)
        result |= 512;
    }
    
    // 7. UNLT condition (unordered or less)
    // Using < with NaN operand
    if (nan_f < f1) {  // This generates UNLT (ult)
        result |= 1024;
    }
    
    // 8. LTGT condition (less or greater, but not equal and not unordered)
    // Using __builtin_islessgreater
    if (__builtin_islessgreater(f1, f2)) {
        result |= 2048;  // f1 < f2, so true
    }
    
    if (__builtin_islessgreater(nan_f, f1)) {
        result |= 4096;  // false with NaN
    }
    
    // SSE intrinsics for explicit condition codes
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 vec_nan = _mm_set_ps(nan_f, nan_f, 1.0f, 2.0f);
    
    // UNORDERED
    __m128 cmp_unord = _mm_cmpunord_ps(vec_f1, vec_nan);
    use_vector(cmp_unord);
    
    // ORDERED
    __m128 cmp_ord = _mm_cmpord_ps(vec_f1, vec_f2);
    use_vector(cmp_ord);
    
    // UNEQ (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(vec_f1, vec_nan);
    use_vector(cmp_neq);
    
    // UNGE (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_f1, vec_nan);
    use_vector(cmp_nlt);
    
    // UNGT (not less or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_f1, vec_nan);
    use_vector(cmp_nle);
    
    // UNLE (unordered or less or equal) - Note: _mm_cmple_ps generates le, not ule
    // We need to create UNLE condition manually
    __m128 cmp_unord2 = _mm_cmpunord_ps(vec_f1, vec_nan);
    __m128 cmp_le = _mm_cmple_ps(vec_f1, vec_nan);
    __m128 cmp_ule = _mm_or_ps(cmp_unord2, cmp_le);
    use_vector(cmp_ule);
    
    // UNLT (unordered or less than)
    __m128 cmp_lt = _mm_cmplt_ps(vec_f1, vec_nan);
    __m128 cmp_ult = _mm_or_ps(cmp_unord2, cmp_lt);
    use_vector(cmp_ult);
    
    // Double precision SSE
    __m128d vec_d1 = _mm_set_pd(1.0, 2.0);
    __m128d vec_d2 = _mm_set_pd(2.0, 1.0);
    __m128d vec_nan_d = _mm_set_pd(nan_d, 1.0);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_d1, vec_nan_d);
    use_vector_d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_d1, vec_d2);
    use_vector_d(cmp_ord_d);
    
    __m128d cmp_neq_d = _mm_cmpneq_pd(vec_d1, vec_nan_d);
    use_vector_d(cmp_neq_d);
    
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(vec_d1, vec_nan_d);
    use_vector_d(cmp_nlt_d);
    
    __m128d cmp_nle_d = _mm_cmpnle_pd(vec_d1, vec_nan_d);
    use_vector_d(cmp_nle_d);
    
    // AVX intrinsics for 256-bit vectors
#ifdef __AVX__
    __m256 vec_f1_avx = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec_nan_avx = _mm256_set_ps(nan_f, 1.0f, nan_f, 2.0f, 3.0f, nan_f, 4.0f, 5.0f);
    
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_UNORD_Q);
    use_vector_avx(cmp_unord_avx);
    
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_ORD_Q);
    use_vector_avx(cmp_ord_avx);
    
    __m256 cmp_neq_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_NEQ_UQ);
    use_vector_avx(cmp_neq_avx);
    
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_NLT_UQ);
    use_vector_avx(cmp_nlt_avx);
    
    __m256 cmp_nle_avx = _mm256_cmp_ps(vec_f1_avx, vec_nan_avx, _CMP_NLE_UQ);
    use_vector_avx(cmp_nle_avx);
    
    // Double precision AVX
    __m256d vec_d1_avx = _mm256_set_pd(1.0, 2.0, 3.0, 4.0);
    __m256d vec_nan_d_avx = _mm256_set_pd(nan_d, 1.0, nan_d, 2.0);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_nan_d_avx, _CMP_UNORD_Q);
    use_vector_avx_d(cmp_unord_avx_d);
    
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_nan_d_avx, _CMP_ORD_Q);
    use_vector_avx_d(cmp_ord_avx_d);
#endif
    
    // Loop to generate more comparison patterns and prevent optimization
    for (int i = 0; i < 10; i++) {
        float dynamic_f = (i % 2 == 0) ? (float)i : nan_f;
        double dynamic_d = (i % 3 == 0) ? (double)i : nan_d;
        
        // Generate various unordered conditions in loop
        if (__builtin_isunordered(dynamic_f, f1)) {
            result += i;
        }
        
        if (!(dynamic_f >= f2)) {  // Could generate UNGE
            result += i * 2;
        }
        
        if (dynamic_d != dynamic_d) {  // UNEQ for NaN
            result += i * 3;
        }
        
        // LTGT condition with mixed values
        if (__builtin_islessgreater(dynamic_f, f1)) {
            result += i * 4;
        }
    }
    
    printf("Result: %d (sink: %d)\n", result, sink);
    return result == 0 ? 1 : 0;
}
