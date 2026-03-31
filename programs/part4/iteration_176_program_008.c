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

// Helper function to generate masks based on loop index
__mmask64 generate_dynamic_mask64(int idx) {
    // Create pattern that changes with index
    uint64_t pattern = 0xAAAAAAAAAAAAAAAAULL ^ (idx * 0x5555555555555555ULL);
    return (__mmask64)pattern;
}

__mmask32 generate_dynamic_mask32(int idx) {
    uint32_t pattern = 0xAAAAAAAA ^ (idx * 0x55555555);
    return (__mmask32)pattern;
}

__mmask16 generate_dynamic_mask16(int idx) {
    uint16_t pattern = 0xAAAA ^ (idx * 0x5555);
    return (__mmask16)pattern;
}

__mmask8 generate_dynamic_mask8(int idx) {
    uint8_t pattern = 0xAA ^ (idx * 0x55);
    return (__mmask8)pattern;
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i v64qi_a = _mm512_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
    );
    
    __m512i v64qi_b = _mm512_set_epi8(
        63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
        47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32,
        31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    );
    
    __m512i v32hi_a = _mm512_set_epi16(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
    );
    
    __m512i v32hi_b = _mm512_set_epi16(
        31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16,
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    );
    
    __m512i v16si_a = _mm512_set_epi32(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    );
    
    __m512i v16si_b = _mm512_set_epi32(
        15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
    );
    
    __m512i v8di_a = _mm512_set_epi64(0, 1, 2, 3, 4, 5, 6, 7);
    __m512i v8di_b = _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0);
    
    __m512 v16sf_a = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m512d v8df_b = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    
    // Control flow with switch statement for different blend modes
    for (int i = 0; i < 4; i++) {
        __mmask64 mask64;
        __mmask32 mask32;
        __mmask16 mask16;
        __mmask8 mask8;
        
        // Generate masks using different methods based on loop index
        switch (i % 3) {
            case 0:
                // Immediate masks
                mask64 = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
                mask32 = (__mmask32)0xAAAAAAAA;
                mask16 = (__mmask16)0xAAAA;
                mask8 = (__mmask8)0xAA;
                break;
                
            case 1:
                // Dynamic masks based on loop index
                mask64 = generate_dynamic_mask64(i);
                mask32 = generate_dynamic_mask32(i);
                mask16 = generate_dynamic_mask16(i);
                mask8 = generate_dynamic_mask8(i);
                break;
                
            case 2:
                // Logical combination of masks
                mask64 = (__mmask64)(0xAAAAAAAAAAAAAAAAULL ^ (0x5555555555555555ULL << i));
                mask32 = (__mmask32)(0xAAAAAAAA ^ (0x55555555 << i));
                mask16 = (__mmask16)(0xAAAA ^ (0x5555 << i));
                mask8 = (__mmask8)(0xAA ^ (0x55 << i));
                break;
        }
        
        // Perform blends with control flow
        if (i % 2 == 0) {
            // V64QImode blend
            __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
            
            // Use result to generate mask for next blend (data dependency chain)
            __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(result64qi, v64qi_a);
            
            // V32HImode blend with generated mask
            __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, cmp_mask);
            
            // Generate mask for float blend from integer result
            __mmask16 float_mask = _mm512_cmpeq_epi32_mask(result32hi, v32hi_a);
            
            // V16SFmode blend
            __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, float_mask);
            
            // Horizontal sum for checksum
            __m256 hi = _mm512_extractf32x8_ps(result16sf, 1);
            __m256 lo = _mm512_extractf32x8_ps(result16sf, 0);
            __m256 sum256 = _mm256_add_ps(hi, lo);
            __m128 hi128 = _mm256_extractf128_ps(sum256, 1);
            __m128 lo128 = _mm256_extractf128_ps(sum256, 0);
            __m128 sum128 = _mm_add_ps(hi128, lo128);
            sum128 = _mm_hadd_ps(sum128, sum128);
            sum128 = _mm_hadd_ps(sum128, sum128);
            checksum += (uint64_t)_mm_cvtss_f32(sum128);
            
        } else {
            // V16SImode blend
            __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
            
            // Use result to generate mask for double blend
            __mmask8 double_mask = _mm512_cmpeq_epi64_mask(result16si, v16si_a);
            
            // V8DFmode blend
            __m512d result8df = blend_v8df(v8df_a, v8df_b, double_mask);
            
            // V8DImode blend with logical mask operation
            __mmask8 combined_mask = _kand_mask8(double_mask, mask8);
            __m512i result8di = blend_v8di(v8di_a, v8di_b, combined_mask);
            
            // Horizontal sum for checksum
            __m256d hi = _mm512_extractf64x4_pd(result8df, 1);
            __m256d lo = _mm512_extractf64x4_pd(result8df, 0);
            __m256d sum256 = _mm256_add_pd(hi, lo);
            __m128d hi128 = _mm256_extractf128_pd(sum256, 1);
            __m128d lo128 = _mm256_extractf128_pd(sum256, 0);
            __m128d sum128 = _mm_add_pd(hi128, lo128);
            sum128 = _mm_hadd_pd(sum128, sum128);
            checksum += (uint64_t)_mm_cvtsd_f64(sum128);
            
            // Add integer result to checksum
            __m256i hi_i = _mm512_extracti64x4_epi64(result8di, 1);
            __m256i lo_i = _mm512_extracti64x4_epi64(result8di, 0);
            __m256i sum256_i = _mm256_add_epi64(hi_i, lo_i);
            __m128i hi128_i = _mm256_extracti128_si256(sum256_i, 1);
            __m128i lo128_i = _mm256_extracti128_si256(sum256_i, 0);
            __m128i sum128_i = _mm_add_epi64(hi128_i, lo128_i);
            checksum += (uint64_t)_mm_extract_epi64(sum128_i, 0);
        }
        
        // Half-precision blends (if supported)
        #ifdef __AVX512FP16__
        if (i == 0) {
            __m512h v32hf_a = _mm512_setzero_ph();
            __m512h v32hf_b = _mm512_set1_ph((__fp16)1.0);
            __m512bh v32bf_a = _mm512_setzero_bh();
            __m512bh v32bf_b = _mm512_set1_bh((__bf16)1.0);
            
            // Generate comparison mask for half-precision
            __mmask32 hf_mask = _mm512_cmp_ph_mask(v32hf_a, v32hf_b, _CMP_EQ_OQ);
            
            // V32HFmode blend
            __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, hf_mask);
            
            // V32BFmode blend with logical mask operation
            __mmask32 bf_mask = _kor_mask32(hf_mask, mask32);
            __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, bf_mask);
            
            // Add to checksum (convert to float for accumulation)
            __m256h hi_h = _mm512_extractf32x8_ph(result32hf, 1);
            __m256h lo_h = _mm512_extractf32x8_ph(result32hf, 0);
            // Note: Actual horizontal sum for half-precision would require conversion
            // For simplicity, we'll just mark that we executed this path
            checksum += 1;
        }
        #endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
