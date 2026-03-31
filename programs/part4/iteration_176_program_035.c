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

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi_alt(__m512i a, __m512i b, __mmask32 k1, __mmask32 k2) {
    // Logical mask operation before blend
    __mmask32 combined = _kor_mask32(k1, k2);
    return _mm512_mask_blend_epi16(combined, a, b);
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
__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Control flow that prevents constant folding
__attribute__((noinline))
__mmask64 generate_dynamic_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    switch (iteration % 4) {
        case 0:
            // Immediate mask
            mask = 0xAAAAAAAAAAAAAAAAULL;
            break;
        case 1:
            // Alternating pattern
            mask = 0x5555555555555555ULL;
            break;
        case 2:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0));
            break;
        case 3:
            // All ones
            mask = 0xFFFFFFFFFFFFFFFFULL;
            break;
    }
    return mask;
}

__attribute__((noinline))
__mmask32 generate_dynamic_mask32(int iteration, __m512i data) {
    __mmask32 mask;
    if (iteration % 2 == 0) {
        // Immediate mask
        mask = 0xAAAAAAAA;
    } else {
        // Comparison mask
        mask = _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(0));
    }
    return mask;
}

__attribute__((noinline))
__mmask16 generate_dynamic_mask16_float(int iteration, __m512 data) {
    __mmask16 mask;
    if (iteration < 10) {
        // Comparison mask with threshold
        mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.5f), _CMP_GT_OQ);
    } else {
        // Immediate mask
        mask = 0xAAAA;
    }
    return mask;
}

__attribute__((noinline))
__mmask8 generate_dynamic_mask8_double(int iteration, __m512d data) {
    __mmask8 mask;
    if (iteration % 3 == 0) {
        mask = 0xAA;
    } else if (iteration % 3 == 1) {
        mask = _mm512_cmp_pd_mask(data, _mm512_set1_pd(0.0), _CMP_GE_OQ);
    } else {
        mask = 0x55;
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
        0.0f,0.1f,0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,
        0.8f,0.9f,1.0f,1.1f,1.2f,1.3f,1.4f,1.5f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        1.5f,1.4f,1.3f,1.2f,1.1f,1.0f,0.9f,0.8f,
        0.7f,0.6f,0.5f,0.4f,0.3f,0.2f,0.1f,0.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7);
    __m512d v8df_b = _mm512_set_pd(0.7,0.6,0.5,0.4,0.3,0.2,0.1,0.0);
    
    // Data dependency chain: Start with integer blends, use results for float blends
    for (int i = 0; i < 4; i++) {
        // V64QImode blend with dynamic mask
        __mmask64 mask64 = generate_dynamic_mask64(i, v64qi_a);
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // V32HImode blend with logical mask operation
        __mmask32 mask32_1 = generate_dynamic_mask32(i, v32hi_a);
        __mmask32 mask32_2 = 0xCCCCCCCC; // Immediate mask
        __m512i result32hi = blend_v32hi_alt(v32hi_a, v32hi_b, mask32_1, mask32_2);
        
        // Use integer result to generate mask for float blend (data dependency)
        __mmask16 mask16 = _mm512_cmpeq_epi32_mask(
            _mm512_and_si512(result64qi, _mm512_set1_epi32(1)),
            _mm512_set1_epi32(0)
        );
        
        // V16SFmode blend
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
        
        // V8DFmode blend with dynamic mask
        __mmask8 mask8 = generate_dynamic_mask8_double(i, v8df_a);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
        
        // Accumulate checksums to prevent optimization
        uint8_t* r64 = (uint8_t*)&result64qi;
        for (int j = 0; j < 64; j++) checksum += r64[j];
        
        uint16_t* r32 = (uint16_t*)&result32hi;
        for (int j = 0; j < 32; j++) checksum += r32[j];
        
        float* r16 = (float*)&result16sf;
        for (int j = 0; j < 16; j++) checksum += (uint64_t)(r16[j] * 100);
        
        double* r8 = (double*)&result8df;
        for (int j = 0; j < 8; j++) checksum += (uint64_t)(r8[j] * 100);
    }
    
    // Additional blends in different control flow paths
    for (int i = 0; i < 8; i++) {
        __mmask16 mask16si;
        if (i < 4) {
            mask16si = 0xAAAA; // Immediate
        } else {
            mask16si = _mm512_cmpeq_epi32_mask(
                _mm512_add_epi32(v16si_a, _mm512_set1_epi32(i)),
                _mm512_set1_epi32(10)
            );
        }
        
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16si);
        
        __mmask8 mask8di;
        switch (i % 3) {
            case 0: mask8di = 0xAA; break;
            case 1: mask8di = 0x55; break;
            case 2: mask8di = _mm512_cmpeq_epi64_mask(v8di_a, v8di_b); break;
        }
        
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8di);
        
        // Accumulate checksums
        uint32_t* r16si = (uint32_t*)&result16si;
        for (int j = 0; j < 16; j++) checksum += r16si[j];
        
        uint64_t* r8di = (uint64_t*)&result8di;
        for (int j = 0; j < 8; j++) checksum += r8di[j];
    }
    
#ifdef __AVX512FP16__
    // Half-precision float blends (requires -mavx512fp16)
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_set1_bh(1.0f);
    __m512bh v32bf_b = _mm512_set1_bh(2.0f);
    
    for (int i = 0; i < 4; i++) {
        __mmask32 mask32hf = (i % 2 == 0) ? 0xAAAAAAAA : 0x55555555;
        
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32hf);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32hf);
        
        // Accumulate checksums
        uint16_t* rhf = (uint16_t*)&result32hf;
        for (int j = 0; j < 32; j++) checksum += rhf[j];
        
        uint16_t* rbf = (uint16_t*)&result32bf;
        for (int j = 0; j < 32; j++) checksum += rbf[j];
    }
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
