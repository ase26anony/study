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
    
    // Combine masks with logical operations
    return _kor_mask64(base_mask, dynamic_mask);
}

__mmask32 generate_complex_mask32(int iteration) {
    __mmask32 mask1 = 0xAAAAAAAA;
    __mmask32 mask2 = 0;
    
    // Pattern that changes with iteration
    for (int i = 0; i < 32; i++) {
        if ((i * iteration) % 5 < 2) {
            mask2 |= (1U << i);
        }
    }
    
    return _kxor_mask32(mask1, mask2);
}

__mmask16 generate_complex_mask16(int iteration) {
    __mmask16 mask1 = 0xAAAA;
    __mmask16 mask2 = 0;
    
    // Different pattern for different modes
    for (int i = 0; i < 16; i++) {
        if ((i + iteration * 2) % 7 < 3) {
            mask2 |= (1U << i);
        }
    }
    
    return _kand_mask16(mask1, mask2);
}

__mmask8 generate_complex_mask8(int iteration) {
    __mmask8 mask1 = 0xAA;
    __mmask8 mask2 = 0;
    
    for (int i = 0; i < 8; i++) {
        if ((i * iteration) % 4 == 0) {
            mask2 |= (1U << i);
        }
    }
    
    return _kor_mask8(mask1, mask2);
}

int main() {
    // Initialize test data with patterns
    int8_t int8_data_a[64], int8_data_b[64];
    int16_t int16_data_a[32], int16_data_b[32];
    int32_t int32_data_a[16], int32_data_b[16];
    int64_t int64_data_a[8], int64_data_b[8];
    float float_data_a[16], float_data_b[16];
    double double_data_a[8], double_data_b[8];
    
    for (int i = 0; i < 64; i++) {
        int8_data_a[i] = i;
        int8_data_b[i] = 64 - i;
        if (i < 32) {
            int16_data_a[i] = i * 2;
            int16_data_b[i] = 100 - i * 3;
        }
        if (i < 16) {
            int32_data_a[i] = i * 10;
            int32_data_b[i] = 500 - i * 20;
            float_data_a[i] = i * 1.5f;
            float_data_b[i] = 100.0f - i * 3.0f;
        }
        if (i < 8) {
            int64_data_a[i] = i * 100LL;
            int64_data_b[i] = 1000LL - i * 150LL;
            double_data_a[i] = i * 2.5;
            double_data_b[i] = 200.0 - i * 5.0;
        }
    }
    
    // Load vectors
    __m512i v64qi_a = _mm512_loadu_si512(int8_data_a);
    __m512i v64qi_b = _mm512_loadu_si512(int8_data_b);
    __m512i v32hi_a = _mm512_loadu_si512(int16_data_a);
    __m512i v32hi_b = _mm512_loadu_si512(int16_data_b);
    __m512i v16si_a = _mm512_loadu_si512(int32_data_a);
    __m512i v16si_b = _mm512_loadu_si512(int32_data_b);
    __m512i v8di_a = _mm512_loadu_si512(int64_data_a);
    __m512i v8di_b = _mm512_loadu_si512(int64_data_b);
    __m512 v16sf_a = _mm512_loadu_ps(float_data_a);
    __m512 v16sf_b = _mm512_loadu_ps(float_data_b);
    __m512d v8df_a = _mm512_loadu_pd(double_data_a);
    __m512d v8df_b = _mm512_loadu_pd(double_data_b);
    
    // Initialize checksum
    uint64_t checksum = 0;
    
    // Control flow: loop with mode-dependent blending
    for (int iter = 0; iter < 4; iter++) {
        // Switch-like behavior based on iteration
        switch (iter % 3) {
            case 0: {
                // Generate masks using comparisons
                __mmask64 k64 = generate_complex_mask64(iter);
                __m512i cmp_result = _mm512_cmpeq_epi8_mask(v64qi_a, v64qi_b);
                __mmask64 dynamic_mask = _kor_mask64(k64, cmp_result);
                
                __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, dynamic_mask);
                
                // Use result to generate mask for next blend
                __mmask32 mask_from_8bit = 0;
                const uint8_t* res_bytes = (const uint8_t*)&result64qi;
                for (int i = 0; i < 32; i++) {
                    if (res_bytes[i*2] > 32) {
                        mask_from_8bit |= (1U << i);
                    }
                }
                
                __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, 
                    _kxor_mask32(mask_from_8bit, generate_complex_mask32(iter)));
                
                // Horizontal sum for checksum
                __m256i sum256 = _mm512_extracti64x4_epi64(result64qi, 0);
                sum256 = _mm256_add_epi64(sum256, _mm512_extracti64x4_epi64(result64qi, 1));
                checksum += _mm256_extract_epi64(sum256, 0);
                break;
            }
            
            case 1: {
                // Generate masks using float comparisons
                __mmask16 k16_float = generate_complex_mask16(iter);
                __mmask16 cmp_float = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OS);
                __mmask16 combined_float_mask = _kand_mask16(k16_float, cmp_float);
                
                __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, combined_float_mask);
                
                // Use float result to generate integer mask
                __mmask16 int_mask_from_float = 0;
                const float* res_floats = (const float*)&result16sf;
                for (int i = 0; i < 16; i++) {
                    if (res_floats[i] > 50.0f) {
                        int_mask_from_float |= (1U << i);
                    }
                }
                
                __m512i result16si = blend_v16si(v16si_a, v16si_b, int_mask_from_float);
                
                // Horizontal reduction
                __m256 sum256f = _mm512_extractf32x8_ps(result16sf, 0);
                sum256f = _mm256_add_ps(sum256f, _mm512_extractf32x8_ps(result16sf, 1));
                checksum += (uint64_t)_mm256_cvtps_epi32(sum256f)[0];
                break;
            }
            
            case 2: {
                // Double precision blends with complex mask generation
                __mmask8 k8_double = generate_complex_mask8(iter);
                __mmask8 cmp_double = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_GT_OS);
                __mmask8 combined_double_mask = _kor_mask8(k8_double, cmp_double);
                
                __m512d result8df = blend_v8df(v8df_a, v8df_b, combined_double_mask);
                
                // Generate mask for 64-bit integer blend from double result
                __mmask8 int64_mask = 0;
                const double* res_doubles = (const double*)&result8df;
                for (int i = 0; i < 8; i++) {
                    if (res_doubles[i] > 100.0) {
                        int64_mask |= (1U << i);
                    }
                }
                
                __m512i result8di = blend_v8di(v8di_a, v8di_b, int64_mask);
                
                // Horizontal sum
                __m256d sum256d = _mm512_extractf64x4_pd(result8df, 0);
                sum256d = _mm256_add_pd(sum256d, _mm512_extractf64x4_pd(result8df, 1));
                checksum += (uint64_t)_mm256_cvttpd_epi64(sum256d)[0];
                break;
            }
        }
        
        // If-else chain for additional mode coverage
        if (iter == 0) {
            // Additional integer blend with immediate mask
            __mmask32 imm_mask32 = 0x55555555;
            __m512i extra_result = blend_v32hi(v32hi_a, v32hi_b, imm_mask32);
            checksum += _mm512_extract_epi64(extra_result, 0);
        } else if (iter == 2) {
            // Additional float blend with comparison mask
            __m512 ones = _mm512_set1_ps(1.0f);
            __mmask16 cmp_mask = _mm512_cmp_ps_mask(v16sf_a, ones, _CMP_GT_OS);
            __m512 extra_float_result = blend_v16sf(v16sf_a, v16sf_b, cmp_mask);
            checksum += (uint64_t)_mm512_cvtps_epi32(extra_float_result)[0];
        }
    }
    
