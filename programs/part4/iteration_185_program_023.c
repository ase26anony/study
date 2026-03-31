#include <immintrin.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifdef __AVX__
#define USE_AVX 1
#else
#define USE_AVX 0
#endif

// Function to prevent optimization
static void use_result(__m128 v) {
    volatile __m128 dummy = v;
    (void)dummy;
}

static void use_result256(__m256 v) {
    volatile __m256 dummy = v;
    (void)dummy;
}

int main() {
    // Initialize test vectors with various values including NaN
    __m128 vec1_f32 = _mm_setr_ps(1.0f, 2.0f, NAN, 4.0f);
    __m128 vec2_f32 = _mm_setr_ps(1.0f, 3.0f, 3.0f, NAN);
    __m128 vec3_f32 = _mm_setr_ps(0.0f, -0.0f, INFINITY, -INFINITY);
    
    __m128d vec1_f64 = _mm_setr_pd(1.0, NAN);
    __m128d vec2_f64 = _mm_setr_pd(2.0, 2.0);
    
    // Results accumulator
    int result_mask = 0;
    
    // Test all condition codes from the uncovered block
    
    // 1. UNORDERED (_CMP_UNORD_Q)
    __m128 cmp_unord = _mm_cmp_ps(vec1_f32, vec2_f32, _CMP_UNORD_Q);
    result_mask |= _mm_movemask_ps(cmp_unord);
    
    // 2. ORDERED (_CMP_ORD_Q)
    __m128 cmp_ord = _mm_cmp_ps(vec1_f32, vec2_f32, _CMP_ORD_Q);
    result_mask |= _mm_movemask_ps(cmp_ord);
    
    // 3. UNEQ (_CMP_UNEQ_UQ)
    __m128 cmp_uneq = _mm_cmp_ps(vec1_f32, vec2_f32, _CMP_UNEQ_UQ);
    result_mask |= _mm_movemask_ps(cmp_uneq);
    
    // 4. UNGE (_CMP_NGE_UQ) - prints "nlt"
    __m128 cmp_unge = _mm_cmp_ps(vec1_f32, vec2_f32, _CMP_NGE_UQ);
    result_mask |= _mm_movemask_ps(cmp_unge);
    
    // 5. UNGT (_CMP_NGT_UQ) - prints "nle"
    __m128 cmp_ungt = _mm_cmp_ps(vec1_f32, vec2_f32, _CMP_NGT_UQ);
    result_mask |= _mm_movemask_ps(cmp_ungt);
    
    // 6. UNLE (_CMP_ULE_UQ) - prints "ule"
    __m128 cmp_unle = _mm_cmp_ps(vec1_f32, vec2_f32, _CMP_ULE_UQ);
    result_mask |= _mm_movemask_ps(cmp_unle);
    
    // 7. UNLT (_CMP_ULT_UQ) - prints "ult"
    __m128 cmp_unlt = _mm_cmp_ps(vec1_f32, vec2_f32, _CMP_ULT_UQ);
    result_mask |= _mm_movemask_ps(cmp_unlt);
    
    // 8. LTGT (_CMP_NEQ_UQ) - prints "une"
    __m128 cmp_ltgt = _mm_cmp_ps(vec1_f32, vec2_f32, _CMP_NEQ_UQ);
    result_mask |= _mm_movemask_ps(cmp_ltgt);
    
    // Test double precision variants
    __m128d cmp_unord_d = _mm_cmp_pd(vec1_f64, vec2_f64, _CMP_UNORD_Q);
    result_mask |= _mm_movemask_pd(cmp_unord_d);
    
    __m128d cmp_ord_d = _mm_cmp_pd(vec1_f64, vec2_f64, _CMP_ORD_Q);
    result_mask |= _mm_movemask_pd(cmp_ord_d);
    
    // Test scalar comparisons (SSE)
    __m128 cmp_ss_unord = _mm_cmp_ss(vec1_f32, vec2_f32, _CMP_UNORD_Q);
    __m128d cmp_sd_ord = _mm_cmp_sd(vec1_f64, vec2_f64, _CMP_ORD_Q);
    
    // Complex expression with blending to force code generation
    __m128 blend_result = _mm_blendv_ps(vec1_f32, vec2_f32, cmp_unord);
    __m128 final_result = _mm_add_ps(blend_result, _mm_mul_ps(cmp_ord, vec3_f32));
    
    // Use results to prevent dead code elimination
    use_result(cmp_unord);
    use_result(cmp_ord);
    use_result(cmp_uneq);
    use_result(cmp_unge);
    use_result(cmp_ungt);
    use_result(cmp_unle);
    use_result(cmp_unlt);
    use_result(cmp_ltgt);
    use_result(final_result);
    
#if USE_AVX
    // AVX 256-bit vector tests if supported
    if (__builtin_cpu_supports("avx")) {
        __m256 vec1_f32_avx = _mm256_setr_ps(1.0f, 2.0f, NAN, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f);
        __m256 vec2_f32_avx = _mm256_setr_ps(1.0f, 3.0f, 3.0f, NAN, 9.0f, 10.0f, 11.0f, 12.0f);
        
        // Test AVX variants of all condition codes
        __m256 cmp_unord_avx = _mm256_cmp_ps(vec1_f32_avx, vec2_f32_avx, _CMP_UNORD_Q);
        __m256 cmp_ord_avx = _mm256_cmp_ps(vec1_f32_avx, vec2_f32_avx, _CMP_ORD_Q);
        __m256 cmp_uneq_avx = _mm256_cmp_ps(vec1_f32_avx, vec2_f32_avx, _CMP_UNEQ_UQ);
        __m256 cmp_unge_avx = _mm256_cmp_ps(vec1_f32_avx, vec2_f32_avx, _CMP_NGE_UQ);
        __m256 cmp_ungt_avx = _mm256_cmp_ps(vec1_f32_avx, vec2_f32_avx, _CMP_NGT_UQ);
        __m256 cmp_unle_avx = _mm256_cmp_ps(vec1_f32_avx, vec2_f32_avx, _CMP_ULE_UQ);
        __m256 cmp_unlt_avx = _mm256_cmp_ps(vec1_f32_avx, vec2_f32_avx, _CMP_ULT_UQ);
        __m256 cmp_ltgt_avx = _mm256_cmp_ps(vec1_f32_avx, vec2_f32_avx, _CMP_NEQ_UQ);
        
        // Use AVX results
        use_result256(cmp_unord_avx);
        use_result256(cmp_ord_avx);
        
        // Complex AVX expression
        __m256 blend_avx = _mm256_blendv_ps(vec1_f32_avx, vec2_f32_avx, cmp_unord_avx);
        use_result256(blend_avx);
    }
#endif
    
    // Force assembly output with inline asm
    __asm__ __volatile__ (
        "# Vector comparison condition codes test\n"
        : 
        : "x" (vec1_f32), "x" (vec2_f32)
        : 
    );
    
    printf("Result mask: %d\n", result_mask);
    return 0;
}
