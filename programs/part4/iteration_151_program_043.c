#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend modes
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    __mmask64 mask64;
    __mmask32 mask32;
    
    if (mode_selector & 1) {
        // Immediate mask for V64QImode
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Dynamic mask from comparison
        __m512i cmp_a = _mm512_set1_epi8(mode_selector);
        __m512i cmp_b = _mm512_set1_epi8(mode_selector ^ 0xFF);
        mask64 = _mm512_cmp_epi8_mask(cmp_a, cmp_b, _MM_CMPINT_EQ);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Chain to V32HImode with different mask generation
    __mmask32 mask32_alt = _mm512_cmp_epi16_mask(result1, _mm512_setzero_si512(), _MM_CMPINT_GT);
    mask32 = _knot_mask32(mask32_alt);
    
    return _mm512_mask_blend_epi16(mask32, result1, _mm512_slli_epi16(b, 1));
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    // Generate mask for V16SFmode using comparison
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    
    // Modify mask based on selector
    if (selector > 0) {
        __mmask16 mask_imm = 0xAAAA;
        mask16 = _kor_mask16(mask16, mask_imm);
    }
    
    __m512 result_sf = _mm512_mask_blend_ps(mask16, a, b);
    
    // Chain to V8DFmode
    __mmask8 mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result_sf), c, _CMP_GT_OQ);
    mask8 = mask8 ^ 0xFF; // XOR with all ones
    
    return _mm512_castpd_ps(_mm512_mask_blend_pd(mask8, _mm512_castps_pd(result_sf), d));
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int iter) {
    // V16SImode with immediate mask
    __mmask16 mask16 = (iter % 2) ? 0x5555 : 0xAAAA;
    __m512i result_si = _mm512_mask_blend_epi32(mask16, a, b);
    
    // Chain to V8DImode with dynamic mask
    __mmask8 mask8 = _mm512_cmp_epi64_mask(result_si, c, _MM_CMPINT_LT);
    
    // Combine with another mask
    __mmask8 mask8_imm = 0xAA;
    mask8 = _kor_mask8(mask8, mask8_imm);
    
    return _mm512_mask_blend_epi64(mask8, result_si, d);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512bh d, int selector) {
    // V32HFmode with comparison mask
    __mmask32 mask32 = _mm512_cmp_ph_mask(a, b, _CMP_NEQ_UQ);
    
    // Modify mask based on selector
    if (selector & 2) {
        mask32 = _knot_mask32(mask32);
    }
    
    __m512h result_hf = _mm512_mask_blend_ph(mask32, a, b);
    
    // For V32BFmode, we need to use the same intrinsic but with __m512bh type
    // Note: _mm512_mask_blend_ph works for both HF and BF modes
    __mmask32 mask32_bf = _mm512_cmp_ph_mask(result_hf, c, _CMP_LT_OQ);
    
    // Create alternating pattern
    mask32_bf = mask32_bf ^ 0xAAAAAAAA;
    
    return _mm512_mask_blend_ph(mask32_bf, result_hf, c);
}
#endif

