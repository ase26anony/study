#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent aggressive optimization
volatile int sink = 0;

// Function to use results and prevent dead code elimination
void use_result(int val) {
    sink += val;
}

void use_vector_result(__m128 v) {
    float f[4];
    _mm_storeu_ps(f, v);
    sink += (int)f[0];
}

void use_vector_result_d(__m128d v) {
    double d[2];
    _mm_storeu_pd(d, v);
    sink += (int)d[0];
}

#ifdef __AVX__
void use_vector_result_avx(__m256 v) {
    float f[8];
    _mm256_storeu_ps(f, v);
    sink += (int)f[0];
}

void use_vector_result_avx_d(__m256d v) {
    double d[4];
    _mm256_storeu_pd(d, v);
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
    double d1 = 3.14, d2 = 2.71;
    
    int result = 0;
    
    // 1. UNORDERED condition - using __builtin_isunordered
    if (__builtin_isunordered(nan_f, f1)) {
        result |= 1;  // Should be true
    }
    
    if (__builtin_isunordered(f1, f2)) {
        result |= 2;  // Should be false
    }
    
    // 2. ORDERED condition - using __builtin_isordered
    if (__builtin_isordered(f1, f2)) {
        result |= 4;  // Should be true
    }
    
    if (__builtin_isordered(nan_f, f1)) {
        result |= 8;  // Should be false
    }
    
    // 3. UNEQ condition - equality with NaN
    if (nan_f != nan_f) {  // NaN != NaN is true (unordered equal)
        result |= 16;
    }
    
    if (__builtin_isunordered(f1, f2) && f1 == f2) {  // UNEQ logic
        result |= 32;  // Should be false
    }
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < f1) {  // Unordered less than
        result |= 64;  // Should be false (but generates UNLT condition)
    }
    
    // 5. UNLE condition - less or equal with NaN operand
    if (nan_f <= f1) {  // Unordered less or equal
        result |= 128;  // Should be false
    }
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > f1) {  // Unordered greater than
        result |= 256;  // Should be false
    }
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= f1) {  // Unordered greater or equal
        result |= 512;  // Should be false
    }
    
    // 8. LTGT condition - using __builtin_islessgreater
    if (__builtin_islessgreater(f1, f2)) {  // f1 < f2 or f1 > f2 (but not equal and not unordered)
        result |= 1024;  // Should be true
    }
    
    if (__builtin_islessgreater(nan_f, f1)) {
        result |= 2048;  // Should be false
    }
    
    // SSE vector comparisons (128-bit)
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128 vec_f3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    __m128d vec_d1 = _mm_set_pd(nan_d, 1.0);
    __m128d vec_d2 = _mm_set_pd(2.0, nan_d);
    __m128d vec_d3 = _mm_set_pd(1.0, 2.0);
    
    // UNORDERED vector comparisons
    __m128 cmp_unord = _mm_cmpunord_ps(vec_f1, vec_f2);
    use_vector_result(cmp_unord);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_unord_d);
    
    // ORDERED vector comparisons
    __m128 cmp_ord = _mm_cmpord_ps(vec_f1, vec_f2);
    use_vector_result(cmp_ord);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_ord_d);
    
    // UNEQ vector comparisons (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(vec_f1, vec_f2);
    use_vector_result(cmp_neq);
    
    __m128d cmp_neq_d = _mm_cmpneq_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_neq_d);
    
    // UNGE vector comparisons (not less than = greater or equal or unordered)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_f1, vec_f2);
    use_vector_result(cmp_nlt);
    
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_nlt_d);
    
    // UNGT vector comparisons (not less or equal = greater or unordered)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_f1, vec_f2);
    use_vector_result(cmp_nle);
    
    __m128d cmp_nle_d = _mm_cmpnle_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_nle_d);
    
    // UNLE vector comparisons (unordered or less or equal)
    // Note: SSE doesn't have direct UNLE intrinsic, but we can simulate
    __m128 cmp_ule = _mm_cmple_ps(vec_f1, vec_f2);  // This generates LE, not UNLE
    // To get UNLE, we need ordered comparison with NaN handling
    use_vector_result(cmp_ule);
    
    // UNLT vector comparisons (unordered or less than)
    __m128 cmp_ult = _mm_cmplt_ps(vec_f1, vec_f2);  // This generates LT, not UNLT
    use_vector_result(cmp_ult);
    
#ifdef __AVX__
    // AVX vector comparisons (256-bit)
    __m256 vec_f1_avx = _mm256_set_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec_f2_avx = _mm256_set_ps(1.0f, 3.0f, 3.0f, nan_f, 5.0f, 6.0f, 7.0f, 8.0f);
    
    __m256d vec_d1_avx = _mm256_set_pd(nan_d, 1.0, 2.0, 3.0);
    __m256d vec_d2_avx = _mm256_set_pd(2.0, nan_d, 3.0, 4.0);
    
    // AVX UNORDERED
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_UNORD_Q);
    use_vector_result_avx(cmp_unord_avx);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_UNORD_Q);
    use_vector_result_avx_d(cmp_unord_avx_d);
    
    // AVX ORDERED
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_ORD_Q);
    use_vector_result_avx(cmp_ord_avx);
    
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_ORD_Q);
    use_vector_result_avx_d(cmp_ord_avx_d);
    
    // AVX UNEQ
    __m256 cmp_neq_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_NEQ_UQ);
    use_vector_result_avx(cmp_neq_avx);
    
    __m256d cmp_neq_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_NEQ_UQ);
    use_vector_result_avx_d(cmp_neq_avx_d);
    
    // AVX UNGE (not less than)
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_NLT_UQ);
    use_vector_result_avx(cmp_nlt_avx);
    
    __m256d cmp_nlt_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_NLT_UQ);
    use_vector_result_avx_d(cmp_nlt_avx_d);
    
    // AVX UNGT (not less or equal)
    __m256 cmp_nle_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_NLE_UQ);
    use_vector_result_avx(cmp_nle_avx);
    
    __m256d cmp_nle_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_NLE_UQ);
    use_vector_result_avx_d(cmp_nle_avx_d);
    
    // AVX UNLE (unordered or less or equal)
    __m256 cmp_ule_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_LE_OS);
    use_vector_result_avx(cmp_ule_avx);
    
    // AVX UNLT (unordered or less than)
    __m256 cmp_ult_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_LT_OS);
    use_vector_result_avx(cmp_ult_avx);
    
    // AVX LTGT (less or greater, ordered)
    __m256 cmp_ltgt_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_NEQ_OQ);
    use_vector_result_avx(cmp_ltgt_avx);
#endif
    
    // Loop to prevent optimization and generate more comparison patterns
    for (int i = 0; i < 10; i++) {
        float dynamic_f = (i % 2 == 0) ? nan_f : (float)i;
        double dynamic_d = (i % 3 == 0) ? nan_d : (double)i;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(dynamic_f, f1)) {
            result++;
        }
        
        if (dynamic_d != dynamic_d) {  // UNEQ
            result++;
        }
        
        if (__builtin_islessgreater(dynamic_f, f1)) {  // LTGT
            result++;
        }
        
        // Ordered comparison with potential NaN
        if (dynamic_f < f1) {  // Could be UNLT if dynamic_f is NaN
            result++;
        }
    }
    
    printf("Result: %d (sink: %d)\n", result, sink);
    return result != 0 ? 0 : 1;
}
