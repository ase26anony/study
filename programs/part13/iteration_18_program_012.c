#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

// Test various unordered comparison conditions
void test_unordered_comparisons() {
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    float f1 = 1.0f, f2 = 2.0f;
    double d1 = 1.0, d2 = 2.0;
    
    int results[32] = {0};
    int idx = 0;
    
    // 1. UNORDERED condition - using __builtin_isunordered
    results[idx++] = __builtin_isunordered(nan_f, f1);  // Should be true
    results[idx++] = __builtin_isunordered(f1, nan_f);  // Should be true
    results[idx++] = __builtin_isunordered(f1, f2);     // Should be false
    
    // 2. ORDERED condition - opposite of unordered
    results[idx++] = !__builtin_isunordered(f1, f2);    // Should be true
    results[idx++] = !__builtin_isunordered(nan_f, f1); // Should be false
    
    // 3. UNEQ condition (unordered or equal)
    // Using direct NaN comparison
    results[idx++] = (nan_f == nan_f) ? 1 : 0;          // NaN != NaN, but UNEQ might be used
    results[idx++] = (f1 == f1) ? 1 : 0;                // Should be true
    
    // 4. UNLT condition (unordered or less than)
    results[idx++] = (nan_f < f1) ? 1 : 0;              // Unordered comparison
    results[idx++] = (f1 < nan_f) ? 1 : 0;              // Unordered comparison
    results[idx++] = (f1 < f2) ? 1 : 0;                 // Ordered comparison
    
    // 5. UNLE condition (unordered or less than or equal)
    results[idx++] = (nan_f <= f1) ? 1 : 0;             // Unordered comparison
    results[idx++] = (f1 <= nan_f) ? 1 : 0;             // Unordered comparison
    results[idx++] = (f1 <= f2) ? 1 : 0;                // Ordered comparison
    
    // 6. UNGT condition (unordered or greater than)
    results[idx++] = (nan_f > f1) ? 1 : 0;              // Unordered comparison
    results[idx++] = (f1 > nan_f) ? 1 : 0;              // Unordered comparison
    results[idx++] = (f2 > f1) ? 1 : 0;                 // Ordered comparison
    
    // 7. UNGE condition (unordered or greater than or equal)
    results[idx++] = (nan_f >= f1) ? 1 : 0;             // Unordered comparison
    results[idx++] = (f1 >= nan_f) ? 1 : 0;             // Unordered comparison
    results[idx++] = (f2 >= f1) ? 1 : 0;                // Ordered comparison
    
    // 8. LTGT condition (less than or greater than, but not equal and not unordered)
    results[idx++] = __builtin_islessgreater(f1, f2);   // Should be true (1 < 2)
    results[idx++] = __builtin_islessgreater(f1, f1);   // Should be false (equal)
    results[idx++] = __builtin_islessgreater(nan_f, f1); // Should be false (unordered)
    
    // Store results to prevent optimization
    for (int i = 0; i < idx; i++) {
        sink = results[i];
    }
}

// Test SSE intrinsics for unordered comparisons
void test_sse_unordered_comparisons() {
    __m128 v1 = _mm_setr_ps(1.0f, 2.0f, __builtin_nanf(""), 4.0f);
    __m128 v2 = _mm_setr_ps(4.0f, 2.0f, 3.0f, __builtin_nanf(""));
    __m128 v3 = _mm_setr_ps(1.0f, 1.0f, 1.0f, 1.0f);
    
    __m128d d1 = _mm_setr_pd(1.0, __builtin_nan(""));
    __m128d d2 = _mm_setr_pd(__builtin_nan(""), 2.0);
    
    // Test various unordered comparison intrinsics
    __m128 cmp_result;
    __m128d cmp_result_d;
    
    // UNORDERED
    cmp_result = _mm_cmpunord_ps(v1, v2);  // Compare for unordered
    sink = _mm_movemask_ps(cmp_result);
    
    // ORDERED
    cmp_result = _mm_cmpord_ps(v1, v2);    // Compare for ordered
    sink = _mm_movemask_ps(cmp_result);
    
    // UNEQ (not equal)
    cmp_result = _mm_cmpneq_ps(v1, v2);    // Compare for not equal
    sink = _mm_movemask_ps(cmp_result);
    
    // UNGE (not less than)
    cmp_result = _mm_cmpnlt_ps(v1, v2);    // Compare for not less than
    sink = _mm_movemask_ps(cmp_result);
    
    // UNGT (not less than or equal)
    cmp_result = _mm_cmpnle_ps(v1, v2);    // Compare for not less than or equal
    sink = _mm_movemask_ps(cmp_result);
    
    // UNLE (unordered or less than or equal) - using SSE4.1 _mm_cmp_ps with _CMP_LE_OS
    cmp_result = _mm_cmple_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_result);
    
    // UNLT (unordered or less than) - using SSE4.1 _mm_cmp_ps with _CMP_LT_OS
    cmp_result = _mm_cmplt_ps(v1, v2);
    sink = _mm_movemask_ps(cmp_result);
    
    // Double precision versions
    cmp_result_d = _mm_cmpunord_pd(d1, d2);
    sink = _mm_movemask_pd(cmp_result_d);
    
    cmp_result_d = _mm_cmpord_pd(d1, d2);
    sink = _mm_movemask_pd(cmp_result_d);
    
    cmp_result_d = _mm_cmpneq_pd(d1, d2);
    sink = _mm_movemask_pd(cmp_result_d);
    
    cmp_result_d = _mm_cmpnlt_pd(d1, d2);
    sink = _mm_movemask_pd(cmp_result_d);
    
    cmp_result_d = _mm_cmpnle_pd(d1, d2);
    sink = _mm_movemask_pd(cmp_result_d);
}

