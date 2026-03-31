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

// Helper function to generate dynamic masks based on loop index
__mmask64 generate_dynamic_mask64(int iteration) {
    // Create pattern that changes with iteration
    __m512i vec1 = _mm512_set1_epi8(iteration);
    __m512i vec2 = _mm512_set1_epi8(iteration * 2);
    
    // Generate mask from comparison - tests comparison path
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(vec1, vec2);
    
    // Combine with immediate mask using logical operations
    __mmask64 imm_mask = (__mmask64)(0xAAAAAAAAAAAAAAAAULL ^ iteration);
    
    return _kor_mask64(cmp_mask, imm_mask);
}

__mmask32 generate_dynamic_mask32(int iteration) {
    __m512i vec1 = _mm512_set1_epi16(iteration);
    __m512i vec2 = _mm512_set1_epi16(iteration * 3);
    
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(vec1, vec2);
    __mmask32 imm_mask = (__mmask32)(0x55555555 ^ iteration);
    
    return _kxor_mask32(cmp_mask, imm_mask);
}

__mmask16 generate_dynamic_mask16_float(int iteration) {
    __m512 vec1 = _mm512_set1_ps(iteration * 1.0f);
    __m512 vec2 = _mm512_set1_ps(iteration * 2.0f);
    
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(vec1, vec2, _CMP_LT_OQ);
    __mmask16 imm_mask = (__mmask16)(0xAAAA ^ iteration);
    
    return _kand_mask16(cmp_mask, imm_mask);
}

__mmask8 generate_dynamic_mask8_double(int iteration) {
    __m512d vec1 = _mm512_set1_pd(iteration * 1.0);
    __m512d vec2 = _mm512_set1_pd(iteration * 1.5);
    
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(vec1, vec2, _CMP_GT_OQ);
    __mmask8 imm_mask = (__mmask8)(0xAA ^ iteration);
    
    return _kor_mask8(cmp_mask, imm_mask);
}

