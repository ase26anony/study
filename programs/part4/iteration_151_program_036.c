#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    // V64QImode blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamic mask
    __mmask32 mask32;
    if (mode_selector > 0) {
        mask32 = _mm512_cmpeq_epi16_mask(result1, _mm512_setzero_si512());
    } else {
        mask32 = 0x55555555;
    }
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // V16SFmode blend with comparison mask
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    // Modify mask based on iteration
    if (iter % 2) {
        mask16 = _knot_mask16(mask16);
    }
    
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend with chained result
    __mmask8 mask8 = 0xAA;
    __m512d temp = _mm512_castps_pd(result1);
    return _mm512_castpd_ps(_mm512_mask_blend_pd(mask8, temp, d));
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, int selector) {
    // V32HFmode blend
    __mmask32 mask32;
    if (selector == 0) {
        mask32 = 0xAAAAAAAA;
    } else {
        // Create mask using comparison
        mask32 = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    }
    
    __m512h result1 = _mm512_mask_blend_ph(mask32, a, b);
    
    // V32BFmode blend (same intrinsic for BF16)
    __mmask32 mask32_bf = _kor_mask32(mask32, 0x55555555);
    return _mm512_mask_blend_ph(mask32_bf, result1, c);
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int counter) {
    // V16SImode blend with varying masks
    __mmask16 mask16;
    if (counter < 10) {
        mask16 = 0xAAAA;
    } else {
        mask16 = _mm512_cmpgt_epi32_mask(a, b);
    }
    
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend
    __mmask8 mask8 = (counter % 3) ? 0x55 : 0xAA;
    return _mm512_mask_blend_epi64(mask8, result1, d);
}

// Function to prevent dead code elimination
static inline float reduce_and_checksum(__m512i vi1, __m512i vi2, __m512 vf1, __m512 vf2,
                                       __m512d vd1, __m512d vd2, __m512h vh1, __m512h vh2) {
    float checksum = 0.0f;
    
    // Reduce integer vectors
    int64_t vi1_data[8];
    int64_t vi2_data[8];
    _mm512_storeu_si512((void*)vi1_data, vi1);
    _mm512_storeu_si512((void*)vi2_data, vi2);
    
    for (int i = 0; i < 8; i++) {
        checksum += (float)(vi1_data[i] + vi2_data[i]);
    }
    
    // Reduce float vectors
    checksum += _mm512_reduce_add_ps(vf1);
    checksum += _mm512_reduce_add_ps(vf2);
    
    // Reduce double vectors
    double vd1_data[8];
    double vd2_data[8];
    _mm512_storeu_pd(vd1_data, vd1);
    _mm512_storeu_pd(vd2_data, vd2);
    
    for (int i = 0; i < 8; i++) {
        checksum += (float)(vd1_data[i] + vd2_data[i]);
    }
    
    // Reduce half-precision vectors (convert to float)
    uint16_t vh1_data[32];
    uint16_t vh2_data[32];
    _mm512_storeu_si512((void*)vh1_data, _mm512_castph_si512(vh1));
    _mm512_storeu_si512((void*)vh2_data, _mm512_castph_si512(vh2));
    
    for (int i = 0; i < 32; i++) {
        // Simple accumulation - in real code you'd convert from half to float
        checksum += (float)(vh1_data[i] + vh2_data[i]) / 65536.0f;
    }
    
    return checksum;
}

