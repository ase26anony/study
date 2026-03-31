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
        __m512i cmp_a = _mm512_set1_epi8(mode_selector);
        __m512i cmp_b = _mm512_set1_epi8(mode_selector ^ 0xFF);
        mask64 = _mm512_cmpgt_epi8_mask(cmp_a, cmp_b);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Chain to V32HImode blend
    __m512i ones = _mm512_set1_epi16(1);
    mask32 = _mm512_cmpeq_epi16_mask(result1, ones);
    
    // Modify mask with bitwise operations
    __mmask32 not_mask = _knot_mask32(mask32);
    __mmask32 alt_mask = _kor_mask32(mask32, 0x55555555);
    
    return _mm512_mask_blend_epi16(alt_mask, result1, _mm512_slli_epi16(b, 1));
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // V16SFmode blend with comparison mask
    __mmask16 mask16_sf = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    // Modify mask based on iteration
    if (iter & 1) {
        mask16_sf = _knot_mask16(mask16_sf);
    }
    
    __m512 result_sf = _mm512_mask_blend_ps(mask16_sf, a, b);
    
    // Chain to V8DFmode blend
    __mmask8 mask8_df;
    if (iter & 2) {
        // Immediate mask
        mask8_df = 0xAA;
    } else {
        // Comparison mask
        mask8_df = _mm512_cmp_pd_mask(_mm512_castps_pd(result_sf), 
                                     _mm512_castps_pd(b), _CMP_GT_OQ);
    }
    
    return _mm512_castpd_ps(_mm512_mask_blend_pd(mask8_df, 
                                                _mm512_castps_pd(result_sf), 
                                                _mm512_castps_pd(d)));
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int selector) {
    // V16SImode blend
    __mmask16 mask16_si;
    
    if (selector > 0) {
        // Comparison-based mask
        mask16_si = _mm512_cmpgt_epi32_mask(a, b);
    } else {
        // Immediate mask
        mask16_si = 0xAAAA;
    }
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16_si, a, b);
    
    // Chain to V8DImode blend
    __mmask8 mask8_di = _mm512_cmpgt_epi64_mask(result_si, c);
    
    // Combine with another mask
    __mmask8 const_mask = 0x55;
    __mmask8 final_mask = _kor_mask8(mask8_di, const_mask);
    
    return _mm512_mask_blend_epi64(final_mask, result_si, d);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d, int pattern) {
    // V32HFmode blend
    __mmask32 mask32_hf;
    
    switch (pattern % 3) {
        case 0:
            mask32_hf = 0xAAAAAAAA;  // Immediate
            break;
        case 1:
            mask32_hf = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
            break;
        case 2:
            mask32_hf = _mm512_cmp_ph_mask(a, c, _CMP_LT_OQ);
            break;
    }
    
    __m512h result_hf = _mm512_mask_blend_ph(mask32_hf, a, b);
    
    // Chain another V32HFmode blend with modified mask
    __mmask32 not_mask = _knot_mask32(mask32_hf);
    __m512h result2 = _mm512_mask_blend_ph(not_mask, result_hf, c);
    
    // For V32BFmode (treated same as HF in current intrinsics)
    __mmask32 mask32_bf = _mm512_cmp_ph_mask(result2, d, _CMP_NEQ_UQ);
    return _mm512_mask_blend_ph(mask32_bf, result2, d);
}
#endif

