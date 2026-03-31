#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent dead code elimination
volatile int sink;

// Helper to print results
void print_result(const char* name, int result) {
    printf("%s: %d\n", name, result);
    sink = result; // Use result to prevent optimization
}

int main() {
    // Initialize NaN values
    float nan_f = NAN;
    double nan_d = NAN;
    float inf_f = INFINITY;
    float normal_f = 1.5f;
    double normal_d = 2.5;
    
    int results = 0;
    
    // 1. UNORDERED condition - using builtin
    if (__builtin_isunordered(nan_f, normal_f)) {
        results |= 1;
    }
    print_result("UNORDERED (builtin)", __builtin_isunordered(nan_f, normal_f));
    
    // 2. ORDERED condition - using builtin
    if (__builtin_isordered(normal_f, normal_d)) {
        results |= 2;
    }
    print_result("ORDERED (builtin)", __builtin_isordered(normal_f, normal_d));
    
    // 3. UNEQ condition - direct NaN comparison
    if (nan_f != nan_f) { // This is always true for NaN
        results |= 4;
    }
    print_result("UNEQ (nan != nan)", nan_f != nan_f);
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < normal_f) { // Unordered less than
        results |= 8;
    }
    print_result("UNLT (nan < normal)", nan_f < normal_f);
    
    // 5. UNLE condition - less or equal with NaN operand
    if (nan_f <= normal_f) { // Unordered less or equal
        results |= 16;
    }
    print_result("UNLE (nan <= normal)", nan_f <= normal_f);
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > normal_f) { // Unordered greater than
        results |= 32;
    }
    print_result("UNGT (nan > normal)", nan_f > normal_f);
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= normal_f) { // Unordered greater or equal
        results |= 64;
    }
    print_result("UNGE (nan >= normal)", nan_f >= normal_f);
    
    // 8. LTGT condition - using builtin
    if (__builtin_islessgreater(normal_f, normal_d)) {
        results |= 128;
    }
    print_result("LTGT (builtin)", __builtin_islessgreater(normal_f, normal_d));
    
    // SSE vector comparisons (128-bit)
    __m128 vec_nan = _mm_set1_ps(nan_f);
    __m128 vec_val = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vec_inf = _mm_set1_ps(inf_f);
    
    // 9. UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_nan, vec_val);
    int unord_mask = _mm_movemask_ps(cmp_unord);
    print_result("UNORDERED vector", unord_mask);
    
    // 10. ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(vec_val, vec_val);
    int ord_mask = _mm_movemask_ps(cmp_ord);
    print_result("ORDERED vector", ord_mask);
    
    // 11. UNEQ vector comparison
    __m128 cmp_uneq = _mm_cmpneq_ps(vec_nan, vec_val);
    int uneq_mask = _mm_movemask_ps(cmp_uneq);
    print_result("UNEQ vector", uneq_mask);
    
    // 12. UNGE vector comparison (nlt)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_nan, vec_val);
    int nlt_mask = _mm_movemask_ps(cmp_nlt);
    print_result("UNGE (nlt) vector", nlt_mask);
    
    // 13. UNGT vector comparison (nle)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_nan, vec_val);
    int nle_mask = _mm_movemask_ps(cmp_nle);
    print_result("UNGT (nle) vector", nle_mask);
    
    // 14. UNLE vector comparison (ule)
    __m128 cmp_ule = _mm_cmpule_ps(vec_nan, vec_val);
    int ule_mask = _mm_movemask_ps(cmp_ule);
    print_result("UNLE (ule) vector", ule_mask);
    
    // 15. UNLT vector comparison (ult)
    __m128 cmp_ult = _mm_cmpult_ps(vec_nan, vec_val);
    int ult_mask = _mm_movemask_ps(cmp_ult);
    print_result("UNLT (ult) vector", ult_mask);
    
    // Double precision SSE comparisons
    __m128d vec_nan_d = _mm_set1_pd(nan_d);
    __m128d vec_val_d = _mm_setr_pd(1.0, 2.0);
    
    // 16. UNORDERED double vector
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_nan_d, vec_val_d);
    int unord_d_mask = _mm_movemask_pd(cmp_unord_d);
    print_result("UNORDERED double vector", unord_d_mask);
    
    // 17. ORDERED double vector
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_val_d, vec_val_d);
    int ord_d_mask = _mm_movemask_pd(cmp_ord_d);
    print_result("ORDERED double vector", ord_d_mask);
    
    // AVX comparisons (256-bit) if available
#ifdef __AVX__
    __m256 vec_nan_avx = _mm256_set1_ps(nan_f);
    __m256 vec_val_avx = _mm256_setr_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    
    // 18. UNORDERED AVX
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_UNORD_Q);
    int unord_avx_mask = _mm256_movemask_ps(cmp_unord_avx);
    print_result("UNORDERED AVX", unord_avx_mask);
    
    // 19. ORDERED AVX
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_val_avx, vec_val_avx, _CMP_ORD_Q);
    int ord_avx_mask = _mm256_movemask_ps(cmp_ord_avx);
    print_result("ORDERED AVX", ord_avx_mask);
    
    // 20. UNEQ AVX
    __m256 cmp_uneq_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_NEQ_UQ);
    int uneq_avx_mask = _mm256_movemask_ps(cmp_uneq_avx);
    print_result("UNEQ AVX", uneq_avx_mask);
    
    // 21. UNGE AVX (nlt)
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_NLT_UQ);
    int nlt_avx_mask = _mm256_movemask_ps(cmp_nlt_avx);
    print_result("UNGE (nlt) AVX", nlt_avx_mask);
    
    // 22. UNGT AVX (nle)
    __m256 cmp_nle_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_NLE_UQ);
    int nle_avx_mask = _mm256_movemask_ps(cmp_nle_avx);
    print_result("UNGT (nle) AVX", nle_avx_mask);
    
    // 23. UNLE AVX (ule)
    __m256 cmp_ule_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_ULE_OQ);
    int ule_avx_mask = _mm256_movemask_ps(cmp_ule_avx);
    print_result("UNLE (ule) AVX", ule_avx_mask);
    
    // 24. UNLT AVX (ult)
    __m256 cmp_ult_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_ULT_OQ);
    int ult_avx_mask = _mm256_movemask_ps(cmp_ult_avx);
    print_result("UNLT (ult) AVX", ult_avx_mask);
#endif
    
    // Loop to prevent optimization and generate more comparisons
    float a = 0.0f;
    float b = 1.0f;
    int loop_results = 0;
    
    for (int i = 0; i < 10; i++) {
        a += 0.1f;
        b += 0.2f;
        
        // Generate various comparison conditions in loop
        if (__builtin_isunordered(a, b)) loop_results++;
        if (__builtin_islessgreater(a, b)) loop_results++;
        if (a != a) loop_results++; // UNEQ
        if (a < b) loop_results++;  // Could be UNLT if a becomes NaN
        if (a <= b) loop_results++; // Could be UNLE
        if (a > b) loop_results++;  // Could be UNGT
        if (a >= b) loop_results++; // Could be UNGE
    }
    
    print_result("Loop results", loop_results);
    print_result("Final aggregated results", results);
    
    return 0;
}
