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
        // Dynamic mask from comparison
        __m512i cmp_a = _mm512_set1_epi8(0x40);
        __m512i cmp_b = _mm512_set1_epi8(0x20);
        mask64 = _mm512_cmpgt_epi8_mask(cmp_a, cmp_b);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Now blend with V32HImode using result1
    if (mode_selector & 2) {
        mask32 = 0xAAAAAAAA;
    } else {
        __m512i cmp_val = _mm512_set1_epi16(100);
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
    
    // V8DFmode blend
    if (selector & 2) {
        mask8 = 0xAA;
    } else {
        __m512d cmp_val_d = _mm512_set1_pd(0.5);
        mask8 = _mm512_cmp_pd_mask(c, cmp_val_d, _CMP_GT_OQ);
    }
    
    __m512d result_df = _mm512_mask_blend_pd(mask8, c, d);
    
    // Convert double result back to float for return (just use SF result)
    return result_sf;
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // V16SImode blend with immediate mask
    mask16 = 0x5555;
    __m512i result_si = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend with dynamic mask
    __m512i cmp_val = _mm512_set1_epi64(1000);
    mask8 = _mm512_cmpgt_epi64_mask(cmp_val, c);
    
    // Combine masks for chained operation
    __mmask8 combined_mask = _kor_mask8(mask8, 0x0F);
    
    return _mm512_mask_blend_epi64(combined_mask, result_si, d);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d) {
    __mmask32 mask32;
    
    // V32HFmode blend with immediate mask
    mask32 = 0xAAAAAAAA;
    __m512h result_hf = _mm512_mask_blend_ph(mask32, a, b);
    
    // V32BFmode blend with dynamic mask
    __m512h cmp_val = _mm512_set1_ph(0.5f);
    mask32 = _mm512_cmp_ph_mask(c, cmp_val, _CMP_GT_OQ);
    
    // Invert mask for variety
    __mmask32 not_mask = _knot_mask32(mask32);
    
    return _mm512_mask_blend_ph(not_mask, result_hf, d);
}
#endif

/* Main test function with data-dependent control flow */
int main() {
    // Initialize vectors with distinct patterns
    __m512i vi64_1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vi64_2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi32_1 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i vi32_2 = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi16_1 = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i vi16_2 = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi64_3 = _mm512_set_epi64(100, 200, 300, 400, 500, 600, 700, 800);
    __m512i vi64_4 = _mm512_set_epi64(800, 700, 600, 500, 400, 300, 200, 100);
    
    __m512 vf32_1 = _mm512_set_ps(
        0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,
        0.8f,0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f
    );
    
    __m512 vf32_2 = _mm512_set_ps(
        1.5f,1.4f,1.3f,1.2f,1.1f,1.0f,0.9f,0.8f,
        0.7f,0.6f,0.5f,0.4f,0.3f,0.2f,0.1f,0.0f
    );
    
    __m512d vf64_1 = _mm512_set_pd(0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7);
    __m512d vf64_2 = _mm512_set_pd(0.7,0.6,0.5,0.4,0.3,0.2,0.1,0.0);
    
#ifdef __AVX512FP16__
    __m512h vf16_1 = _mm512_set_ph(
        0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,
        0.8f,0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f,
        1.6f,1.7f,1.8f,1.9f,2.0f,2.1f,2.2f,2.3f,
        2.4f,2.5f,2.6f,2.7f,2.8f,2.9f,3.0f,3.1f
    );
    
    __m512h vf16_2 = _mm512_set_ph(
        3.1f,3.0f,2.9f,2.8f,2.7f,2.6f,2.5f,2.4f,
        2.3f,2.2f,2.1f,2.0f,1.9f,1.8f,1.7f,1.6f,
        1.5f,1.4f,1.3f,1.2f,1.1f,1.0f,0.9f,0.8f,
        0.7f,0.6f,0.5f,0.4f,0.3f,0.2f,0.1f,0.0f
    );
#endif
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 100; i++) {
        int selector = i % 8;
        
        // Chained blend operations
        __m512i blended_int = blend_v64qi_v32hi(vi64_1, vi64_2, selector);
        
        // Direct blend calls for uncovered modes
        __mmask64 mask64_direct;
        if (selector & 1) {
            mask64_direct = 0x5555555555555555ULL;
        } else {
            mask64_direct = _mm512_cmpeq_epi8_mask(vi64_1, vi64_2);
        }
        __m512i direct_v64qi = _mm512_mask_blend_epi8(mask64_direct, vi64_1, vi64_2);
        
        __mmask32 mask32_direct = (selector & 2) ? 0x55555555 : 0xAAAAAAAA;
        __m512i direct_v32hi = _mm512_mask_blend_epi16(mask32_direct, vi16_1, vi16_2);
        
        // Blend with different modes based on selector
        __m512 blended_float;
        if (selector < 4) {
            blended_float = blend_v16sf_v8df(vf32_1, vf32_2, vf64_1, vf64_2, selector);
        } else {
            __m512i blended_si_di = blend_v16si_v8di(vi32_1, vi32_2, vi64_3, vi64_4);
            // Convert to float for checksum
            blended_float = _mm512_cvtepi32_ps(_mm512_castsi512_si256(blended_si_di));
        }
        
#ifdef __AVX512FP16__
        if (selector == 0 || selector == 7) {
            __m512h blended_half = blend_v32hf_v32bf(vf16_1, vf16_2, vf16_1, vf16_2);
            // Convert to float for checksum
            __m512 temp = _mm512_cvtph_ps(_mm512_castph_si256(_mm512_castph512_ph256(blended_half)));
            blended_float = _mm512_add_ps(blended_float, temp);
        }
#endif
        
        // Horizontal reductions to prevent dead code elimination
        __m256i low256 = _mm512_castsi512_si256(blended_int);
        __m256i high256 = _mm512_extracti64x4_epi64(blended_int, 1);
        
        __m256i sum256 = _mm256_add_epi64(low256, high256);
        __m128i sum128 = _mm_add_epi64(_mm256_castsi256_si128(sum256),
                                      _mm256_extracti128_si256(sum256, 1));
        
        uint64_t sum64 = _mm_extract_epi64(sum128, 0) +
                        _mm_extract_epi64(sum128, 1);
        
        checksum += sum64;
        
        // Reduce float vector
        checksum += _mm512_reduce_add_ps(blended_float);
        
        // Modify inputs slightly for next iteration
        vi64_1 = _mm512_add_epi8(vi64_1, _mm512_set1_epi8(1));
        vf32_1 = _mm512_add_ps(vf32_1, _mm512_set1_ps(0.01f));
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
