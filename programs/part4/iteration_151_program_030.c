#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 mask64, __mmask32 mask32) {
    // Chain epi8 -> epi16 blend
    __m512i blend1 = _mm512_mask_blend_epi8(mask64, a, b);
    __m512i blend2 = _mm512_mask_blend_epi16(mask32, blend1, b);
    return blend2;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d,
                                      __mmask16 mask16, __mmask8 mask8) {
    // Chain ps -> pd blend
    __m512 blend1 = _mm512_mask_blend_ps(mask16, a, b);
    __m512d blend2 = _mm512_mask_blend_pd(mask8, c, d);
    // Convert pd result back to ps for mixing
    __m512 conv = _mm512_castpd_ps(blend2);
    return _mm512_add_ps(blend1, conv);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d,
                                        __mmask32 mask32a, __mmask32 mask32b) {
    // Chain two half-precision blends
    __m512h blend1 = _mm512_mask_blend_ph(mask32a, a, b);
    __m512h blend2 = _mm512_mask_blend_ph(mask32b, c, d);
    return _mm512_add_ph(blend1, blend2);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d,
                                       __mmask16 mask16, __mmask8 mask8) {
    // Chain epi32 -> epi64 blend
    __m512i blend1 = _mm512_mask_blend_epi32(mask16, a, b);
    __m512i blend2 = _mm512_mask_blend_epi64(mask8, c, d);
    return _mm512_add_epi32(blend1, blend2);
}

/* Function with data-dependent control flow */
__m512i conditional_blend(int mode, __m512i a, __m512i b, __mmask64 mask64, __mmask32 mask32) {
    __m512i result;
    
    if (mode & 1) {
        // Use immediate mask for epi8
        result = _mm512_mask_blend_epi8(0xAAAAAAAAAAAAAAAAULL, a, b);
    } else {
        // Use dynamic mask for epi16
        result = _mm512_mask_blend_epi16(mask32, a, b);
    }
    
    // Additional conditional blend
    if (mode & 2) {
        // Create mask via bitwise operations
        __mmask32 not_mask = _knot_mask32(mask32);
        __mmask32 combined = _kor_mask32(mask32, not_mask);
        result = _mm512_mask_blend_epi16(combined, result, a);
    }
    
    return result;
}

int main() {
    uint64_t final_checksum = 0;
    
    /* Initialize source vectors with distinct patterns */
    // For integer vectors
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
    
    __m512i v32hi_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    __m512i v32hi_b = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_a = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i v16si_b = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    __m512i v8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i v8di_b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    // For float vectors
    __m512 v16sf_a = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    __m512 v16sf_b = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d v8df_b = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    #ifdef __AVX512FP16__
    // For half-precision vectors
    __m512h v32hf_a = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    __m512h v32hf_b = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    #endif
    
    /* Generate masks using different methods */
    // Immediate masks
    __mmask64 mask64_imm = 0xAAAAAAAAAAAAAAAAULL;
    __mmask32 mask32_imm = 0xAAAAAAAA;
    __mmask16 mask16_imm = 0xAAAA;
    __mmask8 mask8_imm = 0xAA;
    
    // Dynamic masks from comparisons
    __mmask32 mask32_cmp = _mm512_cmp_epi16_mask(v32hi_a, v32hi_b, _MM_CMPINT_GT);
    __mmask16 mask16_cmp = _mm512_cmp_epi32_mask(v16si_a, v16si_b, _MM_CMPINT_GT);
    __mmask8 mask8_cmp = _mm512_cmp_epi64_mask(v8di_a, v8di_b, _MM_CMPINT_GT);
    __mmask16 mask16_cmp_ps = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_GT_OQ);
    __mmask8 mask8_cmp_pd = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_GT_OQ);
    
    // Masks from bitwise operations
    __mmask32 mask32_bitwise = _kor_mask32(mask32_imm, mask32_cmp);
    __mmask16 mask16_bitwise = _kand_mask16(mask16_imm, mask16_cmp);
    
    /* Loop with data-dependent control flow */
    for (int i = 0; i < 4; i++) {
        __m512i result_int = conditional_blend(i, v64qi_a, v64qi_b, mask64_imm, mask32_cmp);
        
        // Horizontal sum to prevent dead code elimination
        uint64_t sum = _mm512_reduce_add_epi64(result_int);
        final_checksum += sum;
    }
    
    /* Perform blends for all modes */
    // V64QImode
    __m512i blend64qi = _mm512_mask_blend_epi8(mask64_imm, v64qi_a, v64qi_b);
    final_checksum += _mm512_reduce_add_epi64(blend64qi);
    
    // V32HImode
    __m512i blend32hi = _mm512_mask_blend_epi16(mask32_cmp, v32hi_a, v32hi_b);
    final_checksum += _mm512_reduce_add_epi64(blend32hi);
    
    // V16SImode
    __m512i blend16si = _mm512_mask_blend_epi32(mask16_bitwise, v16si_a, v16si_b);
    final_checksum += _mm512_reduce_add_epi64(blend16si);
    
    // V8DImode
    __m512i blend8di = _mm512_mask_blend_epi64(mask8_cmp, v8di_a, v8di_b);
    final_checksum += _mm512_reduce_add_epi64(blend8di);
    
    // V16SFmode
    __m512 blend16sf = _mm512_mask_blend_ps(mask16_cmp_ps, v16sf_a, v16sf_b);
    float sum16sf = _mm512_reduce_add_ps(blend16sf);
    final_checksum += (uint64_t)sum16sf;
    
    // V8DFmode
    __m512d blend8df = _mm512_mask_blend_pd(mask8_cmp_pd, v8df_a, v8df_b);
    double sum8df = _mm512_reduce_add_pd(blend8df);
    final_checksum += (uint64_t)sum8df;
    
    #ifdef __AVX512FP16__
    // V32HFmode and V32BFmode
    __mmask32 mask32_hf_cmp = _mm512_cmp_ph_mask(v32hf_a, v32hf_b, _CMP_GT_OQ);
    __m512h blend32hf = _mm512_mask_blend_ph(mask32_hf_cmp, v32hf_a, v32hf_b);
    
    // Manual reduction for half precision
    __m256h low = _mm512_castph512_ph256(blend32hf);
    __m256h high = _mm512_extractf32x8_ps(_mm512_castph_ps(blend32hf), 1);
    __m512h reduced = _mm512_castph256_ph512(_mm256_add_ph(low, high));
    // Continue reduction...
    #endif
    
    /* Chained blend operations */
    __m512i chained_int = blend_v64qi_v32hi(v64qi_a, v64qi_b, mask64_imm, mask32_cmp);
    final_checksum += _mm512_reduce_add_epi64(chained_int);
    
    __m512 chained_float = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, 
                                           mask16_cmp_ps, mask8_cmp_pd);
    float sum_chained = _mm512_reduce_add_ps(chained_float);
    final_checksum += (uint64_t)sum_chained;
    
    __m512i chained_si_di = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b,
                                            mask16_bitwise, mask8_cmp);
    final_checksum += _mm512_reduce_add_epi64(chained_si_di);
    
    #ifdef __AVX512FP16__
    __m512h chained_hf = blend_v32hf_v32bf(v32hf_a, v32hf_b, v32hf_a, v32hf_b,
                                          mask32_hf_cmp, mask32_imm);
    // Add to checksum...
    #endif
    
    printf("Final checksum: %lu\n", final_checksum);
    return 0;
}
