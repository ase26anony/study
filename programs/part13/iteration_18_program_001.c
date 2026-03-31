#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>

// Prevent constant propagation and dead code elimination
volatile float vf1, vf2;
volatile double vd1, vd2;

// Helper to use values without optimization removing them
__attribute__((noinline)) 
void use_int(int val) {
    volatile static int sink;
    sink = val;
}

__attribute__((noinline))
void use_float(float val) {
    volatile static float sink;
    sink = val;
}

__attribute__((noinline))
void use_double(double val) {
    volatile static double sink;
    sink = val;
}

__attribute__((noinline))
void use_m128(__m128 val) {
    volatile static __m128 sink;
    sink = val;
}

__attribute__((noinline))
void use_m128d(__m128d val) {
    volatile static __m128d sink;
    sink = val;
}

__attribute__((noinline))
void use_m256(__m256 val) {
    volatile static __m256 sink;
    sink = val;
}

__attribute__((noinline))
void use_m256d(__m256d val) {
    volatile static __m256d sink;
    sink = val;
}

int main() {
    int result = 0;
    
    // Generate NaN values
    float nan_f = __builtin_nanf("");
    float inf_f = __builtin_inff();
    double nan_d = __builtin_nan("");
    double inf_d = __builtin_inf();
    
    // Regular float/double variables
    float f1 = 1.5f, f2 = 2.5f, f3 = nan_f, f4 = inf_f;
    double d1 = 1.5, d2 = 2.5, d3 = nan_d, d4 = inf_d;
    
    // ============================================
    // 1. UNORDERED condition (__builtin_isunordered)
    // ============================================
    if (__builtin_isunordered(f1, f3)) {
        result += 1;  // Should be true (f3 is NaN)
    }
    if (__builtin_isunordered(d2, d3)) {
        result += 2;  // Should be true (d3 is NaN)
    }
    
    // ============================================
    // 2. ORDERED condition (negation of unordered)
    // ============================================
    if (!__builtin_isunordered(f1, f2)) {
        result += 4;  // Should be true (both are numbers)
    }
    
    // ============================================
    // 3. UNEQ condition (unordered or equal)
    // ============================================
    // Using != with NaN (x != x is true for NaN)
    if (f3 != f3) {
        result += 8;  // Should be true
    }
    if (d3 != d3) {
        result += 16; // Should be true
    }
    
    // ============================================
    // 4. UNLT condition (unordered or less than)
    // ============================================
    if (f3 < f1) {  // NaN < number is false in ordered sense, but generates UNLT
        // This branch won't be taken, but the comparison generates UNLT
        result += 32;
    }
    
    // ============================================
    // 5. UNLE condition (unordered or less than or equal)
    // ============================================
    if (f3 <= f1) {  // NaN <= number
        result += 64;
    }
    
    // ============================================
    // 6. UNGT condition (unordered or greater than)
    // ============================================
    if (f3 > f1) {  // NaN > number
        result += 128;
    }
    
    // ============================================
    // 7. UNGE condition (unordered or greater than or equal)
    // ============================================
    if (f3 >= f1) {  // NaN >= number
        result += 256;
    }
    
    // ============================================
    // 8. LTGT condition (less than or greater than, but not equal)
    // ============================================
    if (__builtin_islessgreater(f1, f2)) {  // 1.5 < 2.5 is true
        result += 512;
    }
    if (__builtin_islessgreater(f1, f1)) {  // 1.5 < 1.5 is false
        result += 1024;
    }
    
    // ============================================
    // SSE/AVX Intrinsics for explicit condition codes
    // ============================================
    
    // SSE single-precision (__m128)
    __m128 a = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_setr_ps(5.0f, 2.0f, 3.0f, nan_f);
    
    // UNORDERED
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    use_m128(cmp_unord);
    
    // ORDERED
    __m128 cmp_ord = _mm_cmpord_ps(a, b);
    use_m128(cmp_ord);
    
    // UNEQ (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(a, b);
    use_m128(cmp_neq);
    
    // UNGE (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
    use_m128(cmp_nlt);
    
    // UNGT (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);
    use_m128(cmp_nle);
    
    // UNLE (unordered or less than or equal)
    __m128 cmp_ule = _mm_cmpule_ps(a, b);  // Note: ule suffix
    use_m128(cmp_ule);
    
    // UNLT (unordered or less than)
    __m128 cmp_ult = _mm_cmpult_ps(a, b);  // Note: ult suffix
    use_m128(cmp_ult);
    
    // LTGT (unordered or not equal) - same intrinsic as UNEQ
    __m128 cmp_une = _mm_cmpneq_ps(a, b);
    use_m128(cmp_une);
    
    // SSE double-precision (__m128d)
    __m128d ad = _mm_setr_pd(1.0, nan_d);
    __m128d bd = _mm_setr_pd(nan_d, 2.0);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(ad, bd);
    use_m128d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(ad, bd);
    use_m128d(cmp_ord_d);
    
    __m128d cmp_neq_d = _mm_cmpneq_pd(ad, bd);
    use_m128d(cmp_neq_d);
    
    // AVX single-precision (__m256)
#ifdef __AVX__
    __m256 a256 = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 b256 = _mm256_setr_ps(nan_f, 2.0f, 3.0f, nan_f, 5.0f, nan_f, 7.0f, 8.0f);
    
    __m256 cmp_unord256 = _mm256_cmp_ps(a256, b256, _CMP_UNORD_Q);
    use_m256(cmp_unord256);
    
    __m256 cmp_ord256 = _mm256_cmp_ps(a256, b256, _CMP_ORD_Q);
    use_m256(cmp_ord256);
    
    __m256 cmp_neq256 = _mm256_cmp_ps(a256, b256, _CMP_NEQ_UQ);
    use_m256(cmp_neq256);
    
    __m256 cmp_nlt256 = _mm256_cmp_ps(a256, b256, _CMP_NLT_UQ);
    use_m256(cmp_nlt256);
    
    __m256 cmp_nle256 = _mm256_cmp_ps(a256, b256, _CMP_NLE_UQ);
    use_m256(cmp_nle256);
    
    __m256 cmp_ule256 = _mm256_cmp_ps(a256, b256, _CMP_LE_OS);
    use_m256(cmp_ule256);
    
    __m256 cmp_ult256 = _mm256_cmp_ps(a256, b256, _CMP_LT_OS);
    use_m256(cmp_ult256);
    
    __m256 cmp_une256 = _mm256_cmp_ps(a256, b256, _CMP_NEQ_OQ);
    use_m256(cmp_une256);
#endif
    
    // ============================================
    // Loop to prevent optimization and use volatile
    // ============================================
    for (int i = 0; i < 10; i++) {
        vf1 = (i % 2 == 0) ? (float)i : nan_f;
        vf2 = (i % 3 == 0) ? nan_f : (float)(i * 2);
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(vf1, vf2)) {
            result += i;
        }
        
        if (vf1 != vf1) {  // UNEQ when vf1 is NaN
            result -= i;
        }
        
        // Use ordered comparisons with potential NaN
        if (vf1 < vf2) {   // Could generate UNLT
            result += i * 2;
        }
        
        if (vf1 > vf2) {   // Could generate UNGT
            result += i * 3;
        }
    }
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return 0;
}
