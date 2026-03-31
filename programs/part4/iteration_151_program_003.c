#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int selector) {
    /* V64QImode blend with varied mask generation */
    __mmask64 mask64;
    if (selector & 1) {
        // Method 1: Immediate constant mask
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Method 2: Dynamic mask from comparison
        __m512i cmp_a = _mm512_set1_epi8(selector);
        __m512i cmp_b = _mm512_set1_epi8(selector ^ 0xFF);
        mask64 = _mm512_cmpgt_epi8_mask(cmp_a, cmp_b);
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    /* V32HImode blend with chained operation */
    __mmask32 mask32;
    if (selector & 2) {
        // Method 3: Bitwise operation on existing mask
        mask32 = _knot_mask32((__mmask32)(mask64 & 0xFFFFFFFF));
    } else {
        // Method 4: Comparison with different values
        __m512i val = _mm512_set1_epi16(selector);
        mask32 = _mm512_cmpeq_epi16_mask(val, val);
    }
    
    return _mm512_mask_blend_epi16(mask32, result1, b);
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    /* V16SFmode blend */
    __mmask16 mask16;
    if (selector & 4) {
        mask16 = 0xAAAA;
    } else {
        __m512 cmp_val = _mm512_set1_ps((float)selector);
        mask16 = _mm512_cmp_ps_mask(a, cmp_val, _CMP_GT_OQ);
    }
    
    __m512 result_sf = _mm512_mask_blend_ps(mask16, a, b);
    
    /* V8DFmode blend with chained input */
    __mmask8 mask8;
    if (selector & 8) {
        mask8 = 0xAA;
    } else {
        // Create mask from comparison result
        __m512d cmp_val_d = _mm512_set1_pd((double)selector);
        mask8 = _mm512_cmp_pd_mask(c, cmp_val_d, _CMP_LT_OQ);
    }
    
    __m512d result_df = _mm512_mask_blend_pd(mask8, c, d);
    
    // Mix float and double results (convert double to float for mixing)
    __m512 df_as_float = _mm512_castpd_ps(result_df);
    return _mm512_add_ps(result_sf, df_as_float);
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int selector) {
    /* V16SImode blend */
    __mmask16 mask16_si;
    if (selector & 16) {
        mask16_si = 0x5555;
    } else {
        __m512i cmp_val = _mm512_set1_epi32(selector);
        mask16_si = _mm512_cmpgt_epi32_mask(cmp_val, _mm512_setzero_si512());
    }
    
    __m512i result_si = _mm512_mask_blend_epi32(mask16_si, a, b);
    
    /* V8DImode blend */
    __mmask8 mask8_di;
    if (selector & 32) {
        mask8_di = 0x55;
    } else {
        // Create mask from bitwise operation
        __mmask8 temp_mask = _mm512_cmpeq_epi64_mask(c, c);
        mask8_di = _knot_mask8(temp_mask);
    }
    
    __m512i result_di = _mm512_mask_blend_epi64(mask8_di, c, d);
    
    return _mm512_add_epi32(result_si, _mm512_castsi512_si512(result_di));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512h d, int selector) {
    /* V32HFmode blend */
    __mmask32 mask32_hf;
    if (selector & 64) {
        mask32_hf = 0xAAAAAAAA;
    } else {
        __m512h cmp_val = _mm512_set1_ph((_Float16)selector);
        mask32_hf = _mm512_cmp_ph_mask(a, cmp_val, _CMP_GT_OQ);
    }
    
    __m512h result_hf = _mm512_mask_blend_ph(mask32_hf, a, b);
    
    /* V32BFmode blend - reuse same intrinsic for bfloat16 */
    __mmask32 mask32_bf;
    if (selector & 128) {
        mask32_bf = 0x55555555;
    } else {
        // Create inverted mask
        mask32_bf = _knot_mask32(mask32_hf);
    }
    
    __m512h result_bf = _mm512_mask_blend_ph(mask32_bf, c, d);
    
    return _mm512_add_ph(result_hf, result_bf);
}
#endif

