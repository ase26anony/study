#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    __m512i res1 = _mm512_mask_blend_epi8(k8, a, b);
    __m512i res2 = _mm512_mask_blend_epi16(k16, res1, b);
    return _mm512_add_epi8(res1, res2);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d,
                                      __mmask16 k32, __mmask8 k64) {
    __m512 res1 = _mm512_mask_blend_ps(k32, a, b);
    __m512d res2 = _mm512_mask_blend_pd(k64, c, d);
    // Convert double result to float for mixing
    __m512 res2f = _mm512_cvtpd_ps(res2);
    return _mm512_add_ps(res1, res2f);
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 k16) {
    // Use same mask for both HF and BF modes (they share the same intrinsic)
    __m512h res1 = _mm512_mask_blend_ph(k16, a, b);
    __m512h res2 = _mm512_mask_blend_ph(~k16, b, a);  // Inverted mask
    return _mm512_add_ph(res1, res2);
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d,
                                       __mmask16 k32, __mmask8 k64) {
    __m512i res1 = _mm512_mask_blend_epi32(k32, a, b);
    __m512i res2 = _mm512_mask_blend_epi64(k64, c, d);
    return _mm512_add_epi32(res1, _mm512_castsi512_si512(res2));
}

// Function with data-dependent control flow
__m512i conditional_blend(int mode, __m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    __m512i result;
    if (mode & 1) {
        // Use epi8 blend
        result = _mm512_mask_blend_epi8(k8, a, b);
    } else {
        // Use epi16 blend
        result = _mm512_mask_blend_epi16(k16, a, b);
    }
    
    // Additional conditional based on mask values
    if (_cvtmask64_u64(k8) > 0xFFFFFFFF) {
        result = _mm512_mask_blend_epi8(k8 >> 32, result, _mm512_set1_epi8(1));
    }
    
    return result;
}

int main() {
    // Initialize vectors with distinct patterns
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
        15.0f,14.5f,14.0f,13.5f,13.0f,12.5f,12.0f,11.5f,
        11.0f,10.5f,10.0f,9.5f,9.0f,8.5f,8.0f,7.5f
    );
    __m512 v16sf_b = _mm512_set1_ps(500.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.5,6.0,5.5,5.0,4.5,4.0,3.5);
    __m512d v8df_b = _mm512_set1_pd(600.0);
    
    // For FP16 modes (requires AVX512-FP16)
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        31.0h,30.5h,30.0h,29.5h,29.0h,28.5h,28.0h,27.5h,
        27.0h,26.5h,26.0h,25.5h,25.0h,24.5h,24.0h,23.5h,
        23.0h,22.5h,22.0h,21.5h,21.0h,20.5h,20.0h,19.5h,
        19.0h,18.5h,18.0h,17.5h,17.0h,16.5h,16.0h,15.5h
    );
    __m512h v32hf_b = _mm512_set1_ph(700.0h);
    #endif
    
    // Generate masks using different methods
    // 1. Immediate constants
    __mmask64 k8_const = 0xAAAAAAAAAAAAAAAA;
    __mmask32 k16_const = 0x55555555;
    __mmask16 k32_const = 0xAAAA;
    __mmask8 k64_const = 0x55;
    
    // 2. Dynamic masks from comparisons
    __mmask16 k32_cmp = _mm512_cmp_epi32_mask(v16si_a, _mm512_set1_epi32(7), _MM_CMPINT_GT);
    __mmask8 k64_cmp = _mm512_cmp_epi64_mask(v8di_a, _mm512_set1_epi64(3), _MM_CMPINT_GT);
    __mmask16 k32f_cmp = _mm512_cmp_ps_mask(v16sf_a, _mm512_set1_ps(10.0f), _CMP_GT_OQ);
    __mmask8 k64f_cmp = _mm512_cmp_pd_mask(v8df_a, _mm512_set1_pd(5.0), _CMP_GT_OQ);
    
    // 3. Masks from bitwise operations
    __mmask64 k8_combined = _kor_mask64(k8_const, _knot_mask64(k8_const >> 1));
    __mmask32 k16_combined = k16_const ^ 0x33333333;
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 4; i++) {
        // V64QImode blend
        __m512i res64qi = _mm512_mask_blend_epi8(
            (i & 1) ? k8_const : k8_combined,
            v64qi_a,
            v64qi_b
        );
        
        // V32HImode blend
        __m512i res32hi = _mm512_mask_blend_epi16(
            (i & 2) ? k16_const : k16_combined,
            v32hi_a,
            v32hi_b
        );
        
        // Conditional blend based on loop index
        __m512i cond_res = conditional_blend(i, res64qi, res32hi, k8_const, k16_const);
        
        // Chain blends: epi8 -> epi16 -> epi32
        __m512i chained = _mm512_mask_blend_epi8(k8_const, v64qi_a, v64qi_b);
        chained = _mm512_mask_blend_epi16(k16_const, chained, v32hi_b);
        chained = _mm512_mask_blend_epi32(k32_cmp, chained, v16si_b);
        
        // V16SImode blend
        __m512i res16si = _mm512_mask_blend_epi32(k32_cmp, v16si_a, v16si_b);
        
        // V8DImode blend
        __m512i res8di = _mm512_mask_blend_epi64(k64_cmp, v8di_a, v8di_b);
        
        // V16SFmode blend
        __m512 res16sf = _mm512_mask_blend_ps(k32f_cmp, v16sf_a, v16sf_b);
        
        // V8DFmode blend
        __m512d res8df = _mm512_mask_blend_pd(k64f_cmp, v8df_a, v8df_b);
        
        #ifdef __AVX512FP16__
        // V32HFmode and V32BFmode blend (same intrinsic)
        __m512h res32hf = _mm512_mask_blend_ph(k16_combined, v32hf_a, v32hf_b);
        #endif
        
        // Call helper functions
        __m512i helper1 = blend_v64qi_v32hi(v64qi_a, v64qi_b, k8_const, k16_const);
        __m512 helper2 = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, k32f_cmp, k64f_cmp);
        __m512i helper3 = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, k32_cmp, k64_cmp);
        
        #ifdef __AVX512FP16__
        __m512h helper4 = blend_v32hf_v32bf(v32hf_a, v32hf_b, k16_combined);
        #endif
        
        // Reductions to prevent dead code elimination
        // Integer reductions
        checksum += _mm512_reduce_add_epi64(res64qi);
        checksum += _mm512_reduce_add_epi32(res32hi);
        checksum += _mm512_reduce_add_epi32(res16si);
        checksum += _mm512_reduce_add_epi64(res8di);
        checksum += _mm512_reduce_add_epi32(cond_res);
        checksum += _mm512_reduce_add_epi32(chained);
        checksum += _mm512_reduce_add_epi32(helper1);
        checksum += _mm512_reduce_add_epi32(helper3);
        
        // Float reductions
        checksum += _mm512_reduce_add_ps(res16sf);
        checksum += _mm512_reduce_add_pd(res8df);
        checksum += _mm512_reduce_add_ps(helper2);
        
        #ifdef __AVX512FP16__
        // FP16 reduction (convert to float)
        __m512 res32hf_f32 = _mm512_cvtph_ps(res32hf);
        checksum += _mm512_reduce_add_ps(res32hf_f32);
        
        __m512 helper4_f32 = _mm512_cvtph_ps(helper4);
        checksum += _mm512_reduce_add_ps(helper4_f32);
        #endif
        
        // Modify source vectors for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(1.0f));
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
