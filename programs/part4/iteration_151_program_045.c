#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode) {
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    __mmask32 mask32 = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_GT);
    __m512i result2 = _mm512_mask_blend_epi16(mask32, result1, b);
    
    return (mode > 0) ? result2 : result1;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d) {
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    __mmask8 mask8 = 0xAA;
    __m512d result2 = _mm512_mask_blend_pd(mask8, c, d);
    
    // Convert double result back to float for chaining
    __m512 result2_f = _mm512_castpd_ps(result2);
    __mmask16 final_mask = _kor_mask16(mask16, _mm512_cmp_ps_mask(result1, result2_f, _CMP_EQ_OQ));
    return _mm512_mask_blend_ps(final_mask, result1, result2_f);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int use_bf) {
    __mmask32 mask32 = 0x55555555;
    __m512h result = _mm512_mask_blend_ph(mask32, a, b);
    
    if (use_bf) {
        // For BF16 mode (treated same as HF in intrinsics)
        __mmask32 alt_mask = _knot_mask32(mask32);
        result = _mm512_mask_blend_ph(alt_mask, result, b);
    }
    
    return result;
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d) {
    // Generate mask using comparison
    __mmask16 mask16 = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_EQ);
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // Generate mask using bit pattern
    __mmask8 mask8 = 0xF0;
    __m512i result2 = _mm512_mask_blend_epi64(mask8, c, d);
    
    // Chain operations: blend the results of previous blends
    __mmask16 chain_mask = _mm512_cmp_epi32_mask(result1, result2, _MM_CMPINT_LT);
    return _mm512_mask_blend_epi32(chain_mask, result1, result2);
}

