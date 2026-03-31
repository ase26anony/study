#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent dead code elimination
volatile int sink;

// Helper to force conditional branches
__attribute__((noinline)) 
void use_result(int cond) {
    sink = cond;
}

int main() {
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    float normal_f = 3.14f;
    double normal_d = 2.718;
    
    int result = 0;
    
    // 1. UNORDERED condition - using builtin
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1 << 0;
    }
    
    // 2. ORDERED condition - using builtin
    if (!__builtin_isunordered(normal_f, normal_f)) {
        result |= 1 << 1;
    }
    
    // 3. UNEQ condition - direct comparison with NaN
    if (nan_f != nan_f) {  // NaN != NaN is true
        result |= 1 << 2;
    }
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < normal_f) {  // false when unordered
        result |= 1 << 3;
    } else {
        result |= 1 << 4;  // This branch should be taken
    }
    
    // 5. UNLE condition - less or equal with NaN operand
    if (nan_f <= normal_f) {  // false when unordered
        result |= 1 << 5;
    } else {
        result |= 1 << 6;  // This branch should be taken
    }
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > normal_f) {  // false when unordered
        result |= 1 << 7;
    } else {
        result |= 1 << 8;  // This branch should be taken
    }
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= normal_f) {  // false when unordered
        result |= 1 << 9;
    } else {
        result |= 1 << 10;  // This branch should be taken
    }
    
    // 8. LTGT condition - using builtin
    if (__builtin_islessgreater(nan_f, normal_f)) {
        result |= 1 << 11;
    } else {
        result |= 1 << 12;  // This branch should be taken
    }
    
    // SSE vector comparisons (128-bit)
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 v2 = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128 v3 = _mm_setr_ps(nan_f, nan_f, nan_f, nan_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(v1, v2);
    int mask_unord = _mm_movemask_ps(cmp_unord);
    if (mask_unord) result |= 1 << 13;
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(v1, v2);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    if (mask_ord) result |= 1 << 14;
    
    // UNEQ vector comparison
    __m128 cmp_uneq = _mm_cmpneq_ps(v1, v2);
    int mask_uneq = _mm_movemask_ps(cmp_uneq);
    if (mask_uneq) result |= 1 << 15;
    
    // UNGE vector comparison (nlt)
    __m128 cmp_nlt = _mm_cmpnlt_ps(v1, v2);
    int mask_nlt = _mm_movemask_ps(cmp_nlt);
    if (mask_nlt) result |= 1 << 16;
    
    // UNGT vector comparison (nle)
    __m128 cmp_nle = _mm_cmpnle_ps(v1, v2);
    int mask_nle = _mm_movemask_ps(cmp_nle);
    if (mask_nle) result |= 1 << 17;
    
    // UNLE vector comparison (ule)
    __m128 cmp_ule = _mm_cmpule_ps(v1, v2);
    int mask_ule = _mm_movemask_ps(cmp_ule);
    if (mask_ule) result |= 1 << 18;
    
    // UNLT vector comparison (ult)
    __m128 cmp_ult = _mm_cmpult_ps(v1, v2);
    int mask_ult = _mm_movemask_ps(cmp_ult);
    if (mask_ult) result |= 1 << 19;
    
    // Double precision SSE comparisons
    __m128d d1 = _mm_setr_pd(normal_d, nan_d);
    __m128d d2 = _mm_setr_pd(normal_d, normal_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(d1, d2);
    int mask_unord_d = _mm_movemask_pd(cmp_unord_d);
    if (mask_unord_d) result |= 1 << 20;
    
    // AVX comparisons (256-bit) if available
#ifdef __AVX__
    __m256 v256_1 = _mm256_setr_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 v256_2 = _mm256_setr_ps(1.0f, 3.0f, 3.0f, nan_f, 5.0f, 7.0f, 7.0f, nan_f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(v256_1, v256_2, _CMP_UNORD_Q);
    int mask_unord_256 = _mm256_movemask_ps(cmp_unord_256);
    if (mask_unord_256) result |= 1 << 21;
    
    __m256 cmp_ord_256 = _mm256_cmp_ps(v256_1, v256_2, _CMP_ORD_Q);
    int mask_ord_256 = _mm256_movemask_ps(cmp_ord_256);
    if (mask_ord_256) result |= 1 << 22;
    
    __m256 cmp_neq_256 = _mm256_cmp_ps(v256_1, v256_2, _CMP_NEQ_UQ);
    int mask_neq_256 = _mm256_movemask_ps(cmp_neq_256);
    if (mask_neq_256) result |= 1 << 23;
    
    __m256 cmp_nlt_256 = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLT_UQ);
    int mask_nlt_256 = _mm256_movemask_ps(cmp_nlt_256);
    if (mask_nlt_256) result |= 1 << 24;
    
    __m256 cmp_nle_256 = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLE_UQ);
    int mask_nle_256 = _mm256_movemask_ps(cmp_nle_256);
    if (mask_nle_256) result |= 1 << 25;
#endif
    
    // Loop to prevent optimization and generate more comparison patterns
    float accum = 0.0f;
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? normal_f : nan_f;
        float b = (i % 3 == 0) ? normal_f * i : nan_f;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(a, b)) {
            accum += 1.0f;
        }
        if (!__builtin_islessgreater(a, b)) {
            accum += 2.0f;
        }
        if (a != a) {  // UNEQ
            accum += 3.0f;
        }
    }
    
    // Use ternary operators to generate conditional moves
    int r1 = (normal_f < nan_f) ? 100 : 200;  // Should be 200 (UNLT branch)
    int r2 = (nan_f >= normal_f) ? 300 : 400; // Should be 400 (UNGE branch)
    int r3 = (normal_f != normal_f) ? 500 : 600; // Should be 600 (ordered)
    
    result += r1 + r2 + r3 + (int)accum;
    
    printf("Result: %d (0x%x)\n", result, result);
    printf("Sink: %d\n", sink);
    
    return result != 0 ? 0 : 1;
}
