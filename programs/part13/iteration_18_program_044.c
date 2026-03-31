#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink = 0;

int main(void) {
    int results = 0;
    
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    // 1. UNORDERED condition
    if (__builtin_isunordered(nan_f, normal_f)) {
        results |= 1;  // UNORDERED
    }
    
    // 2. ORDERED condition
    if (!__builtin_isunordered(normal_f, normal_f)) {
        results |= 2;  // ORDERED
    }
    
    // 3. UNEQ condition (unordered or equal)
    if (nan_f != nan_f) {  // Always true for NaN
        results |= 4;  // UNEQ
    }
    
    // 4. UNLT condition (unordered or less than)
    if (nan_f < normal_f) {  // Unordered comparison
        results |= 8;  // UNLT
    }
    
    // 5. UNLE condition (unordered or less than or equal)
    if (nan_f <= normal_f) {  // Unordered comparison
        results |= 16;  // UNLE
    }
    
    // 6. UNGT condition (unordered or greater than)
    if (nan_f > normal_f) {  // Unordered comparison
        results |= 32;  // UNGT
    }
    
    // 7. UNGE condition (unordered or greater than or equal)
    if (nan_f >= normal_f) {  // Unordered comparison
        results |= 64;  // UNGE
    }
    
    // 8. LTGT condition (less than or greater than, but not equal)
    if (__builtin_islessgreater(normal_f, normal_f + 1.0f)) {
        results |= 128;  // LTGT
    }
    
    // SSE vector comparisons (128-bit)
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 v2 = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    
    // UNORDERED vector comparison
    __m128 unord_mask = _mm_cmpunord_ps(v1, v2);
    results += _mm_movemask_ps(unord_mask);
    
    // ORDERED vector comparison
    __m128 ord_mask = _mm_cmpord_ps(v1, v2);
    results += _mm_movemask_ps(ord_mask);
    
    // UNEQ vector comparison
    __m128 uneq_mask = _mm_cmpneq_ps(v1, v2);
    results += _mm_movemask_ps(uneq_mask);
    
    // UNGE vector comparison (not less than)
    __m128 unge_mask = _mm_cmpnlt_ps(v1, v2);
    results += _mm_movemask_ps(unge_mask);
    
    // UNGT vector comparison (not less than or equal)
    __m128 ungt_mask = _mm_cmpnle_ps(v1, v2);
    results += _mm_movemask_ps(ungt_mask);
    
    // UNLE vector comparison (unordered or less than or equal)
    __m128 unle_mask = _mm_cmpule_ps(v1, v2);
    results += _mm_movemask_ps(unle_mask);
    
    // UNLT vector comparison (unordered or less than)
    __m128 unlt_mask = _mm_cmpult_ps(v1, v2);
    results += _mm_movemask_ps(unlt_mask);
    
    // Double precision SSE comparisons
    __m128d d1 = _mm_setr_pd(normal_d, nan_d);
    __m128d d2 = _mm_setr_pd(normal_d + 1.0, normal_d);
    
    __m128d d_unord = _mm_cmpunord_pd(d1, d2);
    results += _mm_movemask_pd(d_unord);
    
    __m128d d_ord = _mm_cmpord_pd(d1, d2);
    results += _mm_movemask_pd(d_ord);
    
    // AVX comparisons (256-bit) if available
#ifdef __AVX__
    __m256 v256_1 = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 v256_2 = _mm256_setr_ps(1.0f, 2.0f, nan_f, nan_f, 5.0f, 7.0f, 8.0f, 9.0f);
    
    __m256 v256_unord = _mm256_cmp_ps(v256_1, v256_2, _CMP_UNORD_Q);
    results += _mm256_movemask_ps(v256_unord);
    
    __m256 v256_ord = _mm256_cmp_ps(v256_1, v256_2, _CMP_ORD_Q);
    results += _mm256_movemask_ps(v256_ord);
    
    __m256 v256_uneq = _mm256_cmp_ps(v256_1, v256_2, _CMP_NEQ_UQ);
    results += _mm256_movemask_ps(v256_uneq);
    
    __m256 v256_unge = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLT_UQ);
    results += _mm256_movemask_ps(v256_unge);
    
    __m256 v256_ungt = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLE_UQ);
    results += _mm256_movemask_ps(v256_ungt);
    
    __m256 v256_unle = _mm256_cmp_ps(v256_1, v256_2, _CMP_LE_OS);
    results += _mm256_movemask_ps(v256_unle);
    
    __m256 v256_unlt = _mm256_cmp_ps(v256_1, v256_2, _CMP_LT_OS);
    results += _mm256_movemask_ps(v256_unlt);
    
    __m256 v256_ltgt = _mm256_cmp_ps(v256_1, v256_2, _CMP_NEQ_OQ);
    results += _mm256_movemask_ps(v256_ltgt);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? normal_f : nan_f;
        float b = normal_f + i;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(a, b)) {
            sink = i;  // UNORDERED
        }
        
        if (a != a) {  // UNEQ
            sink = i + 1;
        }
        
        if (a < b) {  // Could be UNLT if a is NaN
            sink = i + 2;
        }
        
        if (__builtin_islessgreater(a, b)) {  // LTGT
            sink = i + 3;
        }
    }
    
    printf("Results: %d\n", results);
    printf("Sink: %d\n", sink);
    
    return 0;
}
