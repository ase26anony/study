#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Helper to prevent optimization
volatile int sink;

int main() {
    // Initialize NaN values
    float nan_f = NAN;
    double nan_d = NAN;
    float inf_f = INFINITY;
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    // Initialize vector values
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 v2 = _mm_setr_ps(5.0f, 2.0f, 3.0f, nan_f);
    __m128d vd1 = _mm_setr_pd(1.0, nan_d);
    __m128d vd2 = _mm_setr_pd(nan_d, 2.0);
    
    // For AVX if available
#ifdef __AVX__
    __m256 v256_1 = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 v256_2 = _mm256_setr_ps(9.0f, 10.0f, 11.0f, 12.0f, nan_f, 14.0f, 15.0f, 16.0f);
    __m256d vd256_1 = _mm256_setr_pd(1.0, 2.0, nan_d, 4.0);
    __m256d vd256_2 = _mm256_setr_pd(5.0, nan_d, 7.0, 8.0);
#endif
    
    int result = 0;
    
    // 1. UNORDERED case - using builtin
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // UNORDERED
    }
    
    // 2. ORDERED case - using builtin
    if (__builtin_isordered(normal_f, normal_d)) {
        result |= 2;  // ORDERED
    }
    
    // 3. UNEQ case - not equal unordered
    if (!(nan_f == nan_f)) {  // NaN != NaN is true
        result |= 4;  // UNEQ
    }
    
    // 4. UNGE case - unordered or greater or equal
    if (!(nan_f < normal_f)) {  // !(NaN < 1.5) generates UNGE
        result |= 8;  // UNGE
    }
    
    // 5. UNGT case - unordered or greater than
    if (!(nan_f <= normal_f)) {  // !(NaN <= 1.5) generates UNGT
        result |= 16;  // UNGT
    }
    
    // 6. UNLE case - unordered or less or equal
    if (!(normal_f > nan_f)) {  // !(1.5 > NaN) generates UNLE
        result |= 32;  // UNLE
    }
    
    // 7. UNLT case - unordered or less than
    if (!(normal_f >= nan_f)) {  // !(1.5 >= NaN) generates UNLT
        result |= 64;  // UNLT
    }
    
    // 8. LTGT case - less or greater (ordered and not equal)
    if (__builtin_islessgreater(normal_f, normal_d)) {
        result |= 128;  // LTGT
    }
    
    // SSE intrinsics for explicit condition codes
    __m128 cmp_result;
    __m128d cmp_result_d;
    
    // UNORDERED using SSE intrinsic
    cmp_result = _mm_cmpunord_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_result);
    
    // ORDERED using SSE intrinsic
    cmp_result = _mm_cmpord_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_result);
    
    // UNEQ using SSE intrinsic (unordered or not equal)
    cmp_result = _mm_cmpneq_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_result);
    
    // UNGE using SSE intrinsic (not less than)
    cmp_result = _mm_cmpnlt_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_result);
    
    // UNGT using SSE intrinsic (not less or equal)
    cmp_result = _mm_cmpnle_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_result);
    
    // UNLE using SSE intrinsic (unordered or less or equal)
    cmp_result_d = _mm_cmpule_pd(vd1, vd2);
    sink = _mm_movemask_pd(cmp_result_d);
    
    // UNLT using SSE intrinsic (unordered or less than)
    cmp_result_d = _mm_cmpult_pd(vd1, vd2);
    sink = _mm_movemask_pd(cmp_result_d);
    
    // Double precision comparisons
    if (__builtin_isunordered(nan_d, normal_d)) {
        result |= 256;
    }
    
    // Mixed type comparisons
    if (__builtin_isless(nan_f, normal_d)) {
        result |= 512;
    }
    
    if (__builtin_isgreater(normal_f, nan_d)) {
        result |= 1024;
    }
    
    // Generate NaN through arithmetic
    float dynamic_nan = nan_f * 2.0f;
    if (__builtin_isunordered(dynamic_nan, normal_f)) {
        result |= 2048;
    }
    
    // Loop to prevent dead code elimination
    for (int i = 0; i < 10; i++) {
        float a = (i % 2) ? nan_f : (float)i;
        float b = (i % 3) ? (float)i : nan_f;
        
        // Generate various condition codes in loop
        if (__builtin_isunordered(a, b)) result++;
        if (!(a < b)) result++;
        if (!(a <= b)) result++;
        if (__builtin_islessgreater(a, b)) result++;
    }
    
#ifdef __AVX__
    // AVX comparisons
    __m256 cmp256_result;
    __m256d cmp256_result_d;
    
    // UNORDERED with AVX
    cmp256_result = _mm256_cmp_ps(v256_1, v256_2, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp256_result);
    
    // ORDERED with AVX
    cmp256_result = _mm256_cmp_ps(v256_1, v256_2, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp256_result);
    
    // UNEQ with AVX
    cmp256_result = _mm256_cmp_ps(v256_1, v256_2, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp256_result);
    
    // UNGE with AVX
    cmp256_result = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cmp256_result);
    
    // UNGT with AVX
    cmp256_result = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cmp256_result);
    
    // Double precision AVX
    cmp256_result_d = _mm256_cmp_pd(vd256_1, vd256_2, _CMP_UNORD_Q);
    sink = _mm256_movemask_pd(cmp256_result_d);
#endif
    
    printf("Result: %d\n", result);
    return result != 0;
}
