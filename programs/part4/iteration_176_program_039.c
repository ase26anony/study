#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure each intrinsic gets expanded independently
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

// Helper function to generate masks based on loop index (control flow)
__mmask64 generate_dynamic_mask64(int iteration) {
    // Create different mask patterns based on iteration
    __m512i pattern1 = _mm512_set1_epi8(iteration % 256);
    __m512i pattern2 = _mm512_set1_epi8((iteration * 7) % 256);
    
    // Comparison mask generation
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(pattern1, pattern2);
    
    // Immediate mask with pattern
    __mmask64 imm_mask = (__mmask64)(0xAAAAAAAAAAAAAAAAULL ^ (iteration & 0xFF));
    
    // Logical combination of masks
    return _kor_mask64(cmp_mask, imm_mask);
}

__mmask32 generate_dynamic_mask32(int iteration) {
    __m512i pattern1 = _mm512_set1_epi16(iteration % 65536);
    __m512i pattern2 = _mm512_set1_epi16((iteration * 13) % 65536);
    
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(pattern1, pattern2);
    __mmask32 imm_mask = (__mmask32)(0xAAAAAAAA ^ (iteration & 0xFFFF));
    
    return _kxor_mask32(cmp_mask, imm_mask);
}

__mmask16 generate_dynamic_mask16(int iteration) {
    __m512 pattern1 = _mm512_set1_ps(iteration * 0.1f);
    __m512 pattern2 = _mm512_set1_ps(iteration * 0.2f);
    
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(pattern1, pattern2, _CMP_LT_OQ);
    __mmask16 imm_mask = (__mmask16)(0xAAAA ^ (iteration & 0xFFFF));
    
    return _kand_mask16(cmp_mask, imm_mask);
}

__mmask8 generate_dynamic_mask8(int iteration) {
    __m512d pattern1 = _mm512_set1_pd(iteration * 0.01);
    __m512d pattern2 = _mm512_set1_pd(iteration * 0.02);
    
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(pattern1, pattern2, _CMP_GT_OQ);
    __mmask8 imm_mask = (__mmask8)(0xAA ^ (iteration & 0xFF));
    
    return _kor_mask8(cmp_mask, imm_mask);
}

