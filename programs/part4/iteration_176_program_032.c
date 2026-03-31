#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Function prototypes with noinline to ensure separate expansion
__attribute__((noinline)) __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k);
__attribute__((noinline)) __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k);
__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k);
__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k);
__attribute__((noinline)) __m512i blend_v16si(__m512i a, __m512i b, __mmask16 k);
__attribute__((noinline)) __m512i blend_v8di(__m512i a, __m512i b, __mmask8 k);
__attribute__((noinline)) __m512d blend_v8df(__m512d a, __m512d b, __mmask8 k);
__attribute__((noinline)) __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k);

// Helper functions for mask generation
__mmask64 generate_complex_mask64(int iteration);
__mmask32 generate_complex_mask32(int iteration);
__mmask16 generate_complex_mask16(int iteration);
__mmask8 generate_complex_mask8(int iteration);

int main() {
    uint64_t checksum = 0;
    
    // Initialize test vectors with patterned data
    // Integer vectors
    __m512i v64qi_a = _mm512_set1_epi8(0x55);
    __m512i v64qi_b = _mm512_set1_epi8(0xAA);
    
    __m512i v32hi_a = _mm512_set1_epi16(0x3333);
    __m512i v32hi_b = _mm512_set1_epi16(0xCCCC);
    
    __m512i v16si_a = _mm512_set1_epi32(0x0F0F0F0F);
    __m512i v16si_b = _mm512_set1_epi32(0xF0F0F0F0);
    
    __m512i v8di_a = _mm512_set1_epi64(0x00FF00FF00FF00FFULL);
    __m512i v8di_b = _mm512_set1_epi64(0xFF00FF00FF00FF00ULL);
    
    // Float vectors
    __m512 v16sf_a = _mm512_set1_ps(1.0f);
    __m512 v16sf_b = _mm512_set1_ps(2.0f);
    
    __m512d v8df_a = _mm512_set1_pd(3.0);
    __m512d v8df_b = _mm512_set1_pd(4.0);
    
    // Half-precision vectors (requires -mavx512fp16)
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.5f);
    __m512h v32hf_b = _mm512_set1_ph(2.5f);
    
    __m512bh v32bf_a = _mm512_castsi512_ph(_mm512_set1_epi16(0x3C00)); // 1.0 in BF16
    __m512bh v32bf_b = _mm512_castsi512_ph(_mm512_set1_epi16(0x4000)); // 2.0 in BF16
    #endif
    
    // Control flow: loop with mask dependency on iteration
    for (int i = 0; i < 4; i++) {
        // Generate masks using different methods based on iteration
        __mmask64 mask64 = generate_complex_mask64(i);
        __mmask32 mask32 = generate_complex_mask32(i);
        __mmask16 mask16 = generate_complex_mask16(i);
        __mmask8 mask8 = generate_complex_mask8(i);
        
        // Switch statement to select blend modes (preventing constant folding)
        switch (i % 3) {
            case 0:
                // Perform blends with comparison-generated masks
                {
                    __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
                    __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
                    
                    // Data dependency chain: use integer result to generate float mask
                    __mmask16 new_mask16 = _mm512_cmpeq_epi32_mask(
                        _mm512_and_si512(result64qi, result32hi),
                        _mm512_setzero_si512()
                    );
                    
                    __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, new_mask16);
                    
                    // Accumulate checksum
                    checksum += _mm512_reduce_add_epi64(result64qi);
                    checksum += _mm512_reduce_add_epi64(result32hi);
                    checksum += (uint64_t)_mm512_reduce_add_ps(result16sf);
                }
                break;
                
            case 1:
                // Perform blends with immediate masks
                {
                    __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
                    __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
                    
                    // Data dependency: use integer result for float blend
                    __mmask8 new_mask8 = _mm512_cmpeq_epi64_mask(
                        result16si,
                        result8di
                    );
                    
                    __m512d result8df = blend_v8df(v8df_a, v8df_b, new_mask8);
                    
                    checksum += _mm512_reduce_add_epi64(result16si);
                    checksum += _mm512_reduce_add_epi64(result8di);
                    checksum += (uint64_t)_mm512_reduce_add_pd(result8df);
                }
                break;
                
            case 2:
                // Half-precision blends (if available)
                #ifdef __AVX512FP16__
                {
                    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
                    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
                    
                    // Cross-mode dependency
                    __mmask64 new_mask64 = _kor_mask64(
                        _mm512_cmpeq_ph_mask(result32hf, v32hf_a),
                        _mm512_cmpeq_ph_mask(result32bf, v32bf_a)
                    );
                    
                    __m512i result64qi_chain = blend_v64qi(v64qi_a, v64qi_b, new_mask64);
                    
                    checksum += _mm512_reduce_add_epi64(_mm512_castph_si512(result32hf));
                    checksum += _mm512_reduce_add_epi64(_mm512_castph_si512(result32bf));
                    checksum += _mm512_reduce_add_epi64(result64qi_chain);
                }
                #endif
                break;
        }
        
        // If-else chain modifying masks for next iteration
        if (i == 1) {
            mask16 = _knot_mask16(mask16);
            __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
            checksum += (uint64_t)_mm512_reduce_add_ps(result16sf);
        } else if (i == 2) {
            mask8 = _kand_mask8(mask8, 0xAA);
            __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
            checksum += (uint64_t)_mm512_reduce_add_pd(result8df);
        }
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}

