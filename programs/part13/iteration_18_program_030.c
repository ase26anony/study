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
        result += 1;  // Should be true
    }
    
    // 2. ORDERED condition - using __builtin_isordered
    if (__builtin_isordered(normal_f, normal_f)) {
        result += 2;  // Should be true
    }
    
    // 3. UNEQ condition - equality with NaN
    if (nan_f != nan_f) {  // NaN != NaN is true
        result += 4;
    }
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < normal_f) {  // false when unordered
        result += 8;  // Should not execute
    }
    
    // 5. UNLE condition - less or equal with NaN operand  
    if (nan_f <= normal_f) {  // false when unordered
        result += 16;  // Should not execute
    }
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > normal_f) {  // false when unordered
        result += 32;  // Should not execute
    }
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= normal_f) {  // false when unordered
        result += 64;  // Should not execute
    }
    
    // 8. LTGT condition - using __builtin_islessgreater
    if (__builtin_islessgreater(normal_f, normal_f + 1.0f)) {
        result += 128;  // Should be true (1.5 < 2.5)
    }
    
    // SSE vector comparisons
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 v2 = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_unord);
    
    // ORDERED vector comparison  
    __m128 cmp_ord = _mm_cmpord_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_ord);
    
    // UNEQ vector comparison
    __m128 cmp_neq = _mm_cmpneq_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_neq);
    
    // UNGE vector comparison (nlt)
    __m128 cmp_nlt = _mm_cmpnlt_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_nlt);
    
    // UNGT vector comparison (nle)
    __m128 cmp_nle = _mm_cmpnle_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_nle);
    
    // Double precision SSE comparisons
    __m128d d1 = _mm_setr_pd(1.0, nan_d);
    __m128d d2 = _mm_setr_pd(2.0, 2.0);
    
    // UNLE vector comparison (ule for doubles)
    __m128d cmp_ule = _mm_cmpule_pd(d1, d2);
    sink = _mm_movemask_pd(cmp_ule);
    
    // UNLT vector comparison (ult for doubles)
    __m128d cmp_ult = _mm_cmpult_pd(d1, d2);
    sink = _mm_movemask_pd(cmp_ult);
    
    // AVX comparisons if available
#ifdef __AVX__
    __m256 v256_1 = _mm256_setr_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 v256_2 = _mm256_setr_ps(1.0f, 3.0f, 3.0f, nan_f, 5.0f, 7.0f, 7.0f, nan_f);
    
    // Various AVX unordered comparisons
    __m256 cmp256_unord = _mm256_cmp_ps(v256_1, v256_2, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp256_unord);
    
    __m256 cmp256_ord = _mm256_cmp_ps(v256_1, v256_2, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp256_ord);
    
    __m256 cmp256_neq = _mm256_cmp_ps(v256_1, v256_2, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp256_neq);
    
    __m256 cmp256_nlt = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cmp256_nlt);
    
    __m256 cmp256_nle = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cmp256_nle);
    
    __m256 cmp256_ule = _mm256_cmp_ps(v256_1, v256_2, _CMP_LE_UQ);
    sink = _mm256_movemask_ps(cmp256_ule);
    
    __m256 cmp256_ult = _mm256_cmp_ps(v256_1, v256_2, _CMP_LT_UQ);
    sink = _mm256_movemask_ps(cmp256_ult);
    
    __m256 cmp256_une = _mm256_cmp_ps(v256_1, v256_2, _CMP_NEQ_OQ);  // LTGT
    sink = _mm256_movemask_ps(cmp256_une);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? normal_f : nan_f;
        float b = normal_f + i;
        
        // Generate various condition codes in loop
        if (__builtin_isunordered(a, b)) {
            result++;
        }
        
        // Ternary operator to force condition code generation
        int cond = (a != a) ? 1 : 0;  // UNEQ
        result += cond;
        
        // More complex expression
        cond = (a < b) ? 1 : 0;  // Could generate UNLT if a is NaN
        result += cond;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
