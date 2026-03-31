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

// Helper function to generate checksum
float horizontal_sum_ps(__m512 v) {
    __m256 v256 = _mm512_castps512_ps256(v) + _mm512_extractf32x8_ps(v, 1);
    __m128 v128 = _mm256_castps256_ps128(v256) + _mm256_extractf128_ps(v256, 1);
    v128 = _mm_hadd_ps(v128, v128);
    v128 = _mm_hadd_ps(v128, v128);
    return _mm_cvtss_f32(v128);
}

double horizontal_sum_pd(__m512d v) {
    __m256d v256 = _mm512_castpd512_pd256(v) + _mm512_extractf64x4_pd(v, 1);
    __m128d v128 = _mm256_castpd256_pd128(v256) + _mm256_extractf128_pd(v256, 1);
    v128 = _mm_hadd_pd(v128, v128);
    return _mm_cvtsd_f64(v128);
}

int main() {
    uint64_t final_checksum = 0;
    
    // Initialize test vectors with patterned data
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
    
    // Control flow with switch statement for mask generation
    int mode_selector = 0;
    
    // Test all modes in a loop with control flow
    for (int iteration = 0; iteration < 4; iteration++) {
        mode_selector = iteration % 4;
        
        switch (mode_selector) {
            case 0: {
                // V64QImode: Use comparison mask
                __mmask64 k64 = _mm512_cmpeq_epi8_mask(
                    _mm512_and_si512(v64qi_a, _mm512_set1_epi8(1)),
                    _mm512_setzero_si512()
                );
                
                // Combine with immediate mask using logical operation
                __mmask64 k64_imm = 0xAAAAAAAAAAAAAAAAULL;
                k64 = _kor_mask64(k64, k64_imm);
                
                __m512i result = blend_v64qi(v64qi_a, v64qi_b, k64);
                
                // Data dependency chain: use result to generate mask for next blend
                __mmask32 k32_from_64qi = _mm512_cmpeq_epi16_mask(
                    _mm512_and_si512(result, _mm512_set1_epi16(1)),
                    _mm512_setzero_si512()
                );
                
                // Accumulate checksum
                __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
                final_checksum += _mm512_reduce_add_epi64(sum64);
                
                // Use the generated mask for V32HImode blend
                __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, k32_from_64qi);
                __m512i sum32hi = _mm512_sad_epu8(result32hi, _mm512_setzero_si512());
                final_checksum += _mm512_reduce_add_epi64(sum32hi);
                break;
            }
            
            case 1: {
                // V32HImode: Immediate mask
                __mmask32 k32 = 0xAAAAAAAA;
                
                __m512i result = blend_v32hi(v32hi_a, v32hi_b, k32);
                
                // Generate mask for V16SFmode using comparison
                __mmask16 k16_from_32hi = _mm512_cmpeq_epi32_mask(
                    _mm512_and_si512(result, _mm512_set1_epi32(1)),
                    _mm512_setzero_si512()
                );
                
                // Accumulate checksum
                __m512i sum32 = _mm512_sad_epu8(result, _mm512_setzero_si512());
                final_checksum += _mm512_reduce_add_epi64(sum32);
                
                // Use generated mask for V16SFmode blend
                __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, k16_from_32hi);
                final_checksum += (uint64_t)horizontal_sum_ps(result16sf);
                break;
            }
            
            case 2: {
                // V16SImode: Comparison mask with control flow
                __mmask16 k16;
                if (iteration % 2 == 0) {
                    k16 = _mm512_cmpeq_epi32_mask(
                        _mm512_and_si512(v16si_a, _mm512_set1_epi32(3)),
                        _mm512_setzero_si512()
                    );
                } else {
                    k16 = 0xAAAA; // Immediate mask
                }
                
                __m512i result = blend_v16si(v16si_a, v16si_b, k16);
                
                // Generate mask for V8DFmode
                __mmask8 k8_from_16si = _mm512_cmpeq_epi64_mask(
                    _mm512_and_si512(result, _mm512_set1_epi64(1)),
                    _mm512_setzero_si512()
                );
                
                // Accumulate checksum
                __m512i sum16 = _mm512_sad_epu8(result, _mm512_setzero_si512());
                final_checksum += _mm512_reduce_add_epi64(sum16);
                
                // Use generated mask for V8DFmode blend
                __m512d result8df = blend_v8df(v8df_a, v8df_b, k8_from_16si);
                final_checksum += (uint64_t)horizontal_sum_pd(result8df);
                break;
            }
            
            case 3: {
                // V8DImode: Complex mask generation with logical operations
                __mmask8 k8_comp = _mm512_cmpeq_epi64_mask(
                    _mm512_and_si512(v8di_a, _mm512_set1_epi64(1)),
                    _mm512_setzero_si512()
                );
                
                __mmask8 k8_imm = 0xAA;
                __mmask8 k8 = _kand_mask8(k8_comp, k8_imm);
                k8 = _kxor_mask8(k8, 0xFF); // Invert mask
                
                __m512i result = blend_v8di(v8di_a, v8di_b, k8);
                
                // Generate mask for V16SFmode from result
                __mmask16 k16_from_8di = _mm512_cmpeq_epi32_mask(
                    _mm512_and_si512(result, _mm512_set1_epi32(1)),
                    _mm512_setzero_si512()
                );
                
                // Accumulate checksum
                __m512i sum8 = _mm512_sad_epu8(result, _mm512_setzero_si512());
                final_checksum += _mm512_reduce_add_epi64(sum8);
                
                // Use generated mask for another V16SFmode blend
                __m512 result16sf2 = blend_v16sf(v16sf_b, v16sf_a, k16_from_8di);
                final_checksum += (uint64_t)horizontal_sum_ps(result16sf2);
                break;
            }
        }
        
        // Additional V16SFmode and V8DFmode tests with different mask types
        if (iteration == 0) {
            // V16SFmode: Comparison mask
            __mmask16 k16_cmp = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OS);
            __m512 result = blend_v16sf(v16sf_a, v16sf_b, k16_cmp);
            final_checksum += (uint64_t)horizontal_sum_ps(result);
        }
        
        if (iteration == 1) {
            // V8DFmode: Comparison mask
            __mmask8 k8_cmp = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OS);
            __m512d result = blend_v8df(v8df_a, v8df_b, k8_cmp);
            final_checksum += (uint64_t)horizontal_sum_pd(result);
        }
    }
    
