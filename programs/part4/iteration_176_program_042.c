#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure separate code generation
__attribute__((noinline))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to create complex control flow
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    
    // Switch statement for control flow diversity
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
            __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(64));
            __mmask64 m2 = (__mmask64)0x5555555555555555ULL;
            mask = _kor_mask64(m1, m2);
            break;
        case 3:
            // Complex logical operations
            __mmask64 m3 = _mm512_cmpgt_epi8_mask(data, _mm512_set1_epi8(32));
            __mmask64 m4 = (__mmask64)0xCCCCCCCCCCCCCCCCULL;
            mask = _kxor_mask64(m3, m4);
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
    
    __m512i v64qi_b = _mm512_set1_epi8(100);
    __m512i v32hi_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    __m512i v32hi_b = _mm512_set1_epi16(200);
    __m512i v16si_a = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    __m512i v16si_b = _mm512_set1_epi32(300);
    __m512i v8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i v8di_b = _mm512_set1_epi64(400);
    
    __m512 v16sf_a = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    __m512 v16sf_b = _mm512_set1_ps(500.0f);
    
    __m512d v8df_a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d v8df_b = _mm512_set1_pd(600.0);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    __m512h v32hf_b = _mm512_set1_ph(700.0f);
    
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
#endif
    
    // Loop with control flow to prevent optimization
    for (int i = 0; i < 8; i++) {
        // Generate masks with different methods based on loop index
        __mmask64 mask64;
        __mmask32 mask32;
        __mmask16 mask16;
        __mmask8 mask8;
        
        // Complex mask generation with control flow
        if (i < 4) {
            // Comparison-based masks
            mask64 = _mm512_cmpeq_epi8_mask(
                _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(i)),
                _mm512_set1_epi8(i * 10)
            );
            mask32 = _mm512_cmpeq_epi16_mask(
                _mm512_add_epi16(v32hi_a, _mm512_set1_epi16(i)),
                _mm512_set1_epi16(i * 5)
            );
            mask16 = _mm512_cmpeq_epi32_mask(
                _mm512_add_epi32(v16si_a, _mm512_set1_epi32(i)),
                _mm512_set1_epi32(i * 2)
            );
            mask8 = _mm512_cmpeq_epi64_mask(
                _mm512_add_epi64(v8di_a, _mm512_set1_epi64(i)),
                _mm512_set1_epi64(i)
            );
        } else {
            // Immediate masks with logical operations
            mask64 = (__mmask64)(0xF0F0F0F0F0F0F0F0ULL >> (i - 4));
            mask32 = (__mmask32)(0xAAAAAAAA >> (i - 4));
            mask16 = (__mmask16)(0xAAAA >> (i - 4));
            mask8 = (__mmask8)(0xAA >> (i - 4));
        }
        
        // Perform blends with data dependency chain
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use result to generate mask for next blend (data dependency)
        __mmask32 mask_from_64qi = _mm512_cmplt_epi16_mask(
            _mm512_cvtepi8_epi16(_mm512_castsi512_si256(result64qi)),
            _mm512_set1_epi16(50)
        );
        
        // Combine masks with logical operation
        mask32 = _kor_mask32(mask32, mask_from_64qi);
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        
        // More data dependency: use integer result for float comparison
        __mmask16 mask_for_sf = _mm512_cmp_ps_mask(
            _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(_mm512_castsi512_si256(result32hi))),
            _mm512_set1_ps(10.0f),
            _CMP_GT_OQ
        );
        
        // Perform blends for all modes
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask_for_sf);
        
        // Generate mask for double precision from single precision result
        __mmask8 mask_for_df = _mm512_cmp_pd_mask(
            _mm512_cvtps_pd(_mm512_castps512_ps256(result16sf)),
            _mm512_set1_pd(5.0),
            _CMP_LT_OQ
        );
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask_for_df);
        
#ifdef __AVX512FP16__
        // Generate mask for half precision from float result
        __mmask32 mask_for_hf = _mm512_cmp_ph_mask(
            _mm512_cvtps_ph(_mm512_castps512_ps256(result16sf), _MM_FROUND_CUR_DIRECTION),
            _mm512_set1_ph(2.5f),
            _CMP_EQ_OQ
        );
        
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask_for_hf);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask_for_hf);
#endif
        
        // Accumulate checksum to prevent optimization
        // Horizontal reduction for each result
        __m512i sum64qi = _mm512_sad_epu8(result64qi, _mm512_setzero_si512());
        __m512i sum32hi = _mm512_madd_epi16(result32hi, _mm512_set1_epi16(1));
        __m512i sum16si = _mm512_add_epi32(result16si, _mm512_setzero_si512());
        __m512i sum8di = _mm512_add_epi64(result8di, _mm512_setzero_si512());
        
        __m512 sum16sf = _mm512_add_ps(result16sf, _mm512_setzero_ps());
        __m512d sum8df = _mm512_add_pd(result8df, _mm512_setzero_pd());
        
        checksum += _mm512_reduce_add_epi64(sum64qi);
        checksum += _mm512_reduce_add_epi32(sum32hi);
        checksum += _mm512_reduce_add_epi32(sum16si);
        checksum += _mm512_reduce_add_epi64(sum8di);
        checksum += (uint64_t)_mm512_reduce_add_ps(sum16sf);
        checksum += (uint64_t)_mm512_reduce_add_pd(sum8df);
        
#ifdef __AVX512FP16__
        __m512h sum32hf = _mm512_add_ph(result32hf, _mm512_setzero_ph());
        // Note: _mm512_reduce_add_ph may not exist, use alternative reduction
        for (int j = 0; j < 32; j++) {
            checksum += (uint64_t)result32hf[j];
        }
#endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
