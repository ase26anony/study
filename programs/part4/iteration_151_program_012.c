#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper function for V64QImode blends
static inline __m512i blend_v64qi(__m512i a, __m512i b, int mask_type) {
    __mmask64 mask;
    if (mask_type == 0) {
        // Immediate constant mask
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (mask_type == 1) {
        // Dynamic mask from comparison
        __m512i cmp_a = _mm512_set1_epi8(32);
        __m512i cmp_b = _mm512_set1_epi8(64);
        mask = _mm512_cmpgt_epi8_mask(cmp_b, cmp_a);
    } else {
        // Complex mask from bitwise operations
        __mmask64 m1 = 0x5555555555555555ULL;
        __mmask64 m2 = 0xAAAAAAAAAAAAAAAAULL;
        mask = _kor_mask64(m1, m2);
    }
    return _mm512_mask_blend_epi8(mask, a, b);
}

// Helper function for V32HImode blends
static inline __m512i blend_v32hi(__m512i a, __m512i b, int mask_type) {
    __mmask32 mask;
    if (mask_type == 0) {
        mask = 0xAAAAAAAA;
    } else if (mask_type == 1) {
        __m512i cmp_a = _mm512_set1_epi16(100);
        __m512i cmp_b = _mm512_set1_epi16(200);
        mask = _mm512_cmpgt_epi16_mask(cmp_b, cmp_a);
    } else {
        __mmask32 m1 = 0x55555555;
        __mmask32 m2 = 0xAAAAAAAA;
        mask = _knot_mask32(_kor_mask32(m1, m2));
    }
    return _mm512_mask_blend_epi16(mask, a, b);
}

// Helper function for V16SImode blends
static inline __m512i blend_v16si(__m512i a, __m512i b, int mask_type) {
    __mmask16 mask;
    if (mask_type == 0) {
        mask = 0xAAAA;
    } else if (mask_type == 1) {
        __m512i cmp_a = _mm512_set1_epi32(500);
        __m512i cmp_b = _mm512_set1_epi32(1000);
        mask = _mm512_cmpgt_epi32_mask(cmp_b, cmp_a);
    } else {
        __mmask16 m1 = 0x5555;
        __mmask16 m2 = 0xAAAA;
        mask = _kor_mask16(m1, m2);
    }
    return _mm512_mask_blend_epi32(mask, a, b);
}

// Helper function for V8DImode blends
static inline __m512i blend_v8di(__m512i a, __m512i b, int mask_type) {
    __mmask8 mask;
    if (mask_type == 0) {
        mask = 0xAA;
    } else if (mask_type == 1) {
        __m512i cmp_a = _mm512_set1_epi64(1000);
        __m512i cmp_b = _mm512_set1_epi64(2000);
        mask = _mm512_cmpgt_epi64_mask(cmp_b, cmp_a);
    } else {
        __mmask8 m1 = 0x55;
        __mmask8 m2 = 0xAA;
        mask = _knot_mask8(_kor_mask8(m1, m2));
    }
    return _mm512_mask_blend_epi64(mask, a, b);
}

// Helper function for V16SFmode blends
static inline __m512 blend_v16sf(__m512 a, __m512 b, int mask_type) {
    __mmask16 mask;
    if (mask_type == 0) {
        mask = 0xAAAA;
    } else if (mask_type == 1) {
        __m512 cmp_a = _mm512_set1_ps(0.5f);
        __m512 cmp_b = _mm512_set1_ps(1.0f);
        mask = _mm512_cmp_ps_mask(cmp_b, cmp_a, _CMP_GT_OQ);
    } else {
        __mmask16 m1 = 0x5555;
        __mmask16 m2 = 0xAAAA;
        mask = _kor_mask16(m1, m2);
    }
    return _mm512_mask_blend_ps(mask, a, b);
}

// Helper function for V8DFmode blends
static inline __m512d blend_v8df(__m512d a, __m512d b, int mask_type) {
    __mmask8 mask;
    if (mask_type == 0) {
        mask = 0xAA;
    } else if (mask_type == 1) {
        __m512d cmp_a = _mm512_set1_pd(0.5);
        __m512d cmp_b = _mm512_set1_pd(1.0);
        mask = _mm512_cmp_pd_mask(cmp_b, cmp_a, _CMP_GT_OQ);
    } else {
        __mmask8 m1 = 0x55;
        __mmask8 m2 = 0xAA;
        mask = _knot_mask8(_kor_mask8(m1, m2));
    }
    return _mm512_mask_blend_pd(mask, a, b);
}

#ifdef __AVX512FP16__
// Helper function for V32HFmode blends
static inline __m512h blend_v32hf(__m512h a, __m512h b, int mask_type) {
    __mmask32 mask;
    if (mask_type == 0) {
        mask = 0xAAAAAAAA;
    } else if (mask_type == 1) {
        __m512h cmp_a = _mm512_set1_ph(0.5f);
        __m512h cmp_b = _mm512_set1_ph(1.0f);
        mask = _mm512_cmp_ph_mask(cmp_b, cmp_a, _CMP_GT_OQ);
    } else {
        __mmask32 m1 = 0x55555555;
        __mmask32 m2 = 0xAAAAAAAA;
        mask = _kor_mask32(m1, m2);
    }
    return _mm512_mask_blend_ph(mask, a, b);
}

// Helper function for V32BFmode blends (using same intrinsic as HF)
static inline __m512bh blend_v32bf(__m512bh a, __m512bh b, int mask_type) {
    __mmask32 mask;
    if (mask_type == 0) {
        mask = 0xAAAAAAAA;
    } else if (mask_type == 1) {
        __m512h cmp_a = _mm512_set1_ph(0.5f);
        __m512h cmp_b = _mm512_set1_ph(1.0f);
        mask = _mm512_cmp_ph_mask(cmp_b, cmp_a, _CMP_GT_OQ);
    } else {
        __mmask32 m1 = 0x55555555;
        __mmask32 m2 = 0xAAAAAAAA;
        mask = _knot_mask32(_kor_mask32(m1, m2));
    }
    return _mm512_mask_blend_ph(mask, a, b);
}
#endif

// Function that chains blends across different modes
static inline double chain_blends(int iterations) {
    double checksum = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        // Data-dependent control flow
        if (i % 3 == 0) {
            // Chain: epi8 -> epi16 -> epi32 -> epi64
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
            
            __m512i result1 = blend_v64qi(v64qi_a, v64qi_b, i % 3);
            
            // Convert to V32HImode
            __m512i v32hi_a = _mm512_set_epi16(
                31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
                15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
            );
            __m512i v32hi_b = _mm512_set_epi16(
                0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
            );
            
            __m512i result2 = blend_v32hi(v32hi_a, v32hi_b, (i + 1) % 3);
            
            // Continue chain with V16SImode
            __m512i v16si_a = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
            __m512i v16si_b = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
            
            __m512i result3 = blend_v16si(v16si_a, v16si_b, (i + 2) % 3);
            
            // Final chain with V8DImode
            __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
            __m512i v8di_b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
            
            __m512i result4 = blend_v8di(v8di_a, v8di_b, i % 3);
            
            // Reduce and accumulate
            checksum += _mm512_reduce_add_epi64(result1);
            checksum += _mm512_reduce_add_epi64(result2);
            checksum += _mm512_reduce_add_epi64(result3);
            checksum += _mm512_reduce_add_epi64(result4);
            
        } else if (i % 3 == 1) {
            // Chain: float -> double
            __m512 v16sf_a = _mm512_set_ps(
                15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
            );
            __m512 v16sf_b = _mm512_set_ps(
                0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
            );
            
            __m512 result1 = blend_v16sf(v16sf_a, v16sf_b, i % 3);
            
            __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
            __m512d v8df_b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
            
            __m512d result2 = blend_v8df(v8df_a, v8df_b, (i + 1) % 3);
            
            // Reduce and accumulate
            checksum += _mm512_reduce_add_ps(result1);
            checksum += _mm512_reduce_add_pd(result2);
            
        } else {
            // FP16/BF16 blends when available
            #ifdef __AVX512FP16__
            __m512h v32hf_a = _mm512_set_ph(
                31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
                23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
                15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
            );
            __m512h v32hf_b = _mm512_set_ph(
                0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
                16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
                24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
            );
            
            __m512h result1 = blend_v32hf(v32hf_a, v32hf_b, i % 3);
            
            // For BF16, we need to use the same intrinsic
            __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
            __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
            
            __m512bh result2 = blend_v32bf(v32bf_a, v32bf_b, (i + 1) % 3);
            
            // Manual reduction for FP16
            __m256h low = _mm512_castph512_ph256(result1);
            __m256h high = _mm512_extractf32x8_ps(_mm512_castph_ps(result1), 1);
            float sum = 0.0f;
            for (int j = 0; j < 16; j++) {
                sum += ((float*)&low)[j];
            }
            for (int j = 0; j < 16; j++) {
                sum += ((float*)&high)[j];
            }
            checksum += sum;
            #endif
        }
    }
    
    return checksum;
}

