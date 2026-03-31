#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int selector) {
    __mmask64 mask64;
    __mmask32 mask32;
    
    /* Generate masks using different patterns based on selector */
    if (selector & 1) {
        mask64 = 0xAAAAAAAAAAAAAAAAULL;  // Immediate constant
    } else {
        mask64 = _mm512_cmpeq_epi8_mask(a, b);  // Comparison-generated
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    /* Chain to epi16 blend */
    mask32 = (selector & 2) ? 0x55555555 : _mm512_cmpeq_epi16_mask(result1, b);
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512 c, __m512 d, int selector) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    /* Generate mask using comparison */
    mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OS);
    
    if (selector > 0) {
        mask16 = _knot_mask16(mask16);  // Bitwise operation on mask
    }
    
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    /* Convert to double and chain to pd blend */
    __m512d ad = _mm512_cvtps_pd(_mm512_extractf32x8_ps(result1, 0));
    __m512d bd = _mm512_cvtps_pd(_mm512_extractf32x8_ps(c, 0));
    __m512d cd = _mm512_cvtps_pd(_mm512_extractf32x8_ps(d, 0));
    
    mask8 = _mm512_cmp_pd_mask(ad, bd, _CMP_GT_OS);
    return _mm512_cvtpd_ps(_mm512_mask_blend_pd(mask8, ad, cd));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, int selector) {
    __mmask32 mask32;
    
    /* Multiple mask generation methods */
    if (selector == 0) {
        mask32 = 0xAAAAAAAA;  // Immediate constant
    } else if (selector == 1) {
        mask32 = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);  // Comparison
    } else {
        __mmask32 mask1 = _mm512_cmp_ph_mask(a, c, _CMP_LT_OQ);
        __mmask32 mask2 = _mm512_cmp_ph_mask(b, c, _CMP_GT_OQ);
        mask32 = _kor_mask32(mask1, mask2);  // Bitwise combination
    }
    
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, int selector) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    /* Dynamic mask generation */
    mask16 = _mm512_cmpeq_epi32_mask(a, b);
    
    if (selector & 1) {
        mask16 = _knot_mask16(mask16);
    }
    
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    /* Chain to epi64 blend */
    mask8 = _mm512_cmpgt_epi64_mask(result1, c);
    return _mm512_mask_blend_epi64(mask8, result1, c);
}

