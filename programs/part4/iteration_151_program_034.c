#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    // V64QImode blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i result64qi = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamic mask based on comparison
    __m512i cmp_val = _mm512_set1_epi16(mode_selector);
    __mmask32 mask32 = _mm512_cmp_epi16_mask(result64qi, cmp_val, _MM_CMPINT_GT);
    return _mm512_mask_blend_epi16(mask32, result64qi, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // V16SFmode blend with comparison-generated mask
    __m512 cmp_val = _mm512_set1_ps(iter * 0.5f);
    __mmask16 mask16sf = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    __m512 result16sf = _mm512_mask_blend_ps(mask16sf, a, b);
    
    // V8DFmode blend with inverted mask
    __m512d cmp_val_d = _mm512_set1_pd(iter * 1.0);
    __mmask8 mask8df = _mm512_cmp_pd_mask(c, cmp_val_d, _CMP_LT_OQ);
    __mmask8 inv_mask = _knot_mask8(mask8df);
    return _mm512_castpd_ps(_mm512_mask_blend_pd(inv_mask, 
                                                 _mm512_castps_pd(result16sf), 
                                                 d));
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    // V32HFmode blend with pattern-based mask
    __mmask32 mask32hf;
    if (pattern & 1) {
        mask32hf = 0x55555555;  // Alternating pattern
    } else {
        mask32hf = 0xAAAAAAAA;  // Opposite alternating pattern
    }
    
    __m512h result32hf = _mm512_mask_blend_ph(mask32hf, a, b);
    
    // V32BFmode blend with combined masks
    __mmask32 mask32bf = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    __mmask32 combined_mask = _kor_mask32(mask32hf, mask32bf);
    return _mm512_mask_blend_ph(combined_mask, result32hf, a);
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __mmask16 base_mask) {
    // V16SImode blend
    __m512i result16si = _mm512_mask_blend_epi32(base_mask, a, b);
    
    // V8DImode blend with derived mask
    __mmask8 mask8di = _cvtmask16_u8(base_mask);
    mask8di = mask8di ^ 0xFF;  // XOR with all ones
    return _mm512_mask_blend_epi64(mask8di, result16si, a);
}

// Reduction helpers
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256 = _mm512_extracti64x4_epi64(v, 0);
    __m256i v256_1 = _mm512_extracti64x4_epi64(v, 1);
    v256 = _mm256_add_epi64(v256, v256_1);
    
    __m128i v128 = _mm256_extracti128_si256(v256, 0);
    __m128i v128_1 = _mm256_extracti128_si256(v256, 1);
    v128 = _mm_add_epi64(v128, v128_1);
    
    return (int64_t)_mm_extract_epi64(v128, 0) + 
           (int64_t)_mm_extract_epi64(v128, 1);
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

static inline double reduce_add_pd(__m512d v) {
    __m256d v256 = _mm512_extractf64x4_pd(v, 0);
    __m256d v256_1 = _mm512_extractf64x4_pd(v, 1);
    v256 = _mm256_add_pd(v256, v256_1);
    
    __m128d v128 = _mm256_extractf128_pd(v256, 0);
    __m128d v128_1 = _mm256_extractf128_pd(v256, 1);
    v128 = _mm_add_pd(v128, v128_1);
    
    v128 = _mm_hadd_pd(v128, v128);
    return _mm_cvtsd_f64(v128);
}

