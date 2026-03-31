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

// Helper function to create complex control flow
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    
    // Switch statement for control flow diversity
    switch (iteration % 4) {
        case 0:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0x55));
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
            // Nested condition
            if (iteration % 2 == 0) {
                mask = (__mmask64)0xFFFFFFFFFFFFFFFFULL;
            } else {
                mask = _mm512_cmpneq_epi8_mask(data, _mm512_setzero_si512());
            }
            break;
    }
    
    return mask;
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
    
    __m512i v64qi_b = _mm512_set1_epi8(0xFF);
    
    __m512i v32hi_a = _mm512_set_epi16(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(0xFFFF);
    
    __m512i v16si_a = _mm512_set_epi32(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    
    __m512i v16si_b = _mm512_set1_epi32(0xFFFFFFFF);
    
    __m512i v8di_a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __m512i v8di_b = _mm512_set1_epi64(0xFFFFFFFFFFFFFFFFULL);
    
    __m512 v16sf_a = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(100.0f);
    
    __m512d v8df_a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m512d v8df_b = _mm512_set1_pd(100.0);
    
    // Loop with control flow to prevent constant folding
    for (int i = 0; i < 8; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_complex_mask64(i, v64qi_a);
        __mmask32 mask32 = (__mmask32)(0xAAAAAAAA >> (i % 4));
        __mmask16 mask16 = _mm512_cmpeq_epi32_mask(v16si_a, _mm512_set1_epi32(i));
        __mmask8 mask8 = (__mmask8)(0xAA >> (i % 2));
        
        // Data dependency chain: use result from one blend to generate mask for another
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use the byte blend result to generate a mask for 16-bit blend
        __mmask32 mask32_from_chain = _mm512_cmplt_epi16_mask(
            result64qi, 
            _mm512_set1_epi16(32)
        );
        
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32_from_chain);
        
        // Use 16-bit result to generate mask for 32-bit blend
        __mmask16 mask16_from_chain = _mm512_cmpeq_epi32_mask(
            result32hi,
            _mm512_set1_epi32(0xFFFF)
        );
        
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16_from_chain);
        
        // Generate mask for float blend using integer comparison
        __mmask16 mask16_float = _mm512_cmp_ps_mask(
            v16sf_a,
            _mm512_set1_ps(i * 2.0f),
            _CMP_LT_OQ
        );
        
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_float);
        
        // Generate mask for double blend
        __mmask8 mask8_double = _mm512_cmp_pd_mask(
            v8df_a,
            _mm512_set1_pd(i * 1.0),
            _CMP_GT_OQ
        );
        
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8_double);
        
        // Blend for 64-bit integers
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
        
        // Horizontal reduction to prevent optimization
        __m512i sum64qi = _mm512_add_epi8(result64qi, result32hi);
        __m512i sum32hi = _mm512_add_epi16(result16si, result8di);
        __m512 sum_float = _mm512_add_ps(result16sf, _mm512_castsi512_ps(sum64qi));
        __m512d sum_double = _mm512_add_pd(result8df, _mm512_castsi512_pd(sum32hi));
        
        // Accumulate to checksum
        checksum += _mm512_reduce_add_epi64(sum64qi);
        checksum += _mm512_reduce_add_epi64(sum32hi);
        checksum += (uint64_t)_mm512_reduce_add_ps(sum_float);
        checksum += (uint64_t)_mm512_reduce_add_pd(sum_double);
        
        // Modify input vectors for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.5f));
    }
    
#ifdef __AVX512FP16__
    // Test half-precision blends if supported
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_castsi512_bh(_mm512_set1_epi16(0x3C00)); // 1.0 in bfloat16
    __m512bh v32bf_b = _mm512_castsi512_bh(_mm512_set1_epi16(0x4000)); // 2.0 in bfloat16
    
    __mmask32 mask_half = (__mmask32)0xAAAAAAAA;
    
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask_half);
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask_half);
    
    // Add to checksum
    __m512i hf_as_int = _mm512_castph_si512(result32hf);
    __m512i bf_as_int = _mm512_castbh_si512(result32bf);
    checksum += _mm512_reduce_add_epi64(hf_as_int);
    checksum += _mm512_reduce_add_epi64(bf_as_int);
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
