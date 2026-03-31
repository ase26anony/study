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

// Helper function to generate masks based on control flow
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask = 0;
    
    // Complex control flow to prevent optimization
    if (iteration % 3 == 0) {
        // Comparison-based mask
        __m512i zero = _mm512_setzero_si512();
        mask = _mm512_cmpeq_epi8_mask(data, zero);
    } else if (iteration % 3 == 1) {
        // Immediate mask with pattern
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Logical combination of masks
        __m512i pattern1 = _mm512_set1_epi8(0x55);
        __m512i pattern2 = _mm512_set1_epi8(0xAA);
        __mmask64 mask1 = _mm512_cmpeq_epi8_mask(data, pattern1);
        __mmask64 mask2 = _mm512_cmpeq_epi8_mask(data, pattern2);
        mask = _kor_mask64(mask1, mask2);
    }
    
    return mask;
}

__mmask32 generate_complex_mask32(int iteration, __m512i data) {
    __mmask32 mask = 0;
    
    switch (iteration % 4) {
        case 0:
            mask = 0xAAAAAAAA;  // Immediate
            break;
        case 1:
            mask = _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(0));
            break;
        case 2:
            mask = _mm512_cmplt_epi16_mask(data, _mm512_set1_epi16(100));
            break;
        case 3:
            __mmask32 m1 = _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(42));
            __mmask32 m2 = _mm512_cmpgt_epi16_mask(data, _mm512_set1_epi16(10));
            mask = _kxor_mask32(m1, m2);
            break;
    }
    
    return mask;
}

__mmask16 generate_complex_mask16_float(int iteration, __m512 data) {
    __mmask16 mask = 0;
    
    if (iteration < 5) {
        mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.0f), _CMP_GT_OQ);
    } else if (iteration < 10) {
        mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(1.0f), _CMP_LT_OQ);
    } else {
        __mmask16 m1 = _mm512_cmp_ps_mask(data, _mm512_set1_ps(0.5f), _CMP_GE_OQ);
        __mmask16 m2 = _mm512_cmp_ps_mask(data, _mm512_set1_ps(1.5f), _CMP_LE_OQ);
        mask = _kand_mask16(m1, m2);
    }
    
    return mask;
}

__mmask8 generate_complex_mask8_double(int iteration, __m512d data) {
    __mmask8 mask = 0;
    
    // Loop-dependent mask generation
    for (int i = 0; i < iteration % 3; i++) {
        mask ^= (1 << i);
    }
    
    // Combine with comparison mask
    __mmask8 cmp_mask = _mm512_cmp_pd_mask(data, _mm512_set1_pd(0.0), _CMP_NEQ_OQ);
    mask = _kor_mask8(mask, cmp_mask);
    
    return mask;
}

// Data dependency chain function
float process_data_dependency_chain(int seed) {
    float checksum = 0.0f;
    
    // Start with integer blend
    __m512i int_data1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i int_data2 = _mm512_set1_epi8(seed);
    __mmask64 mask64 = generate_complex_mask64(seed, int_data1);
    __m512i blended_int = blend_v64qi(int_data1, int_data2, mask64);
    
    // Use integer result to generate mask for float blend
    __m512 float_data1 = _mm512_set1_ps(1.0f);
    __m512 float_data2 = _mm512_set1_ps(2.0f);
    
    // Convert integer comparison to float mask
    __m512i zero = _mm512_setzero_si512();
    __mmask16 int_cmp_mask = _mm512_cmpeq_epi32_mask(blended_int, zero);
    __m512 blended_float = blend_v16sf(float_data1, float_data2, int_cmp_mask);
    
    // Horizontal sum to prevent optimization
    __m256 low = _mm512_castps512_ps256(blended_float);
    __m256 high = _mm512_extractf32x8_ps(blended_float, 1);
    __m256 sum8 = _mm256_add_ps(low, high);
    
    __m128 sum4 = _mm_add_ps(_mm256_castps256_ps128(sum8),
                            _mm256_extractf128_ps(sum8, 1));
    sum4 = _mm_add_ps(sum4, _mm_movehl_ps(sum4, sum4));
    sum4 = _mm_add_ss(sum4, _mm_movehdup_ps(sum4));
    
    _mm_store_ss(&checksum, sum4);
    
    return checksum;
}