// Control flow that prevents constant folding
float process_with_control_flow(int mode, int iterations) {
    float checksum = 0.0f;
    
    // Initialize test vectors with patterns
    __m512i int_vec_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i int_vec_b = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 float_vec_a = _mm512_set_ps(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    );
    
    __m512 float_vec_b = _mm512_set_ps(
        16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f
    );
    
    __m512d double_vec_a = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d double_vec_b = _mm512_set_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    
    for (int i = 0; i < iterations; i++) {
        // Switch statement to select blend mode - prevents optimization
        switch (mode) {
            case 0: {
                // V64QImode blend
                __mmask64 mask64 = generate_dynamic_mask64(i);
                __m512i result64qi = blend_v64qi(int_vec_a, int_vec_b, mask64);
                
                // Use result to generate mask for next blend (data dependency chain)
                __mmask32 derived_mask32 = _mm512_cmpeq_epi16_mask(
                    result64qi, 
                    _mm512_set1_epi8(i)
                );
                
                // V32HImode blend with derived mask
                __m512i result32hi = blend_v32hi(
                    _mm512_set1_epi16(i),
                    _mm512_set1_epi16(i * 2),
                    derived_mask32
                );
                
                // Accumulate checksum
                int64_t sum64qi = _mm512_reduce_add_epi64(result64qi);
                int64_t sum32hi = _mm512_reduce_add_epi64(result32hi);
                checksum += (float)(sum64qi + sum32hi);
                break;
            }
            
            case 1: {
                // V16SImode blend
                __mmask16 mask16_int = (__mmask16)(0xAAAA ^ i);
                __m512i result16si = blend_v16si(
                    _mm512_set1_epi32(i),
                    _mm512_set1_epi32(i * 3),
                    mask16_int
                );
                
                // V8DImode blend
                __mmask8 mask8_int = (__mmask8)(0x55 ^ i);
                __m512i result8di = blend_v8di(
                    _mm512_set1_epi64(i),
                    _mm512_set1_epi64(i * 4),
                    mask8_int
                );
                
                // Accumulate checksum
                int64_t sum16si = _mm512_reduce_add_epi64(result16si);
                int64_t sum8di = _mm512_reduce_add_epi64(result8di);
                checksum += (float)(sum16si + sum8di);
                break;
            }
            
            case 2: {
                // V16SFmode blend
                __mmask16 mask16_float = generate_dynamic_mask16_float(i);
                __m512 result16sf = blend_v16sf(float_vec_a, float_vec_b, mask16_float);
                
                // V8DFmode blend
                __mmask8 mask8_double = generate_dynamic_mask8_double(i);
                __m512d result8df = blend_v8df(double_vec_a, double_vec_b, mask8_double);
                
                // Accumulate checksum
                float sum16sf = _mm512_reduce_add_ps(result16sf);
                double sum8df = _mm512_reduce_add_pd(result8df);
                checksum += sum16sf + (float)sum8df;
                break;
            }
            
            #ifdef __AVX512FP16__
            case 3: {
                // V32HFmode blend
                __m512h half_vec_a = _mm512_set1_ph((_Float16)1.0f);
                __m512h half_vec_b = _mm512_set1_ph((_Float16)2.0f);
                __mmask32 mask32_half = (__mmask32)(0x55555555 ^ i);
                __m512h result32hf = blend_v32hf(half_vec_a, half_vec_b, mask32_half);
                
                // V32BFmode blend
                __m512bh bfloat_vec_a = _mm512_set1_bh((__bf16)1.0f);
                __m512bh bfloat_vec_b = _mm512_set1_bh((__bf16)3.0f);
                __m512bh result32bf = blend_v32bf(bfloat_vec_a, bfloat_vec_b, mask32_half);
                
                // Accumulate checksum (simplified - actual reduction would need more work)
                checksum += (float)i * 2.0f;
                break;
            }
            #endif
            
            default:
                // Fallback with mixed blends
                __mmask64 mixed_mask = (__mmask64)(i);
                __m512i mixed_result = blend_v64qi(int_vec_a, int_vec_b, mixed_mask);
                int64_t mixed_sum = _mm512_reduce_add_epi64(mixed_result);
                checksum += (float)mixed_sum;
                break;
        }
        
        // If-else chain that depends on blend results
        if (checksum > 1000.0f) {
            // Perform additional blend with different mode
            __mmask16 extra_mask = (__mmask16)(checksum);
            __m512 extra_result = blend_v16sf(
                _mm512_set1_ps(checksum),
                _mm512_set1_ps(checksum * 0.5f),
                extra_mask
            );
            checksum += _mm512_reduce_add_ps(extra_result);
        } else if (checksum < 0.0f) {
            __mmask8 extra_mask = (__mmask8)(-checksum);
            __m512d extra_result = blend_v8df(
                _mm512_set1_pd(checksum),
                _mm512_set1_pd(checksum * 2.0),
                extra_mask
            );
            checksum += (float)_mm512_reduce_add_pd(extra_result);
        }
    }
    
    return checksum;
}

int main() {
    float total_checksum = 0.0f;
    
    printf("Starting AVX-512 blend coverage test...\n");
    
    // Test all modes with different iteration counts
    for (int mode = 0; mode < 4; mode++) {
        int iterations = 10 + mode * 5;  // Vary iterations per mode
        
        printf("Testing mode %d with %d iterations...\n", mode, iterations);
        
        float mode_checksum = process_with_control_flow(mode, iterations);
        total_checksum += mode_checksum;
        
        printf("Mode %d checksum: %f\n", mode, mode_checksum);
    }
    
    // Additional test with nested loops for more complex control flow
    printf("\nRunning nested loop test...\n");
    for (int outer = 0; outer < 3; outer++) {
        for (int inner = 0; inner < 5; inner++) {
            // Generate masks using different methods in nested context
            __mmask64 nested_mask = (__mmask64)(outer * 100 + inner);
            __m512i vec_a = _mm512_set1_epi8(outer);
            __m512i vec_b = _mm512_set1_epi8(inner);
            
            __m512i result = blend_v64qi(vec_a, vec_b, nested_mask);
            
            // Use result to generate mask for float blend
            __mmask16 derived_float_mask = _mm512_cmpeq_epi32_mask(
                result,
                _mm512_set1_epi8(outer + inner)
            );
            
            __m512 float_result = blend_v16sf(
                _mm512_set1_ps(outer * 1.0f),
                _mm512_set1_ps(inner * 1.0f),
                derived_float_mask
            );
            
            total_checksum += _mm512_reduce_add_ps(float_result);
        }
    }
    
    printf("\nFinal total checksum: %f\n", total_checksum);
    printf("Test completed.\n");
    
    return 0;
}
