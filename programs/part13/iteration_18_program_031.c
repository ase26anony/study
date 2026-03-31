#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink = 0;

int main() {
    int result = 0;
    
    // Create NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    // 1. UNORDERED case - using __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1 << 0;
    }
    
    // 2. ORDERED case - using __builtin_isordered
    if (__builtin_isordered(normal_f, normal_d)) {
        result |= 1 << 1;
    }
    
    // 3. UNEQ case - compare with NaN using !=
    if (nan_f != nan_f) {  // This is always true for NaN
        result |= 1 << 2;
    }
    
    // 4. UNGE case - greater or equal with NaN operand
    if (!(nan_f >= normal_f)) {  // Generates unordered not less than
        result |= 1 << 3;
    }
    
    // 5. UNGT case - greater than with NaN operand  
    if (!(nan_f > normal_f)) {   // Generates unordered not less or equal
        result |= 1 << 4;
    }
    
    // 6. UNLE case - less or equal with NaN operand
    if (nan_f <= normal_f) {     // Generates unordered less or equal
        result |= 1 << 5;
    }
    
    // 7. UNLT case - less than with NaN operand
    if (nan_f < normal_f) {      // Generates unordered less than
        result |= 1 << 6;
    }
    
    // 8. LTGT case - using __builtin_islessgreater
    if (__builtin_islessgreater(normal_f, normal_d)) {
        result |= 1 << 7;
    }
    
    // SSE vector comparisons
    __m128 a = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    sink = _mm_movemask_ps(cmp_unord);
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(a, b);
    sink = _mm_movemask_ps(cmp_ord);
    
    // UNEQ vector comparison
    __m128 cmp_neq = _mm_cmpneq_ps(a, b);
    sink = _mm_movemask_ps(cmp_neq);
    
    // UNGE vector comparison (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
    sink = _mm_movemask_ps(cmp_nlt);
    
    // UNGT vector comparison (not less or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);
    sink = _mm_movemask_ps(cmp_nle);
    
    // UNLE vector comparison
    __m128 cmp_ule = _mm_cmpule_ps(a, b);
    sink = _mm_movemask_ps(cmp_ule);
    
    // UNLT vector comparison
    __m128 cmp_ult = _mm_cmpult_ps(a, b);
    sink = _mm_movemask_ps(cmp_ult);
    
    // Double precision SSE comparisons
    __m128d ad = _mm_setr_pd(normal_d, nan_d);
    __m128d bd = _mm_setr_pd(normal_d, normal_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(ad, bd);
    sink = _mm_movemask_pd(cmp_unord_d);
    
    // AVX comparisons if available
#ifdef __AVX__
    __m256 a256 = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 b256 = _mm256_setr_ps(1.0f, 2.0f, nan_f, nan_f, 5.0f, 7.0f, 7.0f, 8.0f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(a256, b256, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp_unord_256);
    
    __m256 cmp_ord_256 = _mm256_cmp_ps(a256, b256, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp_ord_256);
    
    __m256 cmp_neq_256 = _mm256_cmp_ps(a256, b256, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_neq_256);
    
    __m256 cmp_nlt_256 = _mm256_cmp_ps(a256, b256, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cmp_nlt_256);
    
    __m256 cmp_nle_256 = _mm256_cmp_ps(a256, b256, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cmp_nle_256);
    
    __m256 cmp_ule_256 = _mm256_cmp_ps(a256, b256, _CMP_LE_UQ);
    sink = _mm256_movemask_ps(cmp_ule_256);
    
    __m256 cmp_ult_256 = _mm256_cmp_ps(a256, b256, _CMP_LT_UQ);
    sink = _mm256_movemask_ps(cmp_ult_256);
#endif
    
    // Mixed precision and complex expressions
    for (int i = 0; i < 10; i++) {
        float x = (i % 2 == 0) ? nan_f : (float)i;
        double y = (i % 3 == 0) ? nan_d : (double)i;
        
        // Generate various condition codes in a loop
        if (__builtin_isunordered(x, y)) {
            result += i;  // UNORDERED
        }
        
        if (!(x > y)) {  // Could generate UNLE or UNLT depending on values
            result += i * 2;
        }
        
        if (x == x) {  // False for NaN - generates ordered comparison
            result += i * 3;
        }
    }
    
    printf("Result: %d\n", result);
    return result != 0;
}
