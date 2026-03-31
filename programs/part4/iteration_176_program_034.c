#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Function declarations with noinline to ensure separate expansion
__attribute__((noinline)) __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline)) __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

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

// Helper function to generate masks with control flow
__mmask64 generate_mask64(int iteration) {
    __mmask64 mask;
    
    // Complex control flow to prevent optimization
    if (iteration % 3 == 0) {
        // Method 1: Immediate mask with pattern
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (iteration % 3 == 1) {
        // Method 2: Alternating pattern
        mask = 0x5555555555555555ULL;
    } else {
        // Method 3: Checkerboard pattern
        mask = 0x3333333333333333ULL;
    }
    
    // Apply logical operations
    if (iteration > 10) {
        mask = _knot_mask64(mask);
    }
    
    return mask;
}

// Data dependency chain function
float process_data_dependency_chain(int seed) {
    // Initialize vectors with patterned data
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
    
    // Generate mask using comparison
    __m512i cmp_a = _mm512_set1_epi8(seed);
    __m512i cmp_b = _mm512_set1_epi8(seed + 1);
    __mmask64 mask64 = _mm512_cmpeq_epi8_mask(cmp_a, cmp_b);
    
    // Blend V64QImode
    __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
    
    // Use result to generate mask for next operation (data dependency)
    __m512i zero = _mm512_setzero_si512();
    __mmask32 mask32 = _mm512_cmpgt_epi16_mask(result64qi, zero);
    
    // Prepare V32HImode vectors
    __m512i v32hi_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i v32hi_b = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Blend V32HImode with mask from previous operation
    __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
    
    // Continue dependency chain to floating point
    __m512 v16sf_a = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    // Generate mask from integer result
    __m512i threshold = _mm512_set1_epi32(8);
    __mmask16 mask16 = _mm512_cmpgt_epi32_mask(result32hi, threshold);
    
    // Blend V16SFmode
    __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
    
    // Horizontal sum for verification
    float sum = _mm512_reduce_add_ps(result16sf);
    return sum;
}

int main() {
    uint64_t checksum = 0;
    
    // Complex control flow with switch statement
    for (int i = 0; i < 5; i++) {
        switch (i % 4) {
            case 0: {
                // Test V16SImode with comparison mask
                __m512i v16si_a = _mm512_set_epi32(
                    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
                );
                __m512i v16si_b = _mm512_set_epi32(
                    15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
                );
                
                // Dynamic mask generation
                __m512i cmp_val = _mm512_set1_epi32(i);
                __mmask16 mask16 = _mm512_cmpeq_epi32_mask(v16si_a, cmp_val);
                
                // Apply logical operation
                mask16 = _kxor_mask16(mask16, 0xFFFF);
                
                __m512i result = blend_v16si(v16si_a, v16si_b, mask16);
                
                // Accumulate to checksum
                checksum += _mm512_reduce_add_epi32(result);
                break;
            }
            
            case 1: {
                // Test V8DImode with immediate mask
                __m512i v8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
                __m512i v8di_b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
                
                __mmask8 mask8 = 0xAA; // 10101010 pattern
                
                // Modify mask based on iteration
                if (i > 2) {
                    mask8 = _knot_mask8(mask8);
                }
                
                __m512i result = blend_v8di(v8di_a, v8di_b, mask8);
                
                // Accumulate to checksum
                checksum += _mm512_reduce_add_epi64(result);
                break;
            }
            
            case 2: {
                // Test V8DFmode with comparison mask
                __m512d v8df_a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
                __m512d v8df_b = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
                
                __m512d threshold = _mm512_set1_pd(3.5);
                __mmask8 mask8 = _mm512_cmp_pd_mask(v8df_a, threshold, _CMP_GT_OQ);
                
                __m512d result = blend_v8df(v8df_a, v8df_b, mask8);
                
                // Accumulate to checksum
                double sum = _mm512_reduce_add_pd(result);
                checksum += (uint64_t)sum;
                break;
            }
            
            case 3: {
                // Test V16SFmode with complex mask generation
                __m512 v16sf_a = _mm512_set_ps(
                    0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
                    8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
                );
                __m512 v16sf_b = _mm512_set_ps(
                    15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
                    7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
                );
                
                // Generate mask using multiple comparisons
                __m512 cmp_val1 = _mm512_set1_ps(i * 1.0f);
                __m512 cmp_val2 = _mm512_set1_ps(i * 2.0f);
                
                __mmask16 mask1 = _mm512_cmp_ps_mask(v16sf_a, cmp_val1, _CMP_LT_OQ);
                __mmask16 mask2 = _mm512_cmp_ps_mask(v16sf_b, cmp_val2, _CMP_GT_OQ);
                
                // Combine masks
                __mmask16 mask16 = _kor_mask16(mask1, mask2);
                
                __m512 result = blend_v16sf(v16sf_a, v16sf_b, mask16);
                
                // Accumulate to checksum
                float sum = _mm512_reduce_add_ps(result);
                checksum += (uint64_t)sum;
                break;
            }
        }
    }
    
#ifdef __AVX512FP16__
    // Test half-precision modes if supported
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
    
    // Generate mask for half-precision
    __mmask32 mask32 = 0xAAAAAAAA; // Alternating pattern
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
    
    // Test brain float mode (use same data)
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
#endif
    
    // Execute data dependency chain
    float chain_result = process_data_dependency_chain(42);
    checksum += (uint64_t)chain_result;
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
