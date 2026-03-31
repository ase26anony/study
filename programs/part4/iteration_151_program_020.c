#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    __m512i res8 = _mm512_mask_blend_epi8(k8, a, b);
    __m512i res16 = _mm512_mask_blend_epi16(k16, res8, b);
    return res16;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, 
                                      __mmask16 k32, __mmask8 k64) {
    __m512 res_ps = _mm512_mask_blend_ps(k32, a, b);
    __m512d res_pd = _mm512_mask_blend_pd(k64, c, d);
    // Convert double result back to float for chaining
    __m512 pd_as_ps = _mm512_castpd_ps(res_pd);
    return _mm512_add_ps(res_ps, pd_as_ps);
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 k16) {
    // Use same intrinsic for both HF and BF modes
    __m512h res = _mm512_mask_blend_ph(k16, a, b);
    return res;
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d,
                                       __mmask16 k32, __mmask8 k64) {
    __m512i res_si = _mm512_mask_blend_epi32(k32, a, b);
    __m512i res_di = _mm512_mask_blend_epi64(k64, c, d);
    return _mm512_add_epi32(res_si, _mm512_castsi512_si256(res_di));
}

// Function with data-dependent control flow
__m512i conditional_blend(int mode, __m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    __m512i result;
    if (mode % 3 == 0) {
        // V64QImode path
        result = _mm512_mask_blend_epi8(k8, a, b);
    } else if (mode % 3 == 1) {
        // V32HImode path
        result = _mm512_mask_blend_epi16(k16, a, b);
    } else {
        // Mixed path
        __m512i temp = _mm512_mask_blend_epi8(k8, a, b);
        result = _mm512_mask_blend_epi16(k16, temp, a);
    }
    return result;
}

int main() {
    // Initialize patterned data for all vector types
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
    
    __m512i v16si_a = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i v16si_b = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
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
    
    // Initialize FP16 vectors if supported
    __m512h v32hf_a, v32hf_b;
    #ifdef __AVX512FP16__
    {
        _Float16 hf_data_a[32], hf_data_b[32];
        for (int i = 0; i < 32; i++) {
            hf_data_a[i] = (_Float16)i;
            hf_data_b[i] = (_Float16)(31 - i);
        }
        v32hf_a = _mm512_loadu_ph(hf_data_a);
        v32hf_b = _mm512_loadu_ph(hf_data_b);
    }
    #endif
    
    // Generate masks using different patterns
    __mmask64 k8_imm = 0xAAAAAAAAAAAAAAAA;  // Immediate constant
    __mmask32 k16_imm = 0x55555555;         // Immediate constant
    
    // Dynamic masks from comparisons
    __mmask64 k8_cmp = _mm512_cmp_epi8_mask(v64qi_a, v64qi_b, _MM_CMPINT_GT);
    __mmask32 k16_cmp = _mm512_cmp_epi16_mask(v32hi_a, v32hi_b, _MM_CMPINT_EQ);
    __mmask16 k32_cmp = _mm512_cmp_epi32_mask(v16si_a, v16si_b, _MM_CMPINT_LT);
    __mmask8 k64_cmp = _mm512_cmp_epi64_mask(v8di_a, v8di_b, _MM_CMPINT_NE);
    
    __mmask16 k32_float = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_GT_OQ);
    __mmask8 k64_double = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
    
    // Masks from bitwise operations
    __mmask64 k8_bitwise = _kor_mask64(k8_imm, k8_cmp);
    __mmask32 k16_bitwise = _kxor_mask32(k16_imm, k16_cmp);
    __mmask16 k32_bitwise = _knot_mask16(k32_cmp);
    __mmask8 k64_bitwise = _kor_mask8(k64_imm, k64_cmp);
    
    // Accumulator for checksum
    float checksum = 0.0f;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        // V64QImode blend with immediate mask
        __m512i res64qi = _mm512_mask_blend_epi8(k8_imm, v64qi_a, v64qi_b);
        
        // V32HImode blend with comparison mask
        __m512i res32hi = _mm512_mask_blend_epi16(k16_cmp, v32hi_a, v32hi_b);
        
        // V16SImode blend with bitwise mask
        __m512i res16si = _mm512_mask_blend_epi32(k32_bitwise, v16si_a, v16si_b);
        
        // V8DImode blend with immediate mask
        __m512i res8di = _mm512_mask_blend_epi64(k64_imm, v8di_a, v8di_b);
        
        // V16SFmode blend with comparison mask
        __m512 res16sf = _mm512_mask_blend_ps(k32_float, v16sf_a, v16sf_b);
        
        // V8DFmode blend with bitwise mask
        __m512d res8df = _mm512_mask_blend_pd(k64_bitwise, v8df_a, v8df_b);
        
        // Chained blend operations
        __m512i chained1 = blend_v64qi_v32hi(v64qi_a, v64qi_b, k8_bitwise, k16_bitwise);
        __m512 chained2 = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, 
                                          k32_cmp, k64_cmp);
        
        // Conditional blend based on loop index
        __m512i cond_res = conditional_blend(i, v64qi_a, v64qi_b, k8_cmp, k16_cmp);
        
        // Prevent dead code elimination with reductions
        __m512i sum_i = _mm512_add_epi64(_mm512_add_epi64(res64qi, res32hi),
                                        _mm512_add_epi64(res16si, res8di));
        sum_i = _mm512_add_epi64(sum_i, _mm512_add_epi64(chained1, cond_res));
        
        __m512 sum_f = _mm512_add_ps(res16sf, _mm512_castpd_ps(res8df));
        sum_f = _mm512_add_ps(sum_f, chained2);
        
        // Horizontal reductions
        int64_t i_sum[8];
        _mm512_storeu_epi64(i_sum, sum_i);
        for (int j = 0; j < 8; j++) {
            checksum += (float)i_sum[j];
        }
        
        float f_sum[16];
        _mm512_storeu_ps(f_sum, sum_f);
        for (int j = 0; j < 16; j++) {
            checksum += f_sum[j];
        }
        
        // FP16 blends if supported
        #ifdef __AVX512FP16__
        __mmask32 k16_half = 0x55555555;
        __m512h res32hf = _mm512_mask_blend_ph(k16_half, v32hf_a, v32hf_b);
        
        // For V32BFmode, use the same intrinsic with different data
        __m512h res32bf = blend_v32hf_v32bf(v32hf_b, v32hf_a, k16_half);
        
        // Reduce FP16 results
        _Float16 hf_sum[32];
        _mm512_storeu_ph(hf_sum, res32hf);
        _mm512_storeu_ph(hf_sum + 16, res32bf);
        for (int j = 0; j < 32; j++) {
            checksum += (float)hf_sum[j];
        }
        #endif
        
        // Modify inputs slightly for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(1.0f));
    }
    
    // Final checksum output
    printf("Final checksum: %f\n", checksum);
    
    return 0;
}
