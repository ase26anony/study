#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>

// Function to prevent optimization
volatile int sink = 0;

int main(void) {
    int result = 0;
    
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    float normal_f = 3.14f;
    double normal_d = 2.71828;
    
    // 1. UNORDERED condition - using builtin
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // Should trigger UNORDERED
    }
    
    // 2. ORDERED condition - using builtin with normal values
    if (!__builtin_isunordered(normal_f, normal_d)) {
        result |= 2;  // Should trigger ORDERED
    }
    
    // 3. UNEQ condition - direct NaN comparison
    if (nan_f != nan_f) {  // NaN != NaN is true
        result |= 4;  // Should trigger UNEQ
    }
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < normal_f) {  // false when unordered
        result |= 8;
    } else {
        result |= 16;  // Should trigger UNLT (unordered less than)
    }
    
    // 5. UNLE condition - less or equal with NaN operand
    if (nan_f <= normal_f) {  // false when unordered
        result |= 32;
    } else {
        result |= 64;  // Should trigger UNLE
    }
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > normal_f) {  // false when unordered
        result |= 128;
    } else {
        result |= 256;  // Should trigger UNGT
    }
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= normal_f) {  // false when unordered
        result |= 512;
    } else {
        result |= 1024;  // Should trigger UNGE
    }
    
    // 8. LTGT condition - using builtin_islessgreater
    if (__builtin_islessgreater(nan_f, normal_f)) {
        result |= 2048;  // Should trigger LTGT
    }
    
    // SSE intrinsics for explicit condition codes
    __m128 a = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128 c;
    
    // UNORDERED SSE
    c = _mm_cmpunord_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // ORDERED SSE
    c = _mm_cmpord_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNEQ SSE (not equal)
    c = _mm_cmpneq_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNGE SSE (not less than)
    c = _mm_cmpnlt_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNGT SSE (not less or equal)
    c = _mm_cmpnle_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNLE SSE (unordered or less or equal)
    c = _mm_cmpule_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // UNLT SSE (unordered or less than)
    c = _mm_cmpult_ps(a, b);
    sink = _mm_movemask_ps(c);
    
    // Double precision SSE
    __m128d d = _mm_setr_pd(nan_d, normal_d);
    __m128d e = _mm_setr_pd(normal_d, nan_d);
    __m128d f;
    
    f = _mm_cmpunord_pd(d, e);
    sink = _mm_movemask_pd(f);
    
    f = _mm_cmpord_pd(d, e);
    sink = _mm_movemask_pd(f);
    
    // AVX intrinsics if available
#ifdef __AVX__
    __m256 avx_a = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 avx_b = _mm256_setr_ps(1.0f, 2.0f, nan_f, nan_f, 5.0f, 7.0f, 7.0f, 8.0f);
    __m256 avx_c;
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(avx_c);
    
    avx_c = _mm256_cmp_ps(avx_a, avx_b, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(avx_c);
    
    __m256d avx_d = _mm256_setr_pd(nan_d, 2.0, nan_d, 4.0);
    __m256d avx_e = _mm256_setr_pd(1.0, nan_d, 3.0, nan_d);
    __m256d avx_f;
    
    avx_f = _mm256_cmp_pd(avx_d, avx_e, _CMP_UNORD_Q);
    sink = _mm256_movemask_pd(avx_f);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float x = (i % 2 == 0) ? nan_f : (float)i;
        float y = (i % 3 == 0) ? nan_f : (float)(i * 2);
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(x, y)) {
            result += i;
        }
        
        if (x != x) {  // UNEQ
            result -= i;
        }
        
        // Ternary operator to force condition code generation
        int cmp_result = (x < y) ? 1 : ((x > y) ? 2 : 0);
        result += cmp_result;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
