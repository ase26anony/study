#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure each blend gets expanded independently
__attribute__((noinline, target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to create complex mask patterns
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    // Mix of immediate and computed masks
    __mmask64 base_mask = 0xAAAAAAAAAAAAAAAAULL;
    __mmask64 dynamic_mask = 0;
    
    // Create pattern based on iteration
    for (int i = 0; i < 64; i++) {
        if ((i + iteration) % 3 == 0) {
            dynamic_mask |= (1ULL << i);
        }
    }
    
    // Combine masks with logical operations
    return _kor_mask64(base_mask, dynamic_mask);
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test vectors with patterned data
    __m512i v64qi_a = _mm512_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
    );
    
    __m512i v64qi_b = _mm512_set_epi8(
        63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
        47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
        31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    );
    
    __m512i v32hi_a = _mm512_set_epi16(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    );
    
    __m512i v32hi_b = _mm512_set_epi16(
        31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    );
    
    __m512i v16si_a = _mm512_set_epi32(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    
    __m512i v16si_b = _mm512_set_epi32(
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    );
    
    __m512i v8di_a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __m512i v8di_b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    
    __m512 v16sf_a = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m512d v8df_b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    
    // Control flow with loop to prevent constant folding
    for (int iter = 0; iter < 3; iter++) {
        // Switch statement to select different blend strategies
        switch (iter) {
            case 0: {
                // Immediate masks
                __mmask64 k64 = 0xAAAAAAAAAAAAAAAAULL;
                __mmask32 k32 = 0xAAAAAAAA;
                __mmask16 k16 = 0xAAAA;
                __mmask8 k8 = 0xAA;
                
                __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, k64);
                __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, k32);
                __m512i result16si = blend_v16si(v16si_a, v16si_b, k16);
                __m512i result8di = blend_v8di(v8di_a, v8di_b, k8);
                __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, k16);
                __m512d result8df = blend_v8df(v8df_a, v8df_b, k8);
                
                // Data dependency chain: use integer result to generate float mask
                __mmask16 cmp_mask = _mm512_cmpeq_epi32_mask(
                    _mm512_and_si512(result16si, _mm512_set1_epi32(1)),
                    _mm512_set1_epi32(0)
                );
                
                __m512 result16sf2 = blend_v16sf(result16sf, v16sf_a, cmp_mask);
                
                // Accumulate checksum
                checksum += _mm512_reduce_add_epi64(result64qi);
                checksum += _mm512_reduce_add_epi64(result32hi);
                checksum += _mm512_reduce_add_epi64(result16si);
                checksum += _mm512_reduce_add_epi64(result8di);
                checksum += (uint64_t)_mm512_reduce_add_ps(result16sf2);
                checksum += (uint64_t)_mm512_reduce_add_pd(result8df);
                break;
            }
            
            case 1: {
                // Comparison masks
                __mmask64 cmp_mask64 = _mm512_cmpeq_epi8_mask(
                    _mm512_and_si512(v64qi_a, _mm512_set1_epi8(1)),
                    _mm512_set1_epi8(0)
                );
                
                __mmask32 cmp_mask32 = _mm512_cmpeq_epi16_mask(
                    _mm512_and_si512(v32hi_a, _mm512_set1_epi16(1)),
                    _mm512_set1_epi16(0)
                );
                
                __mmask16 cmp_mask16_int = _mm512_cmpeq_epi32_mask(
                    _mm512_and_si512(v16si_a, _mm512_set1_epi32(1)),
                    _mm512_set1_epi32(0)
                );
                
                __mmask16 cmp_mask16_float = _mm512_cmp_ps_mask(
                    v16sf_a, 
                    _mm512_set1_ps(7.5f), 
                    _CMP_LT_OQ
                );
                
                __mmask8 cmp_mask8 = _mm512_cmp_pd_mask(
                    v8df_a,
                    _mm512_set1_pd(3.5),
                    _CMP_GT_OQ
                );
                
                // Logical mask operations
                __mmask64 complex_mask64 = _kor_mask64(
                    cmp_mask64,
                    generate_complex_mask64(iter)
                );
                
                __mmask32 complex_mask32 = _kxor_mask32(
                    cmp_mask32,
                    0x55555555
                );
                
                // Perform blends
                __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, complex_mask64);
                __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, complex_mask32);
                __m512i result16si = blend_v16si(v16si_a, v16si_b, cmp_mask16_int);
                __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, cmp_mask16_float);
                __m512d result8df = blend_v8df(v8df_a, v8df_b, cmp_mask8);
                
                // Accumulate checksum
                checksum += _mm512_reduce_add_epi64(result64qi);
                checksum += _mm512_reduce_add_epi64(result32hi);
                checksum += _mm512_reduce_add_epi64(result16si);
                checksum += (uint64_t)_mm512_reduce_add_ps(result16sf);
                checksum += (uint64_t)_mm512_reduce_add_pd(result8df);
                break;
            }
            
            case 2: {
                // Complex mask generation with if-else chain
                __mmask64 mask64;
                if (checksum % 2 == 0) {
                    mask64 = 0xFFFFFFFFFFFFFFFFULL;
                } else {
                    mask64 = _kand_mask64(
                        0x5555555555555555ULL,
                        generate_complex_mask64(iter)
                    );
                }
                
                // Use previous iteration's result to generate new mask
                __m512i temp = blend_v64qi(v64qi_a, v64qi_b, mask64);
                __mmask32 mask32_from_64qi = _mm512_cmpeq_epi16_mask(
                    _mm512_and_si512(temp, _mm512_set1_epi16(0xFF)),
                    _mm512_set1_epi16(0)
                );
                
                // Perform blends with dependent masks
                __m512i result64qi = temp;
                __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32_from_64qi);
                
                // Generate mask for float blend from integer result
                __mmask16 mask16_for_float = _mm512_cmpeq_epi32_mask(
                    _mm512_and_si512(result32hi, _mm512_set1_epi32(0xFFFF)),
                    _mm512_set1_epi32(0)
                );
                
                __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_for_float);
                
                // Accumulate checksum
                checksum += _mm512_reduce_add_epi64(result64qi);
                checksum += _mm512_reduce_add_epi64(result32hi);
                checksum += (uint64_t)_mm512_reduce_add_ps(result16sf);
                break;
            }
        }
        
        // Half-precision float blends (if supported)
        #ifdef __AVX512FP16__
        if (iter == 1) {  // Only in one iteration to reduce code size
            __m512h v32hf_a = _mm512_set_ph(
                0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
                8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
                16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
                24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
            );
            
            __m512h v32hf_b = _mm512_set_ph(
                31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
                23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
                15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
                7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
            );
            
            __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
            __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
            
            __mmask32 mask32_half = 0xAAAAAAAA;
            __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32_half);
            __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32_half);
            
            // Convert back to check (simplified - actual reduction would need more work)
            checksum += _mm512_castph_si512(result32hf)[0];
            checksum += _mm512_castbh_si512(result32bf)[0];
        }
        #endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
