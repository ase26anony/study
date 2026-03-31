#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    // Generate masks using different methods
    __mmask64 mask64;
    if (mode_selector & 1) {
        // Immediate constant mask
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Dynamic mask from comparison
        __m512i ones = _mm512_set1_epi8(1);
        __m512i cmp = _mm512_add_epi8(a, ones);
        mask64 = _mm512_cmpeq_epi8_mask(cmp, _mm512_set1_epi8(2));
    }
    
    __m512i result = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Chain to epi16 blend
    __mmask32 mask32;
    if (mode_selector & 2) {
        mask32 = 0x55555555;
    } else {
        __m512i cmp16 = _mm512_cmpgt_epi16_mask(result, _mm512_set1_epi16(0));
        mask32 = _knot_mask32(cmp16);
    }
    
    return _mm512_mask_blend_epi16(mask32, result, _mm512_slli_epi16(b, 1));
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    // Generate mask using comparison
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OS);
    
    if (selector > 0) {
        // Modify mask with bitwise operations
        __mmask16 mask_const = 0xAAAA;
        mask16 = _kor_mask16(mask16, mask_const);
    }
    
    __m512 result_f = _mm512_mask_blend_ps(mask16, a, b);
    
    // Chain to double precision blend
    __mmask8 mask8;
    if (selector & 1) {
        mask8 = 0xAA;
    } else {
        mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result_f), 
                                  _mm512_castps_pd(b), _CMP_GT_OS);
    }
    
    __m512d result_d = _mm512_mask_blend_pd(mask8, c, d);
    return _mm512_castpd_ps(result_d);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int pattern) {
    __mmask32 mask32;
    
    // Different mask generation patterns
    switch (pattern % 3) {
        case 0:
            mask32 = 0xAAAAAAAA;  // Immediate constant
            break;
        case 1:
            mask32 = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
            break;
        case 2:
            __mmask32 temp_mask = _mm512_cmp_ph_mask(a, _mm512_set1_ph(0.0f), _CMP_GT_OQ);
            mask32 = _knot_mask32(temp_mask);  // Bitwise operation
            break;
    }
    
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int counter) {
    // Data-dependent control flow
    __mmask16 mask16;
    __mmask8 mask8;
    
    if (counter % 2 == 0) {
        mask16 = _mm512_cmpeq_epi32_mask(a, _mm512_set1_epi32(counter));
        mask8 = 0x55;
    } else {
        mask16 = 0xAAAA;
        mask8 = _mm512_cmpgt_epi64_mask(c, d);
    }
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16, a, b);
    __m512i result_di = _mm512_mask_blend_epi64(mask8, c, d);
    
    // Mix results
    return _mm512_add_epi32(result_si, _mm512_castsi512_si512(result_di));
}

// Reduction helpers
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256_lo = _mm512_castsi512_si256(v);
    __m256i v256_hi = _mm512_extracti64x4_epi64(v, 1);
    __m256i sum256 = _mm256_add_epi64(v256_lo, v256_hi);
    
    __m128i v128_lo = _mm256_castsi256_si128(sum256);
    __m128i v128_hi = _mm256_extracti128_si256(sum256, 1);
    __m128i sum128 = _mm_add_epi64(v128_lo, v128_hi);
    
    return (int64_t)_mm_extract_epi64(sum128, 0) + 
           (int64_t)_mm_extract_epi64(sum128, 1);
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

static inline double reduce_add_pd(__m512d v) {
    __m256d v256_lo = _mm512_castpd512_pd256(v);
    __m256d v256_hi = _mm512_extractf64x4_pd(v, 1);
    __m256d sum256 = _mm256_add_pd(v256_lo, v256_hi);
    
    __m128d v128_lo = _mm256_castpd256_pd128(sum256);
    __m128d v128_hi = _mm256_extractf128_pd(sum256, 1);
    __m128d sum128 = _mm_add_pd(v128_lo, v128_hi);
    
    sum128 = _mm_hadd_pd(sum128, sum128);
    
    return _mm_cvtsd_f64(sum128);
}

int main() {
    int64_t final_checksum = 0;
    
    // Initialize patterned data
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
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a, v32hf_b;
    _Float16 hf_data_a[32], hf_data_b[32];
    for (int i = 0; i < 32; i++) {
        hf_data_a[i] = (_Float16)(31 - i);
        hf_data_b[i] = (_Float16)i;
    }
    memcpy(&v32hf_a, hf_data_a, sizeof(v32hf_a));
    memcpy(&v32hf_b, hf_data_b, sizeof(v32hf_b));
#endif
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        // Call helper functions with different parameters
        __m512i result1 = blend_v64qi_v32hi(v64qi_a, v64qi_b, i);
        __m512 result2 = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, i);
        __m512i result3 = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, i);
        
        // Direct intrinsic calls for uncovered modes
        __mmask64 mask64_direct = (i % 2) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
        __m512i direct_epi8 = _mm512_mask_blend_epi8(mask64_direct, v64qi_a, v64qi_b);
        
        __mmask32 mask32_direct = (i < 5) ? 0xAAAAAAAA : 0x55555555;
        __m512i direct_epi16 = _mm512_mask_blend_epi16(mask32_direct, v32hi_a, v32hi_b);
        
        __mmask16 mask16_direct = _mm512_cmp_epi32_mask(v16si_a, v16si_b, _MM_CMPINT_LT);
        __m512i direct_epi32 = _mm512_mask_blend_epi32(mask16_direct, v16si_a, v16si_b);
        
        __mmask8 mask8_direct = _mm512_cmp_epi64_mask(v8di_a, v8di_b, _MM_CMPINT_GT);
        __m512i direct_epi64 = _mm512_mask_blend_epi64(mask8_direct, v8di_a, v8di_b);
        
        __mmask16 mask_ps_direct = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OS);
        __m512 direct_ps = _mm512_mask_blend_ps(mask_ps_direct, v16sf_a, v16sf_b);
        
        __mmask8 mask_pd_direct = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_GT_OS);
        __m512d direct_pd = _mm512_mask_blend_pd(mask_pd_direct, v8df_a, v8df_b);
        
#ifdef __AVX512FP16__
        __m512h direct_ph = blend_v32hf_v32bf(v32hf_a, v32hf_b, i);
        
        // Reduce FP16 result (convert to float for reduction)
        float ph_sum = 0;
        _Float16 ph_elements[32];
        memcpy(ph_elements, &direct_ph, sizeof(direct_ph));
        for (int j = 0; j < 32; j++) {
            ph_sum += (float)ph_elements[j];
        }
        final_checksum += (int64_t)ph_sum;
#endif
        
        // Reduce results to prevent dead code elimination
        final_checksum += reduce_add_epi64(result1);
        final_checksum += (int64_t)reduce_add_ps(result2);
        final_checksum += reduce_add_epi64(result3);
        final_checksum += reduce_add_epi64(direct_epi8);
        final_checksum += reduce_add_epi64(direct_epi16);
        final_checksum += reduce_add_epi64(direct_epi32);
        final_checksum += reduce_add_epi64(direct_epi64);
        final_checksum += (int64_t)reduce_add_ps(direct_ps);
        final_checksum += (int64_t)reduce_add_pd(direct_pd);
        
        // Modify inputs for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(1.0f));
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    return 0;
}
