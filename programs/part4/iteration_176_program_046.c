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

__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline)) __m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline)) __m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

__attribute__((noinline)) __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

// Helper function to generate complex mask patterns
__mmask64 generate_complex_mask64(int iteration) {
    // Mix of immediate, comparison, and logical operations
    __m512i vec1 = _mm512_set1_epi8(iteration);
    __m512i vec2 = _mm512_set1_epi8(iteration * 2);
    
    // Comparison mask
    __mmask64 cmp_mask = _mm512_cmpeq_epi8_mask(vec1, vec2);
    
    // Immediate mask with pattern
    __mmask64 imm_mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
    
    // Logical combination
    __mmask64 final_mask = _kor_mask64(cmp_mask, imm_mask);
    
    // XOR with iteration-based pattern
    __mmask64 iter_mask = (__mmask64)(0x5555555555555555ULL * (iteration & 1));
    return _kxor_mask64(final_mask, iter_mask);
}

__mmask32 generate_complex_mask32(int iteration) {
    __m512i vec1 = _mm512_set1_epi16(iteration);
    __m512i vec2 = _mm512_set1_epi16(iteration * 3);
    
    __mmask32 cmp_mask = _mm512_cmpeq_epi16_mask(vec1, vec2);
    __mmask32 imm_mask = (__mmask32)0xAAAAAAAA;
    __mmask32 final_mask = _kor_mask32(cmp_mask, imm_mask);
    
    // Create dependency on previous mask
    __mmask32 dep_mask = (__mmask32)(0x55555555 * ((iteration >> 1) & 1));
    return _kand_mask32(final_mask, dep_mask);
}

__mmask16 generate_complex_mask16_float(int iteration) {
    __m512 vec1 = _mm512_set1_ps(iteration * 1.0f);
    __m512 vec2 = _mm512_set1_ps(iteration * 2.0f);
    
    __mmask16 cmp_mask = _mm512_cmp_ps_mask(vec1, vec2, _CMP_LT_OQ);
    __mmask16 imm_mask = (__mmask16)0xAAAA;
    
    // Complex logical chain
    __mmask16 temp = _kor_mask16(cmp_mask, imm_mask);
    __mmask16 iter_mask = (__mmask16)(0x5555 * (iteration & 3));
    return _kxor_mask16(temp, iter_mask);
}

__mmask8 generate_complex_mask8_double(int iteration) {
    __m512d vec1 = _mm512_set1_pd(iteration * 1.0);
    __m512d vec2 = _mm512_set1_pd(iteration * 1.5);
    
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(vec1, vec2, _CMP_GT_OQ);
    __mmask8 imm_mask = (__mmask8)0xAA;
    
    // Nested logical operations
    __mmask8 temp1 = _kor_mask8(cmp_mask, imm_mask);
    __mmask8 iter_mask = (__mmask8)(0x55 * ((iteration >> 2) & 1));
    __mmask8 temp2 = _kand_mask8(temp1, iter_mask);
    
    return _kxor_mask8(temp2, (__mmask8)0xFF);
}

// Reduction functions to prevent optimization
uint64_t reduce_v64qi(__m512i v) {
    uint64_t sum = 0;
    alignas(64) uint8_t data[64];
    _mm512_storeu_si512(data, v);
    for (int i = 0; i < 64; i++) {
        sum += data[i];
    }
    return sum;
}

uint64_t reduce_v32hi(__m512i v) {
    uint64_t sum = 0;
    alignas(64) uint16_t data[32];
    _mm512_storeu_si512(data, v);
    for (int i = 0; i < 32; i++) {
        sum += data[i];
    }
    return sum;
}

float reduce_v16sf(__m512 v) {
    float sum = 0.0f;
    alignas(64) float data[16];
    _mm512_storeu_ps(data, v);
    for (int i = 0; i < 16; i++) {
        sum += data[i];
    }
    return sum;
}

double reduce_v8df(__m512d v) {
    double sum = 0.0;
    alignas(64) double data[8];
    _mm512_storeu_pd(data, v);
    for (int i = 0; i < 8; i++) {
        sum += data[i];
    }
    return sum;
}

