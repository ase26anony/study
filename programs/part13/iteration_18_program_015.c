#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Function to prevent optimization
volatile int sink;

// Test various unordered floating-point comparisons
void test_scalar_comparisons() {
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 1.5, d2 = 2.5;
    
    int results[32] = {0};
    int idx = 0;
    
    // UNORDERED: __builtin_isunordered
    results[idx++] = __builtin_isunordered(nan_f, f1);  // true
    results[idx++] = __builtin_isunordered(f1, nan_f);  // true
    results[idx++] = __builtin_isunordered(nan_f, nan_f); // true
    results[idx++] = __builtin_isunordered(f1, f2);     // false
    
    // ORDERED: !__builtin_isunordered
    results[idx++] = !__builtin_isunordered(f1, f2);    // true
    results[idx++] = !__builtin_isunordered(nan_f, f1); // false
    
    // UNEQ: x == y (including NaN == NaN)
    results[idx++] = (nan_f == nan_f);  // false (but unordered equal)
    results[idx++] = (f1 == f1);        // true
    results[idx++] = __builtin_isunordered(nan_f, nan_f) || (nan_f == nan_f);
    
    // UNLT: x < y (unordered)
    results[idx++] = (nan_f < f1);      // false (unordered)
    results[idx++] = (f1 < nan_f);      // false (unordered)
    
    // UNLE: x <= y (unordered)
    results[idx++] = (nan_f <= f1);     // false (unordered)
    results[idx++] = (f1 <= nan_f);     // false (unordered)
    
    // UNGT: x > y (unordered)
    results[idx++] = (nan_f > f1);      // false (unordered)
    results[idx++] = (f1 > nan_f);      // false (unordered)
    
    // UNGE: x >= y (unordered)
    results[idx++] = (nan_f >= f1);     // false (unordered)
    results[idx++] = (f1 >= nan_f);     // false (unordered)
    
    // LTGT: __builtin_islessgreater
    results[idx++] = __builtin_islessgreater(nan_f, f1);  // false
    results[idx++] = __builtin_islessgreater(f1, nan_f);  // false
    results[idx++] = __builtin_islessgreater(f1, f2);     // true
    results[idx++] = __builtin_islessgreater(f2, f1);     // true
    
    // Double precision versions
    results[idx++] = __builtin_isunordered(nan_d, d1);
    results[idx++] = (nan_d == nan_d);
    results[idx++] = (nan_d < d1);
    results[idx++] = __builtin_islessgreater(d1, d2);
    
    // Conditional branches to force code generation
    if (__builtin_isunordered(nan_f, f1)) sink = 1;
    if (!__builtin_isunordered(f1, f2)) sink = 2;
    if (nan_f == nan_f) sink = 3;  // This is false
    if (__builtin_islessgreater(f1, f2)) sink = 4;
    
    // Ternary operators
    int r1 = (nan_f < f1) ? 1 : 0;
    int r2 = (f1 <= nan_f) ? 1 : 0;
    int r3 = (nan_f > f1) ? 1 : 0;
    int r4 = (f1 >= nan_f) ? 1 : 0;
    
    sink = r1 + r2 + r3 + r4;
}

// Test SSE vector comparisons (128-bit)
void test_sse_comparisons() {
    __m128 v_nan = _mm_set1_ps(__builtin_nanf(""));
    __m128 v_val = _mm_set_ps(4.0f, 3.0f, 2.0f, 1.0f);
    __m128 v_inf = _mm_set1_ps(__builtin_inff());
    
    __m128d v_nan_d = _mm_set1_pd(__builtin_nan(""));
    __m128d v_val_d = _mm_set_pd(2.0, 1.0);
    
    // UNORDERED
    __m128 r_unord = _mm_cmpunord_ps(v_nan, v_val);
    __m128d r_unord_d = _mm_cmpunord_pd(v_nan_d, v_val_d);
    
    // ORDERED
    __m128 r_ord = _mm_cmpord_ps(v_val, v_val);
    __m128d r_ord_d = _mm_cmpord_pd(v_val_d, v_val_d);
    
    // UNEQ (not equal)
    __m128 r_uneq = _mm_cmpneq_ps(v_nan, v_val);
    __m128d r_uneq_d = _mm_cmpneq_pd(v_nan_d, v_val_d);
    
    // UNGE (not less than)
    __m128 r_nlt = _mm_cmpnlt_ps(v_nan, v_val);
    __m128d r_nlt_d = _mm_cmpnlt_pd(v_nan_d, v_val_d);
    
    // UNGT (not less than or equal)
    __m128 r_nle = _mm_cmpnle_ps(v_nan, v_val);
    __m128d r_nle_d = _mm_cmpnle_pd(v_nan_d, v_val_d);
    
    // UNLE (unordered or less than or equal)
    __m128 r_ule = _mm_cmpule_ps(v_nan, v_val);
    __m128d r_ule_d = _mm_cmpule_pd(v_nan_d, v_val_d);
    
    // UNLT (unordered or less than)
    __m128 r_ult = _mm_cmpult_ps(v_nan, v_val);
    __m128d r_ult_d = _mm_cmpult_pd(v_nan_d, v_val_d);
    
    // LTGT (unordered or not equal) - same as UNEQ for some
    __m128 r_une = _mm_cmpneq_ps(v_val, v_val);  // Compare with self
    __m128d r_une_d = _mm_cmpneq_pd(v_val_d, v_val_d);
    
    // Store results to prevent optimization
    float store_f[4];
    double store_d[2];
    _mm_store_ps(store_f, r_unord);
    _mm_store_pd(store_d, r_unord_d);
    
    sink = (int)store_f[0] + (int)store_d[0];
}

