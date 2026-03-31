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

__attribute__((noinline, target("avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
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

// Helper function to generate masks based on control flow
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    
    // Switch statement for mask generation diversity
    switch (iteration % 4) {
        case 0:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0));
            break;
        case 1:
            // Immediate mask
            mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
            break;
        case 2:
            // Logical combination of masks
            {
                __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(64));
                __mmask64 m2 = (__mmask64)0x5555555555555555ULL;
                mask = _kor_mask64(m1, m2);
            }
            break;
        default:
            // Complex logical operation
            {
                __mmask64 m1 = _mm512_cmpgt_epi8_mask(data, _mm512_set1_epi8(32));
                __mmask64 m2 = (__mmask64)0x3333333333333333ULL;
                __mmask64 m3 = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(16));
                mask = _kxor_mask64(_kand_mask64(m1, m2), m3);
            }
            break;
    }
    
    return mask;
}

int main() {
    uint64_t checksum = 0;
    
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
    
    // Half-precision data (requires -mavx512fp16)
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        0.0h, 1.0h, 2.0h, 3.0h, 4.0h, 5.0h, 6.0h, 7.0h,
        8.0h, 9.0h, 10.0h, 11.0h, 12.0h, 13.0h, 14.0h, 15.0h,
        16.0h, 17.0h, 18.0h, 19.0h, 20.0h, 21.0h, 22.0h, 23.0h,
        24.0h, 25.0h, 26.0h, 27.0h, 28.0h, 29.0h, 30.0h, 31.0h
    );
    
    __m512h v32hf_b = _mm512_set_ph(
        31.0h, 30.0h, 29.0h, 28.0h, 27.0h, 26.0h, 25.0h, 24.0h,
        23.0h, 22.0h, 21.0h, 20.0h, 19.0h, 18.0h, 17.0h, 16.0h,
        15.0h, 14.0h, 13.0h, 12.0h, 11.0h, 10.0h, 9.0h, 8.0h,
        7.0h, 6.0h, 5.0h, 4.0h, 3.0h, 2.0h, 1.0h, 0.0h
    );
    
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
    #endif
    
    // Loop with control flow to prevent optimization
    for (int i = 0; i < 8; i++) {
        __mmask64 mask64 = generate_complex_mask64(i, v64qi_a);
        
        // V64QImode blend
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use result to generate mask for next blend (data dependency chain)
        __mmask32 mask32 = _mm512_cmpeq_epi16_mask(result64qi, _mm512_set1_epi16(0));
        
        // V32HImode blend with logical mask operation
        __mmask32 alt_mask32 = (__mmask32)0xAAAAAAAA;
        __mmask32 combined_mask32 = _kor_mask32(mask32, alt_mask32);
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, combined_mask32);
        
        // Generate mask for float blends from integer result
        __mmask16 mask16_sf = _mm512_cmpeq_epi32_mask(result32hi, _mm512_set1_epi32(0));
        
        // V16SFmode blend with comparison mask
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_sf);
        
        // Generate mask for double blend from float result
        __mmask8 mask8_df = _mm512_cmp_ps_mask(result16sf, _mm512_set1_ps(0.0f), _CMP_EQ_OQ);
        
        // V8DFmode blend
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8_df);
        
        // V16SImode blend with immediate mask
        __mmask16 mask16_si = (__mmask16)0xAAAA;
        __m512i result16si = blend_v16si(_mm512_castps_si512(v16sf_a), 
                                        _mm512_castps_si512(v16sf_b), mask16_si);
        
        // V8DImode blend with logical mask combination
        __mmask8 mask8_di = _mm512_cmp_pd_mask(result8df, _mm512_set1_pd(0.0), _CMP_GT_OQ);
        __mmask8 imm_mask8 = (__mmask8)0xAA;
        __mmask8 final_mask8 = _kxor_mask8(mask8_di, imm_mask8);
        __m512i result8di = blend_v8di(_mm512_castpd_si512(v8df_a), 
                                      _mm512_castpd_si512(v8df_b), final_mask8);
        
        #ifdef __AVX512FP16__
        // V32HFmode blend with complex mask generation
        __mmask32 mask32_hf;
        if (i % 2 == 0) {
            mask32_hf = (__mmask32)0x55555555;
        } else {
            mask32_hf = _mm512_cmp_ph_mask(v32hf_a, v32hf_b, _CMP_LT_OQ);
        }
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
        
        // V32BFmode blend using result from previous blend
        __mmask32 mask32_bf = _mm512_cmp_ph_mask(result32hf, _mm512_set1_ph(0.0h), _CMP_EQ_OQ);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
        #endif
        
        // Accumulate checksum to prevent optimization
        // Horizontal reduction for each result
        __m512i sum64qi = _mm512_sad_epu8(result64qi, _mm512_setzero_si512());
        __m512i sum32hi = _mm512_madd_epi16(result32hi, _mm512_set1_epi16(1));
        __m512 sum16sf = _mm512_add_ps(result16sf, _mm512_permute_ps(result16sf, 0x4E));
        __m512d sum8df = _mm512_add_pd(result8df, _mm512_permute_pd(result8df, 0x55));
        __m512i sum16si = _mm512_add_epi32(result16si, _mm512_permutexvar_epi32(
            _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15), result16si));
        __m512i sum8di = _mm512_add_epi64(result8di, _mm512_permutexvar_epi64(
            _mm512_set_epi64(0,1,2,3,4,5,6,7), result8di));
        
        checksum += _mm512_reduce_add_epi64(sum64qi);
        checksum += _mm512_reduce_add_epi32(sum32hi);
        checksum += (uint64_t)_mm512_reduce_add_ps(sum16sf);
        checksum += (uint64_t)_mm512_reduce_add_pd(sum8df);
        checksum += _mm512_reduce_add_epi32(sum16si);
        checksum += _mm512_reduce_add_epi64(sum8di);
        
        #ifdef __AVX512FP16__
        // For half-precision, convert to float for reduction
        __m512 result32hf_f32 = _mm512_cvtph_ps(result32hf);
        __m512 sum32hf = _mm512_add_ps(result32hf_f32, 
                                      _mm512_permute_ps(result32hf_f32, 0x4E));
        checksum += (uint64_t)_mm512_reduce_add_ps(sum32hf);
        #endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
