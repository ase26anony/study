#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

int main() {
    // Initialize NaN values
    float nan_f = NAN;
    double nan_d = NAN;
    float inf_f = INFINITY;
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    int result = 0;
    
    // 1. UNORDERED comparisons
    // Using __builtin_isunordered
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // Should generate UNORDERED condition
    }
    
    // Direct unordered comparison
    if (nan_d != nan_d) {  // NaN != NaN is true
        result |= 2;
    }
    
    // 2. ORDERED comparisons
    if (!__builtin_isunordered(normal_f, normal_f)) {
        result |= 4;  // Should generate ORDERED condition
    }
    
    // 3. UNEQ comparisons (unordered or equal)
    float a = nan_f;
    if (!(a < normal_f) && !(a > normal_f)) {  // UNEQ: unordered or equal
        result |= 8;
    }
    
    // 4. UNGE comparisons (unordered or greater or equal)
    if (!(nan_f < normal_f)) {  // UNGE: not less (unordered or >=)
        result |= 16;
    }
    
    // 5. UNGT comparisons (unordered or greater)
    if (!(nan_f <= normal_f)) {  // UNGT: not less or equal (unordered or >)
        result |= 32;
    }
    
    // 6. UNLE comparisons (unordered or less or equal)
    if (!(normal_f < nan_f)) {  // UNLE: unordered or <=
        result |= 64;
    }
    
    // 7. UNLT comparisons (unordered or less)
    if (!(normal_f <= nan_f)) {  // UNLT: unordered or <
        result |= 128;
    }
    
    // 8. LTGT comparisons (less or greater, but not equal and not unordered)
    if (__builtin_islessgreater(normal_f, normal_f + 1.0f)) {
        result |= 256;  // LTGT: less or greater
    }
    
    // SSE vector comparisons (128-bit)
    __m128 vec_nan = _mm_set1_ps(nan_f);
    __m128 vec_val = _mm_set1_ps(normal_f);
    __m128 vec_zero = _mm_setzero_ps();
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(cmp_unord);
    
    // ORDERED vector comparison  
    __m128 cmp_ord = _mm_cmpord_ps(vec_val, vec_val);
    sink = _mm_movemask_ps(cmp_ord);
    
    // UNEQ vector comparison
    __m128 cmp_ueq = _mm_cmpneq_ps(vec_nan, vec_val);  // Note: uneq uses neq mnemonic
    sink = _mm_movemask_ps(cmp_ueq);
    
    // UNGE vector comparison
    __m128 cmp_unge = _mm_cmpnlt_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(cmp_unge);
    
    // UNGT vector comparison
    __m128 cmp_ungt = _mm_cmpnle_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(cmp_ungt);
    
    // UNLE vector comparison
    __m128 cmp_unle = _mm_cmpule_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(cmp_unle);
    
    // UNLT vector comparison
    __m128 cmp_unlt = _mm_cmpult_ps(vec_nan, vec_val);
    sink = _mm_movemask_ps(cmp_unlt);
    
    // Double precision SSE comparisons
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    __m128d vec_val_d = _mm_set1_pd(normal_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_nan_d, vec_val_d);
    sink = _mm_movemask_pd(cmp_unord_d);
    
    // AVX comparisons (256-bit) if available
#ifdef __AVX__
    __m256 vec_nan_256 = _mm256_set1_ps(nan_f);
    __m256 vec_val_256 = _mm256_set1_ps(normal_f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(vec_nan_256, vec_val_256, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp_unord_256);
    
    __m256 cmp_ord_256 = _mm256_cmp_ps(vec_val_256, vec_val_256, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp_ord_256);
    
    __m256 cmp_ueq_256 = _mm256_cmp_ps(vec_nan_256, vec_val_256, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_ueq_256);
    
    __m256 cmp_unge_256 = _mm256_cmp_ps(vec_nan_256, vec_val_256, _CMP_NLT_UQ);
    sink = _mm256_movemask_ps(cmp_unge_256);
    
    __m256 cmp_ungt_256 = _mm256_cmp_ps(vec_nan_256, vec_val_256, _CMP_NLE_UQ);
    sink = _mm256_movemask_ps(cmp_ungt_256);
    
    __m256 cmp_unle_256 = _mm256_cmp_ps(vec_nan_256, vec_val_256, _CMP_LE_UQ);
    sink = _mm256_movemask_ps(cmp_unle_256);
    
    __m256 cmp_unlt_256 = _mm256_cmp_ps(vec_nan_256, vec_val_256, _CMP_LT_UQ);
    sink = _mm256_movemask_ps(cmp_unlt_256);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float x = (i % 2 == 0) ? nan_f : normal_f + i;
        float y = normal_f;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(x, y)) {
            result++;
        }
        
        if (!__builtin_isless(x, y)) {  // UNGE
            result++;
        }
        
        if (!__builtin_islessequal(x, y)) {  // UNGT
            result++;
        }
        
        if (!__builtin_isgreater(x, y)) {  // UNLE
            result++;
        }
        
        if (!__builtin_isgreaterequal(x, y)) {  // UNLT
            result++;
        }
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
