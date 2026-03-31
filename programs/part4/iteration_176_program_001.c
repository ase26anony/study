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

// Helper function to create complex mask patterns
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    // Mix of immediate, comparison, and logical operations
    __m512i pattern1 = _mm512_set1_epi8(iteration % 256);
    __m512i pattern2 = _mm512_set1_epi8((iteration * 7) % 256);
    
    // Comparison mask
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(pattern1, pattern2);
    
    // Immediate mask with pattern
    __mmask64 imm_mask = (__mmask64)(0xAAAAAAAAAAAAAAAAULL ^ (iteration * 0x1111111111111111ULL));
    
    // Logical combination
    return _kor_mask64(cmp_mask, imm_mask);
}

__attribute__((noinline))
__mmask32 generate_complex_mask32(int iteration) {
    __m512i pattern1 = _mm512_set1_epi16(iteration % 65536);
    __m512i pattern2 = _mm512_set1_epi16((iteration * 13) % 65536);
    
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(pattern1, pattern2);
    __mmask32 imm_mask = (__mmask32)(0xAAAAAAAA ^ (iteration * 0x11111111));
    
    return _kxor_mask32(cmp_mask, imm_mask);
}

__attribute__((noinline))
__mmask16 generate_complex_mask16(int iteration) {
    __m512 pattern1 = _mm512_set1_ps(iteration * 0.1f);
    __m512 pattern2 = _mm512_set1_ps(iteration * 0.3f);
    
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(pattern1, pattern2, _CMP_LT_OQ);
    __mmask16 imm_mask = (__mmask16)(0xAAAA ^ (iteration * 0x1111));
    
    return _kand_mask16(cmp_mask, imm_mask);
}

__attribute__((noinline))
__mmask8 generate_complex_mask8(int iteration) {
    __m512d pattern1 = _mm512_set1_pd(iteration * 0.01);
    __m512d pattern2 = _mm512_set1_pd(iteration * 0.03);
    
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(pattern1, pattern2, _CMP_GT_OQ);
    __mmask8 imm_mask = (__mmask8)(0xAA ^ (iteration * 0x11));
    
    return _kor_mask8(cmp_mask, imm_mask);
}