/* Main test function with data-dependent control flow */
int main() {
    /* Initialize vectors with distinct patterns */
    __m512i vi1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vi2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi3 = _mm512_set1_epi8(32);
    __m512i vi4 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    __m512i vi5 = _mm512_set1_epi32(8);
    __m512i vi6 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    
    __m512 vf1 = _mm512_set_ps(
        15.0,14.0,13.0,12.0,11.0,10.0,9.0,8.0,
        7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0
    );
    __m512 vf2 = _mm512_set1_ps(7.5);
    __m512 vf3 = _mm512_set_ps(
        0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,
        8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0
    );
    
    __m512d vd1 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d vd2 = _mm512_set1_pd(3.5);
    
    #ifdef __AVX512FP16__
    __m512h vh1 = _mm512_set_ph(
        0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0,
        8.0,9.0,10.0,11.0,12.0,13.0,14.0,15.0,
        16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,
        24.0,25.0,26.0,27.0,28.0,29.0,30.0,31.0
    );
    __m512h vh2 = _mm512_set1_ph(15.5);
    __m512h vh3 = _mm512_set_ph(
        31.0,30.0,29.0,28.0,27.0,26.0,27.0,26.0,
        25.0,24.0,23.0,22.0,21.0,20.0,19.0,18.0,
        17.0,16.0,15.0,14.0,13.0,12.0,11.0,10.0,
        9.0,8.0,7.0,6.0,5.0,4.0,3.0,2.0
    );
    #endif
    
    float checksum = 0.0f;
    
    /* Loop with data-dependent control flow */
    for (int i = 0; i < 10; i++) {
        __m512i result_int;
        __m512 result_float;
        
        /* Select blend mode based on loop index */
        if (i % 3 == 0) {
            /* Direct epi8 blend with immediate mask */
            __mmask64 mask = 0x5555555555555555ULL;
            result_int = _mm512_mask_blend_epi8(mask, vi1, vi2);
            
            /* Chain to epi16 blend with comparison mask */
            __mmask32 mask32 = _mm512_cmpeq_epi16_mask(result_int, vi3);
            result_int = _mm512_mask_blend_epi16(mask32, result_int, vi3);
        } 
        else if (i % 3 == 1) {
            /* Use helper function for chained blends */
            result_int = blend_v64qi_v32hi(vi1, vi2, i);
        }
        else {
            /* Direct epi32 blend */
            __mmask16 mask = _mm512_cmpgt_epi32_mask(vi4, vi5);
            result_int = _mm512_mask_blend_epi32(mask, vi4, vi5);
            
            /* Chain to epi64 blend */
            __mmask8 mask8 = _mm512_cmpeq_epi64_mask(result_int, vi6);
            result_int = _mm512_mask_blend_epi64(mask8, result_int, vi6);
        }
        
        /* Always perform float blends */
        if (i % 2 == 0) {
            /* Direct ps blend */
            __mmask16 mask = _mm512_cmp_ps_mask(vf1, vf2, _CMP_GT_OS);
            result_float = _mm512_mask_blend_ps(mask, vf1, vf2);
            
            /* Chain to pd blend via helper */
            result_float = blend_v16sf_v8df(result_float, vf3, vf1, vf2, i);
        } else {
            /* Direct pd blend */
            __mmask8 mask = _mm512_cmp_pd_mask(vd1, vd2, _CMP_LT_OS);
            __m512d result_double = _mm512_mask_blend_pd(mask, vd1, vd2);
            result_float = _mm512_cvtpd_ps(result_double);
        }
        
        #ifdef __AVX512FP16__
        /* Half-precision blends */
        __m512h result_half;
        if (i < 5) {
            /* Direct ph blend */
            __mmask32 mask = _mm512_cmp_ph_mask(vh1, vh2, _CMP_EQ_OQ);
            result_half = _mm512_mask_blend_ph(mask, vh1, vh2);
        } else {
            /* Use helper function */
            result_half = blend_v32hf_v32bf(vh1, vh2, vh3, i);
        }
        
        /* Convert half to float for checksum */
        for (int j = 0; j < 16; j++) {
            checksum += ((float*)&result_half)[j];
        }
        #endif
        
        /* Reduce results to prevent dead code elimination */
        /* Integer reduction */
        __m256i low = _mm512_extracti64x4_epi64(result_int, 0);
        __m256i high = _mm512_extracti64x4_epi64(result_int, 1);
        __m256i sum256 = _mm256_add_epi64(low, high);
        __m128i sum128 = _mm_add_epi64(
            _mm256_extracti128_si256(sum256, 0),
            _mm256_extracti128_si256(sum256, 1)
        );
        long long int_sum = _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
        checksum += (float)int_sum;
        
        /* Float reduction */
        checksum += _mm512_reduce_add_ps(result_float);
    }
    
    /* Additional test: blend_v16si_v8di helper */
    __m512i final_int = blend_v16si_v8di(vi4, vi5, vi6, 3);
    
    /* Reduce final result */
    __m256i low = _mm512_extracti64x4_epi64(final_int, 0);
    __m256i high = _mm512_extracti64x4_epi64(final_int, 1);
    __m256i sum256 = _mm256_add_epi64(low, high);
    __m128i sum128 = _mm_add_epi64(
        _mm256_extracti128_si256(sum256, 0),
        _mm256_extracti128_si256(sum256, 1)
    );
    long long final_sum = _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
    checksum += (float)final_sum;
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
