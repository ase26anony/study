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
    __m512 resf = _mm512_mask_blend_ps(k32, a, b);
    __m512d resd = _mm512_mask_blend_pd(k64, c, d);
    // Convert double result back to float for chaining
    __m512 conv = _mm512_cvtpd_ps(resd);
    return _mm512_add_ps(resf, conv);
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 k16) {
    // Use same intrinsic for both HF and BF modes (compiler handles conversion)
    return _mm512_mask_blend_ph(k16, a, b);
}

// Function with data-dependent control flow
__m512i conditional_blend(int mode, __m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    __m512i result;
    if (mode & 1) {
        result = _mm512_mask_blend_epi8(k8, a, b);
    } else {
        result = _mm512_mask_blend_epi16(k16, a, b);
    }
    
    // Chain with another blend based on condition
    if (mode & 2) {
        __mmask16 k32 = _cvtu32_mask16(0xAAAA);
        __m512i temp = _mm512_mask_blend_epi32(k32, result, b);
        return temp;
    }
    return result;
}

int main() {
    // Initialize vectors with distinct patterns
    __m512i vi64 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vi32 = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i vi16 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i vi8 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    
    __m512 vf32 = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512d vf64 = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    
    // For FP16 modes (requires -mavx512fp16)
    #ifdef __AVX512FP16__
    __m512h vhf = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    #endif
    
    // Alternate vectors for blending
    __m512i vi64_alt = _mm512_set1_epi8(0xFF);
    __m512i vi32_alt = _mm512_set1_epi16(0xFFFF);
    __m512i vi16_alt = _mm512_set1_epi32(0xFFFFFFFF);
    __m512i vi8_alt = _mm512_set1_epi64(0xFFFFFFFFFFFFFFFF);
    __m512 vf32_alt = _mm512_set1_ps(100.0f);
    __m512d vf64_alt = _mm512_set1_pd(100.0);
    #ifdef __AVX512FP16__
    __m512h vhf_alt = _mm512_set1_ph(100.0f);
    #endif
    
    // Generate masks using different methods
    __mmask64 k64_imm = 0xAAAAAAAAAAAAAAAA;  // Immediate constant
    __mmask32 k32_imm = 0xAAAAAAAA;
    __mmask16 k16_imm = 0xAAAA;
    __mmask8 k8_imm = 0xAA;
    
    // Dynamic masks from comparisons
    __mmask16 k32_cmp = _mm512_cmp_epi32_mask(vi16, vi16_alt, _MM_CMPINT_LT);
    __mmask8 k64_cmp = _mm512_cmp_epi64_mask(vi8, vi8_alt, _MM_CMPINT_LT);
    __mmask16 k32f_cmp = _mm512_cmp_ps_mask(vf32, vf32_alt, _CMP_LT_OQ);
    __mmask8 k64f_cmp = _mm512_cmp_pd_mask(vf64, vf64_alt, _CMP_LT_OQ);
    
    // Masks from bitwise operations
    __mmask64 k64_bitwise = _kor_mask64(k64_imm, _knot_mask64(k64_imm));
    __mmask32 k32_bitwise = _kxor_mask32(k32_imm, 0x55555555);
    
    // Results storage
    __m512i res_epi8, res_epi16, res_epi32, res_epi64;
    __m512 res_ps, res_chain;
    __m512d res_pd;
    #ifdef __AVX512FP16__
    __m512h res_hf, res_hf2;
    #endif
    
    // 1. V64QImode - epi8 blends
    res_epi8 = _mm512_mask_blend_epi8(k64_imm, vi64, vi64_alt);
    res_epi8 = _mm512_mask_blend_epi8(k64_bitwise, res_epi8, vi64);
    
    // 2. V32HImode - epi16 blends
    res_epi16 = _mm512_mask_blend_epi16(k32_imm, vi32, vi32_alt);
    res_epi16 = _mm512_mask_blend_epi16(k32_bitwise, res_epi16, vi32);
    
    // 3. V16SImode - epi32 blends
    res_epi32 = _mm512_mask_blend_epi32(k16_imm, vi16, vi16_alt);
    res_epi32 = _mm512_mask_blend_epi32(k32_cmp, res_epi32, vi16);
    
    // 4. V8DImode - epi64 blends
    res_epi64 = _mm512_mask_blend_epi64(k8_imm, vi8, vi8_alt);
    res_epi64 = _mm512_mask_blend_epi64(k64_cmp, res_epi64, vi8);
    
    // 5. V16SFmode - ps blends
    res_ps = _mm512_mask_blend_ps(k16_imm, vf32, vf32_alt);
    res_ps = _mm512_mask_blend_ps(k32f_cmp, res_ps, vf32);
    
    // 6. V8DFmode - pd blends
    res_pd = _mm512_mask_blend_pd(k8_imm, vf64, vf64_alt);
    res_pd = _mm512_mask_blend_pd(k64f_cmp, res_pd, vf64);
    
    // 7. V32HFmode and V32BFmode - ph blends (if available)
    #ifdef __AVX512FP16__
    res_hf = _mm512_mask_blend_ph(k32_imm, vhf, vhf_alt);
    res_hf2 = blend_v32hf_v32bf(res_hf, vhf, k32_bitwise);
    #endif
    
    // Chained operations
    res_chain = blend_v16sf_v8df(res_ps, vf32_alt, res_pd, vf64_alt, k16_imm, k8_imm);
    
    // Conditional blends in loop
    __m512i cond_result;
    for (int i = 0; i < 10; i++) {
        cond_result = conditional_blend(i, vi64, vi64_alt, k64_imm, k32_imm);
    }
    
    // Helper function with chaining
    __m512i chain_result = blend_v64qi_v32hi(vi64, vi64_alt, k64_imm, k32_imm);
    
    // Prevent dead code elimination by computing checksums
    uint64_t checksum = 0;
    
    // Reduce each result to scalar
    __m256i low, high;
    low = _mm512_castsi512_si256(res_epi8);
    high = _mm512_extracti64x4_epi64(res_epi8, 1);
    checksum += _mm256_extract_epi64(low, 0) + _mm256_extract_epi64(high, 0);
    
    low = _mm512_castsi512_si256(res_epi16);
    high = _mm512_extracti64x4_epi64(res_epi16, 1);
    checksum += _mm256_extract_epi64(low, 0) + _mm256_extract_epi64(high, 0);
    
    low = _mm512_castsi512_si256(res_epi32);
    high = _mm512_extracti64x4_epi64(res_epi32, 1);
    checksum += _mm256_extract_epi64(low, 0) + _mm256_extract_epi64(high, 0);
    
    low = _mm512_castsi512_si256(res_epi64);
    high = _mm512_extracti64x4_epi64(res_epi64, 1);
    checksum += _mm256_extract_epi64(low, 0) + _mm256_extract_epi64(high, 0);
    
    low = _mm512_castsi512_si256(cond_result);
    high = _mm512_extracti64x4_epi64(cond_result, 1);
    checksum += _mm256_extract_epi64(low, 0) + _mm256_extract_epi64(high, 0);
    
    low = _mm512_castsi512_si256(chain_result);
    high = _mm512_extracti64x4_epi64(chain_result, 1);
    checksum += _mm256_extract_epi64(low, 0) + _mm256_extract_epi64(high, 0);
    
    // Float reductions
    __m256 lowf, highf;
    lowf = _mm512_castps512_ps256(res_ps);
    highf = _mm512_extractf32x8_ps(res_ps, 1);
    checksum += (uint64_t)_mm256_cvtps_f32(lowf) + (uint64_t)_mm256_cvtps_f32(highf);
    
    __m256d lowd, highd;
    lowd = _mm512_castpd512_pd256(res_pd);
    highd = _mm512_extractf64x4_pd(res_pd, 1);
    checksum += (uint64_t)_mm256_cvtpd_f64(lowd) + (uint64_t)_mm256_cvtpd_f64(highd);
    
    lowf = _mm512_castps512_ps256(res_chain);
    highf = _mm512_extractf32x8_ps(res_chain, 1);
    checksum += (uint64_t)_mm256_cvtps_f32(lowf) + (uint64_t)_mm256_cvtps_f32(highf);
    
    #ifdef __AVX512FP16__
    // FP16 reduction (convert to float first)
    __m256h lowh = _mm512_castph512_ph256(res_hf);
    __m256h highh = _mm512_extractf64x4_ph(res_hf, 1);
    // Simple extraction for checksum
    checksum += (uint64_t)_mm256_cvtph_ps(lowh)[0] + (uint64_t)_mm256_cvtph_ps(highh)[0];
    #endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