// Data dependency chain: use result from one blend to generate mask for another
float process_with_dependency_chains(int iterations) {
    float checksum = 0.0f;
    
    // Initialize vectors with patterned data
    __m512i int_vec1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i int_vec2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 float_vec1 = _mm512_set_ps(
        63.0f, 62.0f, 61.0f, 60.0f, 59.0f, 58.0f, 57.0f, 56.0f,
        55.0f, 54.0f, 53.0f, 52.0f, 51.0f, 50.0f, 49.0f, 48.0f,
        47.0f, 46.0f, 45.0f, 44.0f, 43.0f, 42.0f, 41.0f, 40.0f,
        39.0f, 38.0f, 37.0f, 36.0f, 35.0f, 34.0f, 33.0f, 32.0f,
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512 float_vec2 = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
        16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
        24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f,
        32.0f, 33.0f, 34.0f, 35.0f, 36.0f, 37.0f, 38.0f, 39.0f,
        40.0f, 41.0f, 42.0f, 43.0f, 44.0f, 45.0f, 46.0f, 47.0f,
        48.0f, 49.0f, 50.0f, 51.0f, 52.0f, 53.0f, 54.0f, 55.0f,
        56.0f, 57.0f, 58.0f, 59.0f, 60.0f, 61.0f, 62.0f, 63.0f
    );
    
    __m512d double_vec1 = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    __m512d double_vec2 = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    
    for (int i = 0; i < iterations; i++) {
        // Control flow: switch statement to select different blend modes
        switch (i % 8) {
            case 0: {
                // V64QImode blend
                __mmask64 mask64 = generate_dynamic_mask64(i);
                __m512i result64qi = blend_v64qi(int_vec1, int_vec2, mask64);
                
                // Use result to generate mask for next blend (dependency chain)
                __mmask32 mask_from_result = _mm512_cmpeq_epi16_mask(result64qi, int_vec1);
                __m512i result32hi = blend_v32hi(int_vec1, int_vec2, mask_from_result);
                
                // Horizontal sum to prevent optimization
                checksum += (float)_mm512_reduce_add_epi64(result32hi);
                break;
            }
            
            case 1: {
                // V32HImode blend
                __mmask32 mask32 = generate_dynamic_mask32(i);
                __m512i result32hi = blend_v32hi(int_vec1, int_vec2, mask32);
                
                // Use result in comparison for float blend
                __m512i temp = _mm512_add_epi16(result32hi, _mm512_set1_epi16(1));
                __mmask16 mask_for_float = _mm512_cmpeq_epi32_mask(temp, int_vec1);
                __m512 result16sf = blend_v16sf(float_vec1, float_vec2, mask_for_float);
                
                // Horizontal sum
                checksum += _mm512_reduce_add_ps(result16sf);
                break;
            }
            
            case 2: {
                // V16SImode blend
                __mmask16 mask16 = generate_dynamic_mask16(i);
                __m512i result16si = blend_v16si(int_vec1, int_vec2, mask16);
                
                // Dependency: use integer result to conditionally select float blend
                if (_mm512_reduce_add_epi32(result16si) > 0) {
                    __m512 result16sf = blend_v16sf(float_vec1, float_vec2, mask16);
                    checksum += _mm512_reduce_add_ps(result16sf);
                }
                break;
            }
            
            case 3: {
                // V8DImode blend
                __mmask8 mask8 = generate_dynamic_mask8(i);
                __m512i result8di = blend_v8di(int_vec1, int_vec2, mask8);
                
                // Use same mask for double blend
                __m512d result8df = blend_v8df(double_vec1, double_vec2, mask8);
                
                // Combine results
                checksum += (float)_mm512_reduce_add_epi64(result8di);
                checksum += (float)_mm512_reduce_add_pd(result8df);
                break;
            }
            
            case 4: {
                // V16SFmode blend
                __mmask16 mask16 = generate_dynamic_mask16(i);
                __m512 result16sf = blend_v16sf(float_vec1, float_vec2, mask16);
                
                // Generate new mask based on float result
                __m512 threshold = _mm512_set1_ps(32.0f);
                __mmask16 new_mask = _mm512_cmp_ps_mask(result16sf, threshold, _CMP_GT_OQ);
                __m512 result16sf2 = blend_v16sf(float_vec2, float_vec1, new_mask);
                
                checksum += _mm512_reduce_add_ps(result16sf2);
                break;
            }
            
            case 5: {
                // V8DFmode blend
                __mmask8 mask8 = generate_dynamic_mask8(i);
                __m512d result8df = blend_v8df(double_vec1, double_vec2, mask8);
                
                // Nested if-else chain
                if (_mm512_reduce_add_pd(result8df) > 14.0) {
                    __mmask8 alt_mask = (__mmask8)(0x55 ^ (i & 0xFF));
                    __m512d result8df2 = blend_v8df(double_vec2, double_vec1, alt_mask);
                    checksum += (float)_mm512_reduce_add_pd(result8df2);
                } else {
                    checksum += (float)_mm512_reduce_add_pd(result8df);
                }
                break;
            }
            
#ifdef __AVX512FP16__
            case 6: {
                // V32HFmode blend (requires AVX512-FP16)
                __m512h half_vec1 = _mm512_set1_ph(1.0f);
                __m512h half_vec2 = _mm512_set1_ph(2.0f);
                __mmask32 mask32 = generate_dynamic_mask32(i);
                __m512h result32hf = blend_v32hf(half_vec1, half_vec2, mask32);
                
                // Convert to float for checksum
                __m512 float_result = _mm512_cvtph_ps(_mm512_cvtph_ps(result32hf));
                checksum += _mm512_reduce_add_ps(float_result);
                break;
            }
            
            case 7: {
                // V32BFmode blend (requires AVX512-FP16)
                __m512bh bfloat_vec1 = _mm512_set1_bh(1.0f);
                __m512bh bfloat_vec2 = _mm512_set1_bh(2.0f);
                __mmask32 mask32 = generate_dynamic_mask32(i);
                __m512bh result32bf = blend_v32bf(bfloat_vec1, bfloat_vec2, mask32);
                
                // Convert to float for checksum
                __m512 float_result = _mm512_cvtpbh_ps(_mm512_castsi512_si256(_mm512_castph_si512(result32bf)));
                checksum += _mm512_reduce_add_ps(float_result);
                break;
            }
#endif
            
            default:
                // Fallback to simple blend
                __mmask64 mask64 = (__mmask64)(0xAAAAAAAAAAAAAAAAULL);
                __m512i result = blend_v64qi(int_vec1, int_vec2, mask64);
                checksum += (float)_mm512_reduce_add_epi64(result);
                break;
        }
        
        // Modify vectors slightly each iteration to prevent constant folding
        int_vec1 = _mm512_add_epi8(int_vec1, _mm512_set1_epi8(1));
        float_vec1 = _mm512_add_ps(float_vec1, _mm512_set1_ps(0.1f));
        double_vec1 = _mm512_add_pd(double_vec1, _mm512_set1_pd(0.01));
    }
    
    return checksum;
}

int main() {
    printf("Starting AVX-512 blend coverage test...\n");
    
    // Process with multiple iterations to ensure execution
    float final_checksum = process_with_dependency_chains(32);
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Test completed.\n");
    
    return 0;
}
