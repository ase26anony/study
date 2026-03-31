#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __AVX512FP16__
#include <math.h>
#endif

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 mask64, __mmask32 mask32) {
    // Chain epi8 -> epi16 blend
    __m512i blend1 = _mm512_mask_blend_epi8(mask64, a, b);
    __m512i blend2 = _mm512_mask_blend_epi16(mask32, blend1, a);
    return blend2;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d,
                                      __mmask16 mask16, __mmask8 mask8) {
    // Chain ps -> pd blend
    __m512 blend1 = _mm512_mask_blend_ps(mask16, a, b);
    __m512d blend2 = _mm512_mask_blend_pd(mask8, c, d);
    // Convert pd result back to ps for mixing
    __m512 blend2_ps = _mm512_castpd_ps(blend2);
    return _mm512_add_ps(blend1, blend2_ps);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 mask32_1, __mmask32 mask32_2) {
    // Chain two half-precision blends
    __m512h blend1 = _mm512_mask_blend_ph(mask32_1, a, b);
    __m512h blend2 = _mm512_mask_blend_ph(mask32_2, blend1, a);
    return blend2;
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __mmask16 mask16, __mmask8 mask8) {
    // Chain epi32 -> epi64 blend
    __m512i blend1 = _mm512_mask_blend_epi32(mask16, a, b);
    __m512i blend2 = _mm512_mask_blend_epi64(mask8, blend1, a);
    return blend2;
}

/* Mask generation helpers */
static __mmask64 generate_epi8_mask(int pattern) {
    // Different patterns for different contexts
    switch(pattern % 3) {
        case 0: return 0xAAAAAAAAAAAAAAAA;  // Alternating pattern
        case 1: return 0x5555555555555555;  // Opposite alternating
        case 2: return 0xFFFFFFFFFFFFFFFF;  // All ones
        default: return 0x0;
    }
}

static __mmask32 generate_epi16_mask(int pattern) {
    __m512i a = _mm512_set1_epi16(pattern);
    __m512i b = _mm512_set1_epi16(pattern + 1);
    // Generate mask via comparison
    __mmask32 cmp_mask = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_LT);
    
    // Modify with bitwise operations
    if (pattern % 2) {
        cmp_mask = _knot_mask32(cmp_mask);
    }
    return cmp_mask;
}

static __mmask16 generate_epi32_mask(__m512i a, __m512i b) {
    __mmask16 cmp_mask = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_EQ);
    // Combine with another mask
    __mmask16 const_mask = 0xAAAA;
    return _kor_mask16(cmp_mask, const_mask);
}

static __mmask8 generate_epi64_mask(__m512i a, __m512i b) {
    return _mm512_cmp_epi64_mask(a, b, _MM_CMPINT_GT);
}

static __mmask16 generate_ps_mask(__m512 a, __m512 b) {
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    __mmask16 pattern_mask = 0x5555;
    return _kxor_mask16(cmp_mask, pattern_mask);
}

static __mmask8 generate_pd_mask(__m512d a, __m512d b) {
    return _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
}

#ifdef __AVX512FP16__
static __mmask32 generate_ph_mask(__m512h a, __m512h b) {
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
    __mmask32 pattern_mask = 0xAAAAAAAA;
    return _kor_mask32(cmp_mask, pattern_mask);
}
#endif

