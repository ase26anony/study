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
__attribute__((noinline, target("avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to generate complex mask patterns
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    // Method 1: Immediate mask with pattern
    __mmask64 imm_mask = 0xAAAAAAAAAAAAAAAAULL;
    
    // Method 2: Comparison mask (dynamic)
    __m512i vec1 = _mm512_set1_epi8(iteration);
    __m512i vec2 = _mm512_set1_epi8(iteration % 256);
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(vec1, vec2);
    
    // Method 3: Logical combination
    __mmask64 k1 = _mm512_cmpgt_epi8_mask(vec1, _mm512_set1_epi8(0));
    __mmask64 k2 = _mm512_cmplt_epi8_mask(vec1, _mm512_set1_epi8(128));
    __mmask64 logical_mask = _kor_mask64(k1, k2);
    
    // Combine all methods
    return _kxor_mask64(imm_mask, _kand_mask64(cmp_mask, logical_mask));
}

__attribute__((noinline))
__mmask32 generate_complex_mask32(int iteration) {
    __mmask32 imm_mask = 0xAAAAAAAA;
    __m512i vec = _mm512_set1_epi16(iteration);
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(vec, _mm512_set1_epi16(iteration % 65536));
    __mmask32 k1 = _mm512_cmpgt_epi16_mask(vec, _mm512_set1_epi16(0));
    return _kxor_mask32(imm_mask, _kand_mask32(cmp_mask, k1));
}

__attribute__((noinline))
__mmask16 generate_complex_mask16(int iteration) {
    __mmask16 imm_mask = 0xAAAA;
    __m512 vec = _mm512_set1_ps((float)iteration);
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(vec, _mm512_set1_ps((float)(iteration % 100)), _CMP_EQ_OQ);
    __mmask16 k1 = _mm512_cmp_ps_mask(vec, _mm512_setzero_ps(), _CMP_GT_OQ);
    return _kxor_mask16(imm_mask, _kand_mask16(cmp_mask, k1));
}

__attribute__((noinline))
__mmask8 generate_complex_mask8(int iteration) {
    __mmask8 imm_mask = 0xAA;
    __m512d vec = _mm512_set1_pd((double)iteration);
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(vec, _mm512_set1_pd((double)(iteration % 100)), _CMP_EQ_OQ);
    __mmask8 k1 = _mm512_cmp_pd_mask(vec, _mm512_setzero_pd(), _CMP_GT_OQ);
    return _kxor_mask8(imm_mask, _kand_mask8(cmp_mask, k1));
}

int main() {
    uint64_t checksum = 0;
    
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
    
    // Control flow with loop to prevent constant folding
    for (int i = 0; i < 10; i++) {
        // Switch statement to select different blend modes
        switch (i % 4) {
            case 0: {
                // V64QImode blend with complex mask generation
                __mmask64 mask64 = generate_complex_mask64(i);
                __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
                
                // Use result to generate mask for next blend (data dependency chain)
                __mmask32 mask_from_64qi = _mm512_cmpeq_epi16_mask(
                    _mm512_and_si512(result64qi, _mm512_set1_epi16(0xFF)),
                    _mm512_set1_epi16(0)
                );
                
                // V32HImode blend using mask derived from V64QImode result
                __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask_from_64qi);
                
                // Horizontal sum for checksum
                __m256i sum256 = _mm512_extracti64x4_epi64(result64qi, 0);
                sum256 = _mm256_add_epi64(sum256, _mm512_extracti64x4_epi64(result64qi, 1));
                __m128i sum128 = _mm256_extracti128_si256(sum256, 0);
                sum128 = _mm_add_epi64(sum128, _mm256_extracti128_si256(sum256, 1));
                checksum += _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
                break;
            }
            
            case 1: {
                // V16SImode blend
                __mmask16 mask16 = generate_complex_mask16(i);
                __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
                
                // Use result to generate mask for float blend
                __mmask16 float_mask = _mm512_cmpeq_epi32_mask(
                    result16si,
                    _mm512_set1_epi32(i)
                );
                
                // V16SFmode blend using mask derived from V16SImode result
                __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, float_mask);
                
                // Horizontal sum
                __m256 sum256 = _mm512_extractf32x8_ps(result16sf, 0);
                sum256 = _mm256_add_ps(sum256, _mm512_extractf32x8_ps(result16sf, 1));
                __m128 sum128 = _mm256_extractf128_ps(sum256, 0);
                sum128 = _mm_add_ps(sum128, _mm256_extractf128_ps(sum256, 1));
                checksum += (uint64_t)_mm_cvtss_f32(sum128);
                break;
            }
            
            case 2: {
                // V8DImode blend
                __mmask8 mask8 = generate_complex_mask8(i);
                __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
                
                // Use result to generate mask for double blend
                __mmask8 double_mask = _mm512_cmpeq_epi64_mask(
                    result8di,
                    _mm512_set1_epi64(i)
                );
                
                // V8DFmode blend using mask derived from V8DImode result
                __m512d result8df = blend_v8df(v8df_a, v8df_b, double_mask);
                
                // Horizontal sum
                __m256d sum256 = _mm512_extractf64x4_pd(result8df, 0);
                sum256 = _mm256_add_pd(sum256, _mm512_extractf64x4_pd(result8df, 1));
                __m128d sum128 = _mm256_extractf128_pd(sum256, 0);
                sum128 = _mm_add_pd(sum128, _mm256_extractf128_pd(sum256, 1));
                checksum += (uint64_t)_mm_cvtsd_f64(sum128);
                break;
            }
            
            case 3: {
                // If-else chain for different blend combinations
                if (i < 5) {
                    __mmask32 mask32 = generate_complex_mask32(i);
                    __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
                    
                    // Use result in next blend
                    __mmask16 derived_mask = _mm512_cmpeq_epi32_mask(
                        _mm512_cvtepi16_epi32(_mm512_extracti32x8_epi32(result32hi, 0)),
                        _mm512_set1_epi32(i)
                    );
                    
                    __m512i result16si = blend_v16si(v16si_a, v16si_b, derived_mask);
                    
                    // Horizontal sum
                    __m256i sum256 = _mm512_extracti64x4_epi64(result32hi, 0);
                    sum256 = _mm256_add_epi64(sum256, _mm512_extracti64x4_epi64(result32hi, 1));
                    __m128i sum128 = _mm256_extracti128_si256(sum256, 0);
                    sum128 = _mm_add_epi64(sum128, _mm256_extracti128_si256(sum256, 1));
                    checksum += _mm_extract_epi64(sum128, 0);
                } else {
                    // Alternative path
                    __mmask64 mask64 = 0x5555555555555555ULL ^ i;
                    __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
                    
                    // Horizontal sum
                    __m256i sum256 = _mm512_extracti64x4_epi64(result64qi, 0);
                    sum256 = _mm256_add_epi64(sum256, _mm512_extracti64x4_epi64(result64qi, 1));
                    __m128i sum128 = _mm256_extracti128_si256(sum256, 0);
                    sum128 = _mm_add_epi64(sum128, _mm256_extracti128_si256(sum256, 1));
                    checksum += _mm_extract_epi64(sum128, 1);
                }
                break;
            }
        }
        
        // Half-precision float blends (if supported)
        #ifdef __AVX512FP16__
        if (i % 3 == 0) {
            __m512h v32hf_a = _mm512_set1_ph((__fp16)1.0f);
            __m512h v32hf_b = _mm512_set1_ph((__fp16)2.0f);
            __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
            __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
            
            __mmask32 hf_mask = 0xAAAAAAAA | (i & 0xFFFF);
            __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, hf_mask);
            __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, hf_mask);
            
            // Convert to float for checksum
            __m512 result_f32 = _mm512_cvtph_ps(_mm512_castbh_ph(result32bf));
            __m256 sum256 = _mm512_extractf32x8_ps(result_f32, 0);
            sum256 = _mm256_add_ps(sum256, _mm512_extractf32x8_ps(result_f32, 1));
            __m128 sum128 = _mm256_extractf128_ps(sum256, 0);
            sum128 = _mm_add_ps(sum128, _mm256_extractf128_ps(sum256, 1));
            checksum += (uint64_t)_mm_cvtss_f32(sum128);
        }
        #endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
