#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent constant folding
volatile float vf1, vf2;
volatile double vd1, vd2;

// Function to force conditional branch generation
__attribute__((noinline))
int check_condition(int cond) {
    return cond;
}

int main() {
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    // Regular floating point values
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14159, d2 = 2.71828;
    
    int results[32] = {0};
    int idx = 0;
    
    // 1. UNORDERED condition - using builtin
    results[idx++] = check_condition(__builtin_isunordered(nan_f, f1));
    results[idx++] = check_condition(__builtin_isunordered(f1, nan_f));
    results[idx++] = check_condition(__builtin_isunordered(nan_d, d1));
    results[idx++] = check_condition(__builtin_isunordered(d1, nan_d));
    
    // 2. ORDERED condition - using builtin
    results[idx++] = check_condition(!__builtin_isunordered(f1, f2));
    results[idx++] = check_condition(!__builtin_isunordered(d1, d2));
    
    // 3. UNEQ condition (unordered or equal)
    // Using direct NaN comparison
    results[idx++] = check_condition(nan_f == nan_f);  // UNEQ: unordered or equal
    results[idx++] = check_condition(nan_d == nan_d);
    
    // 4. UNLT condition (unordered or less than)
    results[idx++] = check_condition(nan_f < f1);      // UNLT: unordered or less than
    results[idx++] = check_condition(f1 < nan_f);
    results[idx++] = check_condition(nan_d < d1);
    results[idx++] = check_condition(d1 < nan_d);
    
    // 5. UNLE condition (unordered or less than or equal)
    results[idx++] = check_condition(nan_f <= f1);     // UNLE: unordered or less than or equal
    results[idx++] = check_condition(f1 <= nan_f);
    results[idx++] = check_condition(nan_d <= d1);
    results[idx++] = check_condition(d1 <= nan_d);
    
    // 6. UNGT condition (unordered or greater than)
    results[idx++] = check_condition(nan_f > f1);      // UNGT: unordered or greater than
    results[idx++] = check_condition(f1 > nan_f);
    results[idx++] = check_condition(nan_d > d1);
    results[idx++] = check_condition(d1 > nan_d);
    
    // 7. UNGE condition (unordered or greater than or equal)
    results[idx++] = check_condition(nan_f >= f1);     // UNGE: unordered or greater than or equal
    results[idx++] = check_condition(f1 >= nan_f);
    results[idx++] = check_condition(nan_d >= d1);
    results[idx++] = check_condition(d1 >= nan_d);
    
    // 8. LTGT condition (less than or greater than, but not equal and not unordered)
    results[idx++] = check_condition(__builtin_islessgreater(f1, f2));
    results[idx++] = check_condition(__builtin_islessgreater(d1, d2));
    results[idx++] = check_condition(__builtin_islessgreater(f1, nan_f));
    results[idx++] = check_condition(__builtin_islessgreater(nan_d, d1));
    
    // SSE intrinsics for explicit condition codes
    __m128 a = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_set_ps(4.0f, 2.0f, 3.0f, nan_f);
    __m128d ad = _mm_set_pd(nan_d, 1.0);
    __m128d bd = _mm_set_pd(2.0, nan_d);
    
    // UNORDERED with SSE
    __m128 cmp_unord = _mm_cmpunord_ps(a, b);
    __m128d cmp_unord_d = _mm_cmpunord_pd(ad, bd);
    
    // ORDERED with SSE
    __m128 cmp_ord = _mm_cmpord_ps(a, b);
    __m128d cmp_ord_d = _mm_cmpord_pd(ad, bd);
    
    // UNEQ with SSE (not equal)
    __m128 cmp_neq = _mm_cmpneq_ps(a, b);
    __m128d cmp_neq_d = _mm_cmpneq_pd(ad, bd);
    
    // UNGE with SSE (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(a, b);
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(ad, bd);
    
    // UNGT with SSE (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(a, b);
    __m128d cmp_nle_d = _mm_cmpnle_pd(ad, bd);
    
    // UNLE with SSE (unordered or less than or equal)
    __m128 cmp_ule = _mm_cmpule_ps(a, b);
    __m128d cmp_ule_d = _mm_cmpule_pd(ad, bd);
    
    // UNLT with SSE (unordered or less than)
    __m128 cmp_ult = _mm_cmpult_ps(a, b);
    __m128d cmp_ult_d = _mm_cmpult_pd(ad, bd);
    
    // AVX intrinsics if available
#ifdef __AVX__
    __m256 a256 = _mm256_set_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, 6.0f, nan_f, 8.0f);
    __m256 b256 = _mm256_set_ps(8.0f, 7.0f, 6.0f, nan_f, 4.0f, 3.0f, 2.0f, nan_f);
    __m256d ad256 = _mm256_set_pd(nan_d, 3.0, nan_d, 1.0);
    __m256d bd256 = _mm256_set_pd(4.0, nan_d, 2.0, nan_d);
    
    // Various AVX comparisons
    __m256 cmp_unord256 = _mm256_cmp_ps(a256, b256, _CMP_UNORD_Q);
    __m256d cmp_unord_d256 = _mm256_cmp_pd(ad256, bd256, _CMP_UNORD_Q);
    
    __m256 cmp_ord256 = _mm256_cmp_ps(a256, b256, _CMP_ORD_Q);
    __m256d cmp_ord_d256 = _mm256_cmp_pd(ad256, bd256, _CMP_ORD_Q);
    
    __m256 cmp_neq256 = _mm256_cmp_ps(a256, b256, _CMP_NEQ_UQ);
    __m256d cmp_neq_d256 = _mm256_cmp_pd(ad256, bd256, _CMP_NEQ_UQ);
    
    __m256 cmp_nlt256 = _mm256_cmp_ps(a256, b256, _CMP_NLT_UQ);
    __m256d cmp_nlt_d256 = _mm256_cmp_pd(ad256, bd256, _CMP_NLT_UQ);
    
    __m256 cmp_nle256 = _mm256_cmp_ps(a256, b256, _CMP_NLE_UQ);
    __m256d cmp_nle_d256 = _mm256_cmp_pd(ad256, bd256, _CMP_NLE_UQ);
    
    __m256 cmp_ule256 = _mm256_cmp_ps(a256, b256, _CMP_LE_UQ);
    __m256d cmp_ule_d256 = _mm256_cmp_pd(ad256, bd256, _CMP_LE_UQ);
    
    __m256 cmp_ult256 = _mm256_cmp_ps(a256, b256, _CMP_LT_UQ);
    __m256d cmp_ult_d256 = _mm256_cmp_pd(ad256, bd256, _CMP_LT_UQ);
    
    // Store to volatile to prevent optimization
    vf1 = ((float*)&cmp_unord256)[0];
    vd1 = ((double*)&cmp_unord_d256)[0];
