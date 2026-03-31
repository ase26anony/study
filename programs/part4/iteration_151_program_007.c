#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    __mmask64 mask64;
    __mmask32 mask32;
    
    if (mode_selector & 1) {
        // Immediate constant mask for V64QImode
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Dynamic mask generation using comparison
        __m512i ones = _mm512_set1_epi8(1);
        __m512i cmp = _mm512_cmpeq_epi8_mask(a, ones);
        mask64 = _knot_mask64(cmp);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Now blend with V32HImode using the result
    if (mode_selector & 2) {
        mask32 = 0xAAAAAAAA;
    } else {
        __m512i cmp16 = _mm512_cmpgt_epi16_mask(result1, _mm512_setzero_si512());
        mask32 = _kor_mask32(cmp16, 0x55555555);
    }
    
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // Generate mask for V16SFmode using comparison
    if (selector > 0) {
        mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    } else {
        mask16 = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    }
    
    __m512 result_sf = _mm512_mask_blend_ps(mask16, a, b);
    
    // Generate mask for V8DFmode
    if (selector & 1) {
        mask8 = 0xAA;
    } else {
        mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result_sf), c, _CMP_NEQ_UQ);
    }
    
    __m512d result_df = _mm512_mask_blend_pd(mask8, c, d);
    
    // Chain back to single precision
    return _mm512_add_ps(result_sf, _mm512_castpd_ps(result_df));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    __mmask32 mask32;
    
    // Different mask generation patterns
    switch (pattern % 3) {
        case 0:
            mask32 = 0xAAAAAAAA;  // Immediate constant
            break;
        case 1:
            mask32 = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
            break;
        case 2:
            __mmask32 temp = _mm512_cmp_ph_mask(a, _mm512_setzero_ph(), _CMP_EQ_OQ);
            mask32 = _knot_mask32(temp);
            break;
    }
    
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int iter) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // V16SImode with dynamic mask
    if (iter & 1) {
        mask16 = _mm512_cmpeq_epi32_mask(a, b);
    } else {
        mask16 = _mm512_cmpgt_epi32_mask(a, b);
    }
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode with chained result
    mask8 = _mm512_cmpeq_epi64_mask(result_si, c);
    mask8 = _kor_mask8(mask8, (iter & 0xFF));
    
    return _mm512_mask_blend_epi64(mask8, c, d);
}

/* Reduction helpers */
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256 = _mm512_extracti64x4_epi64(v, 0);
    __m256i v256_1 = _mm512_extracti64x4_epi64(v, 1);
    v256 = _mm256_add_epi64(v256, v256_1);
    
    __m128i v128 = _mm256_extracti128_si256(v256, 0);
    __m128i v128_1 = _mm256_extracti128_si256(v256, 1);
    v128 = _mm_add_epi64(v128, v128_1);
    
    return _mm_extract_epi64(v128, 0) + _mm_extract_epi64(v128, 1);
}

static inline float reduce_add_ps(__m512 v) {
    __m256 v256 = _mm512_extractf32x8_ps(v, 0);
    __m256 v256_1 = _mm512_extractf32x8_ps(v, 1);
    v256 = _mm256_add_ps(v256, v256_1);
    
    __m128 v128 = _mm256_extractf128_ps(v256, 0);
    __m128 v128_1 = _mm256_extractf128_ps(v256, 1);
    v128 = _mm_add_ps(v128, v128_1);
    
    v128 = _mm_hadd_ps(v128, v128);
    v128 = _mm_hadd_ps(v128, v128);
    
    return _mm_cvtss_f32(v128);
}

int main() {
    double checksum = 0.0;
    
    // Initialize vectors with distinct patterns
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(0x80);
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(0x8000);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set1_epi32(0x80000000);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(0x8000000000000000ULL);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(100.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set1_pd(100.0);
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        // Data-dependent selection of blend mode
        if (i % 3 == 0) {
            // Chain blends through different modes
            __m512i temp1 = blend_v64qi_v32hi(v64qi_a, v64qi_b, i);
            __m512i temp2 = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, i);
            
            // Final blend with immediate mask
            __mmask16 mask = (i & 1) ? 0xAAAA : 0x5555;
            __m512i result = _mm512_mask_blend_epi32(mask, temp1, temp2);
            
            checksum += reduce_add_epi64(result);
        } else if (i % 3 == 1) {
            // Float blends
            __m512 float_result = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, i);
            checksum += reduce_add_ps(float_result);
            
            // Additional direct blend with comparison mask
            __mmask8 df_mask = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
            __m512d df_result = _mm512_mask_blend_pd(df_mask, v8df_a, v8df_b);
            
            // Convert to float and add to checksum
            __m512 conv_result = _mm512_add_ps(float_result, _mm512_castpd_ps(df_result));
            checksum += reduce_add_ps(conv_result);
        } else {
            // Integer blends with different mask patterns
            __mmask64 mask64;
            if (i & 4) {
                mask64 = 0x5555555555555555ULL;
            } else {
                mask64 = _mm512_cmpgt_epi8_mask(v64qi_a, _mm512_set1_epi8(32));
            }
            
            __m512i qi_result = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
            checksum += reduce_add_epi64(qi_result);
            
            // V32HImode with immediate mask
            __m512i hi_result = _mm512_mask_blend_epi16(0xAAAAAAAA, v32hi_a, v32hi_b);
            checksum += reduce_add_epi64(hi_result);
        }
        
        #ifdef __AVX512FP16__
        if (i % 2 == 0) {
            // Initialize FP16 vectors
            __m512h v32hf_a = _mm512_set_ph(
                31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
                23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
                15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
            );
            
            __m512h v32hf_b = _mm512_set1_ph(50.0f);
            
            __m512h hf_result = blend_v32hf_v32bf(v32hf_a, v32hf_b, i);
            
            // Convert to float for reduction
            __m512 float_hf = _mm512_cvtph_ps(hf_result);
            checksum += reduce_add_ps(float_hf);
        }
        #endif
    }
    
    // Final direct blends to ensure all modes are covered
    __mmask16 final_mask16 = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_NEQ_UQ);
    __m512 final_sf = _mm512_mask_blend_ps(final_mask16, v16sf_a, v16sf_b);
    checksum += reduce_add_ps(final_sf);
    
    __mmask8 final_mask8 = 0xF0;
    __m512d final_df = _mm512_mask_blend_pd(final_mask8, v8df_a, v8df_b);
    checksum += _mm512_reduce_add_pd(final_df);
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