/* Reduction helpers to prevent dead code elimination */
static inline int64_t reduce_i64(__m512i v) {
    __m256i v_low = _mm512_castsi512_si256(v);
    __m256i v_high = _mm512_extracti64x4_epi64(v, 1);
    __m256i sum256 = _mm256_add_epi64(v_low, v_high);
    
    __m128i sum128 = _mm_add_epi64(
        _mm256_castsi256_si128(sum256),
        _mm256_extracti128_si256(sum256, 1)
    );
    
    return (int64_t)_mm_extract_epi64(sum128, 0) +
           (int64_t)_mm_extract_epi64(sum128, 1);
}

static inline float reduce_ps(__m512 v) {
    return _mm512_reduce_add_ps(v);
}

static inline double reduce_pd(__m512d v) {
    __m256d v_low = _mm512_castpd512_pd256(v);
    __m256d v_high = _mm512_extractf64x4_pd(v, 1);
    __m256d sum256 = _mm256_add_pd(v_low, v_high);
    
    __m128d sum128 = _mm_add_pd(
        _mm256_castpd256_pd128(sum256),
        _mm256_extractf128_pd(sum256, 1)
    );
    
    return _mm_cvtsd_f64(_mm_add_pd(sum128, _mm_unpackhi_pd(sum128, sum128)));
}

#ifdef __AVX512FP16__
static inline _Float16 reduce_ph(__m512h v) {
    // Manual reduction for _Float16
    _Float16 buffer[32];
    _mm512_storeu_ph(buffer, v);
    
    _Float16 sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += buffer[i];
    }
    return sum;
}
#endif

