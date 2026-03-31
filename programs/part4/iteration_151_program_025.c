#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend modes
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    __mmask64 mask64;
    __mmask32 mask32;
    
    if (mode_selector & 1) {
        // Immediate constant mask for V64QImode
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Dynamic mask generation for V64QImode
        __m512i cmp_a = _mm512_set1_epi8(32);
        __m512i cmp_b = _mm512_set1_epi8(64);
        __mmask64 cmp_mask = _mm512_cmp_epi8_mask(cmp_a, cmp_b, _MM_CMPINT_LT);
        mask64 = _knot_mask64(cmp_mask);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Generate mask for V32HImode using bitwise operations
    __mmask32 mask_lo = 0x55555555;
    __mmask32 mask_hi = 0xAAAAAAAA;
    mask32 = _kor_mask32(mask_lo, mask_hi);
    
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // Dynamic mask generation using comparison intrinsics
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    mask16 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    
    // Chain blend operations
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // Generate mask for V8DFmode based on iteration count
    if (iter % 2) {
        mask8 = 0xAA;  // Immediate constant
    } else {
        __m512d cmp_val_d = _mm512_set1_pd(1.0);
        mask8 = _mm512_cmp_pd_mask(c, cmp_val_d, _CMP_LT_OQ);
    }
    
    __m512d result2 = _mm512_mask_blend_pd(mask8, c, d);
    
    // Convert double result back to float for return (simplified)
    return result1;
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // Immediate constant masks
    mask16 = 0xAAAA;
    mask8 = 0xAA;
    
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    __m512i result2 = _mm512_mask_blend_epi64(mask8, c, d);
    
    // Combine results
    return _mm512_add_epi32(result1, _mm512_castsi512_si512(result2));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d) {
    __mmask32 mask32;
    
    // Generate mask using comparison
    __m512h threshold = _mm512_set1_ph(0.0f);
    mask32 = _mm512_cmp_ph_mask(a, threshold, _CMP_GT_OQ);
    
    __m512h result1 = _mm512_mask_blend_ph(mask32, a, b);
    
    // Create mask via bitwise operation
    __mmask32 mask_inv = _knot_mask32(mask32);
    __m512h result2 = _mm512_mask_blend_ph(mask_inv, c, d);
    
    return _mm512_add_ph(result1, result2);
}
#endif

// Horizontal reduction functions
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256_lo = _mm512_extracti64x4_epi64(v, 0);
    __m256i v256_hi = _mm512_extracti64x4_epi64(v, 1);
    
    __m256i sum256 = _mm256_add_epi64(v256_lo, v256_hi);
    __m128i sum128_lo = _mm256_extracti128_si256(sum256, 0);
    __m128i sum128_hi = _mm256_extracti128_si256(sum256, 1);
    
    __m128i sum128 = _mm_add_epi64(sum128_lo, sum128_hi);
    
    return (int64_t)_mm_extract_epi64(sum128, 0) + 
           (int64_t)_mm_extract_epi64(sum128, 1);
}

static inline float reduce_add_ps(__m512 v) {
    __m256 v256_lo = _mm512_extractf32x8_ps(v, 0);
    __m256 v256_hi = _mm512_extractf32x8_ps(v, 1);
    
    __m256 sum256 = _mm256_add_ps(v256_lo, v256_hi);
    __m128 sum128_lo = _mm256_extractf128_ps(sum256, 0);
    __m128 sum128_hi = _mm256_extractf128_ps(sum256, 1);
    
    __m128 sum128 = _mm_add_ps(sum128_lo, sum128_hi);
    
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    
    return _mm_cvtss_f32(sum128);
}

