#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent dead code elimination
volatile int sink;

// Helper to print results
void print_result(const char* name, int result) {
    printf("%s: %d\n", name, result);
    sink = result; // Use volatile sink to prevent optimization
}

int main() {
    // Initialize NaN values
    float nan_f = NAN;
    double nan_d = NAN;
    float inf_f = INFINITY;
    float normal_f = 1.5f;
    double normal_d = 2.7;
    
    int results[32] = {0};
    int idx = 0;
    
    // 1. UNORDERED condition - using builtin
    results[idx++] = __builtin_isunordered(nan_f, normal_f);
    results[idx++] = __builtin_isunordered(normal_d, nan_d);
    
    // 2. ORDERED condition - using builtin (negation of unordered)
    results[idx++] = !__builtin_isunordered(normal_f, normal_f);
    results[idx++] = !__builtin_isunordered(nan_f, nan_f); // Should be 0
    
    // 3. UNEQ condition (unordered or equal)
    // Using direct NaN comparison
    results[idx++] = (nan_f != nan_f) ? 1 : 0;  // UNEQ when NaN
    results[idx++] = (normal_f == normal_f) ? 1 : 0;  // Not UNEQ
    
    // 4. UNLT condition (unordered or less than)
    results[idx++] = (nan_f < normal_f) ? 1 : 0;
    results[idx++] = (normal_f < nan_f) ? 1 : 0;
    
    // 5. UNLE condition (unordered or less than or equal)
    results[idx++] = (nan_f <= normal_f) ? 1 : 0;
    results[idx++] = (normal_f <= nan_f) ? 1 : 0;
    
    // 6. UNGT condition (unordered or greater than)
    results[idx++] = (nan_f > normal_f) ? 1 : 0;
    results[idx++] = (normal_f > nan_f) ? 1 : 0;
    
    // 7. UNGE condition (unordered or greater than or equal)
    results[idx++] = (nan_f >= normal_f) ? 1 : 0;
    results[idx++] = (normal_f >= nan_f) ? 1 : 0;
    
    // 8. LTGT condition (less than or greater than, but not equal and not unordered)
    results[idx++] = __builtin_islessgreater(normal_f, normal_f + 1.0f);
    results[idx++] = __builtin_islessgreater(normal_f, normal_f);  // Should be 0
    
    // SSE vector comparisons (128-bit)
    __m128 vec_nan = _mm_set1_ps(nan_f);
    __m128 vec_val = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_inf = _mm_set1_ps(inf_f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_nan, vec_val);
    float unord_result[4];
    _mm_store_ps(unord_result, cmp_unord);
    results[idx++] = unord_result[0] != 0;
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(vec_val, vec_val);
    float ord_result[4];
    _mm_store_ps(ord_result, cmp_ord);
    results[idx++] = ord_result[0] != 0;
    
    // UNEQ vector comparison (not equal)
    __m128 cmp_uneq = _mm_cmpneq_ps(vec_nan, vec_val);
    float uneq_result[4];
    _mm_store_ps(uneq_result, cmp_uneq);
    results[idx++] = uneq_result[0] != 0;
    
    // UNGE vector comparison (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_nan, vec_val);
    float nlt_result[4];
    _mm_store_ps(nlt_result, cmp_nlt);
    results[idx++] = nlt_result[0] != 0;
    
    // UNGT vector comparison (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_nan, vec_val);
    float nle_result[4];
    _mm_store_ps(nle_result, cmp_nle);
    results[idx++] = nle_result[0] != 0;
    
    // UNLE vector comparison (unordered or less than or equal)
    __m128 cmp_ule = _mm_cmpule_ps(vec_nan, vec_val);
    float ule_result[4];
    _mm_store_ps(ule_result, cmp_ule);
    results[idx++] = ule_result[0] != 0;
    
    // UNLT vector comparison (unordered or less than)
    __m128 cmp_ult = _mm_cmpult_ps(vec_nan, vec_val);
    float ult_result[4];
    _mm_store_ps(ult_result, cmp_ult);
    results[idx++] = ult_result[0] != 0;
    
    // Double precision SSE comparisons
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    __m128d vec_val_d = _mm_set_pd(1.0, 2.0);
    
    // UNORDERED double
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_nan_d, vec_val_d);
    double unord_d_result[2];
    _mm_store_pd(unord_d_result, cmp_unord_d);
    results[idx++] = unord_d_result[0] != 0;
    
    // ORDERED double
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_val_d, vec_val_d);
    double ord_d_result[2];
    _mm_store_pd(ord_d_result, cmp_ord_d);
    results[idx++] = ord_d_result[0] != 0;
    
    // Conditional branches to force condition code generation
    if (__builtin_isunordered(nan_f, normal_f)) {
        results[idx++] = 100;
    }
    
    if (!__builtin_isunordered(normal_d, normal_d)) {
        results[idx++] = 200;
    }
    
    if (nan_f != nan_f) {
        results[idx++] = 300;
    }
    
    if (__builtin_islessgreater(normal_f, normal_f + 1.0f)) {
        results[idx++] = 400;
    }
    
    // Loop with varying values to prevent optimization
    float sum = 0.0f;
    for (int i = 0; i < 10; i++) {
        float a = (i % 2 == 0) ? nan_f : (float)i;
        float b = (i % 3 == 0) ? nan_f : (float)(i * 2);
        
        // Generate various condition codes in loop
        if (__builtin_isunordered(a, b)) sum += 1.0f;
        if (!__builtin_isunordered(a, b)) sum += 2.0f;
        if (a != a) sum += 3.0f;
        if (__builtin_islessgreater(a, b)) sum += 4.0f;
        if (a < b) sum += 5.0f;
        if (a <= b) sum += 6.0f;
        if (a > b) sum += 7.0f;
        if (a >= b) sum += 8.0f;
    }
    
    results[idx++] = (int)sum;
    
    // Print summary
    int total = 0;
    for (int i = 0; i < idx; i++) {
        total += results[i];
    }
    
    printf("Total checks: %d\n", idx);
    printf("Sum of results: %d\n", total);
    
    return total > 0 ? 0 : 1;
}