int main() {
    uint64_t final_checksum = 0;
    
    // Initialize test data
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(0xFF);
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    __m512i v32hi_b = _mm512_set1_epi16(0xFFFF);
    __m512i v16si_a = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i v16si_b = _mm512_set1_epi32(0xFFFFFFFF);
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(0xFFFFFFFFFFFFFFFF);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    __m512 v16sf_b = _mm512_set1_ps(100.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set1_pd(50.0);
    
    // Loop with control flow to prevent optimization
    for (int i = 0; i < 10; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_complex_mask64(i, v64qi_a);
        __mmask32 mask32 = generate_complex_mask32(i, v32hi_a);
        __mmask16 mask16_int = _mm512_cmpeq_epi32_mask(v16si_a, _mm512_set1_epi32(i));
        __mmask8 mask8_int = _mm512_cmpeq_epi64_mask(v8di_a, _mm512_set1_epi64(i));
        __mmask16 mask16_float = generate_complex_mask16_float(i, v16sf_a);
        __mmask8 mask8_double = generate_complex_mask8_double(i, v8df_a);
        
        // Perform blends for all modes
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16_int);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8_int);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_float);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8_double);
        
        // Data dependency chain
        float chain_result = process_data_dependency_chain(i);
        
        // Accumulate results to prevent optimization
        // Use horizontal reductions
        __m512i sum64qi = _mm512_sad_epu8(result64qi, _mm512_setzero_si512());
        __m512i sum32hi = _mm512_madd_epi16(result32hi, _mm512_set1_epi16(1));
        __m512i sum16si = _mm512_add_epi32(result16si, _mm512_setzero_si512());
        __m512i sum8di = _mm512_add_epi64(result8di, _mm512_setzero_si512());
        
        // Reduce vectors to scalars
        uint64_t sum1 = _mm512_reduce_add_epi64(sum64qi);
        uint64_t sum2 = _mm512_reduce_add_epi32(sum32hi);
        uint64_t sum3 = _mm512_reduce_add_epi32(sum16si);
        uint64_t sum4 = _mm512_reduce_add_epi64(sum8di);
        
        // Float reductions
        __m256 low_f = _mm512_castps512_ps256(result16sf);
        __m256 high_f = _mm512_extractf32x8_ps(result16sf, 1);
        __m256 sum8f = _mm256_add_ps(low_f, high_f);
        float sum_f = 0.0f;
        for (int j = 0; j < 8; j++) {
            sum_f += ((float*)&sum8f)[j];
        }
        
        // Double reduction
        __m256d low_d = _mm512_castpd512_pd256(result8df);
        __m256d high_d = _mm512_extractf64x4_pd(result8df, 1);
        __m256d sum4d = _mm256_add_pd(low_d, high_d);
        double sum_d = 0.0;
        for (int j = 0; j < 4; j++) {
            sum_d += ((double*)&sum4d)[j];
        }
        
        final_checksum += (uint64_t)(sum1 + sum2 + sum3 + sum4 + 
                                    (uint64_t)sum_f + (uint64_t)sum_d + 
                                    (uint64_t)chain_result);
        
        // Modify inputs for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.1f));
    }
    
#ifdef __AVX512FP16__
    // Test half-precision blends if supported
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_castsi512_bh(_mm512_set1_epi16(0x3C00)); // 1.0 in BF16
    __m512bh v32bf_b = _mm512_castsi512_bh(_mm512_set1_epi16(0x4000)); // 2.0 in BF16
    
    __mmask32 mask_half = 0xAAAAAAAA;
    
    __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask_half);
    __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask_half);
    
    // Add to checksum
    __m256i half_sum = _mm512_castph_si256(_mm512_castph512_ph256(result32hf));
    final_checksum += _mm256_extract_epi64(half_sum, 0);
#endif
    
    printf("Final checksum: %lu\n", final_checksum);
    return 0;
}
