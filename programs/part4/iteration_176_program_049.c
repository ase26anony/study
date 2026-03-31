#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Function declarations with noinline to ensure separate expansion
__attribute__((noinline)) __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline)) __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline)) __m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline)) __m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline)) __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline)) __m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to create complex control flow
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    
    // Control flow that prevents constant folding
    if (iteration % 3 == 0) {
        // Comparison mask
        __m512i cmp_val = _mm512_set1_epi8(iteration);
        mask = _mm512_cmpeq_epi8_mask(data, cmp_val);
    } else if (iteration % 3 == 1) {
        // Immediate mask with pattern
        mask = (__mmask64)(0xAAAAAAAAAAAAAAAAULL ^ (iteration * 0x1111111111111111ULL));
    } else {
        // Logical combination of masks
        __m512i cmp1 = _mm512_set1_epi8(iteration);
        __m512i cmp2 = _mm512_set1_epi8(iteration + 1);
        __mmask64 m1 = _mm512_cmpeq_epi8_mask(data, cmp1);
        __mmask64 m2 = _mm512_cmpeq_epi8_mask(data, cmp2);
        mask = _kor_mask64(m1, m2);
    }
    
    return mask;
}

__mmask32 generate_complex_mask32(int iteration, __m512i data) {
    __mmask32 mask;
    
    switch (iteration % 4) {
        case 0:
            mask = _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(iteration));
            break;
        case 1:
            mask = (__mmask32)(0xAAAAAAAA ^ (iteration * 0x11111111));
            break;
        case 2:
            mask = _mm512_cmpgt_epi16_mask(data, _mm512_set1_epi16(iteration));
            break;
        default:
            mask = _mm512_cmpeq_epi16_mask(data, _mm512_setzero_si512());
            mask = _kxor_mask32(mask, (__mmask32)0xFFFFFFFF);
            break;
    }
    
    return mask;
}

__mmask16 generate_complex_mask16(int iteration, __m512 data) {
    __mmask16 mask;
    
    if (iteration < 10) {
        mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(iteration * 0.1f), _CMP_LT_OQ);
    } else if (iteration < 20) {
        mask = (__mmask16)(0xAAAA ^ (iteration * 0x1111));
    } else {
        __mmask16 m1 = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.5f), _CMP_GT_OQ);
        __mmask16 m2 = _mm512_cmp_ps_mask(data, _mm512_set1_ps(1.5f), _CMP_LT_OQ);
        mask = _kand_mask16(m1, m2);
    }
    
    return mask;
}

__mmask8 generate_complex_mask8(int iteration, __m512d data) {
    __mmask8 mask;
    
    // Complex control flow preventing optimization
    for (int i = 0; i < iteration % 5; i++) {
        if (i % 2 == 0) {
            mask = _mm512_cmp_pd_mask(data, _mm512_set1_pd(i * 0.5), _CMP_EQ_OQ);
        } else {
            mask = (__mmask8)(0xAA ^ (iteration * 0x11));
        }
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
    
    // Data dependency chain across different modes
    __m512i intermediate_result;
    
    // Loop with control flow to prevent optimization
    for (int i = 0; i < 8; i++) {
        // V64QImode blend
        __mmask64 mask64 = generate_complex_mask64(i, v64qi_a);
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use result to generate mask for next operation (data dependency)
        __mmask32 mask32 = _mm512_cmpeq_epi16_mask(result64qi, v32hi_a);
        
        // V32HImode blend
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        
        // Generate mask from integer result for float blend
        __m512 float_compare = _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(_mm512_castsi512_si256(result32hi)));
        __mmask16 mask16 = generate_complex_mask16(i, float_compare);
        
        // V16SFmode blend
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
        
        // Generate mask from float result for double blend
        __m512d double_compare = _mm512_cvtps_pd(_mm512_castps512_ps256(result16sf));
        __mmask8 mask8 = generate_complex_mask8(i, double_compare);
        
        // V8DFmode blend
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
        
        // V16SImode blend (using mask from previous operations)
        __mmask16 mask16si = (__mmask16)(mask16 ^ mask8);
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16si);
        
        // V8DImode blend
        __mmask8 mask8di = (__mmask8)(mask8 ^ (i & 0xFF));
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8di);
        
        // Accumulate checksum from all results
        uint8_t* r64 = (uint8_t*)&result64qi;
        uint16_t* r32 = (uint16_t*)&result32hi;
        int32_t* r16 = (int32_t*)&result16si;
        int64_t* r8 = (int64_t*)&result8di;
        float* rsf = (float*)&result16sf;
        double* rdf = (double*)&result8df;
        
        for (int j = 0; j < 64; j++) checksum += r64[j];
        for (int j = 0; j < 32; j++) checksum += r32[j];
        for (int j = 0; j < 16; j++) checksum += r16[j];
        for (int j = 0; j < 8; j++) checksum += r8[j];
        for (int j = 0; j < 16; j++) checksum += (uint64_t)(rsf[j] * 1000);
        for (int j = 0; j < 8; j++) checksum += (uint64_t)(rdf[j] * 1000);
        
        // Update inputs for next iteration to create dependency
        v64qi_a = result64qi;
        v32hi_a = result32hi;
        v16sf_a = result16sf;
        v8df_a = result8df;
    }
    
#ifdef __AVX512FP16__
    // Half-precision float blends
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_set1_bh(1.0f);
    __m512bh v32bf_b = _mm512_set1_bh(2.0f);
    
    for (int i = 0; i < 4; i++) {
        __mmask32 mask_hf = (__mmask32)(0xAAAAAAAA ^ (i * 0x11111111));
        
        // V32HFmode blend
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask_hf);
        
        // V32BFmode blend
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask_hf);
        
        // Update checksum
        uint16_t* rhf = (uint16_t*)&result32hf;
        uint16_t* rbf = (uint16_t*)&result32bf;
        for (int j = 0; j < 32; j++) {
            checksum += rhf[j];
            checksum += rbf[j];
        }
        
        // Create dependency for next iteration
        mask_hf = _mm512_cmpeq_ph_mask(result32hf, v32hf_a);
    }
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
