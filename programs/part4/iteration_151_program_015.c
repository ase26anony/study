#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    // V64QImode blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamic mask
    __mmask32 mask32;
    if (mode_selector & 1) {
        mask32 = _mm512_cmpeq_epi16_mask(result1, _mm512_setzero_si512());
    } else {
        mask32 = 0x55555555;
    }
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // V16SFmode blend with comparison mask
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OS);
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend with chained mask operations
    __mmask8 mask8 = 0xAA;
    if (iter % 3 == 0) {
        mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result1), d, _CMP_GT_OS);
    }
    __m512d result2 = _mm512_mask_blend_pd(mask8, c, d);
    
    // Chain back to float
    return _mm512_add_ps(result1, _mm512_castpd_ps(result2));
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int selector) {
    // V16SImode blend with immediate mask
    __mmask16 mask16 = 0xAAAA;
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend with conditional mask
    __mmask8 mask8;
    if (selector > 0) {
        mask8 = _mm512_cmpgt_epi64_mask(c, d);
        mask8 = _knot_mask8(mask8); // Test mask negation
    } else {
        mask8 = 0x55;
    }
    __m512i result2 = _mm512_mask_blend_epi64(mask8, c, d);
    
    return _mm512_add_epi32(result1, _mm512_castsi512_si512(result2));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d) {
    // V32HFmode blend with alternating pattern
    __mmask32 mask32 = 0xAAAAAAAA;
    __m512h result1 = _mm512_mask_blend_ph(mask32, a, b);
    
    // V32BFmode blend with comparison mask
    __mmask32 mask32_bf = _mm512_cmp_ph_mask(result1, c, _CMP_NEQ_UQ);
    __m512h result2 = _mm512_mask_blend_ph(mask32_bf, result1, d);
    
    return _mm512_add_ph(result2, c);
}
#endif

// Reduction helpers
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256 = _mm512_extracti64x4_epi64(v, 0);
    __m256i v256_1 = _mm512_extracti64x4_epi64(v, 1);
    v256 = _mm256_add_epi64(v256, v256_1);
    
    __m128i v128 = _mm256_extracti128_si256(v256, 0);
    __m128i v128_1 = _mm256_extracti128_si256(v256, 1);
    v128 = _mm_add_epi64(v128, v128_1);
    
    return (int64_t)_mm_extract_epi64(v128, 0) + (int64_t)_mm_extract_epi64(v128, 1);
}

static inline float reduce_add_ps(__m512 v) {
    __m256 v256 = _mm512_extractf32x8_ps(v, 0);
    __m256 v256_1 = _mm512_extractf32x8_ps(v, 1);
    v256 = _mm256_add_ps(v256, v256_1);
    
    __m128 v128 = _mm256_extractf128_ps(v256, 0);
    __m128 v128_1 = _mm256_extractf128_ps(v256, 1);
    v128 = _mm_add_ps(v128, v128_1);
    
    v128 = _mm_hadd_ps(v128, v128);
    v128 = _mm_hadd_ps(v128, v128);
    
    return _mm_cvtss_f32(v128);
}

static inline double reduce_add_pd(__m512d v) {
    __m256d v256 = _mm512_extractf64x4_pd(v, 0);
    __m256d v256_1 = _mm512_extractf64x4_pd(v, 1);
    v256 = _mm256_add_pd(v256, v256_1);
    
    __m128d v128 = _mm256_extractf128_pd(v256, 0);
    __m128d v128_1 = _mm256_extractf128_pd(v256, 1);
    v128 = _mm_add_pd(v128, v128_1);
    
    return _mm_cvtsd_f64(_mm_hadd_pd(v128, v128));
}

#ifdef __AVX512FP16__
static inline _Float16 reduce_add_ph(__m512h v) {
    // Simple reduction for half precision
    _Float16 sum = 0;
    union {
        __m512h vec;
        _Float16 arr[32];
    } u = {v};
    
    for (int i = 0; i < 32; i++) {
        sum += u.arr[i];
    }
    return sum;
}
#endif

