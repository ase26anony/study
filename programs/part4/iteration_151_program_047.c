#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend modes
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    // V64QImode blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamic mask based on comparison
    __m512i cmp_val = _mm512_set1_epi16(100);
    __mmask32 mask32 = _mm512_cmp_epi16_mask(result1, cmp_val, _MM_CMPINT_LT);
    
    if (mode_selector & 1) {
        mask32 = _knot_mask32(mask32);
    }
    
    return _mm512_mask_blend_epi16(mask32, a, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // V16SFmode blend with comparison mask
    __m512 cmp_val = _mm512_set1_ps(0.5f);
    __mmask16 mask16 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_LT_OS);
    
    // Alternate mask patterns based on iteration
    if (iter % 3 == 0) {
        mask16 = 0xAAAA;  // Immediate constant
    } else if (iter % 3 == 1) {
        mask16 = _mm512_cmp_ps_mask(b, cmp_val, _CMP_GT_OS);
    }
    
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // V8DFmode blend with chained result
    __m512d cmp_val_d = _mm512_set1_pd(1.0);
    __mmask8 mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result1), cmp_val_d, _CMP_LT_OS);
    
    return _mm512_castpd_ps(_mm512_mask_blend_pd(mask8, c, d));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d, int selector) {
    // V32HFmode blend
    __mmask32 mask32 = 0xAAAAAAAA;  // Immediate constant
    __m512h result1 = _mm512_mask_blend_ph(mask32, a, b);
    
    // V32BFmode blend with modified mask
    if (selector > 0) {
        __m512h cmp_val = _mm512_set1_ph(0.0f);
        mask32 = _mm512_cmp_ph_mask(result1, cmp_val, _CMP_GT_OS);
    }
    
    return _mm512_mask_blend_ph(mask32, c, d);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int use_complex_mask) {
    // V16SImode blend
    __mmask16 mask16;
    if (use_complex_mask) {
        __m512i cmp_val = _mm512_set1_epi32(0);
        mask16 = _mm512_cmp_epi32_mask(a, cmp_val, _MM_CMPINT_GT);
        mask16 = _kor_mask16(mask16, 0x5555);  // Combine with immediate
    } else {
        mask16 = 0xAAAA;  // Simple immediate
    }
    
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // V8DImode blend with chained result
    __m512i cmp_val_di = _mm512_set1_epi64(1000);
    __mmask8 mask8 = _mm512_cmp_epi64_mask(result1, cmp_val_di, _MM_CMPINT_LT);
    
    return _mm512_mask_blend_epi64(mask8, c, d);
}

// Reduction helpers
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256_lo = _mm512_extracti64x4_epi64(v, 0);
    __m256i v256_hi = _mm512_extracti64x4_epi64(v, 1);
    
    __m256i sum256 = _mm256_add_epi64(v256_lo, v256_hi);
    __m128i v128_lo = _mm256_extracti128_si256(sum256, 0);
    __m128i v128_hi = _mm256_extracti128_si256(sum256, 1);
    
    __m128i sum128 = _mm_add_epi64(v128_lo, v128_hi);
    return (int64_t)_mm_extract_epi64(sum128, 0) + (int64_t)_mm_extract_epi64(sum128, 1);
}

static inline float reduce_add_ps(__m512 v) {
    __m256 v256_lo = _mm512_extractf32x8_ps(v, 0);
    __m256 v256_hi = _mm512_extractf32x8_ps(v, 1);
    
    __m256 sum256 = _mm256_add_ps(v256_lo, v256_hi);
    __m128 v128_lo = _mm256_extractf128_ps(sum256, 0);
    __m128 v128_hi = _mm256_extractf128_ps(sum256, 1);
    
    __m128 sum128 = _mm_add_ps(v128_lo, v128_hi);
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    
    return _mm_cvtss_f32(sum128);
}

#ifdef __AVX512FP16__
static inline float reduce_add_ph(__m512h v) {
    // Simple reduction for FP16
    float sum = 0.0f;
    _Float16 temp[32];
    _mm512_storeu_ph(temp, v);
    
    for (int i = 0; i < 32; i++) {
        sum += (float)temp[i];
    }
    return sum;
}
#endif