// Test AVX vector comparisons (256-bit)
#ifdef __AVX__
void test_avx_comparisons() {
    __m256 v_nan = _mm256_set1_ps(__builtin_nanf(""));
    __m256 v_val = _mm256_set_ps(8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f);
    
    __m256d v_nan_d = _mm256_set1_pd(__builtin_nan(""));
    __m256d v_val_d = _mm256_set_pd(4.0, 3.0, 2.0, 1.0);
    
    // UNORDERED
    __m256 r_unord = _mm256_cmp_ps(v_nan, v_val, _CMP_UNORD_Q);
    __m256d r_unord_d = _mm256_cmp_pd(v_nan_d, v_val_d, _CMP_UNORD_Q);
    
    // ORDERED
    __m256 r_ord = _mm256_cmp_ps(v_val, v_val, _CMP_ORD_Q);
    __m256d r_ord_d = _mm256_cmp_pd(v_val_d, v_val_d, _CMP_ORD_Q);
    
    // UNEQ
    __m256 r_uneq = _mm256_cmp_ps(v_nan, v_val, _CMP_NEQ_UQ);
    __m256d r_uneq_d = _mm256_cmp_pd(v_nan_d, v_val_d, _CMP_NEQ_UQ);
    
    // UNGE
    __m256 r_nlt = _mm256_cmp_ps(v_nan, v_val, _CMP_NLT_UQ);
    __m256d r_nlt_d = _mm256_cmp_pd(v_nan_d, v_val_d, _CMP_NLT_UQ);
    
    // UNGT
    __m256 r_nle = _mm256_cmp_ps(v_nan, v_val, _CMP_NLE_UQ);
    __m256d r_nle_d = _mm256_cmp_pd(v_nan_d, v_val_d, _CMP_NLE_UQ);
    
    // UNLE
    __m256 r_ule = _mm256_cmp_ps(v_nan, v_val, _CMP_LE_UQ);
    __m256d r_ule_d = _mm256_cmp_pd(v_nan_d, v_val_d, _CMP_LE_UQ);
    
    // UNLT
    __m256 r_ult = _mm256_cmp_ps(v_nan, v_val, _CMP_LT_UQ);
    __m256d r_ult_d = _mm256_cmp_pd(v_nan_d, v_val_d, _CMP_LT_UQ);
    
    // LTGT
    __m256 r_une = _mm256_cmp_ps(v_val, v_val, _CMP_NEQ_OQ);
    __m256d r_une_d = _mm256_cmp_pd(v_val_d, v_val_d, _CMP_NEQ_OQ);
    
    // Store results
    float store_f[8];
    double store_d[4];
    _mm256_store_ps(store_f, r_unord);
    _mm256_store_pd(store_d, r_unord_d);
    
    sink = (int)store_f[0] + (int)store_d[0];
}
#endif

// Test with loops to prevent dead code elimination
void test_with_loop() {
    float data[100];
    double data_d[100];
    
    // Initialize with some NaN values
    for (int i = 0; i < 100; i++) {
        if (i % 10 == 0) {
            data[i] = __builtin_nanf("");
            data_d[i] = __builtin_nan("");
        } else {
            data[i] = (float)i;
            data_d[i] = (double)i;
        }
    }
    
    int count_unordered = 0;
    int count_ordered = 0;
    int count_uneq = 0;
    int count_ltgt = 0;
    
    for (int i = 0; i < 99; i++) {
        // Mix of different comparisons in loop
        if (__builtin_isunordered(data[i], data[i+1])) {
            count_unordered++;
        }
        
        if (!__builtin_isunordered(data_d[i], data_d[i+1])) {
            count_ordered++;
        }
        
        if (data[i] == data[i]) {  // UNEQ when NaN
            count_uneq++;
        }
        
        if (__builtin_islessgreater(data[i], data[i+1])) {
            count_ltgt++;
        }
        
        // Generate UNLT, UNLE, UNGT, UNGE with NaN operands
        if (i % 20 == 0) {
            float nan_f = __builtin_nanf("");
            int r1 = (nan_f < data[i]) ? 1 : 0;
            int r2 = (data[i] <= nan_f) ? 1 : 0;
            int r3 = (nan_f > data[i]) ? 1 : 0;
            int r4 = (data[i] >= nan_f) ? 1 : 0;
            sink = r1 + r2 + r3 + r4;
        }
    }
    
    sink = count_unordered + count_ordered + count_uneq + count_ltgt;
}

int main() {
    printf("Testing floating-point unordered comparisons...\n");
    
    test_scalar_comparisons();
    printf("Scalar comparisons tested.\n");
    
    test_sse_comparisons();
    printf("SSE vector comparisons tested.\n");
    
#ifdef __AVX__
    test_avx_comparisons();
    printf("AVX vector comparisons tested.\n");
#endif
    
    test_with_loop();
    printf("Loop-based comparisons tested.\n");
    
    printf("All unordered comparison tests completed.\n");
    return 0;
}
