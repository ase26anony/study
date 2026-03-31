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
__mmask64 generate_complex_mask64(int iteration) {
    // Use different mask generation methods based on iteration
    switch (iteration % 4) {
        case 0: {
            // Immediate mask
            return (__mmask64)0xAAAAAAAAAAAAAAAAULL;
        }
        case 1: {
            // Comparison mask
            __m512i ones = _mm512_set1_epi8(1);
            __m512i data = _mm512_set_epi8(
                0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
                16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
                32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
                48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
            );
            return _mm512_cmpeq_epi8_mask(data, ones);
        }
        case 2: {
            // Logical operation on masks
            __mmask64 m1 = (__mmask64)0x5555555555555555ULL;
            __mmask64 m2 = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
            return _kor_mask64(m1, m2);
        }
        default: {
            // Pattern based on iteration
            return (__mmask64)(0xFFFFFFFFFFFFFFFFULL >> (iteration % 64));
        }
    }
}

// Control flow that prevents constant folding
__m512i process_with_control_flow(__m512i a, __m512i b, int mode) {
    __m512i result = a;
    
    for (int i = 0; i < 4; i++) {
        __mmask64 mask64 = generate_complex_mask64(i + mode);
        
        // Blend based on mode
        switch (mode) {
            case 0:
                result = blend_v64qi(result, b, mask64);
                break;
            case 1: {
                __mmask32 mask32 = (__mmask32)(mask64 & 0xFFFFFFFF);
                result = blend_v32hi(result, b, mask32);
                break;
            }
            case 2: {
                __mmask16 mask16 = (__mmask16)(mask64 & 0xFFFF);
                result = blend_v16si(result, b, mask16);
                break;
            }
            case 3: {
                __mmask8 mask8 = (__mmask8)(mask64 & 0xFF);
                result = blend_v8di(result, b, mask8);
                break;
            }
        }
        
        // Modify b based on result to create data dependency
        if (i % 2 == 0) {
            b = _mm512_add_epi8(b, result);
        }
    }
    
    return result;
}

// Data dependency chain across different modes
float process_float_chain(__m512 a, __m512 b, __m512d c, __m512d d) {
    // Start with integer blend
    __m512i int_a = _mm512_set1_epi32(1);
    __m512i int_b = _mm512_set1_epi32(2);
    __mmask16 mask16 = _mm512_cmpeq_epi32_mask(int_a, int_b);
    
    // Use integer blend result to generate float mask
    __m512i blend_result = blend_v16si(int_a, int_b, mask16);
    __mmask16 float_mask = _mm512_cmpeq_epi32_mask(blend_result, int_a);
    
    // Perform float blend
    __m512 float_result = blend_v16sf(a, b, float_mask);
    
    // Use float result to generate double mask
    __mmask8 double_mask = _mm512_cmp_ps_mask(float_result, _mm512_set1_ps(0.5f), _CMP_GT_OQ);
    
    // Perform double blend
    __m512d double_result = blend_v8df(c, d, double_mask);
    
    // Horizontal sum for verification
    double sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += double_result[i];
    }
    
    return (float)sum;
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test vectors with patterned data
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
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    __m512d v8df_b = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    
    // Test all modes with control flow
    for (int mode = 0; mode < 4; mode++) {
        __m512i result = process_with_control_flow(v64qi_a, v64qi_b, mode);
        
        // Accumulate checksum
        for (int i = 0; i < 8; i++) {
            checksum += result[i];
        }
    }
    
    // Test float/double chain
    float chain_result = process_float_chain(v16sf_a, v16sf_b, v8df_a, v8df_b);
    checksum += (uint64_t)(chain_result * 1000);
    
    // Direct tests for each mode
    __mmask64 mask64 = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
    __m512i blend64 = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    __mmask32 mask32 = (__mmask32)0xAAAAAAAA;
    __m512i blend32 = blend_v32hi(v64qi_a, v64qi_b, mask32);
    
    __mmask16 mask16 = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_GT_OQ);
    __m512 blend16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    __mmask8 mask8 = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
    __m512d blend8df = blend_v8df(v8df_a, v8df_b, mask8);
    
    // Accumulate more checksum
    for (int i = 0; i < 8; i++) {
        checksum += blend64[i];
        checksum += blend32[i];
        checksum += (uint64_t)(blend16sf[i] * 100);
        checksum += (uint64_t)(blend8df[i] * 1000);
    }
    
#ifdef __AVX512FP16__
    // Test half-precision floats if available
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_set1_bh(1.0f);
    __m512bh v32bf_b = _mm512_set1_bh(2.0f);
    
    __mmask32 hf_mask = (__mmask32)0xAAAAAAAA;
    __m512h blend32hf = blend_v32hf(v32hf_a, v32hf_b, hf_mask);
    __m512bh blend32bf = blend_v32bf(v32bf_a, v32bf_b, hf_mask);
    
    // Add to checksum
    for (int i = 0; i < 32; i++) {
        checksum += (uint64_t)(blend32hf[i] * 100);
        checksum += (uint64_t)(blend32bf[i] * 100);
    }
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
