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
        // Dynamic mask using comparison
        __m512i cmp_a = _mm512_set1_epi8(0x40);
        __m512i cmp_b = _mm512_set1_epi8(0x20);
        mask64 = _mm512_cmpgt_epi8_mask(cmp_a, cmp_b);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Chain to V32HImode
    __m512i ones = _mm512_set1_epi16(1);
    __m512i zeros = _mm512_set1_epi16(0);
    mask32 = _mm512_cmpeq_epi16_mask(ones, zeros);
    mask32 = _knot_mask32(mask32); // Invert mask
    
    return _mm512_mask_blend_epi16(mask32, result1, a);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // V16SFmode blend with immediate mask
    mask16 = selector ? 0xAAAA : 0x5555;
    __m512 result_sf = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend with dynamic mask
    __m512d cmp_val1 = _mm512_set1_pd(2.0);
    __m512d cmp_val2 = _mm512_set1_pd(1.0);
    mask8 = _mm512_cmp_pd_mask(cmp_val1, cmp_val2, _CMP_GT_OQ);
    
    // Chain operations: use float result to influence double blend
    __m512d temp = _mm512_cvtepi32_pd(_mm512_cvttps_epi32(result_sf));
    return _mm512_castpd_ps(_mm512_mask_blend_pd(mask8, c, _mm512_add_pd(d, temp)));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    __mmask32 mask32;
    
    // Generate different mask patterns based on input
    switch (pattern & 3) {
        case 0:
            mask32 = 0xAAAAAAAA; // Immediate
            break;
        case 1:
            mask32 = _mm512_cmp_ph_mask(a, b, _CMP_LT_OQ);
            break;
        case 2:
            __mmask32 temp_mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(0.0f), _CMP_GT_OQ);
            mask32 = _kor_mask32(temp_mask, 0x55555555); // OR with immediate
            break;
        default:
            mask32 = _knot_mask32(0xFFFFFFFF); // All ones inverted
            break;
    }
    
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int iter) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // V16SImode with comparison-based mask
    __m512i threshold = _mm512_set1_epi32(iter * 10);
    mask16 = _mm512_cmpgt_epi32_mask(a, threshold);
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode with immediate mask
    mask8 = (iter % 2) ? 0xAA : 0x55;
    
    // Chain operations
    __m512i shifted = _mm512_slli_epi64(result_si, 1);
    return _mm512_mask_blend_epi64(mask8, c, _mm512_add_epi64(d, shifted));
}

// Reduction helpers
static inline int64_t reduce_i64(__m512i v) {
    __m256i vlow = _mm512_castsi512_si256(v);
    __m256i vhigh = _mm512_extracti64x4_epi64(v, 1);
    __m256i sum256 = _mm256_add_epi64(vlow, vhigh);
    __m128i sum128 = _mm_add_epi64(_mm256_castsi256_si128(sum256),
                                   _mm256_extracti128_si256(sum256, 1));
    return _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
}

static inline float reduce_ps(__m512 v) {
    __m256 vlow = _mm512_castps512_ps256(v);
    __m256 vhigh = _mm512_extractf32x8_ps(v, 1);
    __m256 sum256 = _mm256_add_ps(vlow, vhigh);
    __m128 sum128 = _mm_add_ps(_mm256_castps256_ps128(sum256),
                               _mm256_extractf128_ps(sum256, 1));
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    return _mm_cvtss_f32(sum128);
}

static inline double reduce_pd(__m512d v) {
    __m256d vlow = _mm512_castpd512_pd256(v);
    __m256d vhigh = _mm512_extractf64x4_pd(v, 1);
    __m256d sum256 = _mm256_add_pd(vlow, vhigh);
    __m128d sum128 = _mm_add_pd(_mm256_castpd256_pd128(sum256),
                                _mm256_extractf128_pd(sum256, 1));
    sum128 = _mm_hadd_pd(sum128, sum128);
    return _mm_cvtsd_f64(sum128);
}