// Control flow that prevents constant folding
__attribute__((noinline))
long long process_blends_with_control_flow(int mode_selector, int iterations) {
    long long checksum = 0;
    
    // Initialize test vectors with patterns
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
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512 float_vec2 = _mm512_set_ps(
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512d double_vec1 = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m512d double_vec2 = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    
    for (int i = 0; i < iterations; i++) {
        // Switch statement to select different blend modes
        switch ((mode_selector + i) % 8) {
            case 0: {
                // V64QImode blend
                __mmask64 mask64 = generate_complex_mask64(i);
                __m512i result = blend_v64qi(int_vec1, int_vec2, mask64);
                
                // Use result to generate mask for next operation (data dependency chain)
                __mmask32 mask32 = _mm512_cmpeq_epi16_mask(result, int_vec1);
                __m512i result2 = blend_v32hi(int_vec1, int_vec2, mask32);
                
                // Horizontal sum for checksum
                checksum += _mm512_reduce_add_epi64(result2);
                break;
            }
            
            case 1: {
                // V32HImode blend
                __mmask32 mask32 = generate_complex_mask32(i);
                __m512i result = blend_v32hi(int_vec1, int_vec2, mask32);
                
                // Use result in comparison for float blend
                __mmask16 mask16 = _mm512_cmpeq_epi32_mask(result, int_vec1);
                __m512 float_result = blend_v16sf(float_vec1, float_vec2, mask16);
                
                checksum += (long long)_mm512_reduce_add_ps(float_result);
                break;
            }
            
            case 2: {
                // V16SImode blend
                __mmask16 mask16 = generate_complex_mask16(i);
                __m512i result = blend_v16si(int_vec1, int_vec2, mask16);
                
                // Chain to double blend
                __mmask8 mask8 = _mm512_cmpeq_epi64_mask(result, int_vec1);
                __m512d double_result = blend_v8df(double_vec1, double_vec2, mask8);
                
                checksum += (long long)_mm512_reduce_add_pd(double_result);
                break;
            }
            
            case 3: {
                // V8DImode blend
                __mmask8 mask8 = generate_complex_mask8(i);
                __m512i result = blend_v8di(int_vec1, int_vec2, mask8);
                checksum += _mm512_reduce_add_epi64(result);
                break;
            }
            
            case 4: {
                // V16SFmode blend
                __mmask16 mask16 = generate_complex_mask16(i);
                __m512 result = blend_v16sf(float_vec1, float_vec2, mask16);
                checksum += (long long)_mm512_reduce_add_ps(result);
                break;
            }
            
            case 5: {
                // V8DFmode blend
                __mmask8 mask8 = generate_complex_mask8(i);
                __m512d result = blend_v8df(double_vec1, double_vec2, mask8);
                checksum += (long long)_mm512_reduce_add_pd(result);
                break;
            }
            
            #ifdef __AVX512FP16__
            case 6: {
                // V32HFmode blend
                __m512h half_vec1 = _mm512_set1_ph(1.0f);
                __m512h half_vec2 = _mm512_set1_ph(2.0f);
                __mmask32 mask32 = generate_complex_mask32(i);
                __m512h result = blend_v32hf(half_vec1, half_vec2, mask32);
                
                // Convert to float for checksum
                __m512 float_result = _mm512_cvtph_ps(_mm512_cvtph_ps(result));
                checksum += (long long)_mm512_reduce_add_ps(float_result);
                break;
            }
            
            case 7: {
                // V32BFmode blend
                __m512bh bfloat_vec1 = _mm512_set1_bh(1.0f);
                __m512bh bfloat_vec2 = _mm512_set1_bh(2.0f);
                __mmask32 mask32 = generate_complex_mask32(i);
                __m512bh result = blend_v32bf(bfloat_vec1, bfloat_vec2, mask32);
                
                // Convert to float for checksum
                __m512 float_result = _mm512_cvtpbh_ps(result);
                checksum += (long long)_mm512_reduce_add_ps(float_result);
                break;
            }
            #endif
            
            default:
                break;
        }
        
        // Modify vectors slightly each iteration to prevent optimization
        int_vec1 = _mm512_add_epi8(int_vec1, _mm512_set1_epi8(1));
        float_vec1 = _mm512_add_ps(float_vec1, _mm512_set1_ps(0.1f));
        double_vec1 = _mm512_add_pd(double_vec1, _mm512_set1_pd(0.01));
    }
    
    return checksum;
}

int main() {
    printf("Testing AVX-512 masked blend intrinsics...\n");
    
    // Test with different control flow paths
    long long checksum1 = process_blends_with_control_flow(0, 16);
    long long checksum2 = process_blends_with_control_flow(1, 16);
    long long checksum3 = process_blends_with_control_flow(2, 16);
    
    // Also test each mode directly to ensure coverage
    __m512i int_vec1 = _mm512_set1_epi8(1);
    __m512i int_vec2 = _mm512_set1_epi8(2);
    __m512 float_vec1 = _mm512_set1_ps(1.0f);
    __m512 float_vec2 = _mm512_set1_ps(2.0f);
    __m512d double_vec1 = _mm512_set1_pd(1.0);
    __m512d double_vec2 = _mm512_set1_pd(2.0);
    
    // Direct calls to ensure each intrinsic is used
    __m512i r1 = blend_v64qi(int_vec1, int_vec2, 0xAAAAAAAAAAAAAAAAULL);
    __m512i r2 = blend_v32hi(int_vec1, int_vec2, 0xAAAAAAAA);
    __m512i r3 = blend_v16si(int_vec1, int_vec2, 0xAAAA);
    __m512i r4 = blend_v8di(int_vec1, int_vec2, 0xAA);
    __m512 r5 = blend_v16sf(float_vec1, float_vec2, 0xAAAA);
    __m512d r6 = blend_v8df(double_vec1, double_vec2, 0xAA);
    
    #ifdef __AVX512FP16__
    __m512h half_vec1 = _mm512_set1_ph(1.0f);
    __m512h half_vec2 = _mm512_set1_ph(2.0f);
    __m512h r7 = blend_v32hf(half_vec1, half_vec2, 0xAAAAAAAA);
    
    __m512bh bfloat_vec1 = _mm512_set1_bh(1.0f);
    __m512bh bfloat_vec2 = _mm512_set1_bh(2.0f);
    __m512bh r8 = blend_v32bf(bfloat_vec1, bfloat_vec2, 0xAAAAAAAA);
    #endif
    
    // Accumulate all results
    long long final_checksum = checksum1 + checksum2 + checksum3;
    final_checksum += _mm512_reduce_add_epi64(r1);
    final_checksum += _mm512_reduce_add_epi64(r2);
    final_checksum += _mm512_reduce_add_epi64(r3);
    final_checksum += _mm512_reduce_add_epi64(r4);
    final_checksum += (long long)_mm512_reduce_add_ps(r5);
    final_checksum += (long long)_mm512_reduce_add_pd(r6);
    
    #ifdef __AVX512FP16__
    __m512 float_r7 = _mm512_cvtph_ps(_mm512_cvtph_ps(r7));
    __m512 float_r8 = _mm512_cvtpbh_ps(r8);
    final_checksum += (long long)_mm512_reduce_add_ps(float_r7);
    final_checksum += (long long)_mm512_reduce_add_ps(float_r8);
    #endif
    
    printf("Final checksum: %lld\n", final_checksum);
    printf("Test completed.\n");
    
    return 0;
}
