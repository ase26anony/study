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

// Helper function to generate masks with control flow
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    __mmask64 mask;
    
    // Control flow that prevents constant folding
    if (iteration % 3 == 0) {
        // Comparison mask
        __m512i v1 = _mm512_set1_epi8(iteration);
        __m512i v2 = _mm512_set1_epi8(iteration / 2);
        mask = _mm512_cmpeq_epi8_mask(v1, v2);
    } else if (iteration % 3 == 1) {
        // Immediate mask with pattern
        mask = (__mmask64)(0xAAAAAAAAAAAAAAAAULL);
    } else {
        // Logical combination of masks
        __m512i v1 = _mm512_set1_epi8(iteration);
        __m512i v2 = _mm512_set1_epi8(100);
        __mmask64 m1 = _mm512_cmpgt_epi8_mask(v1, v2);
        __mmask64 m2 = (__mmask64)(0x5555555555555555ULL);
        mask = _kor_mask64(m1, m2);
    }
    
    return mask;
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
    
    // Loop with control flow to prevent optimization
    for (int i = 0; i < 10; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_complex_mask64(i);
        __mmask32 mask32 = (__mmask32)(0xAAAAAAAAUL ^ i);
        __mmask16 mask16 = (__mmask16)(0xAAAA ^ i);
        __mmask8 mask8 = (__mmask8)(0xAA ^ i);
        
        // Switch statement for different blend modes
        switch (i % 4) {
            case 0: {
                // V64QImode blend
                __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
                
                // Use result to generate mask for next blend (data dependency chain)
                __mmask16 new_mask16 = _mm512_cmpeq_epi32_mask(
                    result64qi, 
                    _mm512_set1_epi8(i)
                );
                
                // V16SFmode blend using mask from previous result
                __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, new_mask16);
                
                // Horizontal sum to prevent optimization
                __m256 hi = _mm512_extractf32x8_ps(result16sf, 1);
                __m256 lo = _mm512_extractf32x8_ps(result16sf, 0);
                __m256 sum8 = _mm256_add_ps(hi, lo);
                __m128 hi4 = _mm256_extractf128_ps(sum8, 1);
                __m128 lo4 = _mm256_extractf128_ps(sum8, 0);
                __m128 sum4 = _mm_add_ps(hi4, lo4);
                sum4 = _mm_hadd_ps(sum4, sum4);
                sum4 = _mm_hadd_ps(sum4, sum4);
                checksum += (uint64_t)_mm_cvtss_f32(sum4);
                break;
            }
                
            case 1: {
                // V32HImode blend
                __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
                
                // V8DFmode blend
                __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8);
                
                // Horizontal sum for double
                __m256d hi = _mm512_extractf64x4_pd(result8df, 1);
                __m256d lo = _mm512_extractf64x4_pd(result8df, 0);
                __m256d sum4d = _mm256_add_pd(hi, lo);
                __m128d hi2 = _mm256_extractf128_pd(sum4d, 1);
                __m128d lo2 = _mm256_extractf128_pd(sum4d, 0);
                __m128d sum2d = _mm_add_pd(hi2, lo2);
                sum2d = _mm_hadd_pd(sum2d, sum2d);
                checksum += (uint64_t)_mm_cvtsd_f64(sum2d);
                break;
            }
                
            case 2: {
                // V16SImode blend
                __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
                
                // V8DImode blend
                __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
                
                // Combine results
                __m512i combined = _mm512_add_epi32(result16si, _mm512_castsi512_si512(result8di));
                
                // Horizontal sum
                __m256i hi = _mm512_extracti64x4_epi64(combined, 1);
                __m256i lo = _mm512_extracti64x4_epi64(combined, 0);
                __m256i sum4i = _mm256_add_epi64(hi, lo);
                __m128i hi2 = _mm256_extracti128_si256(sum4i, 1);
                __m128i lo2 = _mm256_extracti128_si256(sum4i, 0);
                __m128i sum2i = _mm_add_epi64(hi2, lo2);
                checksum += (uint64_t)_mm_extract_epi64(sum2i, 0);
                checksum += (uint64_t)_mm_extract_epi64(sum2i, 1);
                break;
            }
                
            case 3: {
#ifdef __AVX512FP16__
                // V32HFmode blend
                __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
                
                // V32BFmode blend
                __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
                
                // Convert back to half for reduction
                __m512h result_combined = _mm512_castbh_ph(result32bf);
                result_combined = _mm512_add_ph(result_combined, result32hf);
                
                // Horizontal sum (simplified - actual half-precision reduction is more complex)
                __m256h hi = _mm512_extractf32x8_ph(result_combined, 1);
                __m256h lo = _mm512_extractf32x8_ph(result_combined, 0);
                // For demonstration, just accumulate a simple value
                checksum += i * 2;
#endif
                break;
            }
        }
        
        // If-else chain for additional control flow
        if (checksum % 2 == 0) {
            // Additional blend with different mask generation
            __mmask16 cmp_mask = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OS);
            __m512 extra_result = blend_v16sf(v16sf_a, v16sf_b, cmp_mask);
            
            // Simple reduction
            __m128 reduced = _mm512_castps512_ps128(extra_result);
            checksum += (uint64_t)_mm_cvtss_f32(reduced);
        } else {
            // Logical mask operations
            __mmask32 m1 = _mm512_cmpeq_epi16_mask(v32hi_a, v32hi_b);
            __mmask32 m2 = (__mmask32)(0x55555555UL);
            __mmask32 combined_mask = _kor_mask32(m1, m2);
            
            __m512i extra_result = blend_v32hi(v32hi_a, v32hi_b, combined_mask);
            
            // Extract and accumulate
            checksum += (uint64_t)_mm512_extract_epi16(extra_result, 0);
        }
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
