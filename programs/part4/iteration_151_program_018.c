#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __AVX512FP16__
#include <float.h>
#endif

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mask_type) {
    __mmask64 mask64;
    __mmask32 mask32;
    
    if (mask_type == 0) {
        // Immediate constant mask for epi8
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
        return _mm512_mask_blend_epi8(mask64, a, b);
    } else {
        // Dynamic mask for epi16 using comparison
        __m512i cmp_val = _mm512_set1_epi16(0);
        mask32 = _mm512_cmp_epi16_mask(a, cmp_val, _MM_CMPINT_GT);
        return _mm512_mask_blend_epi16(mask32, a, b);
    }
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __mmask16 mask16, __mmask8 mask8) {
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    __m512i result2 = _mm512_mask_blend_epi64(mask8, result1, b);
    return _mm512_add_epi32(result1, result2);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d) {
    // Create masks using comparisons
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    __mmask8 mask8 = _mm512_cmp_pd_mask(c, d, _CMP_GT_OQ);
    
    __m512 res_ps = _mm512_mask_blend_ps(mask16, a, b);
    __m512d res_pd = _mm512_mask_blend_pd(mask8, c, d);
    
    // Convert pd to ps and combine
    __m512 pd_as_ps = _mm512_castpd_ps(res_pd);
    return _mm512_add_ps(res_ps, pd_as_ps);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d) {
    // For V32HFmode
    __mmask32 mask32_hf = 0x55555555; // Alternating pattern
    __m512h res_hf = _mm512_mask_blend_ph(mask32_hf, a, b);
    
    // For V32BFmode - use same intrinsic with different data
    __mmask32 mask32_bf = _mm512_cmp_ph_mask(c, d, _CMP_EQ_OQ);
    __m512h res_bf = _mm512_mask_blend_ph(mask32_bf, c, d);
    
    return _mm512_add_ph(res_hf, res_bf);
}
#endif

/* Function with data-dependent control flow */
static void conditional_blend(int selector, __m512i* vi_result, __m512* vf_result,
                              __m512i a_i, __m512i b_i, __m512 a_f, __m512 b_f) {
    if (selector & 1) {
        // Use immediate mask
        __mmask64 mask = 0xCCCCCCCCCCCCCCCCULL;
        *vi_result = _mm512_mask_blend_epi8(mask, a_i, b_i);
    } else {
        // Use comparison-generated mask
        __mmask32 mask = _mm512_cmp_epi16_mask(a_i, b_i, _MM_CMPINT_LT);
        *vi_result = _mm512_mask_blend_epi16(mask, a_i, b_i);
    }
    
    if (selector & 2) {
        __mmask16 mask = _mm512_cmp_ps_mask(a_f, b_f, _CMP_NEQ_OQ);
        *vf_result = _mm512_mask_blend_ps(mask, a_f, b_f);
    }
}

/* Chained blend operations */
static __m512 chain_blends(__m512i vi1, __m512i vi2, __m512 vf1, __m512 vf2,
                          __m512d vd1, __m512d vd2) {
    // Start with epi8 blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i step1 = _mm512_mask_blend_epi8(mask64, vi1, vi2);
    
    // Chain to epi16 blend using step1 as input
    __mmask32 mask32 = _mm512_cmp_epi16_mask(step1, vi2, _MM_CMPINT_EQ);
    __m512i step2 = _mm512_mask_blend_epi16(mask32, step1, vi2);
    
    // Chain to epi32 blend
    __mmask16 mask16 = 0xAAAA; // Alternating pattern
    __m512i step3 = _mm512_mask_blend_epi32(mask16, step2, vi1);
    
    // Chain to epi64 blend
    __mmask8 mask8 = 0xAA;
    __m512i step4 = _mm512_mask_blend_epi64(mask8, step3, vi2);
    
    // Convert to float and chain to ps blend
    __m512 step4_f = _mm512_cvtepi32_ps(_mm512_castsi512_si256(step4));
    __mmask16 mask16_f = _mm512_cmp_ps_mask(step4_f, vf1, _CMP_LT_OQ);
    __m512 step5 = _mm512_mask_blend_ps(mask16_f, step4_f, vf1);
    
    // Final pd blend
    __m512d step4_d = _mm512_cvtepi32_pd(_mm512_castsi512_si256(step4));
    __mmask8 mask8_d = _mm512_cmp_pd_mask(step4_d, vd1, _CMP_GT_OQ);
    __m512d step6 = _mm512_mask_blend_pd(mask8_d, step4_d, vd1);
    
    // Combine results
    __m512 step6_f = _mm512_castpd_ps(step6);
    return _mm512_add_ps(step5, step6_f);
}

