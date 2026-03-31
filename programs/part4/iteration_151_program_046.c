#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend operations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    // V64QImode blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamic mask
    __mmask32 mask32 = _mm512_cmpeq_epi16_mask(a, b);
    if (mode_selector > 0) {
        mask32 = _knot_mask32(mask32);
    }
    __m512i result2 = _mm512_mask_blend_epi16(mask32, result1, b);
    
    return result2;
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __mmask16 mask16) {
    // V16SImode blend
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend with modified mask
    __mmask8 mask8 = _mm512_cmpgt_epi64_mask(a, b);
    mask8 = _kor_mask8(mask8, 0xAA);  // Combine with constant
    __m512i result2 = _mm512_mask_blend_epi64(mask8, result1, b);
    
    return result2;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512 c, __m512 d) {
    // V16SFmode blend with comparison mask
    __mmask16 mask16_float = _mm512_cmp_ps_mask(a, b, _CMP_LT_OS);
    __m512 result1 = _mm512_mask_blend_ps(mask16_float, a, b);
    
    // V8DFmode blend - convert float mask to double mask
    __mmask8 mask8_double = _mm512_cmp_pd_mask(_mm512_castps_pd(result1), 
                                              _mm512_castps_pd(c), _CMP_GT_OS);
    __m512d result2_double = _mm512_mask_blend_pd(mask8_double, 
                                                 _mm512_castps_pd(result1), 
                                                 _mm512_castps_pd(d));
    
    return _mm512_castpd_ps(result2_double);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c) {
    // V32HFmode blend with alternating pattern
    __mmask32 mask32_hf = 0x55555555;  // Alternating 0/1 pattern
    __m512h result1 = _mm512_mask_blend_ph(mask32_hf, a, b);
    
    // V32BFmode blend with dynamic mask
    __mmask32 mask32_bf = _mm512_cmp_ph_mask(result1, c, _CMP_EQ_OQ);
    mask32_bf = _knot_mask32(mask32_bf);
    __m512h result2 = _mm512_mask_blend_ph(mask32_bf, result1, c);
    
    return result2;
}
#endif

// Function to force mode transitions through chained blends
static inline __m512 chained_blends(__m512i int_vec, __m512 float_vec, 
                                   __m512d double_vec, int selector) {
    // Start with integer blend
    __mmask64 mask64 = selector ? 0xFFFFFFFFFFFFFFFFULL : 0xAAAAAAAAAAAAAAAAULL;
    __m512i int_result = _mm512_mask_blend_epi8(mask64, int_vec, _mm512_set1_epi8(selector));
    
    // Convert to float and blend
    __m512 float_from_int = _mm512_cvtepi32_ps(_mm512_cvtepi8_epi32(_mm512_castsi512_si128(int_result)));
    __mmask16 float_mask = _mm512_cmp_ps_mask(float_from_int, float_vec, _CMP_NEQ_UQ);
    __m512 float_result = _mm512_mask_blend_ps(float_mask, float_from_int, float_vec);
    
    // Convert to double and blend
    __m512d double_from_float = _mm512_cvtps_pd(_mm512_castps512_ps256(float_result));
    __mmask8 double_mask = _mm512_cmp_pd_mask(double_from_float, double_vec, _CMP_LT_OQ);
    __m512d double_result = _mm512_mask_blend_pd(double_mask, double_from_float, double_vec);
    
    return _mm512_castpd_ps(double_result);
}

