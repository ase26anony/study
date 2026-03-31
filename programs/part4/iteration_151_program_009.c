#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int selector) {
    __mmask64 mask64;
    __mmask32 mask32;
    
    /* Generate masks using different patterns */
    if (selector & 1) {
        mask64 = 0xAAAAAAAAAAAAAAAAULL;  /* Immediate constant */
    } else {
        /* Dynamic mask generation via comparison */
        __m512i cmp_a = _mm512_set1_epi8(selector);
        __m512i cmp_b = _mm512_set1_epi8(selector ^ 0xFF);
        mask64 = _mm512_cmp_epi8_mask(cmp_a, cmp_b, _MM_CMPINT_EQ);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    /* Chain to epi16 blend */
    mask32 = (__mmask32)(mask64 & 0xFFFFFFFF);
    if (selector & 2) {
        mask32 = _knot_mask32(mask32);  /* Bitwise operation */
    }
    
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    /* Generate mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(selector * 0.5f);
    mask16 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    
    if (selector & 4) {
        mask16 = _kor_mask16(mask16, 0xAAAA);  /* Combine masks */
    }
    
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    /* Chain to double precision blend */
    __m512d cmp_dval = _mm512_set1_pd(selector * 0.25);
    mask8 = _mm512_cmp_pd_mask(c, cmp_dval, _CMP_GT_OQ);
    
    __m512d result2 = _mm512_mask_blend_pd(mask8, c, d);
    
    /* Mix results (convert double to float for return) */
    __m512 result2_f = _mm512_castpd_ps(result2);
    return _mm512_mask_blend_ps(0xFFFF, result1, result2_f);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int selector) {
    __mmask32 mask32;
    
    /* Multiple mask generation methods */
    if (selector & 8) {
        mask32 = 0x55555555;  /* Immediate constant */
    } else {
        /* Dynamic mask via comparison */
        __m512h cmp_val = _mm512_set1_ph(selector * 0.125f);
        mask32 = _mm512_cmp_ph_mask(a, cmp_val, _CMP_GT_OQ);
        
        if (selector & 16) {
            mask32 = _knot_mask32(mask32);
        }
    }
    
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int selector) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    /* Generate mask with comparison */
    __m512i cmp_val = _mm512_set1_epi32(selector);
    mask16 = _mm512_cmp_epi32_mask(a, cmp_val, _MM_CMPINT_GT);
    
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    /* Chain to 64-bit blend */
    __m512i cmp_val64 = _mm512_set1_epi64(selector);
    mask8 = _mm512_cmp_epi64_mask(c, cmp_val64, _MM_CMPINT_GT);
    
    if (selector & 32) {
        mask8 = mask8 ^ 0xFF;  /* Bitwise XOR */
    }
    
    __m512i result2 = _mm512_mask_blend_epi64(mask8, c, d);
    
    /* Combine results */
    return _mm512_add_epi32(result1, _mm512_castsi512_si256(result2));
}

int main() {
    /* Initialize vectors with distinct patterns */
    __m512i v64qi_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v64qi_b = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    __m512d v8df_b = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    
    __m512i v16si_a = _mm512_set_epi32(
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    );
    
    __m512i v16si_b = _mm512_set_epi32(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    
    __m512i v8di_a = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    __m512i v8di_b = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    
    /* Data-dependent control flow with loop */
    unsigned long long checksum = 0;
    
    for (int i = 0; i < 100; i++) {
        __m512i result_int;
        
        /* Conditional blend selection */
        if (i % 3 == 0) {
            result_int = blend_v64qi_v32hi(v64qi_a, v64qi_b, i);
        } else if (i % 3 == 1) {
            result_int = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, i);
        } else {
            /* Direct intrinsic calls for uncovered modes */
            __mmask64 mask64 = (i & 1) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
            result_int = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
            
            __mmask32 mask32 = (__mmask32)(mask64 >> 32);
            result_int = _mm512_mask_blend_epi16(mask32, result_int, v64qi_a);
            
            __mmask16 mask16 = _mm512_cmp_epi32_mask(v16si_a, _mm512_set1_epi32(i), _MM_CMPINT_GT);
            __m512i temp = _mm512_mask_blend_epi32(mask16, v16si_a, v16si_b);
            result_int = _mm512_add_epi32(result_int, temp);
            
            __mmask8 mask8 = _mm512_cmp_epi64_mask(v8di_a, _mm512_set1_epi64(i), _MM_CMPINT_GT);
            temp = _mm512_mask_blend_epi64(mask8, v8di_a, v8di_b);
            result_int = _mm512_add_epi32(result_int, _mm512_castsi512_si256(temp));
        }
        
        /* Process float blends */
        __m512 result_float = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, i);
        
        /* Prevent dead code elimination - reduce results */
        __m256i result_low = _mm512_castsi512_si256(result_int);
        __m256i result_high = _mm512_extracti64x4_epi64(result_int, 1);
        
        long long sum_int[8];
        _mm512_store_epi64(sum_int, result_int);
        for (int j = 0; j < 8; j++) {
            checksum += (unsigned long long)(sum_int[j] & 0xFFFFFFFF);
        }
        
        float sum_float[16];
        _mm512_store_ps(sum_float, result_float);
        for (int j = 0; j < 16; j++) {
            checksum += (unsigned long long)(sum_float[j] * 1000);
        }
        
#ifdef __AVX512FP16__
        /* FP16 blends if available */
        __m512h v32hf_a = _mm512_castsi512_ph(v64qi_a);
        __m512h v32hf_b = _mm512_castsi512_ph(v64qi_b);
        __m512h result_half = blend_v32hf_v32bf(v32hf_a, v32hf_b, i);
        
        /* Reduce half-precision results */
        _Float16 sum_half[32];
        _mm512_store_ph(sum_half, result_half);
        for (int j = 0; j < 32; j++) {
            checksum += (unsigned long long)(sum_half[j] * 100);
        }
#endif
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
