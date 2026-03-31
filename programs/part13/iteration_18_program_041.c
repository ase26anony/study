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
    float inf_f = __builtin_inff();
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    int result = 0;
    
    // 1. UNORDERED comparisons
    // Using __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // UNORDERED
    }
    
    // Direct comparison with NaN
    if (nan_f != nan_f) {  // This is always true for NaN
        result |= 2;  // UNORDERED/UNEQ
    }
    
    // 2. ORDERED comparisons
    if (!__builtin_isunordered(normal_f, normal_d)) {
        result |= 4;  // ORDERED
    }
    
    // 3. UNEQ (unordered or equal)
    // Compare NaN with itself using equality
    if (!(nan_f == nan_f)) {  // NaN != NaN, so this branch is taken
        result |= 8;  // UNEQ
    }
    
    // 4. UNLT (unordered or less than)
    if (!(normal_f < nan_f)) {  // normal_f < NaN is false, but unordered
        result |= 16;  // UNLT
    }
    
    // 5. UNLE (unordered or less than or equal)
    if (!(normal_f <= nan_f)) {  // normal_f <= NaN is false, but unordered
        result |= 32;  // UNLE
    }
    
    // 6. UNGT (unordered or greater than)
    if (!(nan_f > normal_f)) {  // NaN > normal_f is false, but unordered
        result |= 64;  // UNGT
    }
    
    // 7. UNGE (unordered or greater than or equal)
    if (!(nan_f >= normal_f)) {  // NaN >= normal_f is false, but unordered
        result |= 128;  // UNGE
    }
    
    // 8. LTGT (less than or greater than, but not equal and not unordered)
    if (__builtin_islessgreater(normal_f, normal_f + 1.0f)) {
        result |= 256;  // LTGT
    }
    
    // SSE vector comparisons (128-bit)
    __m128 vec_nan = _mm_set1_ps(nan_f);
    __m128 vec_val = _mm_set1_ps(normal_f);
    __m128 vec_result;
    
    // UNORDERED vector comparison
    vec_result = _mm_cmpunord_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(vec_result);
    
    // ORDERED vector comparison  
    vec_result = _mm_cmpord_ps(vec_val, vec_val);
    sink = _mm_movemask_ps(vec_result);
    
    // UNEQ vector comparison
    vec_result = _mm_cmpneq_ps(vec_nan, vec_nan);
    sink = _mm_movemask_ps(vec_result);
    
    // UNGE vector comparison (nlt)
    vec_result = _mm_cmpnlt_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(vec_result);
    
    // UNGT vector comparison (nle)
    vec_result = _mm_cmpnle_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(vec_result);
    
    // UNLE vector comparison (ule)
    vec_result = _mm_cmpule_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(vec_result);
    
    // UNLT vector comparison (ult)
    vec_result = _mm_cmpult_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(vec_result);
    
    // Double precision SSE comparisons
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    __m128d vec_val_d = _mm_set1_pd(normal_d);
    __m128d dbl_result;
    
    dbl_result = _mm_cmpunord_pd(vec_nan_d, vec_val_d);
    sink = _mm_movemask_pd(dbl_result);
    
    dbl_result = _mm_cmpneq_pd(vec_nan_d, vec_nan_d);  // LTGT/UNEQ
    sink = _mm_movemask_pd(dbl_result);
    
    // AVX comparisons (256-bit) if available
#ifdef __AVX__
    __m256 avx_nan = _mm256_set1_ps(nan_f);
    __m256 avx_val = _mm256_set1_ps(normal_f);
    __m256 avx_result;
    
    avx_result = _mm256_cmpunord_ps(avx_nan, avx_val);
    sink = _mm256_movemask_ps(avx_result);
    
    avx_result = _mm256_cmpord_ps(avx_val, avx_val);
    sink = _mm256_movemask_ps(avx_result);
    
    avx_result = _mm256_cmpneq_ps(avx_nan, avx_nan);
    sink = _mm256_movemask_ps(avx_result);
    
    // Double precision AVX
    __m256d avx_nan_d = _mm256_set1_pd(nan_d);
    __m256d avx_val_d = _mm256_set1_pd(normal_d);
    __m256d avx_dbl_result;
    
    avx_dbl_result = _mm256_cmpunord_pd(avx_nan_d, avx_val_d);
    sink = _mm256_movemask_pd(avx_dbl_result);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? nan_f : normal_f + i;
        float b = (i % 3 == 0) ? nan_f : normal_f - i;
        
        // Generate various unordered conditions in loop
        if (__builtin_isunordered(a, b)) {
            result += i;  // UNORDERED
        }
        
        if (!(a == b)) {  // Could be UNEQ or LTGT depending on values
            result -= i;
        }
        
        // Generate UNLT/UNLE/UNGT/UNGE based on values
        if (!(a < b)) {  // Could be UNGE
            result ^= i;
        }
        
        if (!(a > b)) {  // Could be UNLE
            result |= i << 4;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