// Mask generation with diversity
__mmask64 generate_complex_mask64(int iteration) {
    // Mix of immediate and computed masks
    __mmask64 base_mask = 0xAAAAAAAAAAAAAAAAULL;
    
    if (iteration % 2 == 0) {
        // Comparison-based mask
        __m512i cmp_a = _mm512_set1_epi8(iteration);
        __m512i cmp_b = _mm512_set1_epi8(iteration * 2);
        return _mm512_cmpeq_epi8_mask(cmp_a, cmp_b);
    } else {
        // Logical operation on immediate mask
        return _kor_mask64(base_mask, 0x5555555555555555ULL >> (iteration % 8));
    }
}

__mmask32 generate_complex_mask32(int iteration) {
    __mmask32 base_mask = 0xAAAAAAAA;
    
    if (iteration % 3 == 0) {
        __m512i cmp_a = _mm512_set1_epi16(iteration);
        __m512i cmp_b = _mm512_set1_epi16(iteration * 3);
        return _mm512_cmpeq_epi16_mask(cmp_a, cmp_b);
    } else {
        return _kxor_mask32(base_mask, 0x55555555 << (iteration % 4));
    }
}

__mmask16 generate_complex_mask16(int iteration) {
    __mmask16 base_mask = 0xAAAA;
    
    if (iteration % 4 == 0) {
        __m512 cmp_a = _mm512_set1_ps(iteration * 1.0f);
        __m512 cmp_b = _mm512_set1_ps(iteration * 2.0f);
        return _mm512_cmp_ps_mask(cmp_a, cmp_b, _CMP_EQ_OQ);
    } else {
        return _kand_mask16(base_mask, 0x5555 >> (iteration % 8));
    }
}

__mmask8 generate_complex_mask8(int iteration) {
    __mmask8 base_mask = 0xAA;
    
    if (iteration % 5 == 0) {
        __m512d cmp_a = _mm512_set1_pd(iteration * 1.0);
        __m512d cmp_b = _mm512_set1_pd(iteration * 3.0);
        return _mm512_cmp_pd_mask(cmp_a, cmp_b, _CMP_EQ_OQ);
    } else {
        return _kor_mask8(base_mask, 0x55 >> (iteration % 4));
    }
}

// Blend function implementations (each targeting specific expander)
__attribute__((noinline)) __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline)) __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

__attribute__((noinline)) __m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline)) __m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline)) __m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

__attribute__((noinline)) __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}
