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

__attribute__((noinline)) __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline)) __m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

// Helper function to generate masks based on control flow
__mmask64 generate_v64qi_mask(int iteration, __m512i data) {
    __mmask64 mask;
    
    // Control flow to generate different mask types
    if (iteration % 3 == 0) {
        // Immediate mask
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (iteration % 3 == 1) {
        // Comparison mask
        __m512i zero = _mm512_setzero_si512();
        mask = _mm512_cmpeq_epi8_mask(data, zero);
    } else {
        // Logical operation on masks
        __mmask64 mask1 = 0x5555555555555555ULL;
        __mmask64 mask2 = 0xAAAAAAAAAAAAAAAAULL;
        mask = _kor_mask64(mask1, mask2);
    }
    
    return mask;
}

__mmask32 generate_v32hi_mask(int iteration, __m512i data) {
    __mmask32 mask;
    
    switch (iteration % 4) {
        case 0:
            mask = 0xAAAAAAAA;  // Immediate
            break;
        case 1:
            mask = _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(42));
            break;
        case 2:
            mask = _mm512_cmplt_epi16_mask(data, _mm512_set1_epi16(100));
            break;
        default:
            mask = _kand_mask32(0x55555555, 0xAAAAAAAA);
            break;
    }
    
    return mask;
}

__mmask16 generate_v16sf_mask(int iteration, __m512 data) {
    __mmask16 mask;
    
    if (iteration < 5) {
        mask = 0xAAAA;  // Immediate
    } else if (iteration < 10) {
        mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.0f), _CMP_GT_OQ);
    } else {
        __mmask16 mask1 = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.5f), _CMP_LT_OQ);
        __mmask16 mask2 = _mm512_cmp_ps_mask(data, _mm512_set1_ps(-0.5f), _CMP_GT_OQ);
        mask = _kxor_mask16(mask1, mask2);
    }
    
    return mask;
}

__mmask8 generate_v8df_mask(int iteration, __m512d data) {
    __mmask8 mask;
    
    if (iteration % 2 == 0) {
        mask = 0xAA;  // Immediate
    } else {
        mask = _mm512_cmp_pd_mask(data, _mm512_set1_pd(0.0), _CMP_NEQ_OQ);
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
        0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f,
        0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        1.5f, 1.4f, 1.3f, 1.2f, 1.1f, 1.0f, 0.9f, 0.8f,
        0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, 0.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7);
    __m512d v8df_b = _mm512_set_pd(0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0);
    
    __m512i v16si_a = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i v16si_b = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    __m512i v8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i v8di_b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f,
        0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f,
        1.6f, 1.7f, 1.8f, 1.9f, 2.0f, 2.1f, 2.2f, 2.3f,
        2.4f, 2.5f, 2.6f, 2.7f, 2.8f, 2.9f, 3.0f, 3.1f
    );
    
    __m512h v32hf_b = _mm512_set_ph(
        3.1f, 3.0f, 2.9f, 2.8f, 2.7f, 2.6f, 2.5f, 2.4f,
        2.3f, 2.2f, 2.1f, 2.0f, 1.9f, 1.8f, 1.7f, 1.6f,
        1.5f, 1.4f, 1.3f, 1.2f, 1.1f, 1.0f, 0.9f, 0.8f,
        0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, 0.0f
    );
    
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
#endif
    
    // Main test loop with control flow
    for (int i = 0; i < 10; i++) {
        // Generate masks with different methods based on iteration
        __mmask64 mask64 = generate_v64qi_mask(i, v64qi_a);
        __mmask32 mask32_hi = generate_v32hi_mask(i, v32hi_a);
        __mmask16 mask16_sf = generate_v16sf_mask(i, v16sf_a);
        __mmask8 mask8_df = generate_v8df_mask(i, v8df_a);
        
        // Perform blends with control flow
        __m512i result64qi;
        if (i % 2 == 0) {
            result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        } else {
            result64qi = blend_v64qi(v64qi_b, v64qi_a, mask64);
        }
        
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32_hi);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_sf);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8_df);
        
        // Data dependency chain: use integer result to generate mask for float blend
        __mmask16 mask_from_int = _mm512_cmpeq_epi32_mask(result64qi, v16si_a);
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask_from_int);
        
        // Use float result to generate mask for double blend
        __mmask8 mask_from_float = _mm512_cmp_ps_mask(result16sf, _mm512_set1_ps(0.5f), _CMP_GT_OQ);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask_from_float);
        
#ifdef __AVX512FP16__
        // Half-precision blends
        __mmask32 mask32_hf;
        switch (i % 3) {
            case 0:
                mask32_hf = 0xAAAAAAAA;  // Immediate
                break;
            case 1:
                mask32_hf = _mm512_cmp_ph_mask(v32hf_a, _mm512_set1_ph(1.0f), _CMP_LT_OQ);
                break;
            default:
                mask32_hf = _kand_mask32(0x55555555, 0xAAAAAAAA);
                break;
        }
        
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32_hf);
#endif
        
        // Accumulate checksums to prevent optimization
        // Horizontal reduction for each result
        __m512i sum64qi = _mm512_sad_epu8(result64qi, _mm512_setzero_si512());
        checksum += _mm512_reduce_add_epi64(sum64qi);
        
        __m512i sum32hi = _mm512_madd_epi16(result32hi, _mm512_set1_epi16(1));
        checksum += _mm512_reduce_add_epi32(sum32hi);
        
        __m512 sum16sf = _mm512_add_ps(result16sf, _mm512_set1_ps(0.0f));
        checksum += (uint64_t)_mm512_reduce_add_ps(sum16sf);
        
        __m512d sum8df = _mm512_add_pd(result8df, _mm512_set1_pd(0.0));
        checksum += (uint64_t)_mm512_reduce_add_pd(sum8df);
        
        __m512i sum16si = _mm512_add_epi32(result16si, _mm512_set1_epi32(0));
        checksum += _mm512_reduce_add_epi32(sum16si);
        
        __m512i sum8di = _mm512_add_epi64(result8di, _mm512_set1_epi64(0));
        checksum += _mm512_reduce_add_epi64(sum8di);
        
#ifdef __AVX512FP16__
        // For half-precision, we need to convert to float for reduction
        __m512 conv32hf = _mm512_cvtph_ps(result32hf);
        checksum += (uint64_t)_mm512_reduce_add_ps(conv32hf);
#endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
