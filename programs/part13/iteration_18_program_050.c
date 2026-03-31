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

__attribute__((noinline)) void use_vector256(__m256 v) {
    float f = _mm256_cvtss_f32(v);
    sink += (int)f;
}

int main() {
    int result = 0;
    
    // Create NaN values
    float nan_f = __builtin_nanf("");
    double nan_d = __builtin_nan("");
    float inf_f = __builtin_inff();
    float normal_f = 3.14f;
    double normal_d = 2.718;
    
    // 1. UNORDERED condition - using builtin
    if (__builtin_isunordered(nan_f, normal_f)) {
        result |= 1; // Should trigger UNORDERED
    }
    
    // 2. ORDERED condition - using builtin
    if (!__builtin_isunordered(normal_f, normal_f)) {
        result |= 2; // Should trigger ORDERED
    }
    
    // 3. UNEQ condition - equality with NaN
    if (nan_f != nan_f) { // Always false, but generates UNEQ
        // This branch won't be taken, but the comparison generates UNEQ
    }
    
    // Force UNEQ with explicit comparison
    int uneq_result = (nan_f == nan_f) ? 0 : 1; // Generates UNEQ
    use_result(uneq_result);
    
    // 4. UNLT condition - less than with NaN operand
    if (nan_f < normal_f) { // Generates UNLT
        // Won't be taken
    }
    
    // 5. UNLE condition - less or equal with NaN operand
    if (nan_f <= normal_f) { // Generates UNLE
        // Won't be taken
    }
    
    // 6. UNGT condition - greater than with NaN operand
    if (nan_f > normal_f) { // Generates UNGT
        // Won't be taken
    }
    
    // 7. UNGE condition - greater or equal with NaN operand
    if (nan_f >= normal_f) { // Generates UNGE
        // Won't be taken
    }
    
    // 8. LTGT condition - using builtin
    if (__builtin_islessgreater(nan_f, normal_f)) { // Generates LTGT
        // Won't be taken
    }
    
    // Now use SSE intrinsics to generate specific comparison mnemonics
    
    // SSE single-precision comparisons
    __m128 a = _mm_setr_ps(1.0f, 2.0f, nan_f, 4.0f);
    __m128 b = _mm_setr_ps(1.0f, 3.0f, 3.0f, nan_f);
    
    // UNORDERED: _mm_cmpunord_ps
    __m128 unord_mask = _mm_cmpunord_ps(a, b);
    use_vector(unord_mask);
    
    // ORDERED: _mm_cmpord_ps
    __m128 ord_mask = _mm_cmpord_ps(a, b);
    use_vector(ord_mask);
    
    // UNEQ: _mm_cmpneq_ps (with NaN operands)
    __m128 uneq_mask = _mm_cmpneq_ps(a, b);
    use_vector(uneq_mask);
    
    // UNGE: _mm_cmpnlt_ps
    __m128 unge_mask = _mm_cmpnlt_ps(a, b);
    use_vector(unge_mask);
    
    // UNGT: _mm_cmpnle_ps
    __m128 ungt_mask = _mm_cmpnle_ps(a, b);
    use_vector(ungt_mask);
    
    // UNLE: _mm_cmpule_ps (note: ule suffix)
    __m128 unle_mask = _mm_cmpule_ps(a, b);
    use_vector(unle_mask);
    
    // UNLT: _mm_cmpult_ps
    __m128 unlt_mask = _mm_cmpult_ps(a, b);
    use_vector(unlt_mask);
    
    // SSE double-precision comparisons
    __m128d ad = _mm_setr_pd(normal_d, nan_d);
    __m128d bd = _mm_setr_pd(normal_d, normal_d);
    
    __m128d unord_mask_d = _mm_cmpunord_pd(ad, bd);
    use_vector_d(unord_mask_d);
    
    __m128d ord_mask_d = _mm_cmpord_pd(ad, bd);
    use_vector_d(ord_mask_d);
    
    __m128d uneq_mask_d = _mm_cmpneq_pd(ad, bd);
    use_vector_d(uneq_mask_d);
    
    // AVX comparisons (256-bit vectors)
#ifdef __AVX__
    __m256 a256 = _mm256_setr_ps(1.0f, nan_f, 3.0f, 4.0f, 
                                  5.0f, 6.0f, nan_f, 8.0f);
    __m256 b256 = _mm256_setr_ps(1.0f, 2.0f, nan_f, 4.0f,
                                  5.0f, nan_f, 7.0f, 8.0f);
    
    __m256 unord_mask256 = _mm256_cmp_ps(a256, b256, _CMP_UNORD_Q);
    use_vector256(unord_mask256);
    
    __m256 ord_mask256 = _mm256_cmp_ps(a256, b256, _CMP_ORD_Q);
    use_vector256(ord_mask256);
    
    __m256 uneq_mask256 = _mm256_cmp_ps(a256, b256, _CMP_NEQ_UQ);
    use_vector256(uneq_mask256);
    
    __m256 unge_mask256 = _mm256_cmp_ps(a256, b256, _CMP_NLT_UQ);
    use_vector256(unge_mask256);
    
    __m256 ungt_mask256 = _mm256_cmp_ps(a256, b256, _CMP_NLE_UQ);
    use_vector256(ungt_mask256);
    
    __m256 unle_mask256 = _mm256_cmp_ps(a256, b256, _CMP_LE_OS);
    use_vector256(unle_mask256);
    
    __m256 unlt_mask256 = _mm256_cmp_ps(a256, b256, _CMP_LT_OS);
    use_vector256(unlt_mask256);
#endif
    
    // Loop to generate more comparisons with varying values
    for (int i = 0; i < 10; i++) {
        float x = (i % 2 == 0) ? normal_f : nan_f;
        float y = (i % 3 == 0) ? normal_f : nan_f;
        
        // Generate various unordered comparisons in loop
        if (__builtin_isunordered(x, y)) {
            result++;
        }
        
        if (__builtin_islessgreater(x, y)) {
            result--;
        }
        
        // Direct comparisons that generate unordered condition codes
        int cmp1 = (x < y) ? 1 : 0;  // May generate UNLT
        int cmp2 = (x > y) ? 1 : 0;  // May generate UNGT
        int cmp3 = (x <= y) ? 1 : 0; // May generate UNLE
        int cmp4 = (x >= y) ? 1 : 0; // May generate UNGE
        
        use_result(cmp1 + cmp2 + cmp3 + cmp4);
    }
    
    // Mixed precision comparisons
    double mixed_nan = nan_f; // float NaN promoted to double
    if (__builtin_isunordered(mixed_nan, normal_d)) {
        result |= 4;
    }
    
    // Complex expression that might generate LTGT
    float arr[4] = {normal_f, nan_f, inf_f, -inf_f};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (__builtin_islessgreater(arr[i], arr[j])) {
                result++;
            }
            if (!__builtin_isunordered(arr[i], arr[j]) && 
                arr[i] != arr[j]) {
                result--; // May generate ORDERED + UNEQ combinations
            }
        }
    }
    
    printf("Result: %d (sink: %d)\n", result, sink);
    return result != 0 ? 0 : 1;
}