static inline double reduce_add_pd(__m512d v) {
    __m256d v256_lo = _mm512_extractf64x4_pd(v, 0);
    __m256d v256_hi = _mm512_extractf64x4_pd(v, 1);
    
    __m256d sum256 = _mm256_add_pd(v256_lo, v256_hi);
    __m128d sum128_lo = _mm256_extractf128_pd(sum256, 0);
    __m128d sum128_hi = _mm256_extractf128_pd(sum256, 1);
    
    __m128d sum128 = _mm_add_pd(sum128_lo, sum128_hi);
    
    sum128 = _mm_hadd_pd(sum128, sum128);
    
    return _mm_cvtsd_f64(sum128);
}

#ifdef __AVX512FP16__
static inline _Float16 reduce_add_ph(__m512h v) {
    // Simple reduction for half precision
    _Float16 sum = 0.0f;
    union {
        __m512h vec;
        _Float16 arr[32];
    } u;
    u.vec = v;
    
    for (int i = 0; i < 32; i++) {
        sum += u.arr[i];
    }
    return sum;
}
#endif

int main() {
    // Initialize vectors with patterned data
    __m512i vi64_1, vi64_2, vi64_3, vi64_4;
    __m512 vf32_1, vf32_2;
    __m512d vf64_1, vf64_2;
    
    // Initialize integer vectors with sequential numbers
    {
        int8_t arr64_1[64], arr64_2[64];
        int16_t arr32_1[32], arr32_2[32];
        int32_t arr16_1[16], arr16_2[16];
        int64_t arr8_1[8], arr8_2[8];
        
        for (int i = 0; i < 64; i++) {
            arr64_1[i] = i;
            arr64_2[i] = 64 - i;
        }
        for (int i = 0; i < 32; i++) {
            arr32_1[i] = i * 2;
            arr32_2[i] = 64 - i * 2;
        }
        for (int i = 0; i < 16; i++) {
            arr16_1[i] = i * 4;
            arr16_2[i] = 64 - i * 4;
        }
        for (int i = 0; i < 8; i++) {
            arr8_1[i] = i * 8;
            arr8_2[i] = 64 - i * 8;
        }
        
        vi64_1 = _mm512_loadu_si512((const __m512i*)arr64_1);
        vi64_2 = _mm512_loadu_si512((const __m512i*)arr64_2);
        vi64_3 = _mm512_loadu_si512((const __m512i*)arr16_1);
        vi64_4 = _mm512_loadu_si512((const __m512i*)arr16_2);
    }
    
    // Initialize float vectors
    {
        float arr32_1[16], arr32_2[16];
        double arr64_1[8], arr64_2[8];
        
        for (int i = 0; i < 16; i++) {
            arr32_1[i] = i * 0.5f;
            arr32_2[i] = 8.0f - i * 0.5f;
        }
        for (int i = 0; i < 8; i++) {
            arr64_1[i] = i * 1.0;
            arr64_2[i] = 8.0 - i * 1.0;
        }
        
        vf32_1 = _mm512_loadu_ps(arr32_1);
        vf32_2 = _mm512_loadu_ps(arr32_2);
        vf64_1 = _mm512_loadu_pd(arr64_1);
        vf64_2 = _mm512_loadu_pd(arr64_2);
    }
    
    double checksum = 0.0;
    
    // Data-dependent control flow with loops
    for (int iter = 0; iter < 10; iter++) {
        // V64QImode and V32HImode blends
        __m512i blended_int = blend_v64qi_v32hi(vi64_1, vi64_2, iter);
        checksum += reduce_add_epi64(blended_int);
        
        // V16SImode and V8DImode blends
        if (iter % 3 == 0) {
            __m512i blended_si_di = blend_v16si_v8di(vi64_3, vi64_4, 
                                                    _mm512_slli_epi64(vi64_3, 1),
                                                    _mm512_srli_epi64(vi64_4, 1));
            checksum += reduce_add_epi64(blended_si_di);
        }
        
        // V16SFmode and V8DFmode blends
        __m512 blended_float = blend_v16sf_v8df(vf32_1, vf32_2, vf64_1, vf64_2, iter);
        checksum += reduce_add_ps(blended_float);
        checksum += reduce_add_pd(vf64_1);  // Use original for DF mode
        
        // Direct intrinsic calls for all modes
        {
            // V64QImode with immediate mask
            __m512i blend64qi = _mm512_mask_blend_epi8(0xCCCCCCCCCCCCCCCCULL, vi64_1, vi64_2);
            checksum += reduce_add_epi64(blend64qi);
            
            // V32HImode with dynamic mask
            __mmask32 mask32hi = _mm512_cmp_epi16_mask(vi64_1, vi64_2, _MM_CMPINT_GT);
            __m512i blend32hi = _mm512_mask_blend_epi16(mask32hi, vi64_1, vi64_2);
            checksum += reduce_add_epi64(blend32hi);
            
            // V16SImode
            __m512i blend16si = _mm512_mask_blend_epi32(0xF0F0, vi64_3, vi64_4);
            checksum += reduce_add_epi64(blend16si);
            
            // V8DImode
            __m512i blend8di = _mm512_mask_blend_epi64(0xAA, 
                                                      _mm512_slli_epi64(vi64_3, 2),
                                                      _mm512_srli_epi64(vi64_4, 2));
            checksum += reduce_add_epi64(blend8di);
            
            // V16SFmode
            __mmask16 mask16sf = _mm512_cmp_ps_mask(vf32_1, vf32_2, _CMP_LT_OQ);
            __m512 blend16sf = _mm512_mask_blend_ps(mask16sf, vf32_1, vf32_2);
            checksum += reduce_add_ps(blend16sf);
            
            // V8DFmode
            __mmask8 mask8df = _mm512_cmp_pd_mask(vf64_1, vf64_2, _CMP_GT_OQ);
            __m512d blend8df = _mm512_mask_blend_pd(mask8df, vf64_1, vf64_2);
            checksum += reduce_add_pd(blend8df);
        }
        
        // Chain operations across different modes
        if (iter > 5) {
            // Start with epi8, then epi16, then epi32, then ps
            __m512i chain1 = _mm512_mask_blend_epi8(0xAAAAAAAAAAAAAAAAULL, vi64_1, vi64_2);
            __m512i chain2 = _mm512_mask_blend_epi16(0x55555555, chain1, vi64_2);
            __m512i chain3 = _mm512_mask_blend_epi32(0xAAAA, chain2, vi64_3);
            
            // Convert to float for final blend
            __m512 chain3_f = _mm512_cvtepi32_ps(_mm512_castsi512_si256(chain3));
            __m512 chain_final = _mm512_mask_blend_ps(0xFF00, chain3_f, vf32_2);
            
            checksum += reduce_add_ps(chain_final);
        }
    }
    
#ifdef __AVX512FP16__
    // Initialize half-precision vectors
    __m512h vh32_1, vh32_2, vh32_3, vh32_4;
    {
        _Float16 arr32_1[32], arr32_2[32];
        for (int i = 0; i < 32; i++) {
            arr32_1[i] = i * 0.25f;
            arr32_2[i] = 8.0f - i * 0.25f;
        }
        vh32_1 = _mm512_loadu_ph(arr32_1);
        vh32_2 = _mm512_loadu_ph(arr32_2);
        vh32_3 = _mm512_set1_ph(2.0f);
        vh32_4 = _mm512_set1_ph(4.0f);
    }
    
    // V32HFmode and V32BFmode blends
    for (int i = 0; i < 5; i++) {
        __m512h blended_half = blend_v32hf_v32bf(vh32_1, vh32_2, vh32_3, vh32_4);
        checksum += reduce_add_ph(blended_half);
        
        // Direct V32HFmode blend with immediate mask
        __m512h blend32hf = _mm512_mask_blend_ph(0xAAAAAAAA, vh32_1, vh32_2);
        checksum += reduce_add_ph(blend32hf);
        
        // V32BFmode (same intrinsic as V32HFmode)
        __m512h blend32bf = _mm512_mask_blend_ph(0x55555555, vh32_3, vh32_4);
        checksum += reduce_add_ph(blend32bf);
    }
#endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
