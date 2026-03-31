#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure separate code generation for each mode
__attribute__((noinline, target("avx512f,avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline, target("avx512f,avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline, target("avx512f,avx512bw,avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512f,avx512bw,avx512fp16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
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

// Helper function to create complex mask patterns
__mmask64 generate_complex_mask64(int iteration) {
    // Mix of immediate and computed masks
    __mmask64 base_mask = 0xAAAAAAAAAAAAAAAAULL;
    __mmask64 dynamic_mask = 0;
    
    // Create dynamic pattern based on iteration
    for (int i = 0; i < 64; i++) {
        if ((i + iteration) % 3 == 0) {
            dynamic_mask |= (1ULL << i);
        }
    }
    
    // Combine masks using logical operations
    return _kor_mask64(base_mask, dynamic_mask);
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
    
    // Initialize floating-point vectors
    float sf_values[16];
    double df_values[8];
    for (int i = 0; i < 16; i++) sf_values[i] = i * 1.5f;
    for (int i = 0; i < 8; i++) df_values[i] = i * 2.5;
    
    __m512 v16sf_a = _mm512_loadu_ps(sf_values);
    __m512 v16sf_b = _mm512_set1_ps(100.0f);
    __m512d v8df_a = _mm512_loadu_pd(df_values);
    __m512d v8df_b = _mm512_set1_pd(200.0);
    
    // Initialize half-precision vectors (if supported)
    __m512h v32hf_a, v32hf_b;
    __m512bh v32bf_a, v32bf_b;
    
    // Control flow with switch statement
    for (int mode = 0; mode < 8; mode++) {
        switch (mode) {
            case 0: { // V64QImode
                // Generate mask using comparison
                __m512i cmp_a = _mm512_set1_epi8(mode * 8);
                __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(v64qi_a, cmp_a);
                __mmask64 final_mask = _kor_mask64(cmp_mask, 0x5555555555555555ULL);
                
                __m512i result = blend_v64qi(v64qi_a, v64qi_b, final_mask);
                
                // Data dependency chain: use result in next iteration
                v64qi_a = _mm512_add_epi8(result, _mm512_set1_epi8(1));
                
                // Accumulate checksum
                __m512i sum = _mm512_sad_epu8(result, _mm512_setzero_si512());
                checksum += _mm512_extract_epi64(sum, 0);
                break;
            }
            
            case 1: { // V32HImode
                // Mask from loop-dependent condition
                __mmask32 mask = 0;
                for (int i = 0; i < 32; i++) {
                    if ((i + mode) % 4 < 2) {
                        mask |= (1U << i);
                    }
                }
                
                __m512i result = blend_v32hi(v32hi_a, v32hi_b, mask);
                
                // Use result to generate mask for next operation
                __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(result, _mm512_set1_epi16(15));
                __m512i next_input = _mm512_mask_blend_epi16(cmp_mask, result, v32hi_a);
                
                __m512i sum = _mm512_sad_epu8(next_input, _mm512_setzero_si512());
                checksum += _mm512_extract_epi64(sum, 0);
                break;
            }
            
            case 2: { // V16SImode
                // Complex mask generation with logical operations
                __m512i cmp_val = _mm512_set1_epi32(mode * 100);
                __mmask16 cmp_mask = _mm512_cmpeq_epi32_mask(v64qi_a, cmp_val);
                __mmask16 imm_mask = 0xAAAA;
                __mmask16 final_mask = _kxor_mask16(cmp_mask, imm_mask);
                
                __m512i v16si_a = _mm512_set_epi32(
                    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
                );
                __m512i v16si_b = _mm512_set_epi32(
                    15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
                );
                
                __m512i result = blend_v16si(v16si_a, v16si_b, final_mask);
                
                // Horizontal sum
                __m256i sum256 = _mm512_extracti64x4_epi64(result, 0);
                sum256 = _mm256_add_epi64(sum256, _mm512_extracti64x4_epi64(result, 1));
                checksum += _mm256_extract_epi64(sum256, 0);
                break;
            }
            
            case 3: { // V8DImode
                __mmask8 mask = 0;
                for (int i = 0; i < 8; i++) {
                    mask |= ((mode >> i) & 1) << i;
                }
                
                __m512i v8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
                __m512i v8di_b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
                
                __m512i result = blend_v8di(v8di_a, v8di_b, mask);
                
                // Accumulate
                __m256i sum256 = _mm512_extracti64x4_epi64(result, 0);
                sum256 = _mm256_add_epi64(sum256, _mm512_extracti64x4_epi64(result, 1));
                checksum += _mm256_extract_epi64(sum256, 0);
                break;
            }
            
            case 4: { // V16SFmode
                // Generate mask from floating comparison
                __m512 threshold = _mm512_set1_ps(mode * 10.0f);
                __mmask16 cmp_mask = _mm512_cmp_ps_mask(v16sf_a, threshold, _CMP_LT_OQ);
                __mmask16 pattern_mask = 0x5555;
                __mmask16 final_mask = _kor_mask16(cmp_mask, pattern_mask);
                
                __m512 result = blend_v16sf(v16sf_a, v16sf_b, final_mask);
                
                // Data dependency: use result in next iteration
                v16sf_a = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
                
                // Reduce
                __m256 sum256 = _mm512_extractf32x8_ps(result, 0);
                sum256 = _mm256_add_ps(sum256, _mm512_extractf32x8_ps(result, 1));
                checksum += (uint64_t)_mm256_cvtps_epi32(sum256)[0];
                break;
            }
            
            case 5: { // V8DFmode
                // Complex mask with immediate and comparison
                __m512d threshold = _mm512_set1_pd(mode * 5.0);
                __mmask8 cmp_mask = _mm512_cmp_pd_mask(v8df_a, threshold, _CMP_GT_OQ);
                __mmask8 imm_mask = 0xAA;
                __mmask8 final_mask = _kxor_mask8(cmp_mask, imm_mask);
                
                __m512d result = blend_v8df(v8df_a, v8df_b, final_mask);
                
                // Reduce
                __m256d sum256 = _mm512_extractf64x4_pd(result, 0);
                sum256 = _mm256_add_pd(sum256, _mm512_extractf64x4_pd(result, 1));
                checksum += (uint64_t)_mm256_cvtpd_epi64(sum256)[0];
                break;
            }
            
            case 6: { // V32HFmode (if supported)
                #ifdef __AVX512FP16__
                // Initialize half-precision vectors
                _Float16 hf_values[32];
                for (int i = 0; i < 32; i++) hf_values[i] = (_Float16)(i * 0.5f);
                v32hf_a = _mm512_loadu_ph(hf_values);
                v32hf_b = _mm512_set1_ph((_Float16)50.0f);
                
                // Generate mask
                __mmask32 mask = 0;
                for (int i = 0; i < 32; i++) {
                    if ((i * mode) % 5 < 3) {
                        mask |= (1U << i);
                    }
                }
                
                __m512h result = blend_v32hf(v32hf_a, v32hf_b, mask);
                
                // Simple reduction
                _Float16* res_ptr = (_Float16*)&result;
                for (int i = 0; i < 32; i += 4) {
                    checksum += (uint64_t)(res_ptr[i] * 100);
                }
                #endif
                break;
            }
            
            case 7: { // V32BFmode (if supported)
                #ifdef __AVX512FP16__
                // Initialize brain-float vectors
                __bf16 bf_values[32];
                for (int i = 0; i < 32; i++) bf_values[i] = (__bf16)(i * 0.3f);
                v32bf_a = _mm512_loadu_ph((_Float16*)bf_values);
                v32bf_b = _mm512_set1_ph((_Float16)30.0f);
                
                // Complex mask generation
                __mmask32 mask1 = 0xAAAAAAAA;
                __mmask32 mask2 = 0;
                for (int i = 0; i < 32; i++) {
                    if (i % 3 == mode % 3) {
                        mask2 |= (1U << i);
                    }
                }
                __mmask32 final_mask = _kor_mask32(mask1, mask2);
                
                __m512bh result = blend_v32bf(v32bf_a, v32bf_b, final_mask);
                
                // Accumulate
                __bf16* res_ptr = (__bf16*)&result;
                for (int i = 0; i < 32; i += 2) {
                    checksum += (uint64_t)(res_ptr[i] * 50);
                }
                #endif
                break;
            }
        }
    }
    
    // Additional test with if-else chain for control flow
    __m512i test_vec = _mm512_set1_epi32(42);
    __m512i accum = _mm512_setzero_si512();
    
    for (int i = 0; i < 100; i++) {
        if (i % 10 == 0) {
            // V64QImode blend
            __mmask64 mask = generate_complex_mask64(i);
            __m512i blended = blend_v64qi(test_vec, _mm512_set1_epi8(i), mask);
            accum = _mm512_add_epi8(accum, blended);
        } else if (i % 7 == 0) {
            // V32HImode blend
            __mmask32 mask = (i * 0x55555555) & 0xFFFFFFFF;
            __m512i blended = blend_v32hi(_mm512_set1_epi16(i), test_vec, mask);
            accum = _mm512_add_epi16(accum, blended);
        } else if (i % 5 == 0) {
            // V16SFmode blend
            __m512 fvec = _mm512_set1_ps(i * 0.1f);
            __mmask16 mask = _mm512_cmp_ps_mask(fvec, _mm512_set1_ps(2.5f), _CMP_GT_OQ);
            __m512 blended = blend_v16sf(fvec, _mm512_set1_ps(99.9f), mask);
            
            // Convert and accumulate
            __m512i int_vec = _mm512_cvtps_epi32(blended);
            accum = _mm512_add_epi32(accum, int_vec);
        }
        
        // Modify test vector based on iteration
        if (i % 3 == 0) {
            test_vec = _mm512_add_epi32(test_vec, _mm512_set1_epi32(1));
        }
    }
    
    // Final reduction
    __m256i sum256 = _mm512_extracti64x4_epi64(accum, 0);
    sum256 = _mm256_add_epi64(sum256, _mm512_extracti64x4_epi64(accum, 1));
    checksum += _mm256_extract_epi64(sum256, 0);
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
