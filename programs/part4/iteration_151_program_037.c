#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int selector) {
    // V64QImode blend
    __mmask64 mask64 = selector ? 0xAAAAAAAAAAAAAAAAULL : 0x5555555555555555ULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamic mask from comparison
    __m512i cmp_val = _mm512_set1_epi16(selector * 100);
    __mmask32 mask32 = _mm512_cmp_epi16_mask(result1, cmp_val, _MM_CMPINT_GT);
    return _mm512_mask_blend_epi16(mask32, result1, _mm512_add_epi16(b, _mm512_set1_epi16(1)));
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // V16SFmode blend with mask from bitwise operations
    __mmask16 mask16_float = 0xAAAA;
    if (iter % 3 == 0) {
        mask16_float = _knot_mask16(mask16_float);
    }
    __m512 result1 = _mm512_mask_blend_ps(mask16_float, a, b);
    
    // V8DFmode blend with comparison mask
    __mmask8 mask8_double = _mm512_cmp_pd_mask(c, d, _CMP_LT_OQ);
    __m512d blended_double = _mm512_mask_blend_pd(mask8_double, c, d);
    
    // Convert double result back to float for chaining
    return _mm512_add_ps(result1, _mm512_castpd_ps(blended_double));
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    // V32HFmode blend with alternating pattern
    __mmask32 mask32_half = pattern ? 0xAAAAAAAA : 0x55555555;
    __m512h result = _mm512_mask_blend_ph(mask32_half, a, b);
    
    // V32BFmode blend with inverted mask
    __mmask32 mask32_bfloat = _knot_mask32(mask32_half);
    return _mm512_mask_blend_ph(mask32_bfloat, result, _mm512_add_ph(b, _mm512_set1_ph(1.0f)));
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int threshold) {
    // V16SImode blend with comparison mask
    __mmask16 mask16_int = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LT);
    __m512i result1 = _mm512_mask_blend_epi32(mask16_int, a, b);
    
    // V8DImode blend with combined masks
    __mmask8 mask8_long = _mm512_cmp_epi64_mask(c, d, _MM_CMPINT_EQ);
    if (threshold > 0) {
        mask8_long = _kor_mask8(mask8_long, 0x0F);
    }
    return _mm512_mask_blend_epi64(mask8_long, result1, c);
}

// Function to perform horizontal reduction and prevent dead code elimination
static inline float reduce_and_checksum(__m512i vi1, __m512i vi2, __m512 vf1, __m512 vf2,
                                       __m512d vd1, __m512d vd2, __m512h vh1, __m512h vh2) {
    float checksum = 0.0f;
    
    // Reduce integer vectors
    int64_t vi1_sum[8] = {0};
    int64_t vi2_sum[8] = {0};
    _mm512_store_epi64(vi1_sum, vi1);
    _mm512_store_epi64(vi2_sum, vi2);
    for (int i = 0; i < 8; i++) {
        checksum += (float)(vi1_sum[i] + vi2_sum[i]);
    }
    
    // Reduce float vectors
    checksum += _mm512_reduce_add_ps(vf1);
    checksum += _mm512_reduce_add_ps(vf2);
    
    // Reduce double vectors
    double vd_sum[8];
    _mm512_store_pd(vd_sum, vd1);
    for (int i = 0; i < 8; i++) {
        checksum += (float)vd_sum[i];
    }
    
    // Reduce half-precision vectors (convert to float first)
    __m512 vh1_float = _mm512_cvtph_ps(vh1);
    __m512 vh2_float = _mm512_cvtph_ps(vh2);
    checksum += _mm512_reduce_add_ps(vh1_float);
    checksum += _mm512_reduce_add_ps(vh2_float);
    
    return checksum;
}

