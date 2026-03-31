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

#ifdef __AVX__
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
#endif

int main() {
    // Initialize NaN values
    float nan_f = NAN;
    double nan_d = NAN;
    float inf_f = INFINITY;
    float normal_f = 3.14f;
    double normal_d = 2.71828;
    
    int result = 0;
    
    // 1. UNORDERED condition - using __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // Branch taken
    }
    
    // 2. ORDERED condition - using __builtin_isordered
    if (__builtin_isordered(normal_f, normal_d)) {
        result |= 2;  // Branch taken
    }
    
    // 3. UNEQ condition - x != x or comparison with NaN
    if (nan_f != nan_f) {  // Always true for NaN
        result |= 4;
    }
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < normal_f) {  // Unordered less than
        // This branch won't be taken, but generates UNLT condition
    } else {
        result |= 8;
    }
    
    // 5. UNLE condition - less or equal with NaN operand
    if (nan_f <= normal_f) {  // Unordered less or equal
        // This branch won't be taken
    } else {
        result |= 16;
    }
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > normal_f) {  // Unordered greater than
        // This branch won't be taken
    } else {
        result |= 32;
    }
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= normal_f) {  // Unordered greater or equal
        // This branch won't be taken
    } else {
        result |= 64;
    }
    
    // 8. LTGT condition - using __builtin_islessgreater
    if (__builtin_islessgreater(normal_f, normal_d)) {
        result |= 128;  // Branch taken (3.14 > 2.71828)
    }
    
    // SSE vector comparisons (128-bit)
    __m128 vec_a = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vec_b = _mm_setr_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 vec_nan = _mm_set1_ps(nan_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_a, vec_b);
    use_vector(cmp_unord);
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(vec_a, vec_b);
    use_vector(cmp_ord);
    
    // UNEQ vector comparison (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(vec_a, vec_b);
    use_vector(cmp_neq);
    
    // UNGE vector comparison (not less than = nlt)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_a, vec_b);
    use_vector(cmp_nlt);
    
    // UNGT vector comparison (not less or equal = nle)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_a, vec_b);
    use_vector(cmp_nle);
    
    // UNLE vector comparison (unordered less or equal = ule)
    __m128 cmp_ule = _mm_cmpule_ps(vec_nan, vec_b);
    use_vector(cmp_ule);
    
    // UNLT vector comparison (unordered less than = ult)
    __m128 cmp_ult = _mm_cmpult_ps(vec_nan, vec_b);
    use_vector(cmp_ult);
    
    // Double precision SSE comparisons
    __m128d vec_d_a = _mm_setr_pd(normal_d, nan_d);
    __m128d vec_d_b = _mm_setr_pd(1.0, 2.0);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_d_a, vec_d_b);
    use_vector_d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_d_a, vec_d_b);
    use_vector_d(cmp_ord_d);
    
    __m128d cmp_neq_d = _mm_cmpneq_pd(vec_d_a, vec_d_b);
    use_vector_d(cmp_neq_d);
    
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(vec_d_a, vec_d_b);
    use_vector_d(cmp_nlt_d);
    
    __m128d cmp_nle_d = _mm_cmpnle_pd(vec_d_a, vec_d_b);
    use_vector_d(cmp_nle_d);
    
    __m128d cmp_ule_d = _mm_cmpule_pd(_mm_set1_pd(nan_d), vec_d_b);
    use_vector_d(cmp_ule_d);
    
    __m128d cmp_ult_d = _mm_cmpult_pd(_mm_set1_pd(nan_d), vec_d_b);
    use_vector_d(cmp_ult_d);
    
#ifdef __AVX__
    // AVX vector comparisons (256-bit)
    __m256 vec_a_avx = _mm256_setr_ps(1.0f, nan_f, 3.0f, nan_f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec_b_avx = _mm256_setr_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_a_avx, vec_b_avx, _CMP_UNORD_Q);
    use_vector_avx(cmp_unord_avx);
    
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_a_avx, vec_b_avx, _CMP_ORD_Q);
    use_vector_avx(cmp_ord_avx);
    
    __m256 cmp_neq_avx = _mm256_cmp_ps(vec_a_avx, vec_b_avx, _CMP_NEQ_UQ);
    use_vector_avx(cmp_neq_avx);
    
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vec_a_avx, vec_b_avx, _CMP_NLT_UQ);
    use_vector_avx(cmp_nlt_avx);
    
    __m256 cmp_nle_avx = _mm256_cmp_ps(vec_a_avx, vec_b_avx, _CMP_NLE_UQ);
    use_vector_avx(cmp_nle_avx);
    
    __m256 cmp_ule_avx = _mm256_cmp_ps(_mm256_set1_ps(nan_f), vec_b_avx, _CMP_LE_OS);
    use_vector_avx(cmp_ule_avx);
    
    __m256 cmp_ult_avx = _mm256_cmp_ps(_mm256_set1_ps(nan_f), vec_b_avx, _CMP_LT_OS);
    use_vector_avx(cmp_ult_avx);
    
    // AVX double precision
    __m256d vec_d_a_avx = _mm256_setr_pd(normal_d, nan_d, 3.0, 4.0);
    __m256d vec_d_b_avx = _mm256_setr_pd(4.0, 3.0, 2.0, 1.0);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec_d_a_avx, vec_d_b_avx, _CMP_UNORD_Q);
    use_vector_avx_d(cmp_unord_avx_d);
    
    __m256d cmp_ord_avx_d = _mm256_cmp_pd(vec_d_a_avx, vec_d_b_avx, _CMP_ORD_Q);
    use_vector_avx_d(cmp_ord_avx_d);
#endif
    
    // Loop to generate more comparison patterns
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? normal_f : nan_f;
        float b = (i % 3 == 0) ? (float)i : nan_f;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(a, b)) {
            result++;
        }
        
        if (a != a) {  // UNEQ
            result--;
        }
        
        // Ternary operator generating condition codes
        int r1 = (a < b) ? 1 : 0;   // May generate UNLT
        int r2 = (a <= b) ? 1 : 0;  // May generate UNLE
        int r3 = (a > b) ? 1 : 0;   // May generate UNGT
        int r4 = (a >= b) ? 1 : 0;  // May generate UNGE
        
        use_result(r1 + r2 + r3 + r4);
    }
    
    printf("Result: %d (sink: %d)\n", result, sink);
    return result != 0 ? 0 : 1;
}
