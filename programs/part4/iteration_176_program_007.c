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

// Helper function to create complex mask patterns
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    // Method 1: Immediate mask with pattern that changes based on iteration
    __mmask64 imm_mask = (iteration % 2) ? 
        0xAAAAAAAAAAAAAAAAULL : 0x5555555555555555ULL;
    
    // Method 2: Comparison mask
    __m512i vec_a = _mm512_set1_epi8(iteration);
    __m512i vec_b = _mm512_set1_epi8(iteration / 2);
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(vec_a, vec_b);
    
    // Method 3: Logical combination of masks
    return _kor_mask64(imm_mask, cmp_mask);
}

__attribute__((noinline))
__mmask32 generate_complex_mask32(int iteration) {
    // Multiple mask generation methods
    __mmask32 imm_mask = (iteration % 3) ? 0xAAAAAAAA : 0x55555555;
    
    __m512i vec_a = _mm512_set1_epi16(iteration);
    __m512i vec_b = _mm512_set1_epi16(iteration * 2);
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(vec_a, vec_b);
    
    return _kxor_mask32(imm_mask, cmp_mask);
}

__attribute__((noinline))
__mmask16 generate_complex_mask16(int iteration) {
    // Control flow dependent mask generation
    __mmask16 mask;
    if (iteration % 4 == 0) {
        mask = 0xAAAA;
    } else if (iteration % 4 == 1) {
        mask = 0x5555;
    } else if (iteration % 4 == 2) {
        __m512 vec_a = _mm512_set1_ps(iteration * 1.0f);
        __m512 vec_b = _mm512_set1_ps((iteration - 1) * 1.0f);
        mask = _mm512_cmp_ps_mask(vec_a, vec_b, _CMP_GT_OQ);
    } else {
        __m512d vec_a = _mm512_set1_pd(iteration * 1.0);
        __m512d vec_b = _mm512_set1_pd((iteration + 1) * 1.0);
        __mmask8 pd_mask = _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_LT_OQ);
        mask = _cvtu32_mask16(_cvtmask8_u32(pd_mask) * 0x0101);
    }
    return mask;
}

__attribute__((noinline))
__mmask8 generate_complex_mask8(int iteration) {
    // Switch-based mask generation
    switch (iteration % 5) {
        case 0: return 0xAA;
        case 1: return 0x55;
        case 2: return 0xF0;
        case 3: {
            __m512d vec_a = _mm512_set1_pd(iteration * 0.5);
            __m512d vec_b = _mm512_set1_pd(iteration * 0.25);
            return _mm512_cmp_pd_mask(vec_a, vec_b, _CMP_GT_OQ);
        }
        default: return 0xFF;
    }
}

// Horizontal reduction helpers
__attribute__((noinline))
int64_t reduce_v64qi(__m512i v) {
    __m256i v256 = _mm512_extracti64x4_epi64(v, 0);
    __m256i v256_hi = _mm512_extracti64x4_epi64(v, 1);
    v256 = _mm256_add_epi8(v256, v256_hi);
    
    __m128i v128 = _mm256_extracti128_si256(v256, 0);
    __m128i v128_hi = _mm256_extracti128_si256(v256, 1);
    v128 = _mm_add_epi8(v128, v128_hi);
    
    // Sum all bytes
    int64_t sum = 0;
    uint8_t bytes[16];
    _mm_storeu_si128((__m128i*)bytes, v128);
    for (int i = 0; i < 16; i++) {
        sum += bytes[i];
    }
    return sum;
}

__attribute__((noinline))
float reduce_v16sf(__m512 v) {
    return _mm512_reduce_add_ps(v);
}

__attribute__((noinline))
double reduce_v8df(__m512d v) {
    return _mm512_reduce_add_pd(v);
}

int main() {
    int64_t checksum = 0;
    
    // Initialize test vectors with patterned data
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(100);
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(200);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set1_epi32(300);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(400);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(500.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set1_pd(600.0);
    
    // Complex control flow with loop
    for (int i = 0; i < 10; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_complex_mask64(i);
        __mmask32 mask32 = generate_complex_mask32(i);
        __mmask16 mask16 = generate_complex_mask16(i);
        __mmask8 mask8 = generate_complex_mask8(i);
        
        // Perform blends in different modes
        __m512i blended_v64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        __m512i blended_v32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        __m512i blended_v16si = blend_v16si(v16si_a, v16si_b, mask16);
        __m512i blended_v8di = blend_v8di(v8di_a, v8di_b, mask8);
        __m512 blended_v16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
        __m512d blended_v8df = blend_v8df(v8df_a, v8df_b, mask8);
        
        // Data dependency chain: use result from one blend to affect another
        if (i > 0) {
            // Use integer blend result to generate new mask for float blend
            __mmask16 new_mask = _mm512_cmpeq_epi32_mask(
                blended_v16si, 
                _mm512_set1_epi32(i * 10)
            );
            blended_v16sf = blend_v16sf(blended_v16sf, v16sf_b, new_mask);
        }
        
        // Accumulate results to prevent optimization
        checksum += reduce_v64qi(blended_v64qi);
        checksum += _mm512_reduce_add_epi32(blended_v32hi);
        checksum += _mm512_reduce_add_epi32(blended_v16si);
        checksum += _mm512_reduce_add_epi64(blended_v8di);
        checksum += (int64_t)reduce_v16sf(blended_v16sf);
        checksum += (int64_t)reduce_v8df(blended_v8df);
        
        // Modify input vectors for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(1.0f));
    }
    
    // Half-precision blends (if supported)
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
    
    // Generate mask for half-precision
    __mmask32 mask_half = 0xAAAAAAAA;
    __m512h blended_v32hf = blend_v32hf(v32hf_a, v32hf_b, mask_half);
    __m512bh blended_v32bf = blend_v32bf(v32bf_a, v32bf_b, mask_half);
    
    // Add to checksum
    __m512h sum_half = _mm512_add_ph(blended_v32hf, blended_v32bf);
    float half_sum = _mm512_reduce_add_ph(sum_half);
    checksum += (int64_t)half_sum;
#endif
    
    printf("Final checksum: %ld\n", checksum);
    return 0;
}