int main() {
    float final_checksum = 0.0f;
    
    // Initialize vectors with distinct patterns
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set_epi16(
        100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,
        116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131
    );
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.5f,14.0f,13.5f,13.0f,12.5f,12.0f,11.5f,
        11.0f,10.5f,10.0f,9.5f,9.0f,8.5f,8.0f,7.5f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        1.0f,1.5f,2.0f,2.5f,3.0f,3.5f,4.0f,4.5f,
        5.0f,5.5f,6.0f,6.5f,7.0f,7.5f,8.0f,8.5f
    );
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.5,6.0,5.5,5.0,4.5,4.0,3.5);
    __m512d v8df_b = _mm512_set_pd(1.0,1.5,2.0,2.5,3.0,3.5,4.0,4.5);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set_epi32(
        100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115
    );
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set_epi64(100,101,102,103,104,105,106,107);
    
    // Initialize half-precision vectors
    __m512h v32hf_a = _mm512_set_ph(
        31.0f,30.5f,30.0f,29.5f,29.0f,28.5f,28.0f,27.5f,
        27.0f,26.5f,26.0f,25.5f,25.0f,24.5f,24.0f,23.5f,
        23.0f,22.5f,22.0f,21.5f,21.0f,20.5f,20.0f,19.5f,
        19.0f,18.5f,18.0f,17.5f,17.0f,16.5f,16.0f,15.5f
    );
    
    __m512h v32hf_b = _mm512_set_ph(
        1.0f,1.5f,2.0f,2.5f,3.0f,3.5f,4.0f,4.5f,
        5.0f,5.5f,6.0f,6.5f,7.0f,7.5f,8.0f,8.5f,
        9.0f,9.5f,10.0f,10.5f,11.0f,11.5f,12.0f,12.5f,
        13.0f,13.5f,14.0f,14.5f,15.0f,15.5f,16.0f,16.5f
    );
    
    // Data-dependent control flow with loops
    for (int iter = 0; iter < 10; iter++) {
        __m512i blend_result1, blend_result2;
        __m512 blend_result3;
        __m512h blend_result4;
        
        // Conditional blend selection
        if (iter % 2 == 0) {
            // Chain blends through different modes
            blend_result1 = blend_v64qi_v32hi(v64qi_a, v64qi_b, iter);
            blend_result2 = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, iter);
        } else {
            // Direct intrinsic calls for all modes
            __mmask64 mask64 = (iter % 3 == 0) ? 0xF0F0F0F0F0F0F0F0ULL : 0x0F0F0F0F0F0F0F0FULL;
            blend_result1 = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
            
            __mmask32 mask32 = _mm512_cmp_epi16_mask(v32hi_a, v32hi_b, _MM_CMPINT_LT);
            blend_result1 = _mm512_mask_blend_epi16(mask32, blend_result1, v32hi_b);
            
            __mmask16 mask16 = (__mmask16)(0xAAAA ^ iter);
            blend_result2 = _mm512_mask_blend_epi32(mask16, v16si_a, v16si_b);
            
            __mmask8 mask8 = _mm512_cmp_epi64_mask(v8di_a, v8di_b, _MM_CMPINT_GT);
            blend_result2 = _mm512_mask_blend_epi64(mask8, blend_result2, v8di_b);
        }
        
        // Always execute float blends
        blend_result3 = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, iter);
        
        // Execute half-precision blends conditionally
        if (iter > 5) {
            blend_result4 = blend_v32hf_v32bf(v32hf_a, v32hf_b, iter % 4);
        } else {
            __mmask32 mask32_half = 0xAAAAAAAA;
            blend_result4 = _mm512_mask_blend_ph(mask32_half, v32hf_a, v32hf_b);
        }
        
        // Accumulate results
        final_checksum += reduce_and_checksum(
            blend_result1, blend_result2,
            blend_result3, v16sf_b,
            v8df_a, v8df_b,
            blend_result4, v32hf_b
        );
    }
    
    // Additional test: Nested loops with mode transitions
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            __mmask64 mask64 = (i + j) % 2 ? 0xFFFFFFFFFFFFFFFFULL : 0x0000000000000000ULL;
            __m512i temp = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
            
            __mmask32 mask32 = _mm512_cmp_epi16_mask(temp, v32hi_b, _MM_CMPINT_EQ);
            temp = _mm512_mask_blend_epi16(mask32, temp, v32hi_a);
            
            // Use result in float blend
            __m512 temp_float = _mm512_cvtepi32_ps(_mm512_castsi512_si256(temp));
            __mmask16 mask16 = 0x5555;
            __m512 blended_float = _mm512_mask_blend_ps(mask16, temp_float, v16sf_b);
            
            final_checksum += _mm512_reduce_add_ps(blended_float);
        }
    }
    
    printf("Final checksum: %f\n", final_checksum);
    return 0;
}
