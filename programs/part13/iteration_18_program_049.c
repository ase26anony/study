#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <immintrin.h>

// Prevent aggressive optimization
volatile int sink = 0;

// Function to force conditional code generation
__attribute__((noinline)) 
int use_result(int cond) {
    sink += cond;
    return cond;
}

int main() {
    int results[32] = {0};
    int idx = 0;
    
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    float normal_f = 3.14f;
    double normal_d = 2.71828;
    
    // 1. UNORDERED condition
    // Using __builtin_isunordered
    results[idx++] = use_result(__builtin_isunordered(nan_f, normal_f));
    results[idx++] = use_result(__builtin_isunordered(normal_d, nan_d));
    
    // Using direct comparison with NaN
    results[idx++] = use_result(nan_f != nan_f);  // UNORDERED check
    
    // 2. ORDERED condition
    results[idx++] = use_result(!__builtin_isunordered(normal_f, normal_f));
    results[idx++] = use_result(!__builtin_isunordered(normal_d, 1.0));
    
    // 3. UNEQ condition (unordered or equal)
    // Using direct comparison with NaN
    results[idx++] = use_result(nan_f == nan_f);  // UNEQ - always false for NaN
    results[idx++] = use_result(__builtin_isunordered(normal_f, normal_f) || normal_f == normal_f);
    
    // 4. UNLT condition (unordered or less than)
    results[idx++] = use_result(__builtin_isunordered(nan_f, normal_f) || nan_f < normal_f);
    results[idx++] = use_result(__builtin_isless(nan_f, normal_f));  // Should be false
    
    // 5. UNLE condition (unordered or less than or equal)
    results[idx++] = use_result(__builtin_isunordered(nan_d, normal_d) || nan_d <= normal_d);
    results[idx++] = use_result(__builtin_islessequal(nan_d, normal_d));
    
    // 6. UNGT condition (unordered or greater than)
    results[idx++] = use_result(__builtin_isunordered(normal_f, nan_f) || normal_f > nan_f);
    results[idx++] = use_result(__builtin_isgreater(normal_f, nan_f));
    
    // 7. UNGE condition (unordered or greater than or equal)
    results[idx++] = use_result(__builtin_isunordered(normal_d, nan_d) || normal_d >= nan_d);
    results[idx++] = use_result(__builtin_isgreaterequal(normal_d, nan_d));
    
    // 8. LTGT condition (less than or greater than, but not equal and not unordered)
    results[idx++] = use_result(__builtin_islessgreater(normal_f, normal_f + 1.0f));
    results[idx++] = use_result(__builtin_islessgreater(normal_d, normal_d - 1.0));
    
    // SSE vector comparisons (128-bit)
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 v2 = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128 v3 = _mm_setr_ps(nan_f, nan_f, nan_f, nan_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(v1, v2);
    int mask_unord = _mm_movemask_ps(cmp_unord);
    results[idx++] = use_result(mask_unord);
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(v1, v2);
    int mask_ord = _mm_movemask_ps(cmp_ord);
    results[idx++] = use_result(mask_ord);
    
    // UNEQ vector comparison (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(v1, v2);
    int mask_neq = _mm_movemask_ps(cmp_neq);
    results[idx++] = use_result(mask_neq);
    
    // UNGE vector comparison (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(v1, v2);
    int mask_nlt = _mm_movemask_ps(cmp_nlt);
    results[idx++] = use_result(mask_nlt);
    
    // UNGT vector comparison (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(v1, v2);
    int mask_nle = _mm_movemask_ps(cmp_nle);
    results[idx++] = use_result(mask_nle);
    
    // UNLE vector comparison (unordered or less than or equal)
    __m128 cmp_ule = _mm_cmpule_ps(v1, v3);  // Compare with all NaN vector
    int mask_ule = _mm_movemask_ps(cmp_ule);
    results[idx++] = use_result(mask_ule);
    
    // UNLT vector comparison (unordered or less than)
    __m128 cmp_ult = _mm_cmpult_ps(v1, v3);  // Compare with all NaN vector
    int mask_ult = _mm_movemask_ps(cmp_ult);
    results[idx++] = use_result(mask_ult);
    
    // Double precision SSE comparisons
    __m128d d1 = _mm_setr_pd(normal_d, nan_d);
    __m128d d2 = _mm_setr_pd(nan_d, normal_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(d1, d2);
    int mask_unord_d = _mm_movemask_pd(cmp_unord_d);
    results[idx++] = use_result(mask_unord_d);
    
    // AVX comparisons (256-bit) if AVX is available
#ifdef __AVX__
    __m256 v256_1 = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 v256_2 = _mm256_setr_ps(1.0f, 2.0f, nan_f, nan_f, 5.0f, 7.0f, 7.0f, 8.0f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(v256_1, v256_2, _CMP_UNORD_Q);
    int mask_unord_256 = _mm256_movemask_ps(cmp_unord_256);
    results[idx++] = use_result(mask_unord_256);
    
    __m256 cmp_ord_256 = _mm256_cmp_ps(v256_1, v256_2, _CMP_ORD_Q);
    int mask_ord_256 = _mm256_movemask_ps(cmp_ord_256);
    results[idx++] = use_result(mask_ord_256);
    
    __m256 cmp_nlt_256 = _mm256_cmp_ps(v256_1, v256_2, _CMP_NLT_UQ);
    int mask_nlt_256 = _mm256_movemask_ps(cmp_nlt_256);
    results[idx++] = use_result(mask_nlt_256);
#endif
    
    // Complex conditional expressions to force different code generation
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? normal_f : nan_f;
        float b = (i % 3 == 0) ? normal_f + i : nan_f;
        
        // Mix of different conditions in a loop
        if (__builtin_isunordered(a, b)) {
            results[idx % 32] += 1;  // UNORDERED
        }
        if (!__builtin_isunordered(a, b) && a == b) {
            results[(idx + 1) % 32] += 1;  // ORDERED + EQ
        }
        if (__builtin_islessgreater(a, b)) {
            results[(idx + 2) % 32] += 1;  // LTGT
        }
        idx = (idx + 1) % 32;
    }
    
    // Print results to prevent dead code elimination
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += results[i];
    }
    printf("Total comparison results: %d\n", sum);
    printf("Sink value: %d\n", sink);
    
    return sum > 0 ? 0 : 1;
}
