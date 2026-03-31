#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend modes
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    // V64QImode blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    if (mode_selector & 1) {
        mask64 = ~mask64;  // Alternate mask pattern
    }
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with mask from comparison
    __m512i cmp_val = _mm512_set1_epi16(100);
    __mmask32 mask32 = _mm512_cmp_epi16_mask(result1, cmp_val, _MM_CMPINT_LT);
    __m512i result2 = _mm512_mask_blend_epi16(mask32, result1, b);
    
    return result2;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    // V16SFmode blend with dynamic mask
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    
    if (selector > 0) {
        mask16 = _kor_mask16(mask16, 0xAAAA);  // Combine masks
    }
    
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend
    __m512d cmp_val_d = _mm512_set1_pd(1.0);
    __mmask8 mask8 = _mm512_cmp_pd_mask(c, cmp_val_d, _CMP_LT_OQ);
    __m512d result2 = _mm512_mask_blend_pd(mask8, c, d);
    
    // Convert double result to float for return
    return _mm512_add_ps(result1, _mm512_castpd_ps(result2));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    // V32HFmode blend
    __mmask32 mask32;
    if (pattern == 0) {
        mask32 = 0x55555555;  // Alternating pattern
    } else {
        mask32 = 0xAAAAAAAA;  // Opposite pattern
    }
    
    __m512h result = _mm512_mask_blend_ph(mask32, a, b);
    
    // For V32BFmode (same intrinsic, different type)
    // Use different mask pattern
    mask32 = _knot_mask32(mask32);
    result = _mm512_mask_blend_ph(mask32, result, a);
    
    return result;
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int counter) {
    // V16SImode blend with mask from bitwise operations
    __mmask16 mask16 = 0xAAAA;
    if (counter % 3 == 0) {
        mask16 = _knot_mask16(mask16);
    } else if (counter % 3 == 1) {
        mask16 = _kor_mask16(mask16, 0x5555);
    }
    
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend
    __mmask8 mask8 = 0xAA;
    if (counter % 2 == 0) {
        __m512i cmp_val = _mm512_set1_epi64(1000);
        mask8 = _mm512_cmp_epi64_mask(result1, cmp_val, _MM_CMPINT_GT);
    }
    
    __m512i result2 = _mm512_mask_blend_epi64(mask8, c, d);
    
    return _mm512_add_epi32(result1, _mm512_castsi512_si512(result2));
}

// Function with data-dependent control flow
static inline __m512i conditional_blend(int selector, __m512i a, __m512i b, __m512i c) {
    __m512i result;
    
    if (selector < 10) {
        // Use V64QImode blend
        __mmask64 mask64 = (selector % 2 == 0) ? 0xAAAAAAAAAAAAAAAAULL : 0x5555555555555555ULL;
        result = _mm512_mask_blend_epi8(mask64, a, b);
    } else if (selector < 20) {
        // Use V32HImode blend
        __mmask32 mask32 = _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_EQ);
        result = _mm512_mask_blend_epi16(mask32, a, c);
    } else {
        // Use V16SImode blend
        __mmask16 mask16 = 0xAAAA;
        for (int i = 0; i < (selector % 4); i++) {
            mask16 = _kor_mask16(mask16, 1 << (i * 2));
        }
        result = _mm512_mask_blend_epi32(mask16, b, c);
    }
    
    return result;
}

// Reduction helpers
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256_lo = _mm512_castsi512_si256(v);
    __m256i v256_hi = _mm512_extracti64x4_epi64(v, 1);
    __m256i sum256 = _mm256_add_epi64(v256_lo, v256_hi);
    
    __m128i v128_lo = _mm256_castsi256_si128(sum256);
    __m128i v128_hi = _mm256_extracti128_si256(sum256, 1);
    __m128i sum128 = _mm_add_epi64(v128_lo, v128_hi);
    
    return (int64_t)_mm_extract_epi64(sum128, 0) + (int64_t)_mm_extract_epi64(sum128, 1);
}

static inline float reduce_add_ps(__m512 v) {
    __m256 v256_lo = _mm512_castps512_ps256(v);
    __m256 v256_hi = _mm512_extractf32x8_ps(v, 1);
    __m256 sum256 = _mm256_add_ps(v256_lo, v256_hi);
    
    __m128 v128_lo = _mm256_castps256_ps128(sum256);
    __m128 v128_hi = _mm256_extractf128_ps(sum256, 1);
    __m128 sum128 = _mm_add_ps(v128_lo, v128_hi);
    
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    
    return _mm_cvtss_f32(sum128);
}