int main() {
    double checksum = 0.0;
    
    // Initialize vectors with distinct patterns
    __m512i vi1 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi2 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512 vf1 = _mm512_set_ps(
        31.5f,30.5f,29.5f,28.5f,27.5f,26.5f,25.5f,24.5f,
        23.5f,22.5f,21.5f,20.5f,19.5f,18.5f,17.5f,16.5f,
        15.5f,14.5f,13.5f,12.5f,11.5f,10.5f,9.5f,8.5f,
        7.5f,6.5f,5.5f,4.5f,3.5f,2.5f,1.5f,0.5f
    );
    
    __m512 vf2 = _mm512_set_ps(
        0.5f,1.5f,2.5f,3.5f,4.5f,5.5f,6.5f,7.5f,
        8.5f,9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f,
        16.5f,17.5f,18.5f,19.5f,20.5f,21.5f,22.5f,23.5f,
        24.5f,25.5f,26.5f,27.5f,28.5f,29.5f,30.5f,31.5f
    );
    
    __m512d vd1 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d vd2 = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    
    // Initialize FP16 vectors if supported
    __m512h vh1, vh2;
    #ifdef __AVX512FP16__
    {
        _Float16 hvals1[32], hvals2[32];
        for (int i = 0; i < 32; i++) {
            hvals1[i] = (_Float16)(i * 0.5f);
            hvals2[i] = (_Float16)((31 - i) * 0.5f);
        }
        vh1 = _mm512_loadu_ph(hvals1);
        vh2 = _mm512_loadu_ph(hvals2);
    }
    #endif
    
    // Data-dependent control flow with loops
    for (int iter = 0; iter < 4; iter++) {
        int mode_selector = iter % 3;
        
        // Chain blends through different modes
        __m512i blended_int;
        if (mode_selector == 0) {
            // Start with V64QImode -> V32HImode chain
            blended_int = blend_v64qi_v32hi(vi1, vi2, iter);
        } else if (mode_selector == 1) {
            // Direct V16SImode -> V8DImode chain
            __mmask16 mask16 = (iter & 1) ? 0xAAAA : 0x5555;
            blended_int = blend_v16si_v8di(vi1, vi2, mask16);
        } else {
            // Mixed integer blend
            __mmask64 mask64 = 0xCCCCCCCCCCCCCCCCULL;
            blended_int = _mm512_mask_blend_epi8(mask64, vi1, vi2);
            
            __mmask32 mask32 = _mm512_cmp_epi16_mask(blended_int, 
                                                    _mm512_set1_epi16(32), 
                                                    _MM_CMPINT_LT);
            blended_int = _mm512_mask_blend_epi16(mask32, blended_int, vi1);
        }
        
        // Chain float blends
        __m512 blended_float = blend_v16sf_v8df(vf1, vf2, vd1, vd2, iter);
        
        // Chain FP16 blends if supported
        #ifdef __AVX512FP16__
        __m512h blended_half = blend_v32hf_v32bf(vh1, vh2, iter);
        
        // Reduce FP16 result (convert to float for reduction)
        _Float16 hvals[32];
        _mm512_storeu_ph(hvals, blended_half);
        float half_sum = 0.0f;
        for (int i = 0; i < 32; i++) {
            half_sum += (float)hvals[i];
        }
        checksum += half_sum;
        #endif
        
        // Perform reductions to prevent dead code elimination
        checksum += reduce_add_epi64(blended_int);
        checksum += reduce_add_ps(blended_float);
        checksum += reduce_add_pd(vd1);
        checksum += reduce_add_pd(vd2);
        
        // Modify vectors for next iteration
        vi1 = _mm512_add_epi8(vi1, _mm512_set1_epi8(1));
        vf1 = _mm512_add_ps(vf1, _mm512_set1_ps(0.25f));
        vd1 = _mm512_add_pd(vd1, _mm512_set1_pd(0.125));
    }
    
    // Final direct calls to ensure all intrinsics are used
    __mmask64 final_mask64 = _mm512_cmp_epi8_mask(vi1, vi2, _MM_CMPINT_NE);
    __m512i final_i = _mm512_mask_blend_epi8(final_mask64, vi1, vi2);
    checksum += reduce_add_epi64(final_i);
    
    __mmask32 final_mask32 = _mm512_cmp_epi16_mask(vi1, vi2, _MM_CMPINT_EQ);
    final_i = _mm512_mask_blend_epi16(final_mask32, vi1, vi2);
    checksum += reduce_add_epi64(final_i);
    
    __mmask16 final_mask16 = _mm512_cmp_epi32_mask(vi1, vi2, _MM_CMPINT_GT);
    final_i = _mm512_mask_blend_epi32(final_mask16, vi1, vi2);
    checksum += reduce_add_epi64(final_i);
    
    __mmask8 final_mask8 = _mm512_cmp_epi64_mask(vi1, vi2, _MM_CMPINT_LT);
    final_i = _mm512_mask_blend_epi64(final_mask8, vi1, vi2);
    checksum += reduce_add_epi64(final_i);
    
    __mmask16 final_mask16f = _mm512_cmp_ps_mask(vf1, vf2, _CMP_GT_OQ);
    __m512 final_f = _mm512_mask_blend_ps(final_mask16f, vf1, vf2);
    checksum += reduce_add_ps(final_f);
    
    __mmask8 final_mask8d = _mm512_cmp_pd_mask(vd1, vd2, _CMP_LT_OQ);
    __m512d final_d = _mm512_mask_blend_pd(final_mask8d, vd1, vd2);
    checksum += reduce_add_pd(final_d);
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
