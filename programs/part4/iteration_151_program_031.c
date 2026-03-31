#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend modes
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
    
    // Now blend with V32HImode using result1
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
    
    // V16SFmode blend
    if (selector > 0) {
        mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    } else {
        mask16 = 0xAAAA;
    }
    
    __m512 result_sf = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend
    mask8 = _mm512_cmp_pd_mask(c, d, _CMP_GT_OQ);
    mask8 = _knot_mask8(mask8);
    
    __m512d result_df = _mm512_mask_blend_pd(mask8, c, d);
    
    // Convert double result to float for chaining
    return _mm512_add_ps(result_sf, _mm512_castpd_ps(result_df));
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int iter) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // V16SImode blend with varying mask patterns
    switch (iter % 3) {
        case 0:
            mask16 = 0x5555;
            break;
        case 1:
            mask16 = _mm512_cmpeq_epi32_mask(a, b);
            break;
        case 2:
            mask16 = _kor_mask16(0xAAAA, _mm512_cmpgt_epi32_mask(a, b));
            break;
    }
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend
    mask8 = _mm512_cmpeq_epi64_mask(c, d);
    mask8 = _kxor_mask8(mask8, 0xFF);
    
    return _mm512_mask_blend_epi64(mask8, result_si, c);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d) {
    __mmask32 mask32_hf, mask32_bf;
    
    // V32HFmode blend with comparison mask
    mask32_hf = _mm512_cmp_ph_mask(a, b, _CMP_NEQ_UQ);
    
    __m512h result_hf = _mm512_mask_blend_ph(mask32_hf, a, b);
    
    // V32BFmode blend with immediate mask
    mask32_bf = 0xAAAAAAAA;
    
    __m512h result_bf = _mm512_mask_blend_ph(mask32_bf, c, d);
    
    // Chain results
    return _mm512_add_ph(result_hf, result_bf);
}
#endif

int main() {
    // Initialize patterned data
    __m512i v64qi_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(100);
    
    __m512i v32hi_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(200);
    
    __m512 v16sf_a = _mm512_set_ps(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(50.0f);
    
    __m512d v8df_a = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d v8df_b = _mm512_set1_pd(100.0);
    
    __m512i v16si_a = _mm512_set_epi32(
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
    );
    
    __m512i v16si_b = _mm512_set1_epi32(500);
    
    __m512i v8di_a = _mm512_set_epi64(1,2,3,4,5,6,7,8);
    __m512i v8di_b = _mm512_set1_epi64(1000);
    
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f,
        17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
        25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f
    );
    
    __m512h v32hf_b = _mm512_set1_ph(100.0f);
    __m512h v32bf_c = _mm512_set1_ph(200.0f);
    __m512h v32bf_d = _mm512_set1_ph(300.0f);
    #endif
    
    float checksum = 0.0f;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        __m512i result_int;
        
        if (i % 2 == 0) {
            // Call V64QI/V32HI blend helper
            result_int = blend_v64qi_v32hi(v64qi_a, v64qi_b, i);
        } else {
            // Direct V32HImode blend with dynamic mask
            __mmask32 mask32 = _mm512_cmpgt_epi16_mask(v32hi_a, v32hi_b);
            mask32 = _kor_mask32(mask32, (0x55555555 >> (i % 8)));
            result_int = _mm512_mask_blend_epi16(mask32, v32hi_a, v32hi_b);
        }
        
        // Chain with V16SI/V8DI blend
        __m512i result_si_di = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, i);
        
        // Chain with float blends
        __m512 result_float = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, i);
        
        #ifdef __AVX512FP16__
        // FP16 blends
        __m512h result_fp16 = blend_v32hf_v32bf(v32hf_a, v32hf_b, v32bf_c, v32bf_d);
        
        // Reduce FP16 result
        __m256h result_256 = _mm512_castph512_ph256(result_fp16);
        for (int j = 0; j < 16; j++) {
            checksum += result_256[j];
        }
        #endif
        
        // Reduce integer results to prevent dead code elimination
        int64_t sum_int = _mm512_reduce_add_epi64(result_int);
        int64_t sum_si_di = _mm512_reduce_add_epi64(result_si_di);
        
        // Reduce float results
        float sum_float = _mm512_reduce_add_ps(result_float);
        
        checksum += (float)sum_int + (float)sum_si_di + sum_float;
        
        // Modify source data for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.5f));
    }
    
    // Additional direct blends for each mode
    __mmask64 direct_mask64 = 0xCCCCCCCCCCCCCCCCULL;
    __m512i direct_v64qi = _mm512_mask_blend_epi8(direct_mask64, v64qi_a, v64qi_b);
    
    __mmask32 direct_mask32 = 0x88888888;
    __m512i direct_v32hi = _mm512_mask_blend_epi16(direct_mask32, v32hi_a, v32hi_b);
    
    __mmask16 direct_mask16 = 0xF0F0;
    __m512i direct_v16si = _mm512_mask_blend_epi32(direct_mask16, v16si_a, v16si_b);
    
    __mmask8 direct_mask8 = 0xAA;
    __m512i direct_v8di = _mm512_mask_blend_epi64(direct_mask8, v8di_a, v8di_b);
    
    __m512 direct_v16sf = _mm512_mask_blend_ps(0xAAAA, v16sf_a, v16sf_b);
    __m512d direct_v8df = _mm512_mask_blend_pd(0x55, v8df_a, v8df_b);
    
    #ifdef __AVX512FP16__
    __m512h direct_v32hf = _mm512_mask_blend_ph(0xAAAAAAAA, v32hf_a, v32hf_b);
    __m512h direct_v32bf = _mm512_mask_blend_ph(0x55555555, v32bf_c, v32bf_d);
    #endif
    
    // Final reduction
    checksum += _mm512_reduce_add_epi64(direct_v64qi);
    checksum += _mm512_reduce_add_epi64(direct_v32hi);
    checksum += _mm512_reduce_add_epi64(direct_v16si);
    checksum += _mm512_reduce_add_epi64(direct_v8di);
    checksum += _mm512_reduce_add_ps(direct_v16sf);
    checksum += _mm512_reduce_add_pd(direct_v8df);
    
    #ifdef __AVX512FP16__
    __m256h direct_256 = _mm512_castph512_ph256(direct_v32hf);
    for (int i = 0; i < 16; i++) {
        checksum += direct_256[i];
    }
    #endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
