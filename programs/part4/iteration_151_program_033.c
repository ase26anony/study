#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int selector) {
    __mmask64 mask64;
    
    /* Varied mask generation: immediate constant */
    if (selector & 1) {
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        /* Dynamic mask generation via comparison */
        __m512i cmp_a = _mm512_set1_epi8(selector);
        __m512i cmp_b = _mm512_set1_epi8(selector ^ 0xFF);
        mask64 = _mm512_cmpgt_epi8_mask(cmp_a, cmp_b);
    }
    
    __m512i result64 = _mm512_mask_blend_epi8(mask64, a, b);
    
    /* Chain to V32HImode blend */
    __mmask32 mask32;
    if (selector & 2) {
        mask32 = 0x55555555;
    } else {
        /* Generate mask via bitwise operations */
        __mmask32 temp_mask = _mm512_cmpgt_epi16_mask(
            _mm512_set1_epi16(selector),
            _mm512_set1_epi16(0)
        );
        mask32 = _knot_mask32(temp_mask);
    }
    
    return _mm512_mask_blend_epi16(mask32, result64, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    __mmask16 mask16;
    
    /* Different mask generation patterns */
    if (selector % 3 == 0) {
        mask16 = 0xAAAA;
    } else if (selector % 3 == 1) {
        /* Comparison-based mask */
        mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    } else {
        /* Complex mask generation */
        __mmask16 mask1 = _mm512_cmp_ps_mask(a, _mm512_set1_ps(0.0f), _CMP_GT_OQ);
        __mmask16 mask2 = _mm512_cmp_ps_mask(b, _mm512_set1_ps(0.0f), _CMP_LT_OQ);
        mask16 = _kor_mask16(mask1, mask2);
    }
    
    __m512 result_sf = _mm512_mask_blend_ps(mask16, a, b);
    
    /* Chain to V8DFmode blend */
    __mmask8 mask8;
    if (selector & 4) {
        mask8 = 0x55;
    } else {
        mask8 = _mm512_cmp_pd_mask(c, d, _CMP_NEQ_OQ);
    }
    
    __m512d result_df = _mm512_mask_blend_pd(mask8, c, d);
    
    /* Mix results */
    return _mm512_add_ps(result_sf, _mm512_castpd_ps(result_df));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int selector) {
    __mmask32 mask32;
    
    /* Varied mask generation for FP16 */
    if (selector & 8) {
        mask32 = 0xAAAAAAAA;
    } else {
        /* Comparison-based mask for half precision */
        mask32 = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
        mask32 = _knot_mask32(mask32);
    }
    
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int selector) {
    __mmask16 mask16;
    
    /* Data-dependent mask generation */
    if (selector > 100) {
        mask16 = 0x5555;
    } else {
        __m512i threshold = _mm512_set1_epi32(selector);
        mask16 = _mm512_cmpgt_epi32_mask(a, threshold);
    }
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16, a, b);
    
    /* Chain to V8DImode blend */
    __mmask8 mask8;
    if (selector < 50) {
        mask8 = 0xAA;
    } else {
        mask8 = _mm512_cmpgt_epi64_mask(c, d);
    }
    
    __m512i result_di = _mm512_mask_blend_epi64(mask8, c, d);
    
    return _mm512_add_epi32(result_si, _mm512_castsi512_si512(result_di));
}

/* Reduction helpers */
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256_lo = _mm512_castsi512_si256(v);
    __m256i v256_hi = _mm512_extracti64x4_epi64(v, 1);
    __m256i sum256 = _mm256_add_epi64(v256_lo, v256_hi);
    
    __m128i v128_lo = _mm256_castsi256_si128(sum256);
    __m128i v128_hi = _mm256_extracti128_si256(sum256, 1);
    __m128i sum128 = _mm_add_epi64(v128_lo, v128_hi);
    
    return (int64_t)_mm_extract_epi64(sum128, 0) + 
           (int64_t)_mm_extract_epi64(sum128, 1);
}

