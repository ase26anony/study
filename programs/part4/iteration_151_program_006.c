#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

/* Helper functions for different blend combinations */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    __mmask64 mask64;
    __mmask32 mask32;
    
    if (mode_selector & 1) {
        // Immediate mask for V64QImode
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Dynamic mask using comparison
        __m512i cmp_a = _mm512_set1_epi8(32);
        __m512i cmp_b = _mm512_set1_epi8(64);
        mask64 = _mm512_cmpgt_epi8_mask(cmp_b, cmp_a);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Now blend with V32HImode using chained result
    if (mode_selector & 2) {
        mask32 = 0xAAAAAAAA;
    } else {
        __m512i cmp_val = _mm512_set1_epi16(1000);
        mask32 = _mm512_cmpgt_epi16_mask(cmp_val, result1);
    }
    
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // V16SFmode blend
    if (selector & 1) {
        mask16 = 0xAAAA;
    } else {
        __m512 cmp_val = _mm512_set1_ps(0.5f);
        mask16 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    }
    
    __m512 result_sf = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend with chaining
    if (selector & 2) {
        mask8 = 0xAA;
    } else {
        __m512d cmp_val_d = _mm512_set1_pd(1.0);
        mask8 = _mm512_cmp_pd_mask(c, cmp_val_d, _CMP_GT_OQ);
    }
    
    __m512d result_df = _mm512_mask_blend_pd(mask8, c, d);
    
    // Mix results (convert double to float for demonstration)
    __m512 df_as_ps = _mm512_castpd_ps(result_df);
    __mmask16 final_mask = _mm512_cmp_ps_mask(result_sf, df_as_ps, _CMP_LT_OQ);
    return _mm512_mask_blend_ps(final_mask, result_sf, df_as_ps);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    __mmask32 mask32;
    
    // V32HFmode blend
    if (pattern == 0) {
        mask32 = 0xAAAAAAAA;
    } else if (pattern == 1) {
        __m512h threshold = _mm512_set1_ph(0.0f);
        mask32 = _mm512_cmp_ph_mask(a, threshold, _CMP_GT_OQ);
    } else {
        // Complex mask generation using bitwise operations
        __mmask32 mask1 = 0x55555555;
        __mmask32 mask2 = 0xAAAAAAAA;
        mask32 = _kor_mask32(mask1, mask2);
        mask32 = _knot_mask32(mask32);
    }
    
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int mode) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // V16SImode blend
    if (mode & 1) {
        mask16 = 0xAAAA;
    } else {
        __m512i cmp_val = _mm512_set1_epi32(100);
        mask16 = _mm512_cmpgt_epi32_mask(cmp_val, a);
    }
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend with chaining
    if (mode & 2) {
        mask8 = 0xAA;
    } else {
        __m512i cmp_val_di = _mm512_set1_epi64(1000);
        mask8 = _mm512_cmpgt_epi64_mask(cmp_val_di, c);
    }
    
    __m512i result_di = _mm512_mask_blend_epi64(mask8, c, d);
    
    // Combine results
    __mmask16 final_mask = _mm512_cmpeq_epi32_mask(result_si, _mm512_set1_epi32(0));
    return _mm512_mask_blend_epi32(final_mask, result_si, _mm512_castsi512_si256(result_di));
}

int main() {
    // Initialize vectors with distinct patterns
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(100);
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(200);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set1_epi32(300);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(400);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(50.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set1_pd(100.0);
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        __m512i blend_result;
        
        if (i % 3 == 0) {
            // Direct V64QImode blend with immediate mask
            __mmask64 mask = 0xCCCCCCCCCCCCCCCCULL;
            blend_result = _mm512_mask_blend_epi8(mask, v64qi_a, v64qi_b);
        } else if (i % 3 == 1) {
            // Direct V32HImode blend with comparison mask
            __mmask32 mask = _mm512_cmpgt_epi16_mask(v32hi_a, _mm512_set1_epi16(10));
            blend_result = _mm512_mask_blend_epi16(mask, v32hi_a, v32hi_b);
        } else {
            // Use helper function with chained blends
            blend_result = blend_v64qi_v32hi(v64qi_a, v64qi_b, i);
        }
        
        // Reduce and accumulate
        checksum += _mm512_reduce_add_epi64(blend_result);
        
        // V16SImode and V8DImode blends in alternating iterations
        if (i % 2 == 0) {
            __mmask16 mask_si = _mm512_cmpgt_epi32_mask(v16si_a, _mm512_set1_epi32(5));
            __m512i result_si = _mm512_mask_blend_epi32(mask_si, v16si_a, v16si_b);
            checksum += _mm512_reduce_add_epi32(result_si);
            
            // Chain to V8DImode
            __mmask8 mask_di = 0xF0;
            __m512i result_di = _mm512_mask_blend_epi64(mask_di, v8di_a, v8di_b);
            checksum += _mm512_reduce_add_epi64(result_di);
        }
        
        // Floating point blends
        __mmask16 mask_ps = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OQ);
        __m512 result_ps = _mm512_mask_blend_ps(mask_ps, v16sf_a, v16sf_b);
        checksum += _mm512_reduce_add_ps(result_ps);
        
        __mmask8 mask_pd = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
        __m512d result_pd = _mm512_mask_blend_pd(mask_pd, v8df_a, v8df_b);
        checksum += _mm512_reduce_add_pd(result_pd);
        
        // Use helper function for chained SF/DF blends
        __m512 chained_fp = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, i);
        checksum += _mm512_reduce_add_ps(chained_fp);
        
        // Use helper for chained SI/DI blends
        __m512i chained_int = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, i);
        checksum += _mm512_reduce_add_epi64(chained_int);
    }
    
#ifdef __AVX512FP16__
    // FP16 blends if supported
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    
    for (int i = 0; i < 5; i++) {
        __m512h result_hf = blend_v32hf_v32bf(v32hf_a, v32hf_b, i % 3);
        
        // Manual reduction for half precision
        __m256h low = _mm512_castph512_ph256(result_hf);
        __m256h high = _mm512_extractf32x8_ph(result_hf, 1);
        
        float sum = 0.0f;
        for (int j = 0; j < 16; j++) {
            sum += _mm256_cvtph_ps(low)[j];
        }
        for (int j = 0; j < 16; j++) {
            sum += _mm256_cvtph_ps(high)[j];
        }
        checksum += sum;
    }
#endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
