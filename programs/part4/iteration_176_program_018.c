#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// ==================== Helper Functions (noinline to ensure expansion) ====================

__attribute__((noinline))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// ==================== Mask Generation Functions ====================

__mmask64 generate_mask64_comparison(__m512i vec1, __m512i vec2) {
    // Generate mask by comparing bytes for equality
    return _mm512_cmpeq_epi8_mask(vec1, vec2);
}

__mmask32 generate_mask32_logical(__mmask32 m1, __mmask32 m2) {
    // Combine masks using logical operations
    __mmask32 xor_mask = _kxor_mask32(m1, m2);
    __mmask32 and_mask = _kand_mask32(m1, m2);
    return _kor_mask32(xor_mask, and_mask);
}

__mmask16 generate_mask16_immediate() {
    // Pattern: alternating bits (0xAAAA)
    return (__mmask16)0xAAAA;
}

__mmask8 generate_mask8_immediate() {
    // Pattern: 0xAA (binary 10101010)
    return (__mmask8)0xAA;
}

__mmask16 generate_mask16_from_float(__m512 a, __m512 b) {
    // Generate mask by comparing floats
    return _mm512_cmp_ps_mask(a, b, _CMP_LT_OS);
}

__mmask8 generate_mask8_from_double(__m512d a, __m512d b) {
    // Generate mask by comparing doubles
    return _mm512_cmp_pd_mask(a, b, _CMP_GT_OS);
}

// ==================== Main Test Function ====================