int main() {
    double final_checksum = 0.0;
    
    // Test all blend modes independently
    __m512i v64qi_a = _mm512_set1_epi8(1);
    __m512i v64qi_b = _mm512_set1_epi8(2);
    __m512i v64qi_result = blend_v64qi(v64qi_a, v64qi_b, 0);
    final_checksum += _mm512_reduce_add_epi64(v64qi_result);
    
    __m512i v32hi_a = _mm512_set1_epi16(10);
    __m512i v32hi_b = _mm512_set1_epi16(20);
    __m512i v32hi_result = blend_v32hi(v32hi_a, v32hi_b, 1);
    final_checksum += _mm512_reduce_add_epi64(v32hi_result);
    
    __m512i v16si_a = _mm512_set1_epi32(100);
    __m512i v16si_b = _mm512_set1_epi32(200);
    __m512i v16si_result = blend_v16si(v16si_a, v16si_b, 2);
    final_checksum += _mm512_reduce_add_epi64(v16si_result);
    
    __m512i v8di_a = _mm512_set1_epi64(1000);
    __m512i v8di_b = _mm512_set1_epi64(2000);
    __m512i v8di_result = blend_v8di(v8di_a, v8di_b, 0);
    final_checksum += _mm512_reduce_add_epi64(v8di_result);
    
    __m512 v16sf_a = _mm512_set1_ps(1.5f);
    __m512 v16sf_b = _mm512_set1_ps(2.5f);
    __m512 v16sf_result = blend_v16sf(v16sf_a, v16sf_b, 1);
    final_checksum += _mm512_reduce_add_ps(v16sf_result);
    
    __m512d v8df_a = _mm512_set1_pd(1.5);
    __m512d v8df_b = _mm512_set1_pd(2.5);
    __m512d v8df_result = blend_v8df(v8df_a, v8df_b, 2);
    final_checksum += _mm512_reduce_add_pd(v8df_result);
    
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.5f);
    __m512h v32hf_b = _mm512_set1_ph(2.5f);
    __m512h v32hf_result = blend_v32hf(v32hf_a, v32hf_b, 0);
    
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
    __m512bh v32bf_result = blend_v32bf(v32bf_a, v32bf_b, 1);
    
    // Manual reduction for FP16
    __m256h low = _mm512_castph512_ph256(v32hf_result);
    __m256h high = _mm512_extractf32x8_ps(_mm512_castph_ps(v32hf_result), 1);
    float fp16_sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        fp16_sum += ((float*)&low)[i];
    }
    for (int i = 0; i < 16; i++) {
        fp16_sum += ((float*)&high)[i];
    }
    final_checksum += fp16_sum;
    #endif
    
    // Chain blends with data-dependent control flow
    final_checksum += chain_blends(10);
    
    printf("Final checksum: %f\n", final_checksum);
    return 0;
}
