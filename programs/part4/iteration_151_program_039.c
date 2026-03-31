#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend modes
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 mask64, __mmask32 mask32) {
    // Chain epi8 -> epi16 blend
    __m512i blend1 = _mm512_mask_blend_epi8(mask64, a, b);
    __m512i blend2 = _mm512_mask_blend_epi16(mask32, blend1, _mm512_add_epi16(a, b));
    return blend2;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d,
                                      __mmask16 mask16, __mmask8 mask8) {
    // Chain ps -> pd blend
    __m512 blend1 = _mm512_mask_blend_ps(mask16, a, b);
    __m512d blend2 = _mm512_mask_blend_pd(mask8, c, d);
    // Convert pd result back to ps for mixing
    __m512 conv = _mm512_castpd_ps(blend2);
    return _mm512_add_ps(blend1, conv);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 mask32) {
    // Use same intrinsic for both HF and BF modes (compiler handles type)
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

// Function with data-dependent control flow
static void conditional_blend(int mode, __m512i* a, __m512i* b, __m512i* result) {
    if (mode & 1) {
        // Generate mask via comparison
        __mmask64 mask = _mm512_cmpeq_epi8_mask(*a, *b);
        mask = _knot_mask64(mask); // Invert mask
        *result = _mm512_mask_blend_epi8(mask, *a, *b);
    } else {
        // Generate mask via bit pattern
        __mmask32 mask = 0xAAAAAAAA; // Alternating pattern
        *result = _mm512_mask_blend_epi16(mask, *a, *b);
    }
}

// Another function with different mask generation pattern
static __m512i blend_with_dynamic_mask(__m512i a, __m512i b, int threshold) {
    // Create mask by comparing with threshold
    __m512i thresh_vec = _mm512_set1_epi32(threshold);
    __mmask16 mask = _mm512_cmp_epi32_mask(a, thresh_vec, _MM_CMPINT_GT);
    
    // Combine with another mask pattern
    __mmask16 pattern = 0x5555; // Alternating 0101
    __mmask16 final_mask = _kor_mask16(mask, pattern);
    
    return _mm512_mask_blend_epi32(final_mask, a, b);
}

int main() {
    // Initialize vectors with distinct patterns
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
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
#endif
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Loop with varying conditions to stress CFG
    for (int i = 0; i < 4; i++) {
        // Generate masks using different methods
        __mmask64 mask64_imm = 0xAAAAAAAAAAAAAAAA; // Immediate constant
        __mmask64 mask64_cmp = _mm512_cmpeq_epi8_mask(v64qi_a, v64qi_b);
        
        __mmask32 mask32_imm = 0xAAAAAAAA; // Immediate constant
        __mmask32 mask32_cmp = _mm512_cmpeq_epi16_mask(v32hi_a, v32hi_b);
        
        __mmask16 mask16_imm = 0xAAAA; // Immediate constant
        __mmask16 mask16_cmp = _mm512_cmp_epi32_mask(v16si_a, v16si_b, _MM_CMPINT_EQ);
        
        __mmask8 mask8_imm = 0xAA; // Immediate constant
        __mmask8 mask8_cmp = _mm512_cmp_epi64_mask(v8di_a, v8di_b, _MM_CMPINT_EQ);
        
        __mmask16 mask16_ps = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_EQ_OQ);
        __mmask8 mask8_pd = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_EQ_OQ);
        
        // Perform blends for all modes
        __m512i blend_epi8 = _mm512_mask_blend_epi8(mask64_imm, v64qi_a, v64qi_b);
        __m512i blend_epi16 = _mm512_mask_blend_epi16(mask32_imm, v32hi_a, v32hi_b);
        __m512i blend_epi32 = _mm512_mask_blend_epi32(mask16_imm, v16si_a, v16si_b);
        __m512i blend_epi64 = _mm512_mask_blend_epi64(mask8_imm, v8di_a, v8di_b);
        __m512 blend_ps = _mm512_mask_blend_ps(mask16_ps, v16sf_a, v16sf_b);
        __m512d blend_pd = _mm512_mask_blend_pd(mask8_pd, v8df_a, v8df_b);
        
#ifdef __AVX512FP16__
        __mmask32 mask32_hf = 0xAAAAAAAA; // Immediate constant
        __m512h blend_hf = _mm512_mask_blend_ph(mask32_hf, v32hf_a, v32hf_b);
#endif
        
        // Chain blends through helper functions
        __m512i chained1 = blend_v64qi_v32hi(v64qi_a, v64qi_b, mask64_cmp, mask32_cmp);
        __m512 chained2 = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, 
                                          mask16_ps, mask8_pd);
        
        // Conditional blends based on loop iteration
        __m512i cond_result;
        conditional_blend(i, &v64qi_a, &v64qi_b, &cond_result);
        
        __m512i dyn_result = blend_with_dynamic_mask(v16si_a, v16si_b, i * 4);
        
        // Reduce results to prevent dead code elimination
        // Horizontal sums for integer vectors
        __m256i sum256 = _mm512_castsi512_si256(_mm512_add_epi64(
            _mm512_cvtepi32_epi64(_mm512_castsi512_si256(blend_epi32)),
            _mm512_cvtepi32_epi64(_mm512_extracti32x8_epi32(blend_epi32, 1))
        ));
        
        __m128i sum128 = _mm_add_epi64(
            _mm256_castsi256_si128(sum256),
            _mm256_extracti128_si256(sum256, 1)
        );
        
        uint64_t int_sum = (uint64_t)_mm_extract_epi64(sum128, 0) +
                          (uint64_t)_mm_extract_epi64(sum128, 1);
        
        // Reduce floating point vectors
        float float_sum = _mm512_reduce_add_ps(blend_ps);
        double double_sum = _mm512_reduce_add_pd(blend_pd);
        
        // Accumulate to checksum
        checksum += int_sum + float_sum + double_sum;
        
#ifdef __AVX512FP16__
        // Manual reduction for half precision
        __m256h blend256h = _mm512_castph256_ph512(blend_hf);
        __m128h blend128h = _mm256_castph128_ph256(blend256h);
        // Convert to float for accumulation
        float hf_sum = 0;
        for (int j = 0; j < 16; j++) {
            hf_sum += (float)blend128h[j];
        }
        checksum += hf_sum;
#endif
        
        // Modify inputs for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(1.0f));
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