int main() {
    double checksum = 0.0;
    
    // Initialize data with distinct patterns
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
    
    __m512i int_data3 = _mm512_set_epi32(
        1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16
    );
    
    __m512i int_data4 = _mm512_set_epi32(
        16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1
    );
    
    __m512i int_data5 = _mm512_set_epi64(1,2,3,4,5,6,7,8);
    __m512i int_data6 = _mm512_set_epi64(8,7,6,5,4,3,2,1);
    
    __m512 float_data1 = _mm512_set_ps(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    );
    
    __m512 float_data2 = _mm512_set_ps(
        16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f
    );
    
    __m512d double_data1 = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d double_data2 = _mm512_set_pd(8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0);
    
#ifdef __AVX512FP16__
    __m512h half_data1 = _mm512_set_ph(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f,
        17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f, 24.0f,
        25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f, 32.0f
    );
    
    __m512h half_data2 = _mm512_set_ph(
        32.0f, 31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f,
        24.0f, 23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f,
        16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f
    );
#endif
    
    // Data-dependent control flow with loops
    for (int i = 0; i < 10; i++) {
        // Chain blends through different modes
        __m512i blended_int;
        
        if (i % 2 == 0) {
            // Use V64QImode and V32HImode blends
            blended_int = blend_v64qi_v32hi(int_data1, int_data2, i);
            
            // Direct V16SImode blend with complex mask
            __mmask16 mask16 = _mm512_cmpgt_epi32_mask(blended_int, int_data3);
            __m512i temp = _mm512_mask_blend_epi32(mask16, int_data3, int_data4);
            blended_int = _mm512_add_epi32(blended_int, temp);
        } else {
            // Use V16SImode and V8DImode blends
            blended_int = blend_v16si_v8di(int_data3, int_data4, int_data5, int_data6, i);
            
            // Direct V64QImode blend with immediate mask
            __mmask64 mask64 = 0xCCCCCCCCCCCCCCCCULL;
            __m512i temp = _mm512_mask_blend_epi8(mask64, int_data1, int_data2);
            blended_int = _mm512_add_epi32(blended_int, _mm512_castsi512_si512(temp));
        }
        
        // Float blends
        __m512 blended_float = blend_v16sf_v8df(float_data1, float_data2, 
                                               double_data1, double_data2, i);
        
        // Direct V8DFmode blend with comparison mask
        __mmask8 mask8 = _mm512_cmp_pd_mask(double_data1, double_data2, _CMP_LT_OS);
        __m512d blended_double = _mm512_mask_blend_pd(mask8, double_data1, double_data2);
        
#ifdef __AVX512FP16__
        // Half precision blends
        __m512h blended_half = blend_v32hf_v32bf(half_data1, half_data2, 
                                                half_data1, half_data2);
#endif
        
        // Accumulate reductions to prevent dead code elimination
        checksum += reduce_add_epi64(_mm512_castsi512_si512(blended_int));
        checksum += reduce_add_ps(blended_float);
        checksum += reduce_add_pd(blended_double);
        
#ifdef __AVX512FP16__
        checksum += reduce_add_ph(blended_half);
#endif
        
        // Modify data for next iteration
        int_data1 = _mm512_add_epi8(int_data1, _mm512_set1_epi8(1));
        float_data1 = _mm512_add_ps(float_data1, _mm512_set1_ps(0.5f));
        double_data1 = _mm512_add_pd(double_data1, _mm512_set1_pd(0.25));
        
#ifdef __AVX512FP16__
        half_data1 = _mm512_add_ph(half_data1, _mm512_set1_ph(0.125f));
#endif
    }
    
    // Additional direct blends with various mask patterns
    // Test all mask generation methods for each mode
    
    // V64QImode: immediate, dynamic, and combined masks
    __mmask64 mask64_imm = 0xF0F0F0F0F0F0F0F0ULL;
    __mmask64 mask64_dyn = _mm512_cmpeq_epi8_mask(int_data1, int_data2);
    __mmask64 mask64_comb = _kor_mask64(mask64_imm, mask64_dyn);
    
    __m512i blend64qi = _mm512_mask_blend_epi8(mask64_imm, int_data1, int_data2);
    blend64qi = _mm512_mask_blend_epi8(mask64_dyn, blend64qi, int_data2);
    blend64qi = _mm512_mask_blend_epi8(mask64_comb, blend64qi, int_data1);
    
    // V32HImode: immediate and comparison masks
    __mmask32 mask32_imm = 0xAAAAAAAA;
    __mmask32 mask32_cmp = _mm512_cmpgt_epi16_mask(int_data1, int_data2);
    __mmask32 mask32_not = _knot_mask32(mask32_cmp);
    
    __m512i blend32hi = _mm512_mask_blend_epi16(mask32_imm, int_data1, int_data2);
    blend32hi = _mm512_mask_blend_epi16(mask32_cmp, blend32hi, int_data1);
    blend32hi = _mm512_mask_blend_epi16(mask32_not, blend32hi, int_data2);
    
    // V16SImode: various mask patterns
    __mmask16 mask16_imm = 0xAAAA;
    __mmask16 mask16_cmp = _mm512_cmpeq_epi32_mask(int_data3, int_data4);
    
    __m512i blend16si = _mm512_mask_blend_epi32(mask16_imm, int_data3, int_data4);
    blend16si = _mm512_mask_blend_epi32(mask16_cmp, blend16si, int_data3);
    
    // V8DImode: immediate and dynamic masks
    __mmask8 mask8_imm = 0xAA;
    __mmask8 mask8_cmp = _mm512_cmpgt_epi64_mask(int_data5, int_data6);
    
    __m512i blend8di = _mm512_mask_blend_epi64(mask8_imm, int_data5, int_data6);
    blend8di = _mm512_mask_blend_epi64(mask8_cmp, blend8di, int_data5);
    
    // V16SFmode: comparison masks with different predicates
    __mmask16 mask16_lt = _mm512_cmp_ps_mask(float_data1, float_data2, _CMP_LT_OS);
    __mmask16 mask16_eq = _mm512_cmp_ps_mask(float_data1, float_data2, _CMP_EQ_OS);
    __mmask16 mask16_comb = _kor_mask16(mask16_lt, mask16_eq);
    
    __m512 blend16sf = _mm512_mask_blend_ps(mask16_lt, float_data1, float_data2);
    blend16sf = _mm512_mask_blend_ps(mask16_eq, blend16sf, float_data1);
    blend16sf = _mm512_mask_blend_ps(mask16_comb, blend16sf, float_data2);
    
    // V8DFmode: various comparison masks
    __mmask8 mask8_lt = _mm512_cmp_pd_mask(double_data1, double_data2, _CMP_LT_OS);
    __mmask8 mask8_gt = _mm512_cmp_pd_mask(double_data1, double_data2, _CMP_GT_OS);
    
    __m512d blend8df = _mm512_mask_blend_pd(mask8_lt, double_data1, double_data2);
    blend8df = _mm512_mask_blend_pd(mask8_gt, blend8df, double_data1);
    
#ifdef __AVX512FP16__
    // V32HFmode and V32BFmode: half precision blends
    __mmask32 mask32_hf = 0xCCCCCCCC;
    __mmask32 mask32_cmp_hf = _mm512_cmp_ph_mask(half_data1, half_data2, _CMP_LT_OS);
    
    __m512h blend32hf = _mm512_mask_blend_ph(mask32_hf, half_data1, half_data2);
    blend32hf = _mm512_mask_blend_ph(mask32_cmp_hf, blend32hf, half_data1);
#endif
    
    // Final accumulation
    checksum += reduce_add_epi64(blend64qi);
    checksum += reduce_add_epi64(blend32hi);
    checksum += reduce_add_epi64(blend16si);
    checksum += reduce_add_epi64(blend8di);
    checksum += reduce_add_ps(blend16sf);
    checksum += reduce_add_pd(blend8df);
    
#ifdef __AVX512FP16__
    checksum += reduce_add_ph(blend32hf);
#endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
