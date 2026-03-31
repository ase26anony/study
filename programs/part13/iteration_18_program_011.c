#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>
#include <string.h>

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
__attribute__((noinline)) void use_vector256(__m256 v) {
    volatile __m256 sink = v;
    (void)sink;
}

__attribute__((noinline)) void use_vector256_d(__m256d v) {
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
    float normal_f = 3.14f;
    double normal_d = 2.71828;
    
    // Test UNORDERED condition (line 13992-13994)
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // UNORDERED
    }
    
    // Test ORDERED condition (line 13995-13997)
    if (!__builtin_isunordered(normal_f, normal_f)) {
        result |= 2;  // ORDERED
    }
    
    // Test UNEQ condition (line 13998-14000)
    // Using direct NaN comparison
    if (nan_f != nan_f) {  // This is always false, but generates UNEQ
        // This branch won't be taken, but the comparison is generated
        result |= 4;
    }
    
    // Test UNGE condition (line 14001-14003)
    // Compare NaN with normal value using >=
    if (nan_f >= normal_f) {  // Generates UNGE (nlt)
        result |= 8;
    }
    
    // Test UNGT condition (line 14004-14006)
    // Compare NaN with normal value using >
    if (nan_f > normal_f) {  // Generates UNGT (nle)
        result |= 16;
    }
    
    // Test UNLE condition (line 14007-14009)
    // Compare NaN with normal value using <=
    if (nan_f <= normal_f) {  // Generates UNLE (ule)
        result |= 32;
    }
    
    // Test UNLT condition (line 14010-14012)
    // Compare NaN with normal value using <
    if (nan_f < normal_f) {  // Generates UNLT (ult)
        result |= 64;
    }
    
    // Test LTGT condition (line 14013-14015)
    if (__builtin_islessgreater(nan_f, normal_f)) {
        result |= 128;  // LTGT
    }
    
    // Test with double precision
    if (__builtin_isunordered(nan_d, normal_d)) {
        result |= 256;
    }
    
    // SSE intrinsics for explicit condition codes
    __m128 a = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_setr_ps(4.0f, 1.0f, 3.0f, nan_f);
    
    // UNORDERED
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    use_vector(cmp_unord);
    
    // ORDERED
    __m128 cmp_ord = _mm_cmpord_ps(a, b);
    use_vector(cmp_ord);
    
    // UNEQ (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(a, b);
    use_vector(cmp_neq);
    
    // UNGE (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
    use_vector(cmp_nlt);
    
    // UNGT (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);
    use_vector(cmp_nle);
    
    // UNLE (unordered or less than or equal)
    // Note: There's no direct UNLE intrinsic, but we can combine
    __m128 cmp_ule = _mm_cmple_ps(a, b);  // This generates LE, not UNLE
    // For true UNLE, we need unordered comparison
    __m128 unord_mask = _mm_cmpunord_ps(a, b);
    __m128 le_mask = _mm_cmple_ps(a, b);
    __m128 cmp_unle = _mm_or_ps(unord_mask, le_mask);
    use_vector(cmp_unle);
    
    // UNLT (unordered or less than)
    __m128 ult_mask = _mm_cmplt_ps(a, b);
    __m128 cmp_ult = _mm_or_ps(unord_mask, ult_mask);
    use_vector(cmp_ult);
    
    // Double precision SSE
    __m128d ad = _mm_setr_pd(nan_d, 1.0);
    __m128d bd = _mm_setr_pd(2.0, nan_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(ad, bd);
    use_vector_d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(ad, bd);
    use_vector_d(cmp_ord_d);
    
    __m128d cmp_neq_d = _mm_cmpneq_pd(ad, bd);
    use_vector_d(cmp_neq_d);
    
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(ad, bd);
    use_vector_d(cmp_nlt_d);
    
    __m128d cmp_nle_d = _mm_cmpnle_pd(ad, bd);
    use_vector_d(cmp_nle_d);
    
#ifdef __AVX__
    // AVX 256-bit vectors
    __m256 a256 = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 b256 = _mm256_setr_ps(nan_f, 2.0f, 3.0f, nan_f, 5.0f, nan_f, 7.0f, 8.0f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(a256, b256, _CMP_UNORD_Q);
    use_vector256(cmp_unord_256);
    
    __m256 cmp_ord_256 = _mm256_cmp_ps(a256, b256, _CMP_ORD_Q);
    use_vector256(cmp_ord_256);
    
    __m256 cmp_neq_256 = _mm256_cmp_ps(a256, b256, _CMP_NEQ_UQ);
    use_vector256(cmp_neq_256);
    
    __m256 cmp_nlt_256 = _mm256_cmp_ps(a256, b256, _CMP_NLT_UQ);
    use_vector256(cmp_nlt_256);
    
    __m256 cmp_nle_256 = _mm256_cmp_ps(a256, b256, _CMP_NLE_UQ);
    use_vector256(cmp_nle_256);
    
    __m256 cmp_ule_256 = _mm256_cmp_ps(a256, b256, _CMP_LE_UQ);
    use_vector256(cmp_ule_256);
    
    __m256 cmp_ult_256 = _mm256_cmp_ps(a256, b256, _CMP_LT_UQ);
    use_vector256(cmp_ult_256);
    
    // Double precision AVX
    __m256d a256d = _mm256_setr_pd(nan_d, 2.0, 3.0, nan_d);
    __m256d b256d = _mm256_setr_pd(1.0, nan_d, nan_d, 4.0);
    
    __m256d cmp_unord_256d = _mm256_cmp_pd(a256d, b256d, _CMP_UNORD_Q);
    use_vector256_d(cmp_unord_256d);
    
    __m256d cmp_ord_256d = _mm256_cmp_pd(a256d, b256d, _CMP_ORD_Q);
    use_vector256_d(cmp_ord_256d);
#endif
    
    // Loop to generate more comparison patterns
    volatile float x = 0.0f;
    for (int i = 0; i < 10; i++) {
        x += 1.0f;
        float temp = (i % 2 == 0) ? nan_f : x;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(temp, x)) {
            result += i;
        }
        
        // Generate LTGT in loop
        if (__builtin_islessgreater(temp, x)) {
            result -= i;
        }
        
        // Generate ordered comparison that becomes unordered with NaN
        if (temp < x) {  // Generates UNLT when temp is NaN
            result ^= i;
        }
    }
    
    // Use ternary operator to force condition code generation
    int r1 = (nan_f != nan_f) ? 100 : 200;  // UNEQ
    int r2 = (nan_f >= normal_f) ? 300 : 400;  // UNGE
    int r3 = (nan_f > normal_f) ? 500 : 600;  // UNGT
    int r4 = (nan_f <= normal_f) ? 700 : 800;  // UNLE
    int r5 = (nan_f < normal_f) ? 900 : 1000;  // UNLT
    
    result += r1 + r2 + r3 + r4 + r5;
    
    printf("Result: %d\n", result);
    return result != 0;
}