int main() {
    // Initialize patterned data
    __m512i int_data1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i int_data2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i int_data3 = _mm512_set1_epi8(100);
    __m512i int_data4 = _mm512_set1_epi16(200);
    
    // Float data
    float float_array[16];
    double double_array[8];
    for (int i = 0; i < 16; i++) {
        float_array[i] = (i % 2 == 0) ? i * 0.1f : -i * 0.1f;
    }
    for (int i = 0; i < 8; i++) {
        double_array[i] = (i % 3 == 0) ? i * 0.2 : -i * 0.2;
    }
    
    __m512 float_data1 = _mm512_loadu_ps(float_array);
    __m512 float_data2 = _mm512_set1_ps(0.5f);
    __m512d double_data1 = _mm512_loadu_pd(double_array);
    __m512d double_data2 = _mm512_set1_pd(1.0);
    
    // Accumulator for checksum
    int64_t checksum = 0;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 30; i++) {
        __m512i blended_int = conditional_blend(i, int_data1, int_data2, int_data3);
        
        // Chain blends of different types
        __m512i chain_result = blend_v64qi_v32hi(blended_int, int_data4, i);
        chain_result = blend_v16si_v8di(chain_result, int_data2, int_data3, int_data1, i);
        
        // Add to checksum
        checksum += reduce_add_epi64(chain_result);
        
        // Blend floats
        __m512 blended_float = blend_v16sf_v8df(float_data1, float_data2, 
                                               double_data1, double_data2, i);
        checksum += (int64_t)reduce_add_ps(blended_float);
    }
    
    // Additional direct blends for each mode
    // V64QImode
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i v64qi_result = _mm512_mask_blend_epi8(mask64, int_data1, int_data2);
    checksum += reduce_add_epi64(v64qi_result);
    
    // V32HImode
    __mmask32 mask32 = _mm512_cmp_epi16_mask(int_data1, int_data2, _MM_CMPINT_GT);
    __m512i v32hi_result = _mm512_mask_blend_epi16(mask32, int_data1, int_data3);
    checksum += reduce_add_epi64(v32hi_result);
    
    // V16SImode
    __mmask16 mask16 = 0xAAAA;
    __m512i v16si_result = _mm512_mask_blend_epi32(mask16, int_data1, int_data2);
    checksum += reduce_add_epi64(v16si_result);
    
    // V8DImode
    __mmask8 mask8 = 0xAA;
    __m512i v8di_result = _mm512_mask_blend_epi64(mask8, int_data1, int_data2);
    checksum += reduce_add_epi64(v8di_result);
    
    // V16SFmode
    mask16 = _mm512_cmp_ps_mask(float_data1, float_data2, _CMP_GT_OQ);
    __m512 v16sf_result = _mm512_mask_blend_ps(mask16, float_data1, float_data2);
    checksum += (int64_t)reduce_add_ps(v16sf_result);
    
    // V8DFmode
    mask8 = _mm512_cmp_pd_mask(double_data1, double_data2, _CMP_LT_OQ);
    __m512d v8df_result = _mm512_mask_blend_pd(mask8, double_data1, double_data2);
    __m512 v8df_as_float = _mm512_castpd_ps(v8df_result);
    checksum += (int64_t)reduce_add_ps(v8df_as_float);
    
#ifdef __AVX512FP16__
    // V32HFmode and V32BFmode
    __m512h half_data1 = _mm512_set1_ph(1.0f);
    __m512h half_data2 = _mm512_set1_ph(2.0f);
    
    for (int i = 0; i < 4; i++) {
        __m512h half_result = blend_v32hf_v32bf(half_data1, half_data2, i % 2);
        
        // Simple reduction for half precision
        __m256h half256_lo = _mm512_castph512_ph256(half_result);
        __m256h half256_hi = _mm512_extractf32x8_ph(half_result, 1);
        
        // Convert to float for checksum
        __m512 half_as_float_lo = _mm512_cvtph_ps(half256_lo);
        __m512 half_as_float_hi = _mm512_cvtph_ps(half256_hi);
        
        checksum += (int64_t)reduce_add_ps(half_as_float_lo);
        checksum += (int64_t)reduce_add_ps(half_as_float_hi);
    }
#endif
    
    printf("Final checksum: %ld\n", checksum);
    return 0;
}