#ifdef __AVX512FP16__
    // Test half-precision float modes if supported
    __m512h v32hf_a = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    
    __m512h v32hf_b = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
    
    // V32HFmode: Mixed mask generation
    __mmask32 k32_hf_comp = _mm512_cmp_ph_mask(v32hf_a, v32hf_b, _CMP_LT_OS);
    __mmask32 k32_hf_imm = 0xAAAAAAAA;
    __mmask32 k32_hf = _kor_mask32(k32_hf_comp, k32_hf_imm);
    
    __m512h result_hf = blend_v32hf(v32hf_a, v32hf_b, k32_hf);
    
    // V32BFmode: Use same mask
    __m512bh result_bf = blend_v32bf(v32bf_a, v32bf_b, k32_hf);
    
    // Convert back to check results
    __m512h result_bf_as_hf = _mm512_castbh_ph(result_bf);
    
    // Simple checksum for half-precision (convert to float for accumulation)
    __m512 result_hf_f32 = _mm512_cvtph_ps(result_hf);
    __m512 result_bf_f32 = _mm512_cvtph_ps(result_bf_as_hf);
    
    final_checksum += (uint64_t)horizontal_sum_ps(result_hf_f32);
    final_checksum += (uint64_t)horizontal_sum_ps(result_bf_f32);
#endif
    
    printf("Final checksum: %lu\n", final_checksum);
    return 0;
}