int main() {
    // Initialize vectors with patterned data
    __m512i vi1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vi2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 vf1 = _mm512_set_ps(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    );
    
    __m512 vf2 = _mm512_set_ps(
        16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f
    );
    
    __m512d vd1 = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d vd2 = _mm512_set_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    
    // Variables to store results
    __m512i vi_result1, vi_result2;
    __m512 vf_result1, vf_result2;
    __m512 chain_result;
    
    float checksum = 0.0f;
    
    // Test all blend modes in a loop with varying conditions
    for (int i = 0; i < 4; i++) {
        // 1. Direct intrinsic calls for all modes
        __mmask64 mask64 = (i & 1) ? 0xAAAAAAAAAAAAAAAAULL : 0x5555555555555555ULL;
        __m512i blend_epi8 = _mm512_mask_blend_epi8(mask64, vi1, vi2);
        
        __mmask32 mask32 = _mm512_cmp_epi16_mask(vi1, vi2, _MM_CMPINT_LT);
        __m512i blend_epi16 = _mm512_mask_blend_epi16(mask32, vi1, vi2);
        
        __mmask16 mask16 = 0xAAAA;
        __m512i blend_epi32 = _mm512_mask_blend_epi32(mask16, vi1, vi2);
        
        __mmask8 mask8 = 0xAA;
        __m512i blend_epi64 = _mm512_mask_blend_epi64(mask8, vi1, vi2);
        
        __mmask16 mask16_f = _mm512_cmp_ps_mask(vf1, vf2, _CMP_LT_OQ);
        __m512 blend_ps = _mm512_mask_blend_ps(mask16_f, vf1, vf2);
        
        __mmask8 mask8_d = _mm512_cmp_pd_mask(vd1, vd2, _CMP_GT_OQ);
        __m512d blend_pd = _mm512_mask_blend_pd(mask8_d, vd1, vd2);
        
        // 2. Use helper functions
        vi_result1 = blend_v64qi_v32hi(vi1, vi2, i % 2);
        
        __mmask16 mask16_2 = _mm512_cmp_epi32_mask(vi1, vi2, _MM_CMPINT_GT);
        __mmask8 mask8_2 = _mm512_cmp_epi64_mask(vi1, vi2, _MM_CMPINT_EQ);
        vi_result2 = blend_v16si_v8di(vi1, vi2, mask16_2, mask8_2);
        
        vf_result1 = blend_v16sf_v8df(vf1, vf2, vd1, vd2);
        
        // 3. Conditional blends
        conditional_blend(i, &vi_result1, &vf_result2, vi1, vi2, vf1, vf2);
        
        // 4. Chained blends
        chain_result = chain_blends(vi1, vi2, vf1, vf2, vd1, vd2);
        
        // Reduce results to prevent dead code elimination
        // Horizontal sum for integer vectors
        __m256i vi_low = _mm512_castsi512_si256(blend_epi8);
        __m256i vi_high = _mm512_extracti64x4_epi64(blend_epi8, 1);
        __m256i vi_sum = _mm256_add_epi8(vi_low, vi_high);
        
        // Accumulate to checksum
        alignas(64) int32_t temp[16];
        _mm512_store_si512(temp, blend_epi32);
        for (int j = 0; j < 16; j++) {
            checksum += temp[j];
        }
        
        alignas(64) float ftemp[16];
        _mm512_store_ps(ftemp, blend_ps);
        for (int j = 0; j < 16; j++) {
            checksum += ftemp[j];
        }
        
        alignas(64) double dtemp[8];
        _mm512_store_pd(dtemp, blend_pd);
        for (int j = 0; j < 8; j++) {
            checksum += (float)dtemp[j];
        }
    }
    
#ifdef __AVX512FP16__
    // Test FP16 blends if supported
    __m512h vh1 = _mm512_set1_ph(1.0f);
    __m512h vh2 = _mm512_set1_ph(2.0f);
    __m512h vh3 = _mm512_set1_ph(3.0f);
    __m512h vh4 = _mm512_set1_ph(4.0f);
    
    __m512h blend_hf = blend_v32hf_v32bf(vh1, vh2, vh3, vh4);
    
    // Reduce FP16 result
    alignas(64) _Float16 htemp[32];
    _mm512_store_ph(htemp, blend_hf);
    for (int i = 0; i < 32; i++) {
        checksum += (float)htemp[i];
    }
#endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