int main() {
    // Initialize pattern data
    uint8_t pattern8[64];
    int16_t pattern16[32];
    int32_t pattern32[16];
    int64_t pattern64[8];
    float patternf[16];
    double patternd[8];
    
    for (int i = 0; i < 64; i++) pattern8[i] = i;
    for (int i = 0; i < 32; i++) pattern16[i] = i * 2;
    for (int i = 0; i < 16; i++) pattern32[i] = i * 4;
    for (int i = 0; i < 8; i++) pattern64[i] = i * 8;
    for (int i = 0; i < 16; i++) patternf[i] = i * 1.5f;
    for (int i = 0; i < 8; i++) patternd[i] = i * 2.5;
    
    __m512i vec8_a = _mm512_loadu_si512(pattern8);
    __m512i vec8_b = _mm512_loadu_si512((uint8_t[64]){1});
    
    __m512i vec16_a = _mm512_loadu_si512(pattern16);
    __m512i vec16_b = _mm512_loadu_si512((int16_t[32]){2});
    
    __m512i vec32_a = _mm512_loadu_si512(pattern32);
    __m512i vec32_b = _mm512_loadu_si512((int32_t[16]){4});
    
    __m512i vec64_a = _mm512_loadu_si512(pattern64);
    __m512i vec64_b = _mm512_loadu_si512((int64_t[8]){8});
    
    __m512 vecf_a = _mm512_loadu_ps(patternf);
    __m512 vecf_b = _mm512_set1_ps(3.14f);
    
    __m512d vecd_a = _mm512_loadu_pd(patternd);
    __m512d vecd_b = _mm512_set1_pd(2.718);
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Loop with data-dependent control flow
    for (int iter = 0; iter < 10; iter++) {
        // V64QImode and V32HImode blends
        __m512i result1 = blend_v64qi_v32hi(vec8_a, vec8_b, iter);
        
        // V16SImode and V8DImode blends
        __m512i result2 = blend_v16si_v8di(vec32_a, vec32_b, vec64_a, vec64_b, iter);
        
        // V16SFmode and V8DFmode blends
        __m512 result3 = blend_v16sf_v8df(vecf_a, vecf_b, vecd_a, vecd_b, iter);
        
        // Horizontal reductions to prevent dead code elimination
        __m256i sum1 = _mm512_castsi512_si256(_mm512_add_epi64(
            _mm512_unpacklo_epi64(result1, _mm512_setzero_si512()),
            _mm512_unpackhi_epi64(result1, _mm512_setzero_si512())
        ));
        
        __m128i sum2 = _mm_add_epi64(
            _mm256_castsi256_si128(sum1),
            _mm256_extracti128_si256(sum1, 1)
        );
        
        uint64_t sum_val = _mm_extract_epi64(sum2, 0) + _mm_extract_epi64(sum2, 1);
        checksum += (double)sum_val;
        
        // Reduce float results
        checksum += _mm512_reduce_add_ps(result3);
        
        // Direct blends for each mode in control flow
        if (iter % 3 == 0) {
            // V64QImode with immediate mask
            __m512i blend8 = _mm512_mask_blend_epi8(0xCCCCCCCCCCCCCCCCULL, 
                                                   vec8_a, vec8_b);
            checksum += _mm512_reduce_add_epi64(blend8);
        } else if (iter % 3 == 1) {
            // V32HImode with comparison mask
            __mmask32 mask32 = _mm512_cmpeq_epi16_mask(vec16_a, vec16_b);
            __m512i blend16 = _mm512_mask_blend_epi16(mask32, vec16_a, vec16_b);
            checksum += _mm512_reduce_add_epi64(blend16);
        } else {
            // V16SImode with bitwise mask operations
            __mmask16 mask16 = _mm512_cmpgt_epi32_mask(vec32_a, vec32_b);
            __mmask16 inv_mask = _knot_mask16(mask16);
            __m512i blend32 = _mm512_mask_blend_epi32(inv_mask, vec32_a, vec32_b);
            checksum += _mm512_reduce_add_epi64(blend32);
        }
        
        // Always execute V8DImode blend
        __mmask8 mask8 = _mm512_cmpgt_epi64_mask(vec64_a, vec64_b);
        __m512i blend64 = _mm512_mask_blend_epi64(mask8, vec64_a, vec64_b);
        checksum += _mm512_reduce_add_epi64(blend64);
        
        // Float blends
        __mmask16 maskf = _mm512_cmp_ps_mask(vecf_a, vecf_b, _CMP_LT_OQ);
        __m512 blendf = _mm512_mask_blend_ps(maskf, vecf_a, vecf_b);
        checksum += _mm512_reduce_add_ps(blendf);
        
        __mmask8 maskd = _mm512_cmp_pd_mask(vecd_a, vecd_b, _CMP_GT_OQ);
        __m512d blendd = _mm512_mask_blend_pd(maskd, vecd_a, vecd_b);
        
        // Manual reduction for double
        __m256d sumd_hi = _mm512_extractf64x4_pd(blendd, 1);
        __m256d sumd_lo = _mm512_castpd512_pd256(blendd);
        __m256d sumd = _mm256_add_pd(sumd_hi, sumd_lo);
        __m128d sumd_128 = _mm_add_pd(_mm256_extractf128_pd(sumd, 1),
                                     _mm256_castpd256_pd128(sumd));
        checksum += _mm_cvtsd_f64(_mm_add_sd(sumd_128,
                                           _mm_unpackhi_pd(sumd_128, sumd_128)));
    }
    
#ifdef __AVX512FP16__
    // Initialize FP16 data
    _Float16 patternh[32];
    for (int i = 0; i < 32; i++) patternh[i] = i * 0.5f;
    
    __m512h vech_a = _mm512_loadu_ph(patternh);
    __m512h vech_b = _mm512_set1_ph(1.0f);
    __m512h vech_c = _mm512_set1_ph(2.0f);
    __m512h vech_d = _mm512_set1_ph(3.0f);
    
    for (int i = 0; i < 5; i++) {
        __m512h resulth = blend_v32hf_v32bf(vech_a, vech_b, vech_c, vech_d, i);
        
        // Reduce FP16 values
        __m256h resulth_hi = _mm512_castph512_ph256(resulth);
        __m256h resulth_lo = _mm512_extractf32x8_ph(resulth, 1);
        
        // Convert to float for reduction
        __m512 resulth_f = _mm512_cvtph_ps(resulth);
        checksum += _mm512_reduce_add_ps(resulth_f);
    }
#endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