static inline float reduce_add_ps(__m512 v) {
    __m256 v256_lo = _mm512_castps512_ps256(v);
    __m256 v256_hi = _mm512_extractf32x8_ps(v, 1);
    __m256 sum256 = _mm256_add_ps(v256_lo, v256_hi);
    
    __m128 v128_lo = _mm256_castps256_ps128(sum256);
    __m128 v128_hi = _mm256_extractf128_ps(sum256, 1);
    __m128 sum128 = _mm_add_ps(v128_lo, v128_hi);
    
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    
    return _mm_cvtss_f32(sum128);
}

int main() {
    int64_t final_checksum = 0;
    
    /* Initialize vectors with distinct patterns */
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(0xFF);
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(0x7FFF);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set1_epi32(0x7FFFFFFF);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(0x7FFFFFFFFFFFFFFFLL);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(1.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set1_pd(2.0);
    
    /* Data-dependent control flow with loops */
    for (int i = 0; i < 10; i++) {
        __m512i result_int;
        
        /* Select blend mode based on loop index */
        if (i % 4 == 0) {
            result_int = blend_v64qi_v32hi(v64qi_a, v64qi_b, i);
        } else if (i % 4 == 1) {
            result_int = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, i);
        } else if (i % 4 == 2) {
            /* Direct intrinsic calls for uncovered modes */
            __mmask32 mask32 = (i > 5) ? 0x55555555 : 0xAAAAAAAA;
            __m512i v32hi_result = _mm512_mask_blend_epi16(mask32, v32hi_a, v32hi_b);
            
            __mmask8 mask8 = _mm512_cmpgt_epi64_mask(v8di_a, v8di_b);
            __m512i v8di_result = _mm512_mask_blend_epi64(mask8, v8di_a, v8di_b);
            
            result_int = _mm512_add_epi32(v32hi_result, _mm512_castsi512_si512(v8di_result));
        } else {
            /* Mixed integer blends */
            __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
            __m512i v64qi_result = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
            
            __mmask16 mask16 = _mm512_cmpgt_epi32_mask(v16si_a, _mm512_set1_epi32(i));
            __m512i v16si_result = _mm512_mask_blend_epi32(mask16, v16si_a, v16si_b);
            
            result_int = _mm512_add_epi32(v64qi_result, v16si_result);
        }
        
        final_checksum += reduce_add_epi64(result_int);
    }
    
    /* Float blends with chained operations */
    for (int i = 0; i < 5; i++) {
        __m512 result_float = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, i);
        final_checksum += (int64_t)reduce_add_ps(result_float);
        
        /* Direct float blends for uncovered modes */
        __mmask16 mask16_ps = (i % 2 == 0) ? 0xAAAA : 0x5555;
        __m512 v16sf_result = _mm512_mask_blend_ps(mask16_ps, v16sf_a, v16sf_b);
        
        __mmask8 mask8_pd = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
        __m512d v8df_result = _mm512_mask_blend_pd(mask8_pd, v8df_a, v8df_b);
        
        final_checksum += (int64_t)(reduce_add_ps(v16sf_result) + 
                                   reduce_add_ps(_mm512_castpd_ps(v8df_result)));
    }
    
#ifdef __AVX512FP16__
    /* FP16 blends if supported */
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    
    for (int i = 0; i < 3; i++) {
        __m512h result_half = blend_v32hf_v32bf(v32hf_a, v32hf_b, i);
        
        /* Manual reduction for half precision */
        __m256i result256 = _mm512_castph_si256(result_half);
        __m128i result128_lo = _mm256_castsi256_si128(result256);
        __m128i result128_hi = _mm256_extracti128_si256(result256, 1);
        
        uint16_t sum = 0;
        uint16_t* ptr_lo = (uint16_t*)&result128_lo;
        uint16_t* ptr_hi = (uint16_t*)&result128_hi;
        
        for (int j = 0; j < 8; j++) {
            sum += ptr_lo[j] + ptr_hi[j];
        }
        
        final_checksum += sum;
    }
#endif
    
    printf("Final checksum: %ld\n", final_checksum);
    return 0;
}
