#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend modes
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    __mmask64 mask64;
    __mmask32 mask32;
    
    if (mode_selector & 1) {
        // Immediate mask for V64QImode
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Dynamic mask from comparison
        __m512i cmp_a = _mm512_set1_epi8(32);
        __m512i cmp_b = _mm512_set1_epi8(64);
        mask64 = _mm512_cmpgt_epi8_mask(cmp_b, cmp_a);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Generate mask for V32HImode using bitwise operations
    __mmask32 mask_lo = 0x55555555;
    __mmask32 mask_hi = 0xAAAAAAAA;
    mask32 = _kor_mask32(mask_lo, mask_hi);
    mask32 = _knot_mask32(mask32);
    
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // Generate mask for V16SFmode using comparison
    __mmask16 mask16_sf = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    // Modify mask based on iteration
    if (iter & 1) {
        mask16_sf = _knot_mask16(mask16_sf);
    }
    
    __m512 result_sf = _mm512_mask_blend_ps(mask16_sf, a, b);
    
    // Generate mask for V8DFmode
    __mmask8 mask8_df;
    if (iter & 2) {
        // Immediate mask
        mask8_df = 0xAA;
    } else {
        // Comparison mask
        mask8_df = _mm512_cmp_pd_mask(c, d, _CMP_GT_OQ);
    }
    
    __m512d result_df = _mm512_mask_blend_pd(mask8_df, c, d);
    
    // Convert double result to float for chaining
    return _mm512_add_ps(result_sf, _mm512_castpd_ps(result_df));
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d) {
    // Generate mask for V16SImode using comparison
    __mmask16 mask16_si = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LT);
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16_si, a, b);
    
    // Generate mask for V8DImode using bitwise operations
    __mmask8 mask8_di = 0x55;
    mask8_di = _kor_mask8(mask8_di, 0xAA);
    
    return _mm512_mask_blend_epi64(mask8_di, result_si, c);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d) {
    // Generate mask for V32HFmode
    __mmask32 mask32_hf = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    
    __m512h result_hf = _mm512_mask_blend_ph(mask32_hf, a, b);
    
    // Generate different mask for second blend
    __mmask32 mask32_bf = _mm512_cmp_ph_mask(c, d, _CMP_NEQ_UQ);
    
    return _mm512_mask_blend_ph(mask32_bf, result_hf, c);
}
#endif

int main() {
    // Initialize vectors with distinct patterns
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
    
    __m512i vi3 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i vi4 = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vi5 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i vi6 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    __m512 vf1 = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 vf2 = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d vd1 = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d vd2 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    #ifdef __AVX512FP16__
    __m512h vh1 = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    
    __m512h vh2 = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    #endif
    
    float checksum = 0.0f;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        __m512i result_int;
        __m512 result_float;
        
        if (i % 3 == 0) {
            // Chain blends through different modes
            result_int = blend_v64qi_v32hi(vi1, vi2, i);
            
            // Direct blend for V16SImode with immediate mask
            __mmask16 mask_imm = 0xAAAA;
            __m512i blend_si = _mm512_mask_blend_epi32(mask_imm, vi3, vi4);
            
            // Combine results
            result_int = _mm512_add_epi32(result_int, blend_si);
            
            // Reduction
            int64_t sum_int = _mm512_reduce_add_epi64(result_int);
            checksum += (float)sum_int;
        } 
        else if (i % 3 == 1) {
            // Float blends
            result_float = blend_v16sf_v8df(vf1, vf2, vd1, vd2, i);
            
            // Direct blend for V8DFmode with comparison mask
            __mmask8 mask_df = _mm512_cmp_pd_mask(vd1, vd2, _CMP_LT_OQ);
            __m512d blend_df = _mm512_mask_blend_pd(mask_df, vd1, vd2);
            
            // Convert and add
            result_float = _mm512_add_ps(result_float, _mm512_castpd_ps(blend_df));
            
            // Reduction
            float sum_float = _mm512_reduce_add_ps(result_float);
            checksum += sum_float;
        } 
        else {
            // Integer blends with different mask patterns
            result_int = blend_v16si_v8di(vi3, vi4, vi5, vi6);
            
            // Direct blend for V64QImode with dynamic mask
            __m512i cmp_val = _mm512_set1_epi8(i);
            __mmask64 mask_dyn = _mm512_cmpeq_epi8_mask(vi1, cmp_val);
            __m512i blend_qi = _mm512_mask_blend_epi8(mask_dyn, vi1, vi2);
            
            // Combine
            result_int = _mm512_add_epi32(result_int, blend_qi);
            
            // Reduction
            int64_t sum_int = _mm512_reduce_add_epi64(result_int);
            checksum += (float)sum_int;
        }
        
        #ifdef __AVX512FP16__
        if (i % 2 == 0) {
            // FP16 blends
            __m512h result_half = blend_v32hf_v32bf(vh1, vh2, vh1, vh2);
            
            // Direct blend for V32HFmode with immediate mask
            __mmask32 mask_hf_imm = 0x55555555;
            __m512h blend_hf = _mm512_mask_blend_ph(mask_hf_imm, vh1, vh2);
            
            // Manual reduction for half precision
            __m256h low = _mm512_castph512_ph256(result_half);
            __m256h high = _mm512_extractf32x8_ps(_mm512_castph_ps(result_half), 1);
            // Simple accumulation (actual reduction would need more operations)
            checksum += 1.0f; // Placeholder
        }
        #endif
    }
    
    // Additional direct blends with various mask generation methods
    __mmask32 mask_mixed = _kor_mask32(0x33333333, 0xCCCCCCCC);
    __m512i final_blend_hi = _mm512_mask_blend_epi16(mask_mixed, vi1, vi2);
    
    __mmask16 mask_from_cmp = _mm512_cmp_epi32_mask(vi3, vi4, _MM_CMPINT_EQ);
    __m512i final_blend_si = _mm512_mask_blend_epi32(mask_from_cmp, vi3, vi4);
    
    // Combine and reduce
    __m512i combined = _mm512_add_epi32(final_blend_hi, final_blend_si);
    int64_t final_sum = _mm512_reduce_add_epi64(combined);
    checksum += (float)final_sum;
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