int main() {
    uint64_t total_checksum = 0;
    
    // Initialize test vectors with patterned data
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.5f,14.0f,13.5f,13.0f,12.5f,12.0f,11.5f,
        11.0f,10.5f,10.0f,9.5f,9.0f,8.5f,8.0f,7.5f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        0.0f,0.5f,1.0f,1.5f,2.0f,2.5f,3.0f,3.5f,
        4.0f,4.5f,5.0f,5.5f,6.0f,6.5f,7.0f,7.5f
    );
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    
    // Initialize half-precision vectors if supported
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_setzero_ph();
    __m512h v32hf_b = _mm512_set1_ph((_Float16)1.0);
    
    __m512bh v32bf_a = _mm512_setzero_bh();
    __m512bh v32bf_b = _mm512_set1_bh((__bf16)1.0);
    #endif
    
    // Control flow with loop and switch
    for (int iter = 0; iter < 4; iter++) {
        // Generate masks with different methods based on iteration
        __mmask64 mask64;
        __mmask32 mask32;
        __mmask16 mask16;
        __mmask8 mask8;
        
        switch (iter % 3) {
            case 0:
                // Use comparison-based masks
                mask64 = generate_complex_mask64(iter);
                mask32 = generate_complex_mask32(iter);
                mask16 = generate_complex_mask16_float(iter);
                mask8 = generate_complex_mask8_double(iter);
                break;
                
            case 1:
                // Use immediate masks
                mask64 = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
                mask32 = (__mmask32)0x55555555;
                mask16 = (__mmask16)0xAAAA;
                mask8 = (__mmask8)0x55;
                break;
                
            case 2:
                // Use logical combinations
                mask64 = _kor_mask64(
                    (__mmask64)0xAAAAAAAAAAAAAAAAULL,
                    (__mmask64)(0x5555555555555555ULL * (iter & 1))
                );
                mask32 = _kand_mask32(
                    (__mmask32)0xFFFFFFFF,
                    (__mmask32)(0xAAAAAAAA >> (iter & 3))
                );
                mask16 = _kxor_mask16(
                    (__mmask16)0xFFFF,
                    (__mmask16)(0xAAAA * ((iter >> 1) & 1))
                );
                mask8 = _kor_mask8(
                    (__mmask8)0xFF,
                    (__mmask8)(0xAA * (iter & 1))
                );
                break;
        }
        
        // Data dependency chain across different modes
        // Start with integer blend
        __m512i result_v64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use result to influence next mask
        __mmask32 derived_mask32 = _mm512_cmpeq_epi16_mask(
            result_v64qi,
            _mm512_set1_epi16(iter)
        );
        
        // Blend with derived mask
        __m512i result_v32hi = blend_v32hi(v32hi_a, v32hi_b, derived_mask32);
        
        // Create dependency from integer to float
        __mmask16 float_mask = (__mmask16)_mm512_cmpeq_epi32_mask(
            result_v32hi,
            _mm512_set1_epi32(iter * 1000)
        );
        
        // Perform float blend with dependent mask
        __m512 result_v16sf = blend_v16sf(v16sf_a, v16sf_b, float_mask);
        
        // Generate double mask from float result
        __m512d temp_double = _mm512_cvtps_pd(_mm512_castps512_ps256(result_v16sf));
        __mmask8 double_mask = _mm512_cmp_pd_mask(
            temp_double,
            _mm512_set1_pd(5.0),
            _CMP_GT_OQ
        );
        
        // Final double blend
        __m512d result_v8df = blend_v8df(v8df_a, v8df_b, double_mask);
        
        // Blend 64-bit integers
        __m512i result_v8di = blend_v8di(
            _mm512_set1_epi64(iter),
            _mm512_set1_epi64(iter * 2),
            double_mask  // Reuse mask for dependency
        );
        
        // Blend 32-bit integers
        __m512i result_v16si = blend_v16si(
            _mm512_set1_epi32(iter),
            _mm512_set1_epi32(iter * 3),
            float_mask  // Reuse mask for dependency
        );
        
        #ifdef __AVX512FP16__
        // Half-precision blends
        __m512h result_v32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
        __m512bh result_v32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
        #endif
        
        // Accumulate results to prevent optimization
        total_checksum += reduce_v64qi(result_v64qi);
        total_checksum += reduce_v32hi(result_v32hi);
        total_checksum += (uint64_t)reduce_v16sf(result_v16sf);
        total_checksum += (uint64_t)reduce_v8df(result_v8df);
        
        // Add integer reductions
        alignas(64) int64_t di_data[8];
        _mm512_storeu_si512(di_data, result_v8di);
        for (int i = 0; i < 8; i++) total_checksum += di_data[i];
        
        alignas(64) int32_t si_data[16];
        _mm512_storeu_si512(si_data, result_v16si);
        for (int i = 0; i < 16; i++) total_checksum += si_data[i];
    }
    
    // Complex if-else chain with mask dependencies
    int condition = total_checksum & 0xF;
    __mmask64 final_mask64;
    
    if (condition < 4) {
        final_mask64 = (__mmask64)0xFFFFFFFFFFFFFFFFULL;
    } else if (condition < 8) {
        final_mask64 = _mm512_cmpeq_epi8_mask(
            _mm512_set1_epi8(condition),
            _mm512_set1_epi8(condition * 2)
        );
    } else {
        __mmask64 temp1 = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
        __mmask64 temp2 = (__mmask64)0x5555555555555555ULL;
        final_mask64 = _kand_mask64(temp1, temp2);
    }
    
    // Final blend with complex control flow mask
    __m512i final_result = blend_v64qi(v64qi_a, v64qi_b, final_mask64);
    total_checksum += reduce_v64qi(final_result);
    
    printf("Final checksum: %lu\n", total_checksum);
    return 0;
}
