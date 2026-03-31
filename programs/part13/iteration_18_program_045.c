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
    double inf_d = INFINITY;
    
    float f1 = 1.0f, f2 = 2.0f;
    double d1 = 1.0, d2 = 2.0;
    
    int result = 0;
    
    // 1. UNORDERED condition - using builtin
    if (__builtin_isunordered(f1, nan_f)) {
        result |= 1;
    }
    
    // 2. ORDERED condition - using builtin
    if (__builtin_isordered(f1, f2)) {
        result |= 2;
    }
    
    // 3. UNEQ condition - equality with NaN
    if (f1 != nan_f) {  // This generates UNEQ when comparing with NaN
        result |= 4;
    }
    
    // 4. UNGE condition - greater or equal with NaN operand
    if (!(f1 < nan_f)) {  // Equivalent to UNGE: not less than
        result |= 8;
    }
    
    // 5. UNGT condition - greater than with NaN operand  
    if (!(f1 <= nan_f)) {  // Equivalent to UNGT: not less or equal
        result |= 16;
    }
    
    // 6. UNLE condition - less or equal with NaN operand
    if (nan_f <= f1) {  // This should generate UNLE
        result |= 32;
    }
    
    // 7. UNLT condition - less than with NaN operand
    if (nan_f < f1) {  // This should generate UNLT
        result |= 64;
    }
    
    // 8. LTGT condition - using builtin
    if (__builtin_islessgreater(f1, nan_f)) {
        result |= 128;
    }
    
    // SSE vector comparisons (128-bit)
    __m128 vf1 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    __m128 vf_nan = _mm_set_ps(NAN, 2.0f, NAN, 4.0f);
    __m128 vf2 = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
    
    // Generate various unordered comparison conditions with SSE
    __m128 cmp_unord = _mm_cmpunord_ps(vf1, vf_nan);  // UNORDERED
    __m128 cmp_ord = _mm_cmpord_ps(vf1, vf2);         // ORDERED
    __m128 cmp_neq = _mm_cmpneq_ps(vf1, vf_nan);      // UNEQ
    __m128 cmp_nlt = _mm_cmpnlt_ps(vf1, vf_nan);      // UNGE
    __m128 cmp_nle = _mm_cmpnle_ps(vf1, vf_nan);      // UNGT
    __m128 cmp_ule = _mm_cmpule_ps(vf_nan, vf1);      // UNLE
    __m128 cmp_ult = _mm_cmpult_ps(vf_nan, vf1);      // UNLT
    __m128 cmp_une = _mm_cmpneq_ps(vf1, vf2);         // LTGT (same as UNEQ)
    
    // Double precision SSE comparisons
    __m128d vd1 = _mm_set_pd(1.0, 2.0);
    __m128d vd_nan = _mm_set_pd(NAN, 2.0);
    __m128d vd2 = _mm_set_pd(3.0, 4.0);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vd1, vd_nan);
    __m128d cmp_ord_d = _mm_cmpord_pd(vd1, vd2);
    __m128d cmp_neq_d = _mm_cmpneq_pd(vd1, vd_nan);
    __m128d cmp_nlt_d = _mm_cmpnlt_pd(vd1, vd_nan);
    __m128d cmp_nle_d = _mm_cmpnle_pd(vd1, vd_nan);
    __m128d cmp_ule_d = _mm_cmpule_pd(vd_nan, vd1);
    __m128d cmp_ult_d = _mm_cmpult_pd(vd_nan, vd1);
    __m128d cmp_une_d = _mm_cmpneq_pd(vd1, vd2);
    
    // AVX comparisons (256-bit) if AVX is available
#ifdef __AVX__
    __m256 vf1_256 = _mm256_set_ps(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
    __m256 vf_nan_256 = _mm256_set_ps(NAN, 2.0f, NAN, 4.0f, NAN, 6.0f, NAN, 8.0f);
    
    __m256 cmp_unord_256 = _mm256_cmp_ps(vf1_256, vf_nan_256, _CMP_UNORD_Q);
    __m256 cmp_ord_256 = _mm256_cmp_ps(vf1_256, vf_nan_256, _CMP_ORD_Q);
    __m256 cmp_neq_256 = _mm256_cmp_ps(vf1_256, vf_nan_256, _CMP_NEQ_UQ);
    __m256 cmp_nlt_256 = _mm256_cmp_ps(vf1_256, vf_nan_256, _CMP_NLT_UQ);
    __m256 cmp_nle_256 = _mm256_cmp_ps(vf1_256, vf_nan_256, _CMP_NLE_UQ);
    __m256 cmp_ule_256 = _mm256_cmp_ps(vf_nan_256, vf1_256, _CMP_LE_OQ);
    __m256 cmp_ult_256 = _mm256_cmp_ps(vf_nan_256, vf1_256, _CMP_LT_OQ);
    __m256 cmp_une_256 = _mm256_cmp_ps(vf1_256, vf_nan_256, _CMP_NEQ_OQ);
#endif
    
    // Loop to prevent dead code elimination and generate more comparisons
    for (int i = 0; i < 10; i++) {
        float dynamic_f = (i % 2 == 0) ? (float)i : NAN;
        double dynamic_d = (i % 3 == 0) ? (double)i : NAN;
        
        // Generate various conditions in loop
        if (__builtin_isunordered(dynamic_f, f1)) {
            sink = i;
        }
        
        if (dynamic_d != dynamic_d) {  // UNEQ with NaN
            sink = i * 2;
        }
        
        if (!(dynamic_f < f1)) {  // UNGE
            sink = i * 3;
        }
        
        if (__builtin_islessgreater(dynamic_f, f1)) {  // LTGT
            sink = i * 4;
        }
    }
    
    // Use results to prevent optimization
    float* fp_results = (float*)&cmp_unord;
    printf("Result: %d\n", result);
    printf("First SSE comparison result: %f\n", fp_results[0]);
    
    return 0;
}