// Test AVX intrinsics for unordered comparisons
#ifdef __AVX__
void test_avx_unordered_comparisons() {
    __m256 v1 = _mm256_setr_ps(1.0f, 2.0f, __builtin_nanf(""), 4.0f,
                                5.0f, 6.0f, 7.0f, __builtin_nanf(""));
    __m256 v2 = _mm256_setr_ps(4.0f, 2.0f, 3.0f, __builtin_nanf(""),
                                8.0f, 6.0f, 7.0f, 9.0f);
    
    __m256d d1 = _mm256_setr_pd(1.0, __builtin_nan(""), 3.0, 4.0);
    __m256d d2 = _mm256_setr_pd(__builtin_nan(""), 2.0, 3.0, __builtin_nan(""));
    
    __m256 cmp_result;
    __m256d cmp_result_d;
    
    // UNORDERED
    cmp_result = _mm256_cmp_ps(v1, v2, _CMP_UNORD_Q);
    sink = _mm256_movemask_ps(cmp_result);
    
    // ORDERED
    cmp_result = _mm256_cmp_ps(v1, v2, _CMP_ORD_Q);
    sink = _mm256_movemask_ps(cmp_result);
    
    // UNEQ
    cmp_result = _mm256_cmp_ps(v1, v2, _CMP_NEQ_UQ);
    sink = _mm256_movemask_ps(cmp_result);
    
    // UNGE (not less than)
    cmp_result = _mm256_cmp_ps(v1, v2, _CMP_NLT_US);
    sink = _mm256_movemask_ps(cmp_result);
    
    // UNGT (not less than or equal)
    cmp_result = _mm256_cmp_ps(v1, v2, _CMP_NLE_US);
    sink = _mm256_movemask_ps(cmp_result);
    
    // UNLE
    cmp_result = _mm256_cmp_ps(v1, v2, _CMP_LE_OS);
    sink = _mm256_movemask_ps(cmp_result);
    
    // UNLT
    cmp_result = _mm256_cmp_ps(v1, v2, _CMP_LT_OS);
    sink = _mm256_movemask_ps(cmp_result);
    
    // Double precision
    cmp_result_d = _mm256_cmp_pd(d1, d2, _CMP_UNORD_Q);
    sink = _mm256_movemask_pd(cmp_result_d);
    
    cmp_result_d = _mm256_cmp_pd(d1, d2, _CMP_ORD_Q);
    sink = _mm256_movemask_pd(cmp_result_d);
    
    cmp_result_d = _mm256_cmp_pd(d1, d2, _CMP_NEQ_UQ);
    sink = _mm256_movemask_pd(cmp_result_d);
}
#endif

// Test conditional branches based on unordered comparisons
void test_conditional_branches() {
    float nan_f = __builtin_nanf("");
    float f1 = 1.0f, f2 = 2.0f;
    double nan_d = __builtin_nan("");
    double d1 = 1.0, d2 = 2.0;
    
    int branch_taken = 0;
    
    // Branch on UNORDERED
    if (__builtin_isunordered(nan_f, f1)) {
        branch_taken |= 1;
    }
    
    // Branch on ORDERED
    if (!__builtin_isunordered(f1, f2)) {
        branch_taken |= 2;
    }
    
    // Branch on UNEQ (using NaN comparison)
    if (nan_f != nan_f) {  // This is always true for NaN != NaN
        branch_taken |= 4;
    }
    
    // Branch on UNLT
    if (nan_f < f1) {  // Unordered comparison
        branch_taken |= 8;
    }
    
    // Branch on UNLE
    if (nan_f <= f1) {  // Unordered comparison
        branch_taken |= 16;
    }
    
    // Branch on UNGT
    if (nan_f > f1) {  // Unordered comparison
        branch_taken |= 32;
    }
    
    // Branch on UNGE
    if (nan_f >= f1) {  // Unordered comparison
        branch_taken |= 64;
    }
    
    // Branch on LTGT
    if (__builtin_islessgreater(f1, f2)) {
        branch_taken |= 128;
    }
    
    sink = branch_taken;
}

int main() {
    printf("Testing unordered floating-point comparisons...\n");
    
    // Run tests multiple times to ensure coverage
    for (int i = 0; i < 10; i++) {
        test_unordered_comparisons();
        test_sse_unordered_comparisons();
        #ifdef __AVX__
        test_avx_unordered_comparisons();
        #endif
        test_conditional_branches();
    }
    
    printf("Tests completed.\n");
    return 0;
}
