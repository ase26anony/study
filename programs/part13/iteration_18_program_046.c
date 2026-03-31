#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent aggressive optimization
volatile int sink;

// Function to force conditional branch generation
__attribute__((noinline)) 
int check_condition(int cond) {
    sink = cond;
    return cond;
}

int main() {
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    int results = 0;
    
    // 1. UNORDERED condition
    // Using __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        results |= 1 << 0;
    }
    
    // Direct unordered comparison
    if (nan_f != nan_f) {  // NaN != NaN is true
        results |= 1 << 1;
    }
    
    // 2. ORDERED condition
    // Using ordered comparison with normal values
    if (!__builtin_isunordered(normal_f, normal_d)) {
        results |= 1 << 2;
    }
    
    // 3. UNEQ condition (unordered or equal)
    // Compare NaN with itself using ==
    if (!(nan_f == nan_f)) {  // NaN == NaN is false
        results |= 1 << 3;
    }
    
    // Using __builtin_islessgreater with NaN
    if (!__builtin_islessgreater(nan_f, normal_f)) {
        results |= 1 << 4;
    }
    
    // 4. UNGE condition (unordered or greater or equal)
    // Using >= with NaN operand
    if (!(nan_f >= normal_f)) {  // Always false, but generates UNGE
        results |= 1 << 5;
    }
    
    // 5. UNGT condition (unordered or greater)
    // Using > with NaN operand
    if (!(nan_f > normal_f)) {  // Always false, but generates UNGT
        results |= 1 << 6;
    }
    
    // 6. UNLE condition (unordered or less or equal)
    // Using <= with NaN operand
    if (!(nan_f <= normal_f)) {  // Always false, but generates UNLE
        results |= 1 << 7;
    }
    
    // 7. UNLT condition (unordered or less)
    // Using < with NaN operand
    if (!(nan_f < normal_f)) {  // Always false, but generates UNLT
        results |= 1 << 8;
    }
    
    // 8. LTGT condition (less or greater, but not equal and not unordered)
    // Using __builtin_islessgreater with normal values
    if (__builtin_islessgreater(normal_f, normal_f + 1.0f)) {
        results |= 1 << 9;
    }
    
    // SSE vector comparisons (128-bit)
    __m128 v_nan = _mm_set1_ps(nan_f);
    __m128 v_val = _mm_set1_ps(normal_f);
    __m128 v_inf = _mm_set1_ps(inf_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(v_nan, v_val);
    results |= _mm_movemask_ps(cmp_unord) << 10;
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(v_val, v_inf);
    results |= _mm_movemask_ps(cmp_ord) << 14;
    
    // UNEQ vector comparison
    __m128 cmp_uneq = _mm_cmpneq_ps(v_nan, v_val);
    results |= _mm_movemask_ps(cmp_uneq) << 18;
    
    // UNGE vector comparison (nlt)
    __m128 cmp_nlt = _mm_cmpnlt_ps(v_nan, v_val);
    results |= _mm_movemask_ps(cmp_nlt) << 22;
    
    // UNGT vector comparison (nle)
    __m128 cmp_nle = _mm_cmpnle_ps(v_nan, v_val);
    results |= _mm_movemask_ps(cmp_nle) << 26;
    
    // Double precision SSE comparisons
    __m128d v_nan_d = _mm_set1_pd(nan_d);
    __m128d v_val_d = _mm_set1_pd(normal_d);
    
    // UNLE vector comparison for doubles (ule)
    __m128d cmp_ule = _mm_cmpule_pd(v_nan_d, v_val_d);
    results |= _mm_movemask_pd(cmp_ule) << 28;
    
    // UNLT vector comparison for doubles (ult)
    __m128d cmp_ult = _mm_cmpult_pd(v_nan_d, v_val_d);
    results |= _mm_movemask_pd(cmp_ult) << 30;
    
    // AVX comparisons (256-bit) if available
#ifdef __AVX__
    __m256 v256_nan = _mm256_set1_ps(nan_f);
    __m256 v256_val = _mm256_set1_ps(normal_f);
    
    // Various AVX unordered comparisons
    __m256 cmp256_unord = _mm256_cmp_ps(v256_nan, v256_val, _CMP_UNORD_Q);
    results |= _mm256_movemask_ps(cmp256_unord) & 0xFF;
    
    __m256 cmp256_ord = _mm256_cmp_ps(v256_val, v256_val, _CMP_ORD_Q);
    results |= (_mm256_movemask_ps(cmp256_ord) & 0xFF) << 8;
    
    __m256 cmp256_neq = _mm256_cmp_ps(v256_nan, v256_val, _CMP_NEQ_UQ);
    results |= (_mm256_movemask_ps(cmp256_neq) & 0xFF) << 16;
    
    __m256 cmp256_nlt = _mm256_cmp_ps(v256_nan, v256_val, _CMP_NLT_UQ);
    results |= (_mm256_movemask_ps(cmp256_nlt) & 0xFF) << 24;
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float a = normal_f + i;
        float b = (i % 3 == 0) ? nan_f : normal_f + i * 2;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(a, b)) {
            results ^= (1 << (i % 16));
        }
        
        // LTGT in loop
        if (__builtin_islessgreater(a, b)) {
            results ^= (1 << ((i + 1) % 16));
        }
        
        // Ordered comparison in loop
        if (!__builtin_isunordered(a, b) && a < b) {
            results ^= (1 << ((i + 2) % 16));
        }
    }
    
    // Mixed precision comparisons
    if (__builtin_isunordered(nan_f, normal_d)) {
        results ^= 0x5555;
    }
    
    if (__builtin_islessgreater(normal_f, nan_d)) {
        results ^= 0xAAAA;
    }
    
    printf("Results: 0x%08x\n", results);
    return (results != 0) ? 0 : 1;
}