int main() {
    // Initialize vectors with distinct patterns
    __m512i vec_i8 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vec_i16 = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i vec_i32 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i vec_i64 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    
    __m512 vec_float = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512d vec_double = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    
    // Alternate versions for blending
    __m512i vec_i8_alt = _mm512_set1_epi8(0x55);
    __m512i vec_i16_alt = _mm512_set1_epi16(0x3333);
    __m512i vec_i32_alt = _mm512_set1_epi32(0x77777777);
    __m512i vec_i64_alt = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAALL);
    __m512 vec_float_alt = _mm512_set1_ps(3.14159f);
    __m512d vec_double_alt = _mm512_set1_pd(2.71828);
    
    float checksum = 0.0f;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        if (i % 3 == 0) {
            // V64QImode and V32HImode blends
            __m512i result1 = blend_v64qi_v32hi(vec_i8, vec_i8_alt, i);
            
            // Horizontal sum to prevent elimination
            __m256i sum256 = _mm256_add_epi8(_mm512_castsi512_si256(result1),
                                           _mm512_extracti64x4_epi64(result1, 1));
            __m128i sum128 = _mm_add_epi8(_mm256_castsi256_si128(sum256),
                                        _mm256_extracti128_si256(sum256, 1));
            checksum += (float)_mm_extract_epi8(sum128, 0);
        }
        else if (i % 3 == 1) {
            // V16SImode and V8DImode blends
            __mmask16 mask16 = _mm512_cmpgt_epi32_mask(vec_i32, vec_i32_alt);
            __m512i result2 = blend_v16si_v8di(vec_i32, vec_i32_alt, mask16);
            
            // Reduce and accumulate
            checksum += (float)_mm512_reduce_add_epi32(result2);
        }
        else {
            // V16SFmode and V8DFmode blends
            __m512 result3 = blend_v16sf_v8df(vec_float, vec_float_alt,
                                            _mm512_castpd_ps(vec_double),
                                            _mm512_castpd_ps(vec_double_alt));
            
            // Reduce and accumulate
            checksum += _mm512_reduce_add_ps(result3);
        }
        
        // Chained blends across different modes
        __m512 chained_result = chained_blends(vec_i8, vec_float, vec_double, i);
        checksum += _mm512_reduce_add_ps(chained_result);
    }
    
#ifdef __AVX512FP16__
    // Initialize FP16 vectors
    __m512h vec_fp16 = _mm512_setzero_ph();
    __m512h vec_fp16_alt = _mm512_set1_ph((__fp16)1.5f);
    __m512h vec_fp16_alt2 = _mm512_set1_ph((__fp16)2.5f);
    
    // V32HFmode and V32BFmode blends
    __m512h fp16_result = blend_v32hf_v32bf(vec_fp16, vec_fp16_alt, vec_fp16_alt2);
    
    // Accumulate FP16 results
    for (int i = 0; i < 32; i++) {
        checksum += (float)fp16_result[i];
    }
#endif
    
    // Additional direct intrinsic calls for each mode
    // V64QImode with immediate mask
    __m512i blend_epi8 = _mm512_mask_blend_epi8(0xCCCCCCCCCCCCCCCCULL, vec_i8, vec_i8_alt);
    
    // V32HImode with comparison mask
    __mmask32 blend_mask32 = _mm512_cmpeq_epi16_mask(vec_i16, vec_i16_alt);
    __m512i blend_epi16 = _mm512_mask_blend_epi16(blend_mask32, vec_i16, vec_i16_alt);
    
    // V16SImode with bitwise mask operations
    __mmask16 blend_mask16 = _mm512_cmp_epi32_mask(vec_i32, vec_i32_alt, _MM_CMPINT_LT);
    blend_mask16 = _kor_mask16(blend_mask16, 0xAAAA);
    __m512i blend_epi32 = _mm512_mask_blend_epi32(blend_mask16, vec_i32, vec_i32_alt);
    
    // V8DImode
    __mmask8 blend_mask8 = _mm512_cmp_epi64_mask(vec_i64, vec_i64_alt, _MM_CMPINT_EQ);
    __m512i blend_epi64 = _mm512_mask_blend_epi64(blend_mask8, vec_i64, vec_i64_alt);
    
    // V16SFmode
    __mmask16 float_mask16 = _mm512_cmp_ps_mask(vec_float, vec_float_alt, _CMP_GT_OQ);
    __m512 blend_ps = _mm512_mask_blend_ps(float_mask16, vec_float, vec_float_alt);
    
    // V8DFmode
    __mmask8 double_mask8 = _mm512_cmp_pd_mask(vec_double, vec_double_alt, _CMP_LE_OQ);
    __m512d blend_pd = _mm512_mask_blend_pd(double_mask8, vec_double, vec_double_alt);
    
    // Final accumulation from direct blends
    checksum += (float)_mm512_reduce_add_epi64(blend_epi8);
    checksum += (float)_mm512_reduce_add_epi32(blend_epi16);
    checksum += (float)_mm512_reduce_add_epi32(blend_epi32);
    checksum += (float)_mm512_reduce_add_epi64(blend_epi64);
    checksum += _mm512_reduce_add_ps(blend_ps);
    checksum += (float)_mm512_reduce_add_pd(blend_pd);
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
