#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

int main() {
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    int result = 0;
    
    // 1. UNORDERED condition - using __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1 << 0;
    }
    
    // 2. ORDERED condition - using __builtin_isordered
    if (__builtin_isordered(normal_f, normal_d)) {
        result |= 1 << 1;
    }
    
    // 3. UNEQ condition - equality comparison with NaN
    if (nan_f != nan_f) {  // This is always false for NaN, but generates UNEQ
        // This branch won't be taken, but the comparison is generated
        result |= 1 << 2;
    }
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < normal_f) {  // Generates UNLT when comparing with NaN
        result |= 1 << 3;
    }
    
    // 5. UNLE condition - less or equal with NaN operand
    if (nan_f <= normal_f) {  // Generates UNLE when comparing with NaN
        result |= 1 << 4;
    }
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > normal_f) {  // Generates UNGT when comparing with NaN
        result |= 1 << 5;
    }
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= normal_f) {  // Generates UNGE when comparing with NaN
        result |= 1 << 6;
    }
    
    // 8. LTGT condition - using __builtin_islessgreater
    if (__builtin_islessgreater(normal_f, normal_d)) {
        result |= 1 << 7;
    }
    
    // SSE vector comparisons (128-bit)
    __m128 vec_nan_f = _mm_set1_ps(nan_f);
    __m128 vec_normal_f = _mm_set1_ps(normal_f);
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    __m128d vec_normal_d = _mm_set1_pd(normal_d);
    
    // 9. UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_nan_f, vec_normal_f);
    sink = _mm_movemask_ps(cmp_unord);
    
    // 10. ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(vec_normal_f, vec_normal_f);
    sink = _mm_movemask_ps(cmp_ord);
    
    // 11. UNEQ vector comparison
    __m128 cmp_uneq = _mm_cmpneq_ps(vec_nan_f, vec_normal_f);
    sink = _mm_movemask_ps(cmp_uneq);
    
    // 12. UNGE vector comparison (nlt)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_nan_f, vec_normal_f);
    sink = _mm_movemask_ps(cmp_nlt);
    
    // 13. UNGT vector comparison (nle)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_nan_f, vec_normal_f);
    sink = _mm_movemask_ps(cmp_nle);
    
    // 14. UNLE vector comparison (ule)
    __m128 cmp_ule = _mm_cmpule_ps(vec_nan_f, vec_normal_f);
    sink = _mm_movemask_ps(cmp_ule);
    
    // 15. UNLT vector comparison (ult)
    __m128 cmp_ult = _mm_cmpult_ps(vec_nan_f, vec_normal_f);
    sink = _mm_movemask_ps(cmp_ult);
    
    // 16. LTGT vector comparison (une)
    __m128 cmp_une = _mm_cmpneq_ps(vec_normal_f, vec_normal_f);
    sink = _mm_movemask_ps(cmp_une);
    
    // Double precision vector comparisons
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_nan_d, vec_normal_d);
    sink = _mm_movemask_pd(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_normal_d, vec_normal_d);
    sink = _mm_movemask_pd(cmp_ord_d);
    
    // AVX comparisons (256-bit) if AVX is available
#ifdef __AVX__
    __m256 vec_nan_f_256 = _mm256_set1_ps(nan_f);
    __m256 vec_normal_f_256 = _mm256_set1_ps(normal_f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp_unord_256);
    
    __m256 cmp_ord_256 = _mm256_cmp_ps(vec_normal_f_256, vec_normal_f_256, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp_ord_256);
    
    __m256 cmp_uneq_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_uneq_256);
    
    __m256 cmp_nlt_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cmp_nlt_256);
    
    __m256 cmp_nle_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cmp_nle_256);
    
    __m256 cmp_ule_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_LE_UQ);
    sink = _mm256_movemask_ps(cmp_ule_256);
    
    __m256 cmp_ult_256 = _mm256_cmp_ps(vec_nan_f_256, vec_normal_f_256, _CMP_LT_UQ);
    sink = _mm256_movemask_ps(cmp_ult_256);
    
    __m256 cmp_une_256 = _mm256_cmp_ps(vec_normal_f_256, vec_normal_f_256, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_une_256);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? nan_f : normal_f;
        float b = (i % 3 == 0) ? nan_f : normal_f;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(a, b)) {
            result += i;
        }
        
        if (a != a) {  // UNEQ
            result += i * 2;
        }
        
        if (a < b) {  // Could be UNLT if a or b is NaN
            result += i * 3;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
