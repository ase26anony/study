#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent aggressive optimization
volatile int sink = 0;

int main() {
    int results = 0;
    
    // Create NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    // ============================================
    // 1. Scalar unordered comparisons using builtins
    // ============================================
    
    // UNORDERED - __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        results |= 1;
    }
    
    // ORDERED - __builtin_isordered
    if (__builtin_isordered(normal_f, normal_d)) {
        results |= 2;
    }
    
    // UNEQ - x != x (or x == NaN)
    if (nan_f != nan_f) {  // Always true for NaN
        results |= 4;
    }
    
    // UNLT - less than with NaN operand
    if (nan_f < normal_f) {  // False (unordered)
        // This generates UNLT condition
    }
    
    // UNLE - less or equal with NaN operand  
    if (nan_f <= normal_f) {  // False (unordered)
        // This generates UNLE condition
    }
    
    // UNGT - greater than with NaN operand
    if (nan_f > normal_f) {  // False (unordered)
        // This generates UNGT condition
    }
    
    // UNGE - greater or equal with NaN operand
    if (nan_f >= normal_f) {  // False (unordered)
        // This generates UNGE condition
    }
    
    // LTGT - __builtin_islessgreater
    if (__builtin_islessgreater(normal_f, normal_d)) {
        results |= 8;
    }
    
    // ============================================
    // 2. SSE vector comparisons (128-bit)
    // ============================================
    
    __m128 vec_nan_f = _mm_set1_ps(nan_f);
    __m128 vec_normal_f = _mm_set1_ps(normal_f);
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    __m128d vec_normal_d = _mm_set1_pd(normal_d);
    
    // UNORDERED
    __m128 cmp_unord = _mm_cmpunord_ps(vec_nan_f, vec_normal_f);
    sink += _mm_movemask_ps(cmp_unord);
    
    // ORDERED
    __m128 cmp_ord = _mm_cmpord_ps(vec_normal_f, vec_normal_f);
    sink += _mm_movemask_ps(cmp_ord);
    
    // UNEQ (not equal)
    __m128 cmp_uneq = _mm_cmpneq_ps(vec_nan_f, vec_normal_f);
    sink += _mm_movemask_ps(cmp_uneq);
    
    // UNGE (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_nan_f, vec_normal_f);
    sink += _mm_movemask_ps(cmp_nlt);
    
    // UNGT (not less or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_nan_f, vec_normal_f);
    sink += _mm_movemask_ps(cmp_nle);
    
    // UNLE (unordered or less or equal)
    __m128 cmp_ule = _mm_cmpule_ps(vec_nan_f, vec_normal_f);
    sink += _mm_movemask_ps(cmp_ule);
    
    // UNLT (unordered or less than)
    __m128 cmp_ult = _mm_cmpult_ps(vec_nan_f, vec_normal_f);
    sink += _mm_movemask_ps(cmp_ult);
    
    // Double precision versions
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_nan_d, vec_normal_d);
    sink += _mm_movemask_pd(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_normal_d, vec_normal_d);
    sink += _mm_movemask_pd(cmp_ord_d);
    
    // ============================================
    // 3. AVX vector comparisons (256-bit)
    // ============================================
    
#ifdef __AVX__
    __m256 vec256_nan_f = _mm256_set1_ps(nan_f);
    __m256 vec256_normal_f = _mm256_set1_ps(normal_f);
    __m256d vec256_nan_d = _mm256_set1_pd(nan_d);
    __m256d vec256_normal_d = _mm256_set1_pd(normal_d);
    
    // UNORDERED (AVX)
    __m256 cmp256_unord = _mm256_cmp_ps(vec256_nan_f, vec256_normal_f, _CMP_UNORD_Q);
    sink += _mm256_movemask_ps(cmp256_unord);
    
    // ORDERED (AVX)
    __m256 cmp256_ord = _mm256_cmp_ps(vec256_normal_f, vec256_normal_f, _CMP_ORD_Q);
    sink += _mm256_movemask_ps(cmp256_ord);
    
    // UNEQ (AVX)
    __m256 cmp256_uneq = _mm256_cmp_ps(vec256_nan_f, vec256_normal_f, _CMP_NEQ_UQ);
    sink += _mm256_movemask_ps(cmp256_uneq);
    
    // UNGE (AVX) - not less than
    __m256 cmp256_nlt = _mm256_cmp_ps(vec256_nan_f, vec256_normal_f, _CMP_NLT_UQ);
    sink += _mm256_movemask_ps(cmp256_nlt);
    
    // UNGT (AVX) - not less or equal
    __m256 cmp256_nle = _mm256_cmp_ps(vec256_nan_f, vec256_normal_f, _CMP_NLE_UQ);
    sink += _mm256_movemask_ps(cmp256_nle);
    
    // UNLE (AVX)
    __m256 cmp256_ule = _mm256_cmp_ps(vec256_nan_f, vec256_normal_f, _CMP_LE_OS);
    sink += _mm256_movemask_ps(cmp256_ule);
    
    // UNLT (AVX)
    __m256 cmp256_ult = _mm256_cmp_ps(vec256_nan_f, vec256_normal_f, _CMP_LT_OS);
    sink += _mm256_movemask_ps(cmp256_ult);
    
    // Double precision AVX
    __m256d cmp256_unord_d = _mm256_cmp_pd(vec256_nan_d, vec256_normal_d, _CMP_UNORD_Q);
    sink += _mm256_movemask_pd(cmp256_unord_d);
#endif
    
    // ============================================
    // 4. Loop to prevent dead code elimination
    // ============================================
    
    float a = 1.0f;
    float b = 2.0f;
    int loop_result = 0;
    
    for (int i = 0; i < 10; i++) {
        a += 0.1f;
        b += 0.2f;
        
        // Mix ordered and unordered comparisons in loop
        if (__builtin_isunordered(a, b)) {
            loop_result++;
        }
        if (__builtin_islessgreater(a, b)) {
            loop_result++;
        }
        if (a != a) {  // Check for NaN
            loop_result++;
        }
    }
    
    results += loop_result;
    
    // ============================================
    // 5. Complex expressions with NaN propagation
    // ============================================
    
    float expr1 = nan_f * 2.0f + 1.0f;  // Propagates NaN
    float expr2 = normal_f / 0.0f;      // May produce Inf
    
    // This should generate various condition codes
    if (__builtin_isunordered(expr1, expr2)) {
        results |= 16;
    }
    
    if (expr1 < expr2) {  // UNLT when expr1 is NaN
        // Empty branch
    }
    
    // Ternary operator forcing condition code generation
    int cond1 = (nan_f == nan_f) ? 1 : 0;  // UNEQ
    int cond2 = (nan_f < normal_f) ? 1 : 0; // UNLT
    int cond3 = (normal_f > nan_f) ? 1 : 0; // UNGT
    
    results += cond1 + cond2 + cond3;
    
    // Print results to ensure observable behavior
    printf("Results: %d (sink: %d)\n", results, sink);
    
    return 0;
}