int main() {
    int64_t final_checksum = 0;
    
    // Initialize data with distinct patterns
    int8_t i8_data[64];
    int16_t i16_data[32];
    int32_t i32_data[16];
    int64_t i64_data[8];
    float f32_data[16];
    double f64_data[8];
    
    for (int i = 0; i < 64; i++) i8_data[i] = (i % 3) - 1;
    for (int i = 0; i < 32; i++) i16_data[i] = (i * 7) % 100;
    for (int i = 0; i < 16; i++) i32_data[i] = i * i - 50;
    for (int i = 0; i < 8; i++) i64_data[i] = (i64_t)i * 1000000000LL;
    for (int i = 0; i < 16; i++) f32_data[i] = (i % 2) ? i * 0.5f : -i * 0.25f;
    for (int i = 0; i < 8; i++) f64_data[i] = (i % 3) ? i * 1.5 : -i * 0.75;
    
    __m512i vi8_a = _mm512_loadu_si512(i8_data);
    __m512i vi8_b = _mm512_slli_epi8(vi8_a, 1);
    
    __m512i vi16_a = _mm512_loadu_si512(i16_data);
    __m512i vi16_b = _mm512_srai_epi16(vi16_a, 2);
    
    __m512i vi32_a = _mm512_loadu_si512(i32_data);
    __m512i vi32_b = _mm512_slli_epi32(vi32_a, 1);
    
    __m512i vi64_a = _mm512_loadu_si512(i64_data);
    __m512i vi64_b = _mm512_add_epi64(vi64_a, _mm512_set1_epi64(1));
    
    __m512 vf32_a = _mm512_loadu_ps(f32_data);
    __m512 vf32_b = _mm512_mul_ps(vf32_a, _mm512_set1_ps(2.0f));
    
    __m512d vf64_a = _mm512_loadu_pd(f64_data);
    __m512d vf64_b = _mm512_div_pd(vf64_a, _mm512_set1_pd(3.0));
    
    // Data-dependent control flow
    for (int iter = 0; iter < 4; iter++) {
        if (iter % 2 == 0) {
            // Blend integer vectors
            __m512i blended_i = blend_v64qi_v32hi(vi8_a, vi8_b, iter);
            __m512i blended_i2 = blend_v16si_v8di(vi32_a, vi32_b, vi64_a, vi64_b, iter);
            
            final_checksum += reduce_i64(blended_i);
            final_checksum += reduce_i64(blended_i2);
        } else {
            // Blend float vectors
            __m512 blended_f = blend_v16sf_v8df(vf32_a, vf32_b, vf64_a, vf64_b, iter);
            
            final_checksum += (int64_t)reduce_ps(blended_f);
            final_checksum += (int64_t)reduce_pd(_mm512_castps_pd(blended_f));
        }
        
        // Direct intrinsic calls for all modes
        __mmask64 m64 = (iter == 0) ? 0xCCCCCCCCCCCCCCCCULL : 
                       _mm512_cmpeq_epi8_mask(vi8_a, vi8_b);
        __m512i direct1 = _mm512_mask_blend_epi8(m64, vi8_a, vi8_b);
        final_checksum += reduce_i64(direct1);
        
        __mmask32 m32 = _mm512_cmpgt_epi16_mask(vi16_a, vi16_b);
        __m512i direct2 = _mm512_mask_blend_epi16(m32, vi16_a, vi16_b);
        final_checksum += reduce_i64(direct2);
        
        __mmask16 m16 = (iter < 2) ? 0xF0F0 : 0x0F0F;
        __m512i direct3 = _mm512_mask_blend_epi32(m16, vi32_a, vi32_b);
        final_checksum += reduce_i64(direct3);
        
        __mmask8 m8 = _mm512_cmpgt_epi64_mask(vi64_a, vi64_b);
        __m512i direct4 = _mm512_mask_blend_epi64(m8, vi64_a, vi64_b);
        final_checksum += reduce_i64(direct4);
        
        __mmask16 mf16 = _mm512_cmp_ps_mask(vf32_a, vf32_b, _CMP_LT_OQ);
        __m512 direct5 = _mm512_mask_blend_ps(mf16, vf32_a, vf32_b);
        final_checksum += (int64_t)reduce_ps(direct5);
        
        __mmask8 md8 = _mm512_cmp_pd_mask(vf64_a, vf64_b, _CMP_GT_OQ);
        __m512d direct6 = _mm512_mask_blend_pd(md8, vf64_a, vf64_b);
        final_checksum += (int64_t)reduce_pd(direct6);
        
        #ifdef __AVX512FP16__
        // Initialize FP16 data
        _Float16 f16_data[32];
        for (int i = 0; i < 32; i++) f16_data[i] = (_Float16)(i * 0.1f);
        
        __m512h vf16_a = _mm512_loadu_ph(f16_data);
        __m512h vf16_b = _mm512_mul_ph(vf16_a, _mm512_set1_ph(2.0f));
        
        __m512h blended_hf = blend_v32hf_v32bf(vf16_a, vf16_b, iter);
        
        // Manual reduction for __m512h
        _Float16 hsum = 0;
        for (int i = 0; i < 32; i++) {
            hsum += blended_hf[i];
        }
        final_checksum += (int64_t)hsum;
        #endif
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    return 0;
}