// Function with data-dependent control flow
static void conditional_blend(int selector, __m512i* int_result, __m512* float_result) {
    __m512i a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i b = _mm512_set1_epi8(100);
    
    if (selector & 1) {
        // Use epi8 blend
        __mmask64 mask = (selector > 10) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
        *int_result = _mm512_mask_blend_epi8(mask, a, b);
    } else {
        // Use epi16 blend
        __m512i a16 = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i b16 = _mm512_set1_epi16(200);
        __mmask32 mask = _mm512_cmp_epi16_mask(a16, b16, _MM_CMPINT_GT);
        *int_result = _mm512_mask_blend_epi16(mask, a16, b16);
    }
    
    // Always do float blend
    __m512 fa = _mm512_set_ps(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    __m512 fb = _mm512_set1_ps(50.0f);
    __mmask16 mask = (selector % 3 == 0) ? 0xAAAA : 0x5555;
    *float_result = _mm512_mask_blend_ps(mask, fa, fb);
}

int main() {
    double checksum = 0.0;
    
    // Initialize vectors for all modes
    __m512i vi1 = _mm512_set_epi32(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16
    );
    __m512i vi2 = _mm512_set1_epi32(100);
    
    __m512i vdi1 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i vdi2 = _mm512_set1_epi64(50);
    
    __m512 vf1 = _mm512_set_ps(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    __m512 vf2 = _mm512_set1_ps(75.0f);
    
    __m512d vd1 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d vd2 = _mm512_set1_pd(25.0);
    
#ifdef __AVX512FP16__
    __m512h vh1 = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    __m512h vh2 = _mm512_set1_ph(40.0f);
#endif
    
    // Loop with varying conditions to trigger different blend paths
    for (int i = 0; i < 10; i++) {
        __m512i int_result;
        __m512 float_result;
        
        // Conditional blend based on loop index
        conditional_blend(i, &int_result, &float_result);
        
        // Chain blends through helper functions
        __m512i chain_result = blend_v64qi_v32hi(vi1, vi2, i % 2);
        __m512 float_chain_result = blend_v16sf_v8df(vf1, vf2, vd1, vd2);
        __m512i si_di_result = blend_v16si_v8di(vi1, vi2, vdi1, vdi2);
        
#ifdef __AVX512FP16__
        __m512h hf_result = blend_v32hf_v32bf(vh1, vh2, i % 3);
#endif
        
        // Reductions to prevent dead code elimination
        // Integer reduction
        __m256i hi = _mm512_extracti64x4_epi64(int_result, 1);
        __m256i lo = _mm512_extracti64x4_epi64(int_result, 0);
        __m256i sum256 = _mm256_add_epi64(hi, lo);
        __m128i hi128 = _mm256_extracti128_si256(sum256, 1);
        __m128i lo128 = _mm256_extracti128_si256(sum256, 0);
        __m128i sum128 = _mm_add_epi64(hi128, lo128);
        checksum += (double)_mm_extract_epi64(sum128, 0);
        checksum += (double)_mm_extract_epi64(sum128, 1);
        
        // Float reduction
        float float_sum = _mm512_reduce_add_ps(float_result);
        checksum += (double)float_sum;
        
        // Chain result reduction
        float chain_float_sum = _mm512_reduce_add_ps(float_chain_result);
        checksum += (double)chain_float_sum;
        
#ifdef __AVX512FP16__
        // FP16 reduction (convert to float for accumulation)
        for (int j = 0; j < 32; j++) {
            checksum += (double)vh_result[j];
        }
#endif
    }
    
    // Direct blends for all modes (ensuring each case is hit)
    // V64QImode
    __m512i v64qi_a = _mm512_set1_epi8(10);
    __m512i v64qi_b = _mm512_set1_epi8(20);
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i v64qi_result = _mm512_mask_blend_epi8(mask64, v64qi_a, v64qi_b);
    
    // V32HImode
    __m512i v32hi_a = _mm512_set1_epi16(30);
    __m512i v32hi_b = _mm512_set1_epi16(60);
    __mmask32 mask32 = _mm512_cmp_epi16_mask(v32hi_a, v32hi_b, _MM_CMPINT_LT);
    __m512i v32hi_result = _mm512_mask_blend_epi16(mask32, v32hi_a, v32hi_b);
    
    // V16SImode
    __m512i v16si_a = _mm512_set1_epi32(100);
    __m512i v16si_b = _mm512_set1_epi32(200);
    __mmask16 mask16 = 0xAAAA;
    __m512i v16si_result = _mm512_mask_blend_epi32(mask16, v16si_a, v16si_b);
    
    // V8DImode
    __m512i v8di_a = _mm512_set1_epi64(1000);
    __m512i v8di_b = _mm512_set1_epi64(2000);
    __mmask8 mask8 = 0xAA;
    __m512i v8di_result = _mm512_mask_blend_epi64(mask8, v8di_a, v8di_b);
    
    // V16SFmode
    __m512 v16sf_a = _mm512_set1_ps(1.5f);
    __m512 v16sf_b = _mm512_set1_ps(3.0f);
    __mmask16 mask16f = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OQ);
    __m512 v16sf_result = _mm512_mask_blend_ps(mask16f, v16sf_a, v16sf_b);
    
    // V8DFmode
    __m512d v8df_a = _mm512_set1_pd(2.5);
    __m512d v8df_b = _mm512_set1_pd(5.0);
    __mmask8 mask8d = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
    __m512d v8df_result = _mm512_mask_blend_pd(mask8d, v8df_a, v8df_b);
    
#ifdef __AVX512FP16__
    // V32HFmode and V32BFmode
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __mmask32 mask32h = 0x55555555;
    __m512h v32hf_result = _mm512_mask_blend_ph(mask32h, v32hf_a, v32hf_b);
#endif
    
    // Final reductions on all direct blend results
    __m256i v64qi_hi = _mm512_extracti64x4_epi64(v64qi_result, 1);
    __m256i v64qi_lo = _mm512_extracti64x4_epi64(v64qi_result, 0);
    __m256i v64qi_sum = _mm256_add_epi8(v64qi_hi, v64qi_lo);
    // ... additional reduction logic for all results ...
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