int main() {
    uint64_t checksum = 0;
    
    // Initialize test vectors with patterned data
    __m512i vec_i8_1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vec_i8_2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vec_i16_1 = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i vec_i16_2 = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vec_i32_1 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i vec_i32_2 = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vec_i64_1 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i vec_i64_2 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    __m512 vec_f32_1 = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 vec_f32_2 = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d vec_f64_1 = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d vec_f64_2 = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    // ==================== Control Flow Integration ====================
    
    // Loop with mask condition depending on loop index
    for (int i = 0; i < 4; i++) {
        // Switch statement selecting blend modes
        switch (i) {
            case 0: {
                // V64QImode with comparison mask
                __mmask64 mask64 = generate_mask64_comparison(vec_i8_1, vec_i8_2);
                __m512i result = blend_v64qi(vec_i8_1, vec_i8_2, mask64);
                
                // Horizontal sum for checksum
                __m512i sum64 = _mm512_sad_epu8(result, _mm512_setzero_si512());
                checksum += _mm512_reduce_add_epi64(sum64);
                break;
            }
            
            case 1: {
                // V32HImode with logical mask operations
                __mmask32 mask32_comp = _mm512_cmpeq_epi16_mask(vec_i16_1, vec_i16_2);
                __mmask32 mask32_imm = (__mmask32)0xAAAAAAAA;
                __mmask32 mask32 = generate_mask32_logical(mask32_comp, mask32_imm);
                
                __m512i result = blend_v32hi(vec_i16_1, vec_i16_2, mask32);
                
                // Horizontal sum
                __m512i sum32 = _mm512_madd_epi16(result, _mm512_set1_epi16(1));
                checksum += _mm512_reduce_add_epi32(sum32);
                break;
            }
            
            case 2: {
                // V16SImode with immediate mask
                __mmask16 mask16 = generate_mask16_immediate();
                __m512i result = blend_v16si(vec_i32_1, vec_i32_2, mask16);
                
                // Horizontal sum
                checksum += _mm512_reduce_add_epi32(result);
                break;
            }
            
            case 3: {
                // V8DImode with immediate mask
                __mmask8 mask8 = generate_mask8_immediate();
                __m512i result = blend_v8di(vec_i64_1, vec_i64_2, mask8);
                
                // Horizontal sum
                checksum += _mm512_reduce_add_epi64(result);
                break;
            }
        }
    }
    
    // ==================== Data Dependency Chains ====================
    
    // Chain 1: Integer blend -> Float blend
    {
        // First do an epi8 blend
        __mmask64 mask64_chain = _mm512_cmpeq_epi8_mask(vec_i8_1, _mm512_set1_epi8(32));
        __m512i int_result = blend_v64qi(vec_i8_1, vec_i8_2, mask64_chain);
        
        // Use integer result to generate mask for float blend
        __m512i int_as_float = _mm512_and_si512(int_result, _mm512_set1_epi32(1));
        __m512 float_mask_source = _mm512_cvtepi32_ps(int_as_float);
        __mmask16 float_mask = _mm512_cmp_ps_mask(float_mask_source, 
                                                 _mm512_set1_ps(0.5f), 
                                                 _CMP_GT_OS);
        
        __m512 float_result = blend_v16sf(vec_f32_1, vec_f32_2, float_mask);
        
        // Accumulate to checksum
        __m512 sum_float = _mm512_add_ps(float_result, _mm512_setzero_ps());
        checksum += (uint64_t)_mm512_reduce_add_ps(sum_float);
    }
    
    // Chain 2: Float blend -> Double blend
    {
        // First do a float blend
        __mmask16 mask16_chain = generate_mask16_from_float(vec_f32_1, vec_f32_2);
        __m512 float_result = blend_v16sf(vec_f32_1, vec_f32_2, mask16_chain);
        
        // Use float result to generate mask for double blend
        __m512d float_as_double = _mm512_cvtps_pd(_mm512_castps512_ps256(float_result));
        __mmask8 double_mask = _mm512_cmp_pd_mask(float_as_double,
                                                 _mm512_set1_pd(3.0),
                                                 _CMP_LT_OS);
        
        __m512d double_result = blend_v8df(vec_f64_1, vec_f64_2, double_mask);
        
        // Accumulate to checksum
        __m512d sum_double = _mm512_add_pd(double_result, _mm512_setzero_pd());
        checksum += (uint64_t)_mm512_reduce_add_pd(sum_double);
    }
    
    // ==================== If-Else Chain ====================
    
    {
        __mmask16 initial_mask = _mm512_cmp_ps_mask(vec_f32_1, vec_f32_2, _CMP_LT_OS);
        __m512 float_blend_result;
        
        if (initial_mask != 0) {
            // First blend
            float_blend_result = blend_v16sf(vec_f32_1, vec_f32_2, initial_mask);
            
            // Use result to generate new mask
            __mmask16 new_mask = _mm512_cmp_ps_mask(float_blend_result, 
                                                   _mm512_set1_ps(7.5f), 
                                                   _CMP_GT_OS);
            
            // Second blend with new mask
            float_blend_result = blend_v16sf(float_blend_result, 
                                            _mm512_set1_ps(100.0f), 
                                            new_mask);
        } else {
            float_blend_result = blend_v16sf(vec_f32_1, vec_f32_2, 0xFFFF);
        }
        
        // Accumulate
        checksum += (uint64_t)_mm512_reduce_add_ps(float_blend_result);
    }
    
    // ==================== Half Precision (if available) ====================
    
#ifdef __AVX512FP16__
    {
        // Initialize half-precision vectors
        __m512h vec_half1 = _mm512_set_ph(
            0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
            8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
            16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
            24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
        );
        
        __m512h vec_half2 = _mm512_set_ph(
            31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
            23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
            15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
            7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
        );
        
        // V32HFmode blend
        __mmask32 mask_half = (__mmask32)0xAAAAAAAA;
        __m512h half_result = blend_v32hf(vec_half1, vec_half2, mask_half);
        
        // V32BFmode blend (using same data)
        __m512bh bfloat_vec1 = _mm512_castph_bh(vec_half1);
        __m512bh bfloat_vec2 = _mm512_castph_bh(vec_half2);
        __m512bh bfloat_result = blend_v32bf(bfloat_vec1, bfloat_vec2, mask_half);
        
        // Convert back and accumulate
        __m512h bfloat_as_half = _mm512_castbh_ph(bfloat_result);
        __m512h sum_half = _mm512_add_ph(half_result, bfloat_as_half);
        
        // Simple reduction for checksum
        __m256h low = _mm512_castph512_ph256(sum_half);
        __m256h high = _mm512_extractph256_ph(sum_half, 1);
        __m256h half_sum = _mm256_add_ph(low, high);
        
        // Further reduction (simplified)
        float half_accum = 0;
        _Float16 temp[32];
        _mm512_storeu_ph(temp, sum_half);
        for (int i = 0; i < 32; i++) {
            half_accum += temp[i];
        }
        checksum += (uint64_t)half_accum;
    }
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