#endif
    
    // Store SSE results to volatile to prevent optimization
    vf1 = ((float*)&cmp_unord)[0];
    vf2 = ((float*)&cmp_ord)[1];
    vd1 = ((double*)&cmp_unord_d)[0];
    vd2 = ((double*)&cmp_ord_d)[1];
    
    // Loop to prevent dead code elimination and generate more comparisons
    float sum = 0.0f;
    for (int i = 0; i < 10; i++) {
        float x = (i % 2 == 0) ? (float)i : nan_f;
        float y = (i % 3 == 0) ? nan_f : (float)(i * 2);
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(x, y)) {
            sum += 1.0f;  // UNORDERED
        }
        if (!__builtin_isunordered(x, y)) {
            sum += 2.0f;  // ORDERED
        }
        if (x == x) {
            sum += 0.5f;  // UNEQ (NaN == NaN is false, but triggers the code)
        }
        if (x < y) {
            sum += 1.5f;  // UNLT
        }
        if (x <= y) {
            sum += 2.5f;  // UNLE
        }
        if (x > y) {
            sum += 3.5f;  // UNGT
        }
        if (x >= y) {
            sum += 4.5f;  // UNGE
        }
        if (__builtin_islessgreater(x, y)) {
            sum += 5.5f;  // LTGT
        }
    }
    
    // Print results to ensure code isn't optimized away
    printf("Results: ");
    for (int i = 0; i < idx && i < 32; i++) {
        printf("%d ", results[i]);
    }
    printf("\nSum: %f\n", sum);
    
    return 0;
}
