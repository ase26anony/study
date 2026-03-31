#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend operations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int selector) {
    // V64QImode blend
    __mmask64 mask64 = selector ? 0xAAAAAAAAAAAAAAAAULL : 0x5555555555555555ULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamically generated mask
    __m512i cmp_val = _mm512_set1_epi16(selector * 100);
    __mmask32 mask32 = _mm512_cmpgt_epi16_mask(result1, cmp_val);
    return _mm512_mask_blend_epi16(mask32, result1, _mm512_add_epi16(b, _mm512_set1_epi16(1)));
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // V16SFmode blend with comparison mask
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend with chained operation
    __mmask8 mask8 = iter & 0xFF;
    __m512d result2 = _mm512_mask_blend_pd(mask8, c, d);
    
    // Convert double result to float for chaining
    __m512 conv_result = _mm512_cvtpd_ps(result2);
    return _mm512_add_ps(result1, conv_result);
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    // V32HFmode blend with alternating pattern
    __mmask32 mask32 = pattern ? 0xAAAAAAAA : 0x55555555;
    __m512h result = _mm512_mask_blend_ph(mask32, a, b);
    
    // Apply bitwise NOT to mask for variation
    __mmask32 not_mask = _knot_mask32(mask32);
    return _mm512_mask_blend_ph(not_mask, result, _mm512_add_ph(b, _mm512_set1_ph(1.0f)));
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int threshold) {
    // V16SImode blend with comparison-generated mask
    __m512i thresh_vec = _mm512_set1_epi32(threshold);
    __mmask16 mask16 = _mm512_cmpgt_epi32_mask(a, thresh_vec);
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend with combined masks
    __mmask8 mask8_low = _mm512_cmpgt_epi64_mask(result1, _mm512_set1_epi64(0));
    __mmask8 mask8_high = threshold & 0xFF;
    __mmask8 combined_mask = _kor_mask8(mask8_low, mask8_high);
    
    return _mm512_mask_blend_epi64(combined_mask, c, d);
}

// Function with data-dependent control flow
static __m512i conditional_blend(int mode, __m512i a, __m512i b, __m512i c) {
    __m512i result;
    
    if (mode & 1) {
        // V64QImode path
        __mmask64 mask = (mode > 10) ? 0xFFFFFFFFFFFFFFFFULL : 0x0F0F0F0F0F0F0F0FULL;
        result = _mm512_mask_blend_epi8(mask, a, b);
    } else {
        // V32HImode path  
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, _mm512_set1_epi16(mode));
        result = _mm512_mask_blend_epi16(mask, b, c);
    }
    
    return result;
}

// Reduction helpers
static int64_t reduce_i64(__m512i v) {
    return _mm512_reduce_add_epi64(v);
}

static float reduce_f32(__m512 v) {
    return _mm512_reduce_add_ps(v);
}

static double reduce_f64(__m512d v) {
    __m256d low = _mm512_castpd512_pd256(v);
    __m256d high = _mm512_extractf64x4_pd(v, 1);
    __m256d sum256 = _mm256_add_pd(low, high);
    __m128d sum128 = _mm_add_pd(_mm256_castpd256_pd128(sum256), 
                                _mm256_extractf128_pd(sum256, 1));
    return _mm_cvtsd_f64(_mm_add_pd(sum128, _mm_unpackhi_pd(sum128, sum128)));
}

int main() {
    // Initialize patterned data
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
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.5f,14.0f,13.5f,13.0f,12.5f,12.0f,11.5f,
        11.0f,10.5f,10.0f,9.5f,9.0f,8.5f,8.0f,7.5f
    );
    __m512 v16sf_b = _mm512_set1_ps(20.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.5,6.0,5.5,5.0,4.5,4.0,3.5);
    __m512d v8df_b = _mm512_set1_pd(10.0);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    __m512i v16si_b = _mm512_set1_epi32(50);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(100);
    
    // Initialize FP16 vectors if supported
    __m512h v32hf_a, v32hf_b;
    #ifdef __AVX512FP16__
    {
        _Float16 hf_vals[32];
        for (int i = 0; i < 32; i++) {
            hf_vals[i] = (_Float16)(i * 0.5f);
        }
        v32hf_a = _mm512_loadu_ph(hf_vals);
        v32hf_b = _mm512_set1_ph(10.0f);
    }
    #endif
    
    double checksum = 0.0;
    
    // Loop with varying conditions to trigger different blend paths
    for (int i = 0; i < 10; i++) {
        // Chain blends through different modes
        __m512i int_result = blend_v64qi_v32hi(v64qi_a, v64qi_b, i);
        
        // Conditional blend based on loop iteration
        __m512i cond_result = conditional_blend(i, v32hi_a, v32hi_b, int_result);
        
        // Integer mode blends
        __m512i si_di_result = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, i);
        
        // Floating point blends
        __m512 sf_df_result = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, i);
        
        #ifdef __AVX512FP16__
        // FP16 blends
        __m512h hf_result = blend_v32hf_v32bf(v32hf_a, v32hf_b, i);
        
        // Reduce FP16 result (manual reduction)
        _Float16 hf_vals[32];
        _mm512_storeu_ph(hf_vals, hf_result);
        float hf_sum = 0.0f;
        for (int j = 0; j < 32; j++) {
            hf_sum += (float)hf_vals[j];
        }
        checksum += hf_sum;
        #endif
        
        // Accumulate reductions
        checksum += reduce_i64(int_result);
        checksum += reduce_i64(cond_result);
        checksum += reduce_i64(si_di_result);
        checksum += reduce_f32(sf_df_result);
        checksum += reduce_f64(v8df_a);
        
        // Modify inputs for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.1f));
    }
    
    // Final direct blends with all mask generation patterns
    // Immediate constant mask
    __mmask64 imm_mask64 = 0xF0F0F0F0F0F0F0F0ULL;
    __m512i final_blend1 = _mm512_mask_blend_epi8(imm_mask64, v64qi_a, v64qi_b);
    
    // Comparison-generated mask
    __mmask32 cmp_mask32 = _mm512_cmpeq_epi16_mask(v32hi_a, v32hi_b);
    __m512i final_blend2 = _mm512_mask_blend_epi16(cmp_mask32, v32hi_a, v32hi_b);
    
    // Bitwise operation on mask
    __mmask16 mask16_a = _mm512_cmpgt_epi32_mask(v16si_a, _mm512_set1_epi32(5));
    __mmask16 mask16_b = _mm512_cmplt_epi32_mask(v16si_b, _mm512_set1_epi32(55));
    __mmask16 combined_mask16 = _kor_mask16(mask16_a, mask16_b);
    __m512i final_blend3 = _mm512_mask_blend_epi32(combined_mask16, v16si_a, v16si_b);
    
    // Add final reductions
    checksum += reduce_i64(final_blend1);
    checksum += reduce_i64(final_blend2);
    checksum += reduce_i64(final_blend3);
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
