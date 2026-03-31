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
    __mmask32 mask_hi = 0x55555555;
    if (mode_selector & 2) {
        mask_hi = _knot_mask32(mask_hi);
    }
    
    return _mm512_mask_blend_epi16(mask_hi, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    __mmask16 mask_ps;
    __mmask8 mask_pd;
    
    // Generate mask using comparison for V16SFmode
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    mask_ps = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    
    if (selector > 0) {
        mask_ps = _kor_mask16(mask_ps, 0xAAAA);
    }
    
    __m512 result_ps = _mm512_mask_blend_ps(mask_ps, a, b);
    
    // Generate mask for V8DFmode
    __m512d cmp_val_d = _mm512_set1_pd(0.0);
    mask_pd = _mm512_cmp_pd_mask(c, cmp_val_d, _CMP_GE_OQ);
    
    return _mm512_castpd_ps(_mm512_mask_blend_pd(mask_pd, 
        _mm512_castps_pd(result_ps), d));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    __mmask32 mask_hf;
    
    // Different mask patterns based on input
    switch (pattern & 3) {
        case 0:
            mask_hf = 0xFFFFFFFF;  // All ones
            break;
        case 1:
            mask_hf = 0xAAAAAAAA;  // Alternating
            break;
        case 2: {
            // Dynamic mask from comparison
            __m512h threshold = _mm512_set1_ph(0.0f);
            mask_hf = _mm512_cmp_ph_mask(a, threshold, _CMP_GT_OQ);
            break;
        }
        default:
            mask_hf = 0x55555555;  // Alternating opposite
    }
    
    return _mm512_mask_blend_ph(mask_hf, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int iter) {
    __mmask16 mask_si;
    __mmask8 mask_di;
    
    // V16SImode mask from comparison
    __m512i threshold = _mm512_set1_epi32(100);
    mask_si = _mm512_cmpgt_epi32_mask(a, threshold);
    
    // Modify mask based on iteration
    if (iter & 1) {
        mask_si = _knot_mask16(mask_si);
    }
    
    __m512i result_si = _mm512_mask_blend_epi32(mask_si, a, b);
    
    // V8DImode mask - immediate constant
    mask_di = (iter & 2) ? 0xFF : 0xAA;
    
    return _mm512_mask_blend_epi64(mask_di, result_si, c);
}

int main() {
    // Initialize vectors with distinct patterns
    __m512i vec_i8_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vec_i8_b = _mm512_set1_epi8(100);
    __m512i vec_i16_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    __m512i vec_i16_b = _mm512_set1_epi16(200);
    
    __m512 vec_ps_a = _mm512_set_ps(
        0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,
        0.8f,0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f
    );
    __m512 vec_ps_b = _mm512_set1_ps(2.0f);
    
    __m512d vec_pd_a = _mm512_set_pd(0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7);
    __m512d vec_pd_b = _mm512_set1_pd(1.0);
    
    __m512i vec_i32_a = _mm512_set_epi32(
        0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150
    );
    __m512i vec_i32_b = _mm512_set1_epi32(255);
    
    __m512i vec_i64_a = _mm512_set_epi64(0,100,200,300,400,500,600,700);
    __m512i vec_i64_b = _mm512_set1_epi64(1000);
    
    #ifdef __AVX512FP16__
    __m512h vec_hf_a = _mm512_set_ph(
        0.0f,0.5f,1.0f,1.5f,2.0f,2.5f,3.0f,3.5f,
        4.0f,4.5f,5.0f,5.5f,6.0f,6.5f,7.0f,7.5f,
        8.0f,8.5f,9.0f,9.5f,10.0f,10.5f,11.0f,11.5f,
        12.0f,12.5f,13.0f,13.5f,14.0f,14.5f,15.0f,15.5f
    );
    __m512h vec_hf_b = _mm512_set1_ph(10.0f);
    #endif
    
    float checksum = 0.0f;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        __m512i result_int;
        __m512 result_float;
        __m512d result_double;
        
        // Control flow selects different blend modes
        if (i % 3 == 0) {
            // Chain blends through different modes
            result_int = blend_v64qi_v32hi(vec_i8_a, vec_i8_b, i);
            
            // Horizontal reduction to prevent elimination
            __m512i sum_i8 = _mm512_sad_epu8(result_int, _mm512_setzero_si512());
            checksum += (float)_mm512_reduce_add_epi64(sum_i8);
        } 
        else if (i % 3 == 1) {
            // Blend float/double vectors
            result_float = blend_v16sf_v8df(vec_ps_a, vec_ps_b, vec_pd_a, vec_pd_b, i);
            
            // Reduce float vector
            checksum += _mm512_reduce_add_ps(result_float);
        } 
        else {
            // Blend 32-bit and 64-bit integers
            result_int = blend_v16si_v8di(vec_i32_a, vec_i32_b, vec_i64_a, vec_i64_b, i);
            
            // Reduce integer vector
            __m512i sum_i32 = _mm512_add_epi32(result_int, _mm512_setzero_si512());
            checksum += (float)_mm512_reduce_add_epi32(sum_i32);
        }
        
        #ifdef __AVX512FP16__
        if (i % 2 == 0) {
            // Half-precision blend
            __m512h result_hf = blend_v32hf_v32bf(vec_hf_a, vec_hf_b, i);
            
            // Convert to float for reduction (simplified)
            __m512 result_hf_f32 = _mm512_cvtph_ps(_mm512_cvtph_ph(result_hf));
            checksum += _mm512_reduce_add_ps(result_hf_f32);
        }
        #endif
    }
    
    // Additional direct intrinsic calls for each mode
    __mmask64 m64 = 0xCCCCCCCCCCCCCCCCULL;
    __m512i blend_epi8 = _mm512_mask_blend_epi8(m64, vec_i8_a, vec_i8_b);
    
    __mmask32 m32 = 0x88888888;
    __m512i blend_epi16 = _mm512_mask_blend_epi16(m32, vec_i16_a, vec_i16_b);
    
    __mmask16 m16 = 0xF0F0;
    __m512i blend_epi32 = _mm512_mask_blend_epi32(m16, vec_i32_a, vec_i32_b);
    
    __mmask8 m8 = 0xCC;
    __m512i blend_epi64 = _mm512_mask_blend_epi64(m8, vec_i64_a, vec_i64_b);
    
    __m512 blend_ps = _mm512_mask_blend_ps(0xAAAA, vec_ps_a, vec_ps_b);
    __m512d blend_pd = _mm512_mask_blend_pd(0xAA, vec_pd_a, vec_pd_b);
    
    // Reduce all results
    checksum += (float)_mm512_reduce_add_epi64(blend_epi8);
    checksum += (float)_mm512_reduce_add_epi32(blend_epi16);
    checksum += (float)_mm512_reduce_add_epi32(blend_epi32);
    checksum += (float)_mm512_reduce_add_epi64(blend_epi64);
    checksum += _mm512_reduce_add_ps(blend_ps);
    
    __m512d sum_pd = _mm512_add_pd(blend_pd, _mm512_setzero_pd());
    checksum += (float)_mm512_reduce_add_pd(sum_pd);
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
