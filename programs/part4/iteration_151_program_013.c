#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    // Chain epi8 -> epi16 blend
    __m512i res8 = _mm512_mask_blend_epi8(k8, a, b);
    return _mm512_mask_blend_epi16(k16, res8, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, 
                                      __mmask16 k32, __mmask8 k64) {
    // Chain ps -> pd blend
    __m512 res32 = _mm512_mask_blend_ps(k32, a, b);
    __m512d res64 = _mm512_mask_blend_pd(k64, c, d);
    // Convert double result back to float for mixing
    return _mm512_add_ps(res32, _mm512_castpd_ps(res64));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 k16) {
    // FP16 blend (works for both HF and BF modes with appropriate casting)
    return _mm512_mask_blend_ph(k16, a, b);
}
#endif

// Function with data-dependent control flow
__m512i conditional_blend(int mode, __m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    if (mode & 1) {
        // Use epi8 blend
        return _mm512_mask_blend_epi8(k8, a, b);
    } else {
        // Use epi16 blend  
        return _mm512_mask_blend_epi16(k16, a, b);
    }
}

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
    
    __m512 vf32_1 = _mm512_set_ps(
        63.0f, 62.0f, 61.0f, 60.0f, 59.0f, 58.0f, 57.0f, 56.0f,
        55.0f, 54.0f, 53.0f, 52.0f, 51.0f, 50.0f, 49.0f, 48.0f,
        47.0f, 46.0f, 45.0f, 44.0f, 43.0f, 42.0f, 41.0f, 40.0f,
        39.0f, 38.0f, 37.0f, 36.0f, 35.0f, 34.0f, 33.0f, 32.0f,
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512 vf32_2 = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
        16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
        24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f,
        32.0f, 33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f,
        40.0f, 41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f,
        48.0f, 49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f,
        56.0f, 57.0f, 58.0f, 59.0f, 60.0f, 61.0f, 62.0f, 63.0f
    );
    
    __m512d vf64_1 = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    __m512d vf64_2 = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    
    // Generate masks using different methods
    // 1. Immediate constants
    __mmask64 k8_imm = 0xAAAAAAAAAAAAAAAA;
    __mmask32 k16_imm = 0xAAAAAAAA;
    __mmask16 k32_imm = 0xAAAA;
    __mmask8 k64_imm = 0xAA;
    
    // 2. Dynamic masks from comparisons
    __mmask32 k16_cmp = _mm512_cmp_epi16_mask(vi64_1, vi64_2, _MM_CMPINT_GT);
    __mmask16 k32_cmp = _mm512_cmp_epi32_mask(vi64_1, vi64_2, _MM_CMPINT_LT);
    __mmask8 k64_cmp = _mm512_cmp_epi64_mask(vi64_1, vi64_2, _MM_CMPINT_EQ);
    
    __mmask16 k32_fcmp = _mm512_cmp_ps_mask(vf32_1, vf32_2, _CMP_GT_OQ);
    __mmask8 k64_fcmp = _mm512_cmp_pd_mask(vf64_1, vf64_2, _CMP_LT_OQ);
    
    // 3. Masks from bitwise operations
    __mmask64 k8_bw = _kor_mask64(k8_imm, _mm512_cmp_epi8_mask(vi64_1, vi64_2, _MM_CMPINT_NE));
    __mmask32 k16_bw = _knot_mask32(k16_cmp);
    __mmask16 k32_bw = _kxor_mask16(k32_imm, k32_cmp);
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 4; i++) {
        // V64QImode blend
        __m512i res8 = _mm512_mask_blend_epi8(
            (i & 1) ? k8_imm : k8_bw, 
            vi64_1, 
            vi64_2
        );
        
        // V32HImode blend  
        __m512i res16 = _mm512_mask_blend_epi16(
            (i & 2) ? k16_imm : k16_bw,
            vi64_1,
            vi64_2
        );
        
        // Conditional blend based on loop index
        __m512i res_cond = conditional_blend(i, res8, res16, k8_imm, k16_cmp);
        
        // Chained blend
        __m512i res_chain = blend_v64qi_v32hi(vi64_1, vi64_2, k8_bw, k16_bw);
        
        // V16SImode blend
        __m512i res32 = _mm512_mask_blend_epi32(k32_bw, vi64_1, vi64_2);
        
        // V8DImode blend
        __m512i res64 = _mm512_mask_blend_epi64(k64_cmp, vi64_1, vi64_2);
        
        // V16SFmode blend
        __m512 res_ps = _mm512_mask_blend_ps(k32_fcmp, vf32_1, vf32_2);
        
        // V8DFmode blend
        __m512d res_pd = _mm512_mask_blend_pd(k64_fcmp, vf64_1, vf64_2);
        
        // Chained float blend
        __m512 res_float_chain = blend_v16sf_v8df(vf32_1, vf32_2, vf64_1, vf64_2, 
                                                 k32_imm, k64_imm);
        
        // Reduce results to prevent dead code elimination
        checksum += _mm512_reduce_add_epi64(res8);
        checksum += _mm512_reduce_add_epi64(res16);
        checksum += _mm512_reduce_add_epi64(res_cond);
        checksum += _mm512_reduce_add_epi64(res_chain);
        checksum += _mm512_reduce_add_epi64(res32);
        checksum += _mm512_reduce_add_epi64(res64);
        checksum += _mm512_reduce_add_ps(res_ps);
        checksum += _mm512_reduce_add_pd(res_pd);
        checksum += _mm512_reduce_add_ps(res_float_chain);
    }
    
#ifdef __AVX512FP16__
    // Initialize FP16 vectors
    __m512h vh32_1 = _mm512_set_ph(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
        16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
        24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
    );
    
    __m512h vh32_2 = _mm512_set_ph(
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    // Generate FP16 mask
    __mmask32 k16_fp16 = _mm512_cmp_ph_mask(vh32_1, vh32_2, _CMP_GT_OQ);
    
    // V32HFmode and V32BFmode blends
    __m512h res_hf = _mm512_mask_blend_ph(k16_fp16, vh32_1, vh32_2);
    __m512h res_bf = blend_v32hf_v32bf(vh32_1, vh32_2, k16_fp16);
    
    // Manual reduction for FP16
    __m256h res_hf_lo = _mm512_castph512_ph256(res_hf);
    __m256h res_hf_hi = _mm512_extractf32x8_ps(_mm512_castph_ps(res_hf), 1);
    
    for (int i = 0; i < 16; i++) {
        checksum += ((float*)&res_hf_lo)[i];
        checksum += ((float*)&res_hf_hi)[i];
    }
#endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