int main() {
    int64_t final_checksum = 0;
    
    /* Initialize patterned data for all vector types */
    // For integer vectors
    int8_t pattern_i8[64];
    int16_t pattern_i16[32];
    int32_t pattern_i32[16];
    int64_t pattern_i64[8];
    
    for (int i = 0; i < 64; i++) pattern_i8[i] = (i % 3) - 1;
    for (int i = 0; i < 32; i++) pattern_i16[i] = (i % 5) * 10;
    for (int i = 0; i < 16; i++) pattern_i32[i] = i * 100;
    for (int i = 0; i < 8; i++) pattern_i64[i] = i * 1000;
    
    __m512i vec_i8_a = _mm512_loadu_si512(pattern_i8);
    __m512i vec_i8_b = _mm512_loadu_si512((int8_t*)(pattern_i8 + 32));
    __m512i vec_i16_a = _mm512_loadu_si512(pattern_i16);
    __m512i vec_i16_b = _mm512_loadu_si512((int16_t*)(pattern_i16 + 16));
    __m512i vec_i32_a = _mm512_loadu_si512(pattern_i32);
    __m512i vec_i32_b = _mm512_loadu_si512((int32_t*)(pattern_i32 + 8));
    __m512i vec_i64_a = _mm512_loadu_si512(pattern_i64);
    __m512i vec_i64_b = _mm512_loadu_si512((int64_t*)(pattern_i64 + 4));
    
    // For float vectors
    float pattern_f32[16];
    double pattern_f64[8];
    
    for (int i = 0; i < 16; i++) pattern_f32[i] = (i % 7) * 1.5f;
    for (int i = 0; i < 8; i++) pattern_f64[i] = (i % 3) * 2.5;
    
    __m512 vec_f32_a = _mm512_loadu_ps(pattern_f32);
    __m512 vec_f32_b = _mm512_loadu_ps(pattern_f32 + 8);
    __m512d vec_f64_a = _mm512_loadu_pd(pattern_f64);
    __m512d vec_f64_b = _mm512_loadu_pd(pattern_f64 + 4);
    
#ifdef __AVX512FP16__
    // For half-precision vectors
    _Float16 pattern_f16[32];
    for (int i = 0; i < 32; i++) pattern_f16[i] = (_Float16)((i % 9) * 0.5f);
    
    __m512h vec_f16_a = _mm512_loadu_ph(pattern_f16);
    __m512h vec_f16_b = _mm512_loadu_ph(pattern_f16 + 16);
    __m512h vec_f16_c = _mm512_loadu_ph(pattern_f16 + 8);
    __m512h vec_f16_d = _mm512_loadu_ph(pattern_f16 + 24);
#endif
    
    /* Data-dependent control flow with loop */
    for (int iter = 0; iter < 10; iter++) {
        int selector = iter * 37;  // Vary selector across iterations
        
        /* Chain blends through different modes */
        __m512i int_result = blend_v64qi_v32hi(vec_i8_a, vec_i8_b, selector);
        
        // Conditional branch to select blend type
        if (selector % 3 == 0) {
            __m512 float_result = blend_v16sf_v8df(vec_f32_a, vec_f32_b, 
                                                  vec_f64_a, vec_f64_b, selector);
            __m512i int_result2 = blend_v16si_v8di(vec_i32_a, vec_i32_b,
                                                  vec_i64_a, vec_i64_b, selector);
            
            // Mix results from different blend chains
            __m512 mixed = _mm512_add_ps(float_result, 
                                        _mm512_castsi512_ps(int_result2));
            final_checksum += (int64_t)reduce_ps(mixed);
        } else {
            __m512i int_result2 = blend_v16si_v8di(vec_i32_a, vec_i32_b,
                                                  vec_i64_a, vec_i64_b, selector);
            final_checksum += reduce_i64(_mm512_add_epi64(int_result, int_result2));
        }
        
#ifdef __AVX512FP16__
        if (selector % 5 == 0) {
            __m512h half_result = blend_v32hf_v32bf(vec_f16_a, vec_f16_b,
                                                   vec_f16_c, vec_f16_d, selector);
            _Float16 half_sum = reduce_ph(half_result);
            final_checksum += (int64_t)(half_sum * 1000);
        }
#endif
        
        // Additional direct intrinsic calls for each mode
        __mmask64 direct_mask64 = (selector & 1) ? 0xCCCCCCCCCCCCCCCCULL : 0x3333333333333333ULL;
        __m512i direct_blend_8 = _mm512_mask_blend_epi8(direct_mask64, vec_i8_a, vec_i8_b);
        final_checksum += reduce_i64(direct_blend_8);
        
        __mmask32 direct_mask32 = (selector & 2) ? 0xCCCCCCCC : 0x33333333;
        __m512i direct_blend_16 = _mm512_mask_blend_epi16(direct_mask32, vec_i16_a, vec_i16_b);
        final_checksum += reduce_i64(direct_blend_16);
        
        __mmask16 direct_mask16 = (selector & 4) ? 0xCCCC : 0x3333;
        __m512i direct_blend_32 = _mm512_mask_blend_epi32(direct_mask16, vec_i32_a, vec_i32_b);
        final_checksum += reduce_i64(direct_blend_32);
        
        __mmask8 direct_mask8 = (selector & 8) ? 0xCC : 0x33;
        __m512i direct_blend_64 = _mm512_mask_blend_epi64(direct_mask8, vec_i64_a, vec_i64_b);
        final_checksum += reduce_i64(direct_blend_64);
        
        __m512 direct_blend_ps = _mm512_mask_blend_ps(direct_mask16, vec_f32_a, vec_f32_b);
        final_checksum += (int64_t)reduce_ps(direct_blend_ps);
        
        __m512d direct_blend_pd = _mm512_mask_blend_pd(direct_mask8, vec_f64_a, vec_f64_b);
        final_checksum += (int64_t)reduce_pd(direct_blend_pd);
    }
    
    printf("Final checksum: %ld\n", final_checksum);
    return 0;
}
