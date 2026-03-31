#include <stdio.h>
#include <math.h>
#include <immintrin.h>
#include <stdint.h>

// Prevent optimization
volatile int sink = 0;

// Function to use results and prevent dead code elimination
__attribute__((noinline)) void use_result(int val) {
    sink += val;
}

__attribute__((noinline)) void use_vector(__m128 v) {
    float f = _mm_cvtss_f32(v);
    sink += (int)f;
}

__attribute__((noinline)) void use_vector_d(__m128d v) {
    double d = _mm_cvtsd_f64(v);
    sink += (int)d;
}

#ifdef __AVX__
__attribute__((noinline)) void use_vector_avx(__m256 v) {
    float f = _mm256_cvtss_f32(v);
    sink += (int)f;
}

__attribute__((noinline)) void use_vector_avx_d(__m256d v) {
    double d = _mm256_cvtsd_f64(v);
    sink += (int)d;
}
#endif

int main() {
    int result = 0;
    
    // Create NaN values
    float nan_f = NAN;
    float inf_f = INFINITY;
    float normal_f = 3.14f;
    float zero_f = 0.0f;
    
    double nan_d = NAN;
    double inf_d = INFINITY;
    double normal_d = 2.718281828459045;
    double zero_d = 0.0;
    
    // 1. UNORDERED condition (__builtin_isunordered)
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1;  // Should be true
    }
    
    if (__builtin_isunordered(normal_f, normal_f)) {
        result |= 2;  // Should be false
    }
    
    // 2. ORDERED condition (opposite of unordered)
    if (!__builtin_isunordered(normal_f, normal_f)) {
        result |= 4;  // Should be true
    }
    
    // 3. UNEQ condition (unordered or equal)
    // Using direct comparison with NaN
    if (nan_f != nan_f) {  // NaN != NaN is true (unordered)
        result |= 8;
    }
    
    // 4. UNLT condition (unordered or less than)
    if (nan_f < normal_f) {  // NaN < anything is false (unordered)
        result |= 16;
    } else {
        result |= 32;  // Take the else branch
    }
    
    // 5. UNLE condition (unordered or less than or equal)
    if (nan_f <= normal_f) {  // NaN <= anything is false
        result |= 64;
    } else {
        result |= 128;
    }
    
    // 6. UNGT condition (unordered or greater than)
    if (nan_f > normal_f) {  // NaN > anything is false
        result |= 256;
    } else {
        result |= 512;
    }
    
    // 7. UNGE condition (unordered or greater than or equal)
    if (nan_f >= normal_f) {  // NaN >= anything is false
        result |= 1024;
    } else {
        result |= 2048;
    }
    
    // 8. LTGT condition (less than or greater than, but not equal and not unordered)
    if (__builtin_islessgreater(normal_f, zero_f)) {
        result |= 4096;  // 3.14 != 0.0, so true
    }
    
    if (__builtin_islessgreater(nan_f, normal_f)) {
        result |= 8192;  // NaN comparison, false
    }
    
    // Double precision versions
    if (__builtin_isunordered(nan_d, normal_d)) {
        result |= 16384;
    }
    
    if (nan_d != nan_d) {
        result |= 32768;
    }
    
    // SSE intrinsics for explicit condition codes
    __m128 vec_nan = _mm_set1_ps(NAN);
    __m128 vec_val = _mm_set1_ps(1.0f);
    __m128 vec_zero = _mm_set1_ps(0.0f);
    
    // UNORDERED: _mm_cmpunord_ps
    __m128 cmp_unord = _mm_cmpunord_ps(vec_nan, vec_val);
    use_vector(cmp_unord);
    
    // ORDERED: _mm_cmpord_ps
    __m128 cmp_ord = _mm_cmpord_ps(vec_val, vec_zero);
    use_vector(cmp_ord);
    
    // UNEQ: _mm_cmpneq_ps (not equal, which includes unordered)
    __m128 cmp_uneq = _mm_cmpneq_ps(vec_nan, vec_val);
    use_vector(cmp_uneq);
    
    // UNGE: _mm_cmpnlt_ps (not less than)
    __m128 cmp_nlt = _mm_cmpnlt_ps(vec_nan, vec_val);
    use_vector(cmp_nlt);
    
    // UNGT: _mm_cmpnle_ps (not less than or equal)
    __m128 cmp_nle = _mm_cmpnle_ps(vec_nan, vec_val);
    use_vector(cmp_nle);
    
    // UNLE: _mm_cmpule_ps (unordered or less than or equal)
    __m128 cmp_ule = _mm_cmpule_ps(vec_nan, vec_val);
    use_vector(cmp_ule);
    
    // UNLT: _mm_cmpult_ps (unordered or less than)
    __m128 cmp_ult = _mm_cmpult_ps(vec_nan, vec_val);
    use_vector(cmp_ult);
    
    // Double precision SSE
    __m128d vec_nan_d = _mm_set1_pd(NAN);
    __m128d vec_val_d = _mm_set1_pd(1.0);
    
    __m128d cmp_unord_d = _mm_cmpunord_pd(vec_nan_d, vec_val_d);
    use_vector_d(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmpord_pd(vec_val_d, _mm_set1_pd(0.0));
    use_vector_d(cmp_ord_d);
    
    // AVX versions if available
#ifdef __AVX__
    __m256 vec_nan_avx = _mm256_set1_ps(NAN);
    __m256 vec_val_avx = _mm256_set1_ps(1.0f);
    
    __m256 cmp_unord_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_UNORD_Q);
    use_vector_avx(cmp_unord_avx);
    
    __m256 cmp_ord_avx = _mm256_cmp_ps(vec_val_avx, _mm256_set1_ps(0.0f), _CMP_ORD_Q);
    use_vector_avx(cmp_ord_avx);
    
    __m256 cmp_neq_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_NEQ_UQ);
    use_vector_avx(cmp_neq_avx);
    
    __m256 cmp_nlt_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_NLT_UQ);
    use_vector_avx(cmp_nlt_avx);
    
    __m256 cmp_nle_avx = _mm256_cmp_ps(vec_nan_avx, vec_val_avx, _CMP_NLE_UQ);
    use_vector_avx(cmp_nle_avx);
    
    // Double precision AVX
    __m256d vec_nan_avx_d = _mm256_set1_pd(NAN);
    __m256d vec_val_avx_d = _mm256_set1_pd(1.0);
    
    __m256d cmp_unord_avx_d = _mm256_cmp_pd(vec_nan_avx_d, vec_val_avx_d, _CMP_UNORD_Q);
    use_vector_avx_d(cmp_unord_avx_d);
#endif
    
    // Loop to generate more comparison patterns
    float arr1[4] = {NAN, 1.0f, 2.0f, NAN};
    float arr2[4] = {1.0f, NAN, 2.0f, 3.0f};
    
    for (int i = 0; i < 4; i++) {
        // Generate various unordered comparisons in a loop
        if (__builtin_isunordered(arr1[i], arr2[i])) {
            result += i * 100;
        }
        
        if (arr1[i] != arr1[i]) {  // UNEQ pattern
            result += i * 200;
        }
        
        if (arr1[i] < arr2[i]) {  // May generate UNLT
            result += i * 300;
        }
        
        if (arr1[i] > arr2[i]) {  // May generate UNGT
            result += i * 400;
        }
    }
    
    // Use results to prevent optimization
    printf("Result: %d\n", result);
    printf("Sink: %d\n", sink);
    
    return result != 0 ? 0 : 1;
}