int main() {
    int64_t final_checksum = 0;
    
    // Initialize data with distinct patterns
    int8_t data_i8[64];
    int16_t data_i16[32];
    int32_t data_i32[16];
    int64_t data_i64[8];
    float data_f32[16];
    double data_f64[8];
    
    for (int i = 0; i < 64; i++) data_i8[i] = (i % 3) - 1;
    for (int i = 0; i < 32; i++) data_i16[i] = (i * 7) % 100;
    for (int i = 0; i < 16; i++) data_i32[i] = i * 100 - 500;
    for (int i = 0; i < 8; i++) data_i64[i] = i * 1000 - 4000;
    for (int i = 0; i < 16; i++) data_f32[i] = (i - 8) * 0.125f;
    for (int i = 0; i < 8; i++) data_f64[i] = (i - 4) * 0.25;
    
    __m512i vec_i8_a = _mm512_loadu_si512(data_i8);
    __m512i vec_i8_b = _mm512_loadu_si512(data_i8 + 32);
    __m512i vec_i16_a = _mm512_loadu_si512(data_i16);
    __m512i vec_i16_b = _mm512_loadu_si512(data_i16 + 16);
    __m512i vec_i32_a = _mm512_loadu_si512(data_i32);
    __m512i vec_i32_b = _mm512_loadu_si512(data_i32 + 8);
    __m512i vec_i64_a = _mm512_loadu_si512(data_i64);
    __m512i vec_i64_b = _mm512_loadu_si512(data_i64 + 4);
    __m512 vec_f32_a = _mm512_loadu_ps(data_f32);
    __m512 vec_f32_b = _mm512_loadu_ps(data_f32 + 8);
    __m512d vec_f64_a = _mm512_loadu_pd(data_f64);
    __m512d vec_f64_b = _mm512_loadu_pd(data_f64 + 4);
    
    // Loop with data-dependent control flow
    for (int iter = 0; iter < 10; iter++) {
        // Control flow determines which blends to perform
        if (iter % 2 == 0) {
            // Chain integer blends
            __m512i int_result = blend_v64qi_v32hi(vec_i8_a, vec_i8_b, iter);
            int_result = blend_v16si_v8di(int_result, vec_i32_b, vec_i64_a, vec_i64_b, iter % 3);
            final_checksum += reduce_add_epi64(int_result);
        } else {
            // Chain float blends
            __m512 float_result = blend_v16sf_v8df(vec_f32_a, vec_f32_b, vec_f64_a, vec_f64_b, iter);
            final_checksum += (int64_t)reduce_add_ps(float_result);
        }
        
        // Direct intrinsic calls for all modes
        __mmask64 mask64_direct = (iter % 5 == 0) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
        __m512i blend_epi8 = _mm512_mask_blend_epi8(mask64_direct, vec_i8_a, vec_i8_b);
        final_checksum += reduce_add_epi64(blend_epi8);
        
        __mmask32 mask32_direct = _mm512_cmp_epi16_mask(vec_i16_a, vec_i16_b, _MM_CMPINT_NE);
        __m512i blend_epi16 = _mm512_mask_blend_epi16(mask32_direct, vec_i16_a, vec_i16_b);
        final_checksum += reduce_add_epi64(blend_epi16);
        
        __mmask16 mask16_direct = (__mmask16)(0xAAAA + iter);
        __m512i blend_epi32 = _mm512_mask_blend_epi32(mask16_direct, vec_i32_a, vec_i32_b);
        final_checksum += reduce_add_epi64(blend_epi32);
        
        __mmask8 mask8_direct = _mm512_cmp_epi64_mask(vec_i64_a, vec_i64_b, _MM_CMPINT_LT);
        __m512i blend_epi64 = _mm512_mask_blend_epi64(mask8_direct, vec_i64_a, vec_i64_b);
        final_checksum += reduce_add_epi64(blend_epi64);
        
        __mmask16 mask_ps = _mm512_cmp_ps_mask(vec_f32_a, vec_f32_b, _CMP_LT_OS);
        __m512 blend_ps = _mm512_mask_blend_ps(mask_ps, vec_f32_a, vec_f32_b);
        final_checksum += (int64_t)reduce_add_ps(blend_ps);
        
        __mmask8 mask_pd = (iter % 4 == 0) ? 0xAA : 0x55;
        __m512d blend_pd = _mm512_mask_blend_pd(mask_pd, vec_f64_a, vec_f64_b);
        __m512 blend_pd_as_ps = _mm512_castpd_ps(blend_pd);
        final_checksum += (int64_t)reduce_add_ps(blend_pd_as_ps);
        
        #ifdef __AVX512FP16__
        // FP16 data
        _Float16 data_f16[32];
        for (int i = 0; i < 32; i++) data_f16[i] = (i - 16) * 0.0625f;
        
        __m512h vec_f16_a = _mm512_loadu_ph(data_f16);
        __m512h vec_f16_b = _mm512_loadu_ph(data_f16 + 16);
        
        __mmask32 mask_ph = 0x55555555 + iter;
        __m512h blend_ph = _mm512_mask_blend_ph(mask_ph, vec_f16_a, vec_f16_b);
        final_checksum += (int64_t)reduce_add_ph(blend_ph);
        #endif
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    return 0;
}