#ifdef __AVX512FP16__
    // Half-precision float coverage (requires -mavx512fp16)
    _Float16 half_data_a[32], half_data_b[32];
    for (int i = 0; i < 32; i++) {
        half_data_a[i] = i * 0.5f;
        half_data_b[i] = 10.0f - i * 0.3f;
    }
    
    __m512h v32hf_a = _mm512_loadu_ph(half_data_a);
    __m512h v32hf_b = _mm512_loadu_ph(half_data_b);
    
    // Create bfloat16 data (just reuse half data for testing)
    __m512bh v32bf_a = _mm512_loadu_ph(half_data_a);
    __m512bh v32bf_b = _mm512_loadu_ph(half_data_b);
    
    // Generate masks for half-precision
    __mmask32 k32_half = generate_complex_mask32(5);
    __mmask32 cmp_half = _mm512_cmp_ph_mask(v32hf_a, v32hf_b, _CMP_EQ_OQ);
    __mmask32 combined_half_mask = _kxor_mask32(k32_half, cmp_half);
    
    // Perform half-precision blends
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, combined_half_mask);
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, combined_half_mask);
    
    // Add to checksum
    __m256i half_sum = _mm512_extracti64x4_epi64(_mm512_castph_si512(result32hf), 0);
    checksum += _mm256_extract_epi64(half_sum, 0);
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
