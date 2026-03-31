#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink = 0;

int main(void) {
    int result = 0;
    
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    // 1. UNORDERED condition
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1 << 0;
    }
    
    // 2. ORDERED condition
    if (__builtin_isordered(normal_f, normal_d)) {
        result |= 1 << 1;
    }
    
    // 3. UNEQ condition (unordered or equal)
    if (!(nan_f == nan_f)) {  // NaN != NaN generates UNEQ
        result |= 1 << 2;
    }
    
    // 4. UNLT condition (unordered or less than)
    if (nan_f < normal_f) {  // NaN < x generates UNLT
        result |= 1 << 3;
    }
    
    // 5. UNLE condition (unordered or less than or equal)
    if (nan_f <= normal_f) {  // NaN <= x generates UNLE
        result |= 1 << 4;
    }
    
    // 6. UNGT condition (unordered or greater than)
    if (nan_f > normal_f) {  // NaN > x generates UNGT
        result |= 1 << 5;
    }
    
    // 7. UNGE condition (unordered or greater than or equal)
    if (nan_f >= normal_f) {  // NaN >= x generates UNGE
        result |= 1 << 6;
    }
    
    // 8. LTGT condition (less than or greater than, but not equal and not unordered)
    if (__builtin_islessgreater(normal_f, normal_d)) {
        result |= 1 << 7;
    }
    
    // SSE vector comparisons (128-bit)
    __m128 a = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128 c;
    
    // UNORDERED vector comparison
    c = _mm_cmpunord_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // ORDERED vector comparison
    c = _mm_cmpord_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNEQ vector comparison
    c = _mm_cmpneq_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNGE vector comparison (not less than)
    c = _mm_cmpnlt_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNGT vector comparison (not less than or equal)
    c = _mm_cmpnle_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNLE vector comparison
    c = _mm_cmpule_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNLT vector comparison
    c = _mm_cmpult_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // Double precision SSE comparisons
    __m128d ad = _mm_setr_pd(normal_d, nan_d);
    __m128d bd = _mm_setr_pd(normal_d, normal_d);
    __m128d cd;
    
    cd = _mm_cmpunord_pd(ad, bd);
    sink = _mm_movemask_pd(cd);
    
    cd = _mm_cmpord_pd(ad, bd);
    sink = _mm_movemask_pd(cd);
    
    // AVX comparisons (256-bit) if available
    #ifdef __AVX__
    __m256 av = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 bv = _mm256_setr_ps(1.0f, 2.0f, nan_f, nan_f, 5.0f, 7.0f, 7.0f, 8.0f);
    __m256 cv;
    
    cv = _mm256_cmp_ps(av, bv, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cv);
    
    cv = _mm256_cmp_ps(av, bv, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cv);
    
    cv = _mm256_cmp_ps(av, bv, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cv);
    
    cv = _mm256_cmp_ps(av, bv, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cv);
    
    cv = _mm256_cmp_ps(av, bv, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cv);
    
    cv = _mm256_cmp_ps(av, bv, _CMP_LE_OS);
    sink = _mm256_movemask_ps(cv);
    
    cv = _mm256_cmp_ps(av, bv, _CMP_LT_OS);
    sink = _mm256_movemask_ps(cv);
    #endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float x = (i % 2 == 0) ? normal_f : nan_f;
        double y = (i % 3 == 0) ? nan_d : normal_d;
        
        // Generate various unordered conditions in loop
        if (__builtin_isunordered(x, y)) {
            result += i;
        }
        
        if (!(x == x)) {  // UNEQ
            result += i * 2;
        }
        
        if (x < y) {  // UNLT when NaN involved
            result += i * 3;
        }
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
