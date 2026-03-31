#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent optimization
volatile float vf1, vf2;
volatile double vd1, vd2;
volatile int vi;

// Function to force conditional jumps
__attribute__((noinline))
int use_condition(int cond) {
    return cond;
}

int main() {
    // Initialize NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    double inf_d = __builtin_inf();
    
    float f1 = 1.5f, f2 = 2.5f;
    double d1 = 3.14, d2 = 2.71;
    
    int results = 0;
    
    // 1. UNORDERED condition
    // Using __builtin_isunordered
    if (__builtin_isunordered(nan_f, f1)) {
        results |= 1;
    }
    
    // Direct comparison with NaN
    if (nan_f != nan_f) {  // This is always true for NaN
        results |= 2;
    }
    
    // 2. ORDERED condition
    if (!__builtin_isunordered(f1, f2)) {
        results |= 4;
    }
    
    // 3. UNEQ condition (unordered or equal)
    // Compare NaN with itself - unordered equal
    if (nan_f == nan_f) {  // This is false for NaN, but generates UNEQ
        // This branch won't be taken, but generates the comparison
    }
    
    // Using __builtin_islessgreater for LTGT
    if (__builtin_islessgreater(f1, f2)) {
        results |= 8;
    }
    
    // 4. UNLT condition (unordered or less than)
    if (nan_f < f1) {  // Unordered comparison with NaN
        // Branch won't be taken but generates UNLT
    }
    
    // 5. UNLE condition (unordered or less than or equal)
    if (nan_f <= f1) {  // Unordered comparison with NaN
        // Branch won't be taken but generates UNLE
    }
    
    // 6. UNGT condition (unordered or greater than)
    if (nan_f > f1) {  // Unordered comparison with NaN
        // Branch won't be taken but generates UNGT
    }
    
    // 7. UNGE condition (unordered or greater than or equal)
    if (nan_f >= f1) {  // Unordered comparison with NaN
        // Branch won't be taken but generates UNGE
    }
    
    // 8. LTGT condition (less than or greater than, but not equal)
    if (__builtin_islessgreater(d1, d2)) {
        results |= 16;
    }
    
    // SSE vector comparisons (128-bit)
    __m128 vec_f1 = _mm_set_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 vec_f2 = _mm_set_ps(1.0f, 3.0f, 3.0f, nan_f);
    __m128 vec_f3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    // UNORDERED vector comparison
    __m128 cmp_unord = _mm_cmpunord_ps(vec_f1, vec_f2);
    
    // ORDERED vector comparison
    __m128 cmp_ord = _mm_cmpord_ps(vec_f1, vec_f2);
    
    // UNEQ vector comparison
    __m128 cmp_uneq = _mm_cmpneq_ps(vec_f1, vec_f2);
    
    // UNGE vector comparison (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_f1, vec_f3);
    
    // UNGT vector comparison (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_f1, vec_f3);
    
    // UNLE vector comparison
    __m128 cmp_ule = _mm_cmpule_ps(vec_f1, vec_f3);
    
    // UNLT vector comparison
    __m128 cmp_ult = _mm_cmpult_ps(vec_f1, vec_f3);
    
    // Double precision SSE comparisons
    __m128d vec_d1 = _mm_set_pd(nan_d, 1.0);
    __m128d vec_d2 = _mm_set_pd(2.0, nan_d);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_d1, vec_d2);
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_d1, vec_d2);
    __m128d cmp_uneq_d = _mm_cmpneq_pd(vec_d1, vec_d2);
    
    // AVX comparisons (256-bit) if AVX is available
#ifdef __AVX__
    __m256 vec256_f1 = _mm256_set_ps(1.0f, 2.0f, nan_f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vec256_f2 = _mm256_set_ps(1.0f, 3.0f, 3.0f, nan_f, 5.0f, 6.0f, nan_f, 8.0f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_UNORD_Q);
    __m256 cmp_ord_256 = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_ORD_Q);
    __m256 cmp_uneq_256 = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NEQ_UQ);
    __m256 cmp_nlt_256 = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NLT_UQ);
    __m256 cmp_nle_256 = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_NLE_UQ);
    __m256 cmp_ule_256 = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_LE_UQ);
    __m256 cmp_ult_256 = _mm256_cmp_ps(vec256_f1, vec256_f2, _CMP_LT_UQ);
    
    // Store to volatile to prevent optimization
    float temp[8];
    _mm256_storeu_ps(temp, cmp_unord_256);
    vi = (int)temp[0];
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float a = (float)i + 0.5f;
        float b = (i % 2 == 0) ? nan_f : (float)i + 1.0f;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(a, b)) {
            results++;
        }
        
        if (__builtin_islessgreater(a, b)) {
            results += 2;
        }
        
        // Ordered comparison
        if (!__builtin_isunordered(a, b) && a < b) {
            results += 3;
        }
    }
    
    // Mix with volatile variables to force actual code generation
    vf1 = nan_f;
    vf2 = f1;
    
    // Force conditional jumps with unordered comparisons
    if (__builtin_isunordered(vf1, vf2)) {
        vi = 1;
    }
    
    if (!__builtin_isunordered(vf1, vf2)) {
        vi = 2;
    }
    
    if (__builtin_islessgreater(vf1, vf2)) {
        vi = 3;
    }
    
    // Print results to prevent optimization
    printf("Results: %d\n", results);
    
    return 0;
}
