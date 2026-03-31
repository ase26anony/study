#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent aggressive optimization
volatile int sink = 0;

int main() {
    int result = 0;
    
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    // 1. UNORDERED condition
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // Trigger UNORDERED
    }
    
    // 2. ORDERED condition
    if (!__builtin_isunordered(normal_f, normal_d)) {
        result |= 2;  // Trigger ORDERED
    }
    
    // 3. UNEQ condition (unordered or equal)
    if (nan_f != nan_f) {  // Always true for NaN
        result |= 4;  // Trigger UNEQ
    }
    
    // 4. UNGE condition (unordered or greater or equal)
    if (nan_f >= normal_f) {  // Unordered comparison
        result |= 8;  // Trigger UNGE
    }
    
    // 5. UNGT condition (unordered or greater)
    if (nan_f > normal_f) {  // Unordered comparison
        result |= 16;  // Trigger UNGT
    }
    
    // 6. UNLE condition (unordered or less or equal)
    if (nan_f <= normal_f) {  // Unordered comparison
        result |= 32;  // Trigger UNLE
    }
    
    // 7. UNLT condition (unordered or less)
    if (nan_f < normal_f) {  // Unordered comparison
        result |= 64;  // Trigger UNLT
    }
    
    // 8. LTGT condition (less or greater, but not equal and not unordered)
    if (__builtin_islessgreater(normal_f, normal_d)) {
        result |= 128;  // Trigger LTGT
    }
    
    // SSE intrinsics for explicit condition codes
    __m128 a = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_set_ps(1.0f, 3.0f, 5.0f, nan_f);
    __m128 c;
    
    // UNORDERED with SSE
    c = _mm_cmpunord_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // ORDERED with SSE
    c = _mm_cmpord_ps(a, b);
    sink += _mm_movemask_ps(c);
    
    // UNEQ with SSE (note: same as neq for unordered)
    c = _mm_cmpneq_ps(a, b);
    sink += _mm_movemask_ps(c);
    
    // UNGE with SSE (not less than)
    c = _mm_cmpnlt_ps(a, b);
    sink += _mm_movemask_ps(c);
    
    // UNGT with SSE (not less or equal)
    c = _mm_cmpnle_ps(a, b);
    sink += _mm_movemask_ps(c);
    
    // UNLE with SSE (unordered or less or equal)
    c = _mm_cmpule_ps(a, b);
    sink += _mm_movemask_ps(c);
    
    // UNLT with SSE (unordered or less than)
    c = _mm_cmpult_ps(a, b);
    sink += _mm_movemask_ps(c);
    
    // Double precision SSE
    __m128d d = _mm_set_pd(nan_d, 1.0);
    __m128d e = _mm_set_pd(2.0, nan_d);
    __m128d f;
    
    f = _mm_cmpunord_pd(d, e);
    sink += _mm_movemask_pd(f);
    
    f = _mm_cmpord_pd(d, e);
    sink += _mm_movemask_pd(f);
    
    // AVX intrinsics for 256-bit vectors
#ifdef __AVX__
    __m256 avx_a = _mm256_set_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 avx_b = _mm256_set_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, 6.0f, 7.0f, nan_f);
    __m256 avx_c;
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNORD_Q);
    sink += _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_ORD_Q);
    sink += _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_NEQ_UQ);
    sink += _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_NLT_UQ);
    sink += _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_NLE_UQ);
    sink += _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_LE_OS);
    sink += _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_LT_OS);
    sink += _mm256_movemask_ps(avx_c);
    
    // AVX double precision
    __m256d avx_d = _mm256_set_pd(nan_d, 2.0, 3.0, 4.0);
    __m256d avx_e = _mm256_set_pd(1.0, nan_d, 3.0, nan_d);
    __m256d avx_f;
    
    avx_f = _mm256_cmp_pd(avx_d, avx_e, _CMP_UNORD_Q);
    sink += _mm256_movemask_pd(avx_f);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    float arr[4] = {1.0f, nan_f, 3.0f, 4.0f};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            // Generate various unordered comparisons in loop
            if (__builtin_isunordered(arr[i], arr[j])) {
                result++;
            }
            if (arr[i] != arr[i]) {  // UNEQ
                result++;
            }
            if (arr[i] < arr[j]) {  // May be UNLT if NaN involved
                result++;
            }
            if (arr[i] > arr[j]) {  // May be UNGT if NaN involved
                result++;
            }
        }
    }
    
    // Mixed precision comparisons
    if (__builtin_islessgreater(normal_f, nan_d)) {
        result++;
    }
    
    // Ternary operator usage
    int r1 = (nan_f == nan_f) ? 0 : 1;  // UNEQ
    int r2 = (normal_f < nan_d) ? 0 : 1;  // UNGE
    int r3 = (nan_f > normal_d) ? 0 : 1;  // UNGT
    int r4 = (normal_f <= nan_f) ? 0 : 1;  // UNLE
    int r5 = (nan_d >= normal_f) ? 0 : 1;  // UNLT
    
    result += r1 + r2 + r3 + r4 + r5;
    
    printf("Result: %d (sink: %d)\n", result, sink);
    return result > 0 ? 0 : 1;
}
