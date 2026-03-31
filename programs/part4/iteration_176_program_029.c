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
    __mmask64 mask = 0;
    
    // Switch statement for control flow diversity
    switch (iteration % 4) {
        case 0:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0));
            break;
        case 1:
            // Immediate mask
            mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
            break;
        case 2:
            // Logical combination of masks
            __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(10));
            __mmask64 m2 = (__mmask64)0x5555555555555555ULL;
            mask = _kor_mask64(m1, m2);
            break;
        case 3:
            // Pattern based on iteration
            mask = (__mmask64)((iteration & 1) ? 0xFFFFFFFFFFFFFFFFULL : 0x0);
            break;
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
    
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
#endif
    
    // Loop with control flow to prevent constant folding
    for (int i = 0; i < 8; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_complex_mask64(i, v64qi_a);
        __mmask32 mask32 = (__mmask32)(0xAAAAAAAA ^ (i * 0x11111111));
        __mmask16 mask16 = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OQ);
        __mmask8 mask8 = (__mmask8)(0xAA ^ i);
        
        // Create data dependency chain: use result from one blend to generate mask for next
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use the byte blend result to generate a mask for 16-bit blend
        __mmask32 mask32_from_chain = _mm512_cmpeq_epi16_mask(
            result64qi, 
            _mm512_set1_epi16(0)
        );
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32_from_chain);
        
        // Use 16-bit result to generate mask for 32-bit blend
        __mmask16 mask16_from_chain = _mm512_cmpeq_epi32_mask(
            result32hi,
            _mm512_set1_epi32(0)
        );
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16_from_chain);
        
        // Blend with immediate mask
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
        
        // Generate mask from comparison for float blends
        __mmask16 mask16_float = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_GT_OQ);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_float);
        
        __mmask8 mask8_double = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_EQ_OQ);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8_double);
        
#ifdef __AVX512FP16__
        // Half-precision blends
        __mmask32 mask32_half = (__mmask32)(0x55555555 ^ (i * 0x33333333));
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32_half);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32_half);
#endif
        
        // Accumulate checksums to prevent optimization
        uint8_t* bytes64 = (uint8_t*)&result64qi;
        for (int j = 0; j < 64; j++) checksum += bytes64[j];
        
        uint16_t* words32 = (uint16_t*)&result32hi;
        for (int j = 0; j < 32; j++) checksum += words32[j];
        
        uint32_t* dwords16 = (uint32_t*)&result16si;
        for (int j = 0; j < 16; j++) checksum += dwords16[j];
        
        uint64_t* qwords8 = (uint64_t*)&result8di;
        for (int j = 0; j < 8; j++) checksum += qwords8[j];
        
        float* floats16 = (float*)&result16sf;
        for (int j = 0; j < 16; j++) checksum += (uint64_t)floats16[j];
        
        double* doubles8 = (double*)&result8df;
        for (int j = 0; j < 8; j++) checksum += (uint64_t)doubles8[j];
        
#ifdef __AVX512FP16__
        _Float16* halves32 = (_Float16*)&result32hf;
        for (int j = 0; j < 32; j++) checksum += (uint64_t)halves32[j];
        
        __mmask32* bf_mask = (__mmask32*)&result32bf;
        checksum += *bf_mask;
#endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
