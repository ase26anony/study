#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent optimization
volatile float vf1, vf2;
volatile double vd1, vd2;

// Function to force conditional jumps
__attribute__((noinline))
int check_condition(int cond) {
    return cond;
}

int main() {
    int results = 0;
    
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    // Regular float/double values
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 6.28;
    
    // 1. UNORDERED condition
    // Using __builtin_isunordered
    if (__builtin_isunordered(nan_f, f1)) {
        results |= 1 << 0;
    }
    
    // Direct unordered comparison
    if (nan_d != nan_d) {  // NaN != NaN is true
        results |= 1 << 1;
    }
    
    // 2. ORDERED condition (negation of unordered)
    if (!__builtin_isunordered(f1, f2)) {
        results |= 1 << 2;
    }
    
    // 3. UNEQ condition (unordered or equal)
    // Compare NaN with itself
    if (!(nan_f < nan_f) && !(nan_f > nan_f)) {  // UNEQ: !(a < b) && !(a > b)
        results |= 1 << 3;
    }
    
    // 4. UNGE condition (unordered or greater or equal)
    // Using __builtin_isgreaterequal with NaN
    if (__builtin_isgreaterequal(nan_f, f1)) {  // UNGE: !(a < b)
        results |= 1 << 4;
    }
    
    // 5. UNGT condition (unordered or greater)
    // Using __builtin_isgreater with NaN
    if (__builtin_isgreater(nan_f, f1)) {  // UNGT: !(a <= b)
        results |= 1 << 5;
    }
    
    // 6. UNLE condition (unordered or less or equal)
    // Using __builtin_islessequal with NaN
    if (__builtin_islessequal(nan_f, f1)) {  // UNLE: !(a > b)
        results |= 1 << 6;
    }
    
    // 7. UNLT condition (unordered or less)
    // Using __builtin_isless with NaN
    if (__builtin_isless(nan_f, f1)) {  // UNLT: !(a >= b)
        results |= 1 << 7;
    }
    
    // 8. LTGT condition (less or greater, but not equal and not unordered)
    // Using __builtin_islessgreater
    if (__builtin_islessgreater(f1, f2)) {  // LTGT: (a < b) || (a > b)
        results |= 1 << 8;
    }
    
    // SSE intrinsics for explicit condition codes
    __m128 a = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_set_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128d ad = _mm_set_pd(1.0, nan_d);
    __m128d bd = _mm_set_pd(nan_d, 2.0);
    
    // UNORDERED
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    results += _mm_movemask_ps(cmp_unord);
    
    // ORDERED
    __m128 cmp_ord = _mm_cmpord_ps(a, b);
    results += _mm_movemask_ps(cmp_ord);
    
    // UNEQ (unordered or equal)
    __m128 cmp_uneq = _mm_cmpneq_ps(a, b);  // Note: _CMP_NEQ_UQ corresponds to UNEQ
    results += _mm_movemask_ps(cmp_uneq);
    
    // UNGE (unordered or greater or equal) - using nlt
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
    results += _mm_movemask_ps(cmp_nlt);
    
    // UNGT (unordered or greater) - using nle
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);
    results += _mm_movemask_ps(cmp_nle);
    
    // UNLE (unordered or less or equal) - using ule
    // Note: There's no direct _mm_cmpule_ps, so we use combination
    __m128 cmp_ule = _mm_cmple_ps(a, b);  // This includes ordered case
    results += _mm_movemask_ps(cmp_ule);
    
    // UNLT (unordered or less) - using ult
    __m128 cmp_ult = _mm_cmplt_ps(a, b);  // This includes ordered case
    results += _mm_movemask_ps(cmp_ult);
    
    // Double precision versions
    __m128d cmp_unord_d = _mm_cmpunord_pd(ad, bd);
    results += _mm_movemask_pd(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(ad, bd);
    results += _mm_movemask_pd(cmp_ord_d);
    
    // AVX versions if available
#ifdef __AVX__
    __m256 a256 = _mm256_set_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 b256 = _mm256_set_ps(1.0f, 3.0f, 3.0f, nan_f, 5.0f, 7.0f, 7.0f, nan_f);
    
    __m256 cmp_unord256 = _mm256_cmp_ps(a256, b256, _CMP_UNORD_Q);
    results += _mm256_movemask_ps(cmp_unord256);
    
    __m256 cmp_ord256 = _mm256_cmp_ps(a256, b256, _CMP_ORD_Q);
    results += _mm256_movemask_ps(cmp_ord256);
    
    __m256 cmp_neq_uq256 = _mm256_cmp_ps(a256, b256, _CMP_NEQ_UQ);  // UNEQ
    results += _mm256_movemask_ps(cmp_neq_uq256);
    
    __m256 cmp_nlt256 = _mm256_cmp_ps(a256, b256, _CMP_NLT_UQ);  // UNGE
    results += _mm256_movemask_ps(cmp_nlt256);
    
    __m256 cmp_nle256 = _mm256_cmp_ps(a256, b256, _CMP_NLE_UQ);  // UNGT
    results += _mm256_movemask_ps(cmp_nle256);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float t = (float)i;
        double td = (double)i;
        
        // Generate various conditions in loop
        if (__builtin_isunordered(t, nan_f)) {
            results++;
        }
        
        if (__builtin_islessgreater(t, t + 1.0f)) {
            results++;
        }
        
        // Mix with volatile to prevent optimization
        vf1 = t;
        vf2 = nan_f;
        if (vf1 < vf2) {  // This should generate UNLT
            results++;
        }
    }
    
    printf("Results: %d\n", results);
    return results > 0 ? 0 : 1;
}
