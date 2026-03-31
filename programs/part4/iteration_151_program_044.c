#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __AVX512FP16__
#include <math.h>
#endif

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    __m512i res1 = _mm512_mask_blend_epi8(k8, a, b);
    __m512i res2 = _mm512_mask_blend_epi16(k16, res1, b);
    return _mm512_add_epi8(res1, res2);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, 
                                      __mmask16 k32, __mmask8 k64) {
    __m512 res1 = _mm512_mask_blend_ps(k32, a, b);
    __m512d res2 = _mm512_mask_blend_pd(k64, c, d);
    // Convert double result back to float for chaining
    __m512 res2f = _mm512_cvtpd_ps(res2);
    return _mm512_add_ps(res1, res2f);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 k16) {
    __m512h res1 = _mm512_mask_blend_ph(k16, a, b);
    // For BF16, we use the same intrinsic since it's stored in __m512h
    __m512h res2 = _mm512_mask_blend_ph(k16 ^ 0xAAAAAAAA, b, a);
    return _mm512_add_ph(res1, res2);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __mmask16 k32, __mmask8 k64) {
    __m512i res1 = _mm512_mask_blend_epi32(k32, a, b);
    __m512i res2 = _mm512_mask_blend_epi64(k64, res1, b);
    return _mm512_add_epi32(res1, res2);
}

/* Mask generation helpers */
static __mmask64 generate_mask64_pattern(int pattern_type) {
    if (pattern_type == 0) {
        return 0xAAAAAAAAAAAAAAAA;  // Alternating pattern
    } else if (pattern_type == 1) {
        return 0x5555555555555555;  // Inverse alternating
    } else {
        return 0xFFFFFFFFFFFFFFFF;  // All ones
    }
}

static __mmask32 generate_mask32_from_comparison(__m512i a, __m512i b) {
    return _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_GT);
}

static __mmask16 generate_mask16_from_float_comp(__m512 a, __m512 b) {
    return _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
}

static __mmask8 generate_mask8_from_double_comp(__m512d a, __m512d b) {
    return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
}

int main() {
    volatile int seed = 42;  // Prevent constant propagation
    int i, iterations = 100;
    float checksum = 0.0f;
    
    // Initialize vectors with distinct patterns
    __m512i vi64qi_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vi64qi_b = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi32hi_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i vi32hi_b = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi16si_a = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i vi16si_b = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i vi8di_b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    __m512 vf16sf_a = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 vf16sf_b = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d vf8df_a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d vf8df_b = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
#ifdef __AVX512FP16__
    __m512h vf32hf_a = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    
    __m512h vf32hf_b = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
#endif
    
    // Generate various masks
    __mmask64 mk64_imm = generate_mask64_pattern(seed & 3);
    __mmask32 mk32_comp = generate_mask32_from_comparison(vi32hi_a, vi32hi_b);
    __mmask16 mk16_comp = generate_mask16_from_float_comp(vf16sf_a, vf16sf_b);
    __mmask8 mk8_comp = generate_mask8_from_double_comp(vf8df_a, vf8df_b);
    
    // Combine masks using bitwise operations
    __mmask32 mk32_combined = _kor_mask32(mk32_comp, 0x55555555);
    __mmask16 mk16_combined = _kxor_mask16(mk16_comp, 0xAAAA);
    __mmask8 mk8_combined = _knot_mask8(mk8_comp);
    
    // Data-dependent control flow with loops
    for (i = 0; i < iterations; i++) {
        __m512i result_int;
        __m512 result_float;
        
        if (i % 3 == 0) {
            // V64QImode blend with immediate mask
            result_int = _mm512_mask_blend_epi8(mk64_imm, vi64qi_a, vi64qi_b);
        } else if (i % 3 == 1) {
            // V32HImode blend with comparison mask
            result_int = _mm512_mask_blend_epi16(mk32_comp, vi32hi_a, vi32hi_b);
        } else {
            // V16SImode blend with combined mask
            result_int = _mm512_mask_blend_epi32(mk16_combined, vi16si_a, vi16si_b);
        }
        
        // Chained blend operations
        __m512i chained = blend_v64qi_v32hi(vi64qi_a, vi64qi_b, mk64_imm, mk32_combined);
        result_int = _mm512_add_epi32(result_int, chained);
        
        // V8DImode blend
        __m512i result_di = _mm512_mask_blend_epi64(mk8_combined, vi8di_a, vi8di_b);
        result_int = _mm512_add_epi64(result_int, result_di);
        
        // More chaining with different types
        result_int = blend_v16si_v8di(result_int, vi16si_b, mk16_combined, mk8_comp);
        
        // Floating point blends with data-dependent selection
        if (i % 2 == 0) {
            result_float = _mm512_mask_blend_ps(mk16_comp, vf16sf_a, vf16sf_b);
        } else {
            __m512d result_double = _mm512_mask_blend_pd(mk8_comp, vf8df_a, vf8df_b);
            result_float = _mm512_cvtpd_ps(result_double);
        }
        
        // Chain float and double blends
        result_float = blend_v16sf_v8df(result_float, vf16sf_b, vf8df_a, vf8df_b, 
                                       mk16_combined, mk8_combined);
        
#ifdef __AVX512FP16__
        // Half-precision blends (V32HFmode and V32BFmode)
        if (i % 4 == 0) {
            __m512h result_half = _mm512_mask_blend_ph(mk32_combined, vf32hf_a, vf32hf_b);
            // Convert to float for checksum
            __m512 result_half_f = _mm512_cvtph_ps(result_half);
            result_float = _mm512_add_ps(result_float, result_half_f);
        } else {
            __m512h result_half = blend_v32hf_v32bf(vf32hf_a, vf32hf_b, mk32_combined);
            __m512 result_half_f = _mm512_cvtph_ps(result_half);
            result_float = _mm512_add_ps(result_float, result_half_f);
        }
#endif
        
        // Reductions to prevent dead code elimination
        // Integer reduction
        __m256i hi = _mm512_extracti64x4_epi64(result_int, 1);
        __m256i lo = _mm512_extracti64x4_epi64(result_int, 0);
        __m256i sum256 = _mm256_add_epi64(hi, lo);
        __m128i hi128 = _mm256_extracti128_si256(sum256, 1);
        __m128i lo128 = _mm256_extracti128_si256(sum256, 0);
        __m128i sum128 = _mm_add_epi64(hi128, lo128);
        uint64_t int_sum = _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
        
        // Float reduction
        float float_sum = _mm512_reduce_add_ps(result_float);
        
        checksum += float_sum + (float)int_sum;
        
        // Modify masks slightly each iteration
        mk64_imm = (mk64_imm << 1) | (mk64_imm >> 63);
        mk32_combined = _knot_mask32(mk32_combined);
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
