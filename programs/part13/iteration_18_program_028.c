#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent dead code elimination
volatile int sink;

// Function to force conditional branch generation
__attribute__((noinline)) 
int use_condition(int cond) {
    sink = cond;
    return cond;
}

int main() {
    int result = 0;
    
    // Initialize NaN values
    float nan_f = NAN;
    double nan_d = NAN;
    float inf_f = INFINITY;
    float normal_f = 3.14f;
    double normal_d = 2.71828;
    
    // 1. UNORDERED condition - using builtin
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1 << 0;
    }
    
    // 2. ORDERED condition - using builtin
    if (!__builtin_isunordered(normal_f, normal_f)) {
        result |= 1 << 1;
    }
    
    // 3. UNEQ condition - equality with NaN
    if (nan_f != nan_f) {  // This is always true for NaN != NaN
        result |= 1 << 2;
    }
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < normal_f) {  // Unordered less than
        result |= 1 << 3;
    }
    
    // 5. UNLE condition - less or equal with NaN operand
    if (nan_f <= normal_f) {  // Unordered less or equal
        result |= 1 << 4;
    }
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > normal_f) {  // Unordered greater than
        result |= 1 << 5;
    }
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= normal_f) {  // Unordered greater or equal
        result |= 1 << 6;
    }
    
    // 8. LTGT condition - using builtin_islessgreater
    if (__builtin_islessgreater(nan_f, normal_f)) {
        result |= 1 << 7;
    }
    
    // SSE vector comparisons (128-bit)
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 v2 = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128 v3 = _mm_setr_ps(nan_f, nan_f, nan_f, nan_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(v1, v2);
    result |= (_mm_movemask_ps(cmp_unord) != 0) << 8;
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(v1, v2);
    result |= (_mm_movemask_ps(cmp_ord) != 0) << 9;
    
    // UNEQ vector comparison (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(v1, v2);
    result |= (_mm_movemask_ps(cmp_neq) != 0) << 10;
    
    // UNGE vector comparison (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(v1, v2);
    result |= (_mm_movemask_ps(cmp_nlt) != 0) << 11;
    
    // UNGT vector comparison (not less or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(v1, v2);
    result |= (_mm_movemask_ps(cmp_nle) != 0) << 12;
    
    // UNLE vector comparison (unordered less or equal)
    __m128 cmp_ule = _mm_cmpule_ps(v1, v3);  // Compare with all NaN
    result |= (_mm_movemask_ps(cmp_ule) != 0) << 13;
    
    // UNLT vector comparison (unordered less than)
    __m128 cmp_ult = _mm_cmpult_ps(v1, v3);  // Compare with all NaN
    result |= (_mm_movemask_ps(cmp_ult) != 0) << 14;
    
    // Double precision SSE comparisons
    __m128d d1 = _mm_setr_pd(normal_d, nan_d);
    __m128d d2 = _mm_setr_pd(normal_d, normal_d);
    
    // UNORDERED for doubles
    __m128d cmp_unord_d = _mm_cmpunord_pd(d1, d2);
    result |= (_mm_movemask_pd(cmp_unord_d) != 0) << 15;
    
    // ORDERED for doubles
    __m128d cmp_ord_d = _mm_cmpord_pd(d1, d2);
    result |= (_mm_movemask_pd(cmp_ord_d) != 0) << 16;
    
    // AVX comparisons (256-bit) if available
#ifdef __AVX__
    __m256 v256_1 = _mm256_setr_ps(1.0f, nan_f, 3.0f, nan_f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 v256_2 = _mm256_setr_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, nan_f, 7.0f, 8.0f);
    
    // Various unordered AVX comparisons
    __m256 cmp256_unord = _mm256_cmp_ps(v256_1, v256_2, _CMP_UNORD_Q);
    result |= (_mm256_movemask_ps(cmp256_unord) != 0) << 17;
    
    __m256 cmp256_ord = _mm256_cmp_ps(v256_1, v256_2, _CMP_ORD_Q);
    result |= (_mm256_movemask_ps(cmp256_ord) != 0) << 18;
    
    __m256 cmp256_neq = _mm256_cmp_ps(v256_1, v256_2, _CMP_NEQ_UQ);
    result |= (_mm256_movemask_ps(cmp256_neq) != 0) << 19;
    
    __m256 cmp256_nlt = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLT_UQ);
    result |= (_mm256_movemask_ps(cmp256_nlt) != 0) << 20;
    
    __m256 cmp256_nle = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLE_UQ);
    result |= (_mm256_movemask_ps(cmp256_nle) != 0) << 21;
    
    __m256 cmp256_ule = _mm256_cmp_ps(v256_1, v256_2, _CMP_LE_UQ);
    result |= (_mm256_movemask_ps(cmp256_ule) != 0) << 22;
    
    __m256 cmp256_ult = _mm256_cmp_ps(v256_1, v256_2, _CMP_LT_UQ);
    result |= (_mm256_movemask_ps(cmp256_ult) != 0) << 23;
#endif
    
    // Loop to prevent optimization and generate more comparison patterns
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? normal_f : nan_f;
        float b = (i % 3 == 0) ? normal_f : nan_f;
        
        // Generate various unordered comparisons in loop
        int cond1 = __builtin_isunordered(a, b);
        int cond2 = (a != a) || (b != b);
        int cond3 = (a < b) && __builtin_isunordered(a, b);
        int cond4 = (a > b) && __builtin_isunordered(a, b);
        
        result += cond1 + cond2 + cond3 + cond4;
    }
    
    // Mixed precision comparisons
    {
        double d = normal_d;
        float f = normal_f;
        
        // Promote float to double for comparison
        if (__builtin_isunordered((double)f, d)) {
            result += 1;
        }
        
        // Demote double to float
        if (__builtin_isunordered(f, (float)d)) {
            result += 2;
        }
    }
    
    // Ternary operator usage to force condition code generation
    int r1 = (nan_f == nan_f) ? 0 : 1;  // UNEQ
    int r2 = (nan_f < normal_f) ? 1 : 0; // UNLT
    int r3 = (normal_f < nan_f) ? 1 : 0; // UNGT (reversed)
    int r4 = __builtin_islessgreater(nan_f, normal_f) ? 1 : 0; // LTGT
    
    result += r1 + r2 + r3 + r4;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
