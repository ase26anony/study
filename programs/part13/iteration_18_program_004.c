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
    int result = 0;
    
    // Create NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    // Regular float/double variables
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 2.71;
    
    // ============================================
    // Test UNORDERED condition (__builtin_isunordered)
    // ============================================
    if (__builtin_isunordered(f1, nan_f)) {
        result |= 1;  // Should be true
    }
    
    if (__builtin_isunordered(nan_d, d1)) {
        result |= 2;  // Should be true
    }
    
    if (__builtin_isunordered(f1, f2)) {
        result |= 4;  // Should be false
    }
    
    // ============================================
    // Test ORDERED condition (negation of unordered)
    // ============================================
    if (!__builtin_isunordered(f1, f2)) {
        result |= 8;  // Should be true
    }
    
    if (!__builtin_isunordered(nan_f, f2)) {
        result |= 16; // Should be false
    }
    
    // ============================================
    // Test UNEQ condition (x != x or x == NaN)
    // ============================================
    if (nan_f != nan_f) {  // Always true for NaN
        result |= 32;
    }
    
    if (f1 != f1) {  // False for normal numbers
        result |= 64;
    }
    
    // ============================================
    // Test UNLT, UNLE, UNGT, UNGE conditions
    // Using comparison operators with NaN operands
    // ============================================
    // UNLT: unordered or less than
    if (nan_f < f1) {  // False when unordered
        result |= 128;
    }
    
    // UNLE: unordered or less than or equal
    if (nan_f <= f1) {  // False when unordered
        result |= 256;
    }
    
    // UNGT: unordered or greater than
    if (nan_f > f1) {  // False when unordered
        result |= 512;
    }
    
    // UNGE: unordered or greater than or equal
    if (nan_f >= f1) {  // False when unordered
        result |= 1024;
    }
    
    // ============================================
    // Test LTGT condition (__builtin_islessgreater)
    // ============================================
    if (__builtin_islessgreater(f1, f2)) {
        result |= 2048;  // True: 1.5 < 2.5
    }
    
    if (__builtin_islessgreater(nan_f, f2)) {
        result |= 4096;  // False with NaN
    }
    
    // ============================================
    // SSE Intrinsics for explicit condition codes
    // ============================================
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128 vec_nan = _mm_set1_ps(nan_f);
    
    // UNORDERED: _mm_cmpunord_ps
    __m128 cmp_unord = _mm_cmpunord_ps(vec_f1, vec_f2);
    use_vector_result(cmp_unord);
    
    // ORDERED: _mm_cmpord_ps
    __m128 cmp_ord = _mm_cmpord_ps(vec_f1, vec_f2);
    use_vector_result(cmp_ord);
    
    // UNEQ: _mm_cmpneq_ps (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(vec_f1, vec_f2);
    use_vector_result(cmp_neq);
    
    // UNGE: _mm_cmpnlt_ps (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_f1, vec_f2);
    use_vector_result(cmp_nlt);
    
    // UNGT: _mm_cmpnle_ps (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_f1, vec_f2);
    use_vector_result(cmp_nle);
    
    // UNLE: Using combination (note: SSE doesn't have direct UNLE intrinsic)
    // We'll use ordered less than or equal and combine with unordered
    __m128 cmp_ole = _mm_cmple_ps(vec_f1, vec_f2);
    __m128 cmp_unord2 = _mm_cmpunord_ps(vec_f1, vec_f2);
    __m128 cmp_ule = _mm_or_ps(cmp_ole, cmp_unord2);
    use_vector_result(cmp_ule);
    
    // UNLT: Using combination
    __m128 cmp_olt = _mm_cmplt_ps(vec_f1, vec_f2);
    __m128 cmp_unord3 = _mm_cmpunord_ps(vec_f1, vec_f2);
    __m128 cmp_ult = _mm_or_ps(cmp_olt, cmp_unord3);
    use_vector_result(cmp_ult);
    
    // ============================================
    // Double precision SSE comparisons
    // ============================================
    __m128d vec_d1 = _mm_set_pd(nan_d, 1.0);
    __m128d vec_d2 = _mm_set_pd(2.0, nan_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_ord_d);
    
    __m128d cmp_neq_d = _mm_cmpneq_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_neq_d);
    
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_nlt_d);
    
    __m128d cmp_nle_d = _mm_cmpnle_pd(vec_d1, vec_d2);
    use_vector_result_d(cmp_nle_d);
    
    // ============================================
    // AVX comparisons (if available)
    // ============================================
#ifdef __AVX__
    __m256 vec_f1_avx = _mm256_set_ps(1.0f, nan_f, 3.0f, 4.0f, 
                                      5.0f, 6.0f, nan_f, 8.0f);
    __m256 vec_f2_avx = _mm256_set_ps(1.0f, 2.0f, nan_f, nan_f,
                                      5.0f, 7.0f, 7.0f, 8.0f);
    
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_UNORD_Q);
    use_vector_result_avx(cmp_unord_avx);
    
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_ORD_Q);
    use_vector_result_avx(cmp_ord_avx);
    
    __m256 cmp_neq_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_NEQ_UQ);
    use_vector_result_avx(cmp_neq_avx);
    
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_NLT_UQ);
    use_vector_result_avx(cmp_nlt_avx);
    
    __m256 cmp_nle_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_NLE_UQ);
    use_vector_result_avx(cmp_nle_avx);
    
    __m256 cmp_ule_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_LE_UQ);
    use_vector_result_avx(cmp_ule_avx);
    
    __m256 cmp_ult_avx = _mm256_cmp_ps(vec_f1_avx, vec_f2_avx, _CMP_LT_UQ);
    use_vector_result_avx(cmp_ult_avx);
    
    // Double precision AVX
    __m256d vec_d1_avx = _mm256_set_pd(nan_d, 2.0, nan_d, 4.0);
    __m256d vec_d2_avx = _mm256_set_pd(1.0, nan_d, 3.0, nan_d);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_UNORD_Q);
    use_vector_result_avx_d(cmp_unord_avx_d);
    
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec_d1_avx, vec_d2_avx, _CMP_ORD_Q);
    use_vector_result_avx_d(cmp_ord_avx_d);
#endif
    
    // ============================================
    // Loop with varying values to prevent optimization
    // ============================================
    float values[] = {1.0f, 2.0f, nan_f, 4.0f, 5.0f};
    double values_d[] = {1.0, nan_d, 3.0, 4.0, nan_d};
    
    for (int i = 0; i < 5; i++) {
        // Generate various condition codes in a loop
        if (__builtin_isunordered(values[i], values[(i+1)%5])) {
            result += i;
        }
        
        if (values[i] != values[i]) {  // UNEQ when NaN
            result += i * 2;
        }
        
        if (__builtin_islessgreater(values_d[i], values_d[(i+2)%5])) {
            result += i * 3;
        }
        
        // Generate UNLT, UNLE, etc. with potential NaN
        if (values[i] < values[(i+3)%5]) {  // May be UNLT if NaN
            result += i * 4;
        }
    }
    
    printf("Result: %d (sink: %d)\n", result, sink);
    return result != 0 ? 0 : 1;
}