// Function to force usage of all blend modes with control flow
static float process_all_blends(int iterations) {
    float total_checksum = 0.0f;
    
    // Initialize data with distinct patterns
    __m512i data_i8_1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i data_i8_2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i data_i16_1 = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i data_i16_2 = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 data_f32_1 = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 data_f32_2 = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d data_f64_1 = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d data_f64_2 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    __m512i data_i32_1 = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    __m512i data_i32_2 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    __m512i data_i64_1 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i data_i64_2 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    for (int i = 0; i < iterations; i++) {
        // Data-dependent control flow to select blend modes
        if (i % 3 == 0) {
            // Chain V64QImode -> V32HImode
            __m512i result1 = blend_v64qi_v32hi(data_i8_1, data_i8_2, i);
            
            // Horizontal sum to prevent elimination
            __m256i sum256 = _mm512_castsi512_si256(_mm512_add_epi64(
                _mm512_cvtepi8_epi64(_mm512_castsi512_si128(result1)),
                _mm512_setzero_si512()
            ));
            long long sum = _mm256_extract_epi64(sum256, 0);
            total_checksum += (float)sum;
        } 
        else if (i % 3 == 1) {
            // Chain V16SFmode -> V8DFmode
            __m512 result2 = blend_v16sf_v8df(data_f32_1, data_f32_2, data_f64_1, data_f64_2, i);
            
            // Reduce and accumulate
            float sum = _mm512_reduce_add_ps(result2);
            total_checksum += sum;
        } 
        else {
            // Chain V16SImode -> V8DImode
            __m512i result3 = blend_v16si_v8di(data_i32_1, data_i32_2, data_i64_1, data_i64_2, i);
            
            // Reduce and accumulate
            __m256i sum256 = _mm512_castsi512_si256(_mm512_add_epi64(
                _mm512_cvtepi32_epi64(_mm512_castsi512_si256(result3)),
                _mm512_setzero_si512()
            ));
            long long sum = _mm256_extract_epi64(sum256, 0);
            total_checksum += (float)sum;
        }
        
        // Direct blend calls for all modes (ensures each is used)
        if (i < 2) {
            // V64QImode with immediate mask
            __m512i blend_i8 = _mm512_mask_blend_epi8(0xCCCCCCCCCCCCCCCCULL, data_i8_1, data_i8_2);
            
            // V32HImode with comparison mask
            __mmask32 mask32 = _mm512_cmp_epi16_mask(data_i16_1, data_i16_2, _MM_CMPINT_LT);
            __m512i blend_i16 = _mm512_mask_blend_epi16(mask32, data_i16_1, data_i16_2);
            
            // V16SImode with immediate mask
            __m512i blend_i32 = _mm512_mask_blend_epi32(0xAAAA, data_i32_1, data_i32_2);
            
            // V8DImode with comparison mask
            __mmask8 mask8 = _mm512_cmp_epi64_mask(data_i64_1, data_i64_2, _MM_CMPINT_GT);
            __m512i blend_i64 = _mm512_mask_blend_epi64(mask8, data_i64_1, data_i64_2);
            
            // V16SFmode with immediate mask
            __m512 blend_f32 = _mm512_mask_blend_ps(0x5555, data_f32_1, data_f32_2);
            
            // V8DFmode with comparison mask
            __mmask8 mask8_f = _mm512_cmp_pd_mask(data_f64_1, data_f64_2, _CMP_EQ_OQ);
            __m512d blend_f64 = _mm512_mask_blend_pd(mask8_f, data_f64_1, data_f64_2);
            
            // Accumulate some results
            total_checksum += _mm512_reduce_add_ps(blend_f32);
        }
    }
    
#ifdef __AVX512FP16__
    // Process half-precision blends if supported
    __m512h data_hf1 = _mm512_castsi512_ph(_mm512_set1_epi16(0x3C00)); // 1.0 in FP16
    __m512h data_hf2 = _mm512_castsi512_ph(_mm512_set1_epi16(0x4000)); // 2.0 in FP16
    __m512h data_hf3 = _mm512_castsi512_ph(_mm512_set1_epi16(0x4200)); // 3.0 in FP16
    __m512bh data_bf = _mm512_castsi512_bh(_mm512_set1_epi16(0x3F80)); // ~1.0 in BF16
    
    for (int i = 0; i < iterations && i < 4; i++) {
        __m512h result_hf = blend_v32hf_v32bf(data_hf1, data_hf2, data_hf3, data_bf, i);
        
        // Simple accumulation (convert to float for reduction)
        __m512 result_f32 = _mm512_cvtph_ps(_mm512_castph_si512(result_hf));
        total_checksum += _mm512_reduce_add_ps(result_f32);
        
        // Direct V32HFmode blend
        __mmask32 mask_hf = 0xAAAAAAAA;
        __m512h blend_hf = _mm512_mask_blend_ph(mask_hf, data_hf1, data_hf2);
        __m512 blend_f32_from_hf = _mm512_cvtph_ps(_mm512_castph_si512(blend_hf));
        total_checksum += _mm512_reduce_add_ps(blend_f32_from_hf);
    }
#endif
    
    return total_checksum;
}

int main() {
    // Process blends with multiple iterations to exercise different paths
    float checksum = process_all_blends(10);
    
    // Print result to prevent dead code elimination
    printf("Final checksum: %f\n", checksum);
    
    // Verify with a simple test
    __m512i test_a = _mm512_set1_epi32(1);
    __m512i test_b = _mm512_set1_epi32(2);
    __mmask16 test_mask = 0x5555;
    __m512i test_result = _mm512_mask_blend_epi32(test_mask, test_a, test_b);
    
    // Extract and print one element to ensure execution
    int test_val = _mm512_extract_epi32(test_result, 0);
    printf("Test blend result: %d (expected 2 with mask 0x5555)\n", test_val);
    
    return 0;
}
