#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Function declarations with noinline to ensure separate expansion
__attribute__((noinline)) __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline)) __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline)) __m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline)) __m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline)) __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline)) __m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to generate complex mask patterns
__mmask64 generate_complex_mask64(int iteration) {
    __mmask64 mask;
    
    // Control flow to generate different mask patterns
    if (iteration % 3 == 0) {
        // Pattern 1: Alternating bits
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (iteration % 3 == 1) {
        // Pattern 2: Every 3rd bit set
        mask = 0x9249249249249249ULL;
    } else {
        // Pattern 3: Checkerboard pattern
        mask = 0x5555555555555555ULL;
    }
    
    // Apply logical operations to create more complex patterns
    if (iteration > 5) {
        mask = _kor_mask64(mask, 0xF0F0F0F0F0F0F0F0ULL);
    }
    
    return mask;
}

int main() {
    uint64_t final_checksum = 0;
    
    // Initialize test data with patterns
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
    
    __m512i v16si_a = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i v16si_b = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i v8di_b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
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
    __m512h v32hf_a = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    
    __m512h v32hf_b = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,22.0f,23.0f,24.0f,25.0f,26.0f,
        27.0f,28.0f,29.0f,30.0f,31.0f,32.0f,33.0f,34.0f,
        35.0f,36.0f,37.0f,38.0f,39.0f,40.0f,41.0f,42.0f
    );
    
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
#endif
    
    // Loop with control flow to prevent constant folding
    for (int i = 0; i < 10; i++) {
        __mmask64 mask64;
        __mmask32 mask32;
        __mmask16 mask16;
        __mmask8 mask8;
        
        // Complex mask generation with multiple methods
        switch (i % 4) {
            case 0:
                // Immediate mask
                mask64 = generate_complex_mask64(i);
                mask32 = 0xAAAAAAAAU;
                mask16 = 0xAAAA;
                mask8 = 0xAA;
                break;
                
            case 1:
                // Comparison mask
                mask64 = _mm512_cmpeq_epi8_mask(v64qi_a, v64qi_b);
                mask32 = _mm512_cmpeq_epi16_mask(v32hi_a, v32hi_b);
                mask16 = _mm512_cmpeq_epi32_mask(v16si_a, v16si_b);
                mask8 = _mm512_cmpeq_epi64_mask(v8di_a, v8di_b);
                break;
                
            case 2:
                // Logical combination of masks
                mask64 = _kor_mask64(
                    _mm512_cmplt_epi8_mask(v64qi_a, v64qi_b),
                    0x5555555555555555ULL
                );
                mask32 = _kxor_mask32(
                    _mm512_cmplt_epi16_mask(v32hi_a, v32hi_b),
                    0xFFFFFFFFU
                );
                mask16 = _kand_mask16(
                    _mm512_cmplt_epi32_mask(v16si_a, v16si_b),
                    0xFFFF
                );
                mask8 = _kor_mask8(
                    _mm512_cmplt_epi64_mask(v8di_a, v8di_b),
                    0x55
                );
                break;
                
            case 3:
                // Float comparison masks
                mask64 = 0xCCCCCCCCCCCCCCCCULL;
                mask32 = 0xCCCCCCCCU;
                mask16 = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OQ);
                mask8 = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
                break;
        }
        
        // Data dependency chain: Use result from one blend to affect next
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Generate new mask based on previous result
        __mmask32 mask_from_prev = _mm512_cmpeq_epi16_mask(
            result64qi,
            _mm512_set1_epi8(0)
        ) & 0xFFFFFFFFU;
        
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask_from_prev);
        
        // More blends with different modes
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
        
#ifdef __AVX512FP16__
        // Half-precision blends (require AVX512-FP16)
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
        
        // Use half-precision results in subsequent operations
        __mmask16 mask_from_half = _mm512_cmp_ph_mask(
            result32hf,
            _mm512_set1_ph(0.0f),
            _CMP_GT_OQ
        );
        
        // Another blend using mask derived from half-precision
        result16sf = blend_v16sf(result16sf, v16sf_b, mask_from_half);
#endif
        
        // Prevent optimization: accumulate results into checksum
        // Horizontal reduction for each result type
        __m512i sum64qi = _mm512_sad_epu8(result64qi, _mm512_setzero_si512());
        __m512i sum32hi = _mm512_madd_epi16(result32hi, _mm512_set1_epi16(1));
        __m512i sum16si = _mm512_add_epi32(result16si, _mm512_setzero_si512());
        __m512i sum8di = _mm512_add_epi64(result8di, _mm512_setzero_si512());
        
        // Float reductions
        __m512 sum16sf = _mm512_add_ps(result16sf, _mm512_setzero_ps());
        __m512d sum8df = _mm512_add_pd(result8df, _mm512_setzero_pd());
        
        // Extract and accumulate to checksum
        final_checksum += _mm512_reduce_add_epi64(sum64qi);
        final_checksum += _mm512_reduce_add_epi32(sum32hi);
        final_checksum += _mm512_reduce_add_epi32(sum16si);
        final_checksum += _mm512_reduce_add_epi64(sum8di);
        
        // Convert float sums to integer for checksum
        final_checksum += (uint64_t)_mm512_reduce_add_ps(sum16sf);
        final_checksum += (uint64_t)_mm512_reduce_add_pd(sum8df);
        
#ifdef __AVX512FP16__
        // Half-precision reduction
        __m512h sum32hf = _mm512_add_ph(result32hf, _mm512_setzero_ph());
        float half_sum = _mm512_reduce_add_ph(sum32hf);
        final_checksum += (uint64_t)half_sum;
#endif
    }
    
    printf("Final checksum: %lu\n", final_checksum);
    return 0;
}