int main() {
    double checksum = 0.0;
    
    // Initialize vectors with distinct patterns
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(0xFF);
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(0x7FFF);
    
    __m512i v16si_a = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i v16si_b = _mm512_set1_epi32(0xFFFFFFFF);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(0x7FFFFFFFFFFFFFFF);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(1.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set1_pd(2.0);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512h v32hf_b = _mm512_set1_ph(0.5f);
#endif
    
    // Data-dependent control flow
    int iterations = 100;
    int mode_selector = 0;
    
    for (int i = 0; i < iterations; i++) {
        // Vary mask patterns based on loop iteration
        __mmask64 mask64 = generate_epi8_mask(i);
        __mmask32 mask32_epi16 = generate_epi16_mask(i);
        __mmask16 mask16_epi32 = generate_epi32_mask(v16si_a, v16si_b);
        __mmask8 mask8_epi64 = generate_epi64_mask(v8di_a, v8di_b);
        __mmask16 mask16_ps = generate_ps_mask(v16sf_a, v16sf_b);
        __mmask8 mask8_pd = generate_pd_mask(v8df_a, v8df_b);
        
#ifdef __AVX512FP16__
        __mmask32 mask32_hf = generate_ph_mask(v32hf_a, v32hf_b);
        __mmask32 mask32_hf2 = _knot_mask32(mask32_hf);
#endif
        
        // Data-dependent selection of blend operations
        if (i % 3 == 0) {
            // Direct intrinsic calls for all modes
            __m512i blend_epi8 = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
            __m512i blend_epi16 = _mm512_mask_blend_epi16(mask32_epi16, v32hi_a, v32hi_b);
            __m512i blend_epi32 = _mm512_mask_blend_epi32(mask16_epi32, v16si_a, v16si_b);
            __m512i blend_epi64 = _mm512_mask_blend_epi64(mask8_epi64, v8di_a, v8di_b);
            __m512 blend_ps = _mm512_mask_blend_ps(mask16_ps, v16sf_a, v16sf_b);
            __m512d blend_pd = _mm512_mask_blend_pd(mask8_pd, v8df_a, v8df_b);
            
#ifdef __AVX512FP16__
            __m512h blend_hf = _mm512_mask_blend_ph(mask32_hf, v32hf_a, v32hf_b);
#endif
            
            // Reductions to prevent dead code elimination
            __m256i sum256 = _mm512_castsi512_si256(blend_epi8);
            int64_t sum_epi8 = _mm512_reduce_add_epi64(blend_epi8);
            int64_t sum_epi16 = _mm512_reduce_add_epi64(blend_epi16);
            int64_t sum_epi32 = _mm512_reduce_add_epi64(blend_epi32);
            int64_t sum_epi64 = _mm512_reduce_add_epi64(blend_epi64);
            
            float sum_ps = _mm512_reduce_add_ps(blend_ps);
            double sum_pd = _mm512_reduce_add_pd(blend_pd);
            
            checksum += sum_epi8 + sum_epi16 + sum_epi32 + sum_epi64 + sum_ps + sum_pd;
            
#ifdef __AVX512FP16__
            // Manual reduction for half precision
            __m256h blend_hf_256 = _mm512_castph512_ph256(blend_hf);
            for (int j = 0; j < 16; j++) {
                checksum += blend_hf_256[j];
            }
#endif
        } else if (i % 3 == 1) {
            // Use helper functions with chained operations
            __m512i chain1 = blend_v64qi_v32hi(v64qi_a, v64qi_b, mask64, mask32_epi16);
            __m512i chain2 = blend_v16si_v8di(v16si_a, v16si_b, mask16_epi32, mask8_epi64);
            __m512 chain3 = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, 
                                            mask16_ps, mask8_pd);
            
#ifdef __AVX512FP16__
            __m512h chain4 = blend_v32hf_v32bf(v32hf_a, v32hf_b, mask32_hf, mask32_hf2);
#endif
            
            // Reductions
            int64_t sum_chain1 = _mm512_reduce_add_epi64(chain1);
            int64_t sum_chain2 = _mm512_reduce_add_epi64(chain2);
            float sum_chain3 = _mm512_reduce_add_ps(chain3);
            
            checksum += sum_chain1 + sum_chain2 + sum_chain3;
            
#ifdef __AVX512FP16__
            __m256h chain4_256 = _mm512_castph512_ph256(chain4);
            for (int j = 0; j < 16; j++) {
                checksum += chain4_256[j];
            }
#endif
        } else {
            // Mix of direct and chained operations
            __m512i direct_epi8 = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
            __m512i chained = blend_v16si_v8di(v16si_a, v16si_b, mask16_epi32, mask8_epi64);
            __m512 direct_ps = _mm512_mask_blend_ps(mask16_ps, v16sf_a, v16sf_b);
            
            // Use results in another blend
            __m512i final_blend = _mm512_mask_blend_epi32(
                generate_epi32_mask(direct_epi8, chained),
                direct_epi8,
                chained
            );
            
            int64_t sum_final = _mm512_reduce_add_epi64(final_blend);
            float sum_ps = _mm512_reduce_add_ps(direct_ps);
            
            checksum += sum_final + sum_ps;
        }
        
        // Modify vectors slightly each iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.1f));
        v8df_a = _mm512_add_pd(v8df_a, _mm512_set1_pd(0.01));
        
#ifdef __AVX512FP16__
        v32hf_a = _mm512_add_ph(v32hf_a, _mm512_set1_ph(0.05f));
#endif
        
        mode_selector = (mode_selector + 1) % 4;
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