int main() {
    float total_checksum = 0.0f;
    
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
    
    __m512i vi64_3 = _mm512_set_epi64(1,2,3,4,5,6,7,8);
    __m512i vi64_4 = _mm512_set_epi64(8,7,6,5,4,3,2,1);
    
    __m512 vf32_1 = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 vf32_2 = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d vf64_1 = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d vf64_2 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    // Initialize half-precision vectors (FP16 and BF16 use same type)
    __m512h vh32_1 = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    
    __m512h vh32_2 = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512h vh32_3 = _mm512_set_ph(
        0.5f,1.5f,2.5f,3.5f,4.5f,5.5f,6.5f,7.5f,
        8.5f,9.5f,10.5f,11.5f,12.5f,13.5f,14.5f,15.5f,
        16.5f,17.5f,18.5f,19.5f,20.5f,21.5f,22.5f,23.5f,
        24.5f,25.5f,26.5f,27.5f,28.5f,29.5f,30.5f,31.5f
    );
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 20; i++) {
        __m512i blend_result1, blend_result2;
        __m512 blend_result3;
        __m512h blend_result4;
        
        // Control flow selects different blend modes
        if (i % 3 == 0) {
            // Chain blends: V64QI -> V32HI -> V16SI -> V8DI
            blend_result1 = blend_v64qi_v32hi(vi64_1, vi64_2, i);
            blend_result2 = blend_v16si_v8di(vi32_1, vi32_2, vi64_3, vi64_4, i);
            
            // Direct V16SF blend with comparison mask
            __mmask16 mask_direct = _mm512_cmp_ps_mask(vf32_1, vf32_2, _CMP_GT_OQ);
            if (i % 4 == 0) {
                mask_direct = _knot_mask16(mask_direct);
            }
            blend_result3 = _mm512_mask_blend_ps(mask_direct, vf32_1, vf32_2);
            
            // Direct V8DF blend
            __mmask8 mask_direct_df = (i % 5 == 0) ? 0xF0 : 0x0F;
            __m512d temp_df = _mm512_mask_blend_pd(mask_direct_df, vf64_1, vf64_2);
            blend_result3 = _mm512_add_ps(blend_result3, _mm512_castpd_ps(temp_df));
        } 
        else if (i % 3 == 1) {
            // Different blend chain
            blend_result1 = blend_v16si_v8di(vi32_1, vi32_2, vi64_1, vi64_2, i);
            blend_result2 = blend_v64qi_v32hi(vi64_3, vi64_4, i);
            
            blend_result3 = blend_v16sf_v8df(vf32_1, vf32_2, vf64_1, vf64_2, i);
            
            // V32HF and V32BF blends
            blend_result4 = blend_v32hf_v32bf(vh32_1, vh32_2, vh32_3, i % 2);
        } 
        else {
            // All direct blends
            // V64QImode
            __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL ^ (i * 0x1111111111111111ULL);
            blend_result1 = _mm512_mask_blend_epi8(mask64, vi64_1, vi64_2);
            
            // V32HImode
            __mmask32 mask32 = _mm512_cmpeq_epi16_mask(blend_result1, _mm512_setzero_si512());
            blend_result2 = _mm512_mask_blend_epi16(mask32, vi32_1, vi32_2);
            
            // V16SFmode
            __mmask16 mask16 = (i < 10) ? 0xAAAA : 0x5555;
            blend_result3 = _mm512_mask_blend_ps(mask16, vf32_1, vf32_2);
            
            // V32HFmode
            __mmask32 mask32_hf = 0xAAAAAAAA ^ (i * 0x11111111);
            blend_result4 = _mm512_mask_blend_ph(mask32_hf, vh32_1, vh32_2);
        }
        
        // Accumulate results to prevent elimination
        total_checksum += reduce_and_checksum(
            blend_result1, blend_result2,
            blend_result3, vf32_1,
            vf64_1, vf64_2,
            blend_result4, vh32_1
        );
    }
    
    // Additional direct blends for all modes
    // V16SImode with immediate mask
    __m512i direct_si = _mm512_mask_blend_epi32(0xAAAA, vi32_1, vi32_2);
    
    // V8DImode with immediate mask
    __m512i direct_di = _mm512_mask_blend_epi64(0xAA, vi64_3, vi64_4);
    
    // V8DFmode with comparison mask
    __mmask8 mask_df = _mm512_cmp_pd_mask(vf64_1, vf64_2, _CMP_LT_OQ);
    __m512d direct_df = _mm512_mask_blend_pd(mask_df, vf64_1, vf64_2);
    
    // V32BFmode (same as V32HFmode)
    __mmask32 mask_bf = 0x55555555;
    __m512h direct_bf = _mm512_mask_blend_ph(mask_bf, vh32_1, vh32_3);
    
    // Final accumulation
    total_checksum += reduce_and_checksum(
        direct_si, direct_di,
        _mm512_castpd_ps(direct_df), vf32_2,
        vf64_1, vf64_2,
        direct_bf, vh32_2
    );
    
    printf("Final checksum: %f\n", total_checksum);
    return 0;
}
