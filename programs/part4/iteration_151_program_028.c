#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    __m512i result;
    
    if (mode_selector & 1) {
        // V64QImode blend with immediate mask
        __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
        result = _mm512_mask_blend_epi8(mask64, a, b);
        
        // Also blend with computed mask
        __m512i cmp = _mm512_set1_epi8(0);
        __mmask64 cmp_mask = _mm512_cmp_epi8_mask(a, cmp, _MM_CMPINT_GT);
        result = _mm512_mask_blend_epi8(cmp_mask, result, b);
    } else {
        // V32HImode blend with immediate mask
        __mmask32 mask32 = 0xAAAAAAAA;
        result = _mm512_mask_blend_epi16(mask32, a, b);
        
        // Also blend with computed mask
        __m512i cmp = _mm512_set1_epi16(0);
        __mmask32 cmp_mask = _mm512_cmp_epi16_mask(a, cmp, _MM_CMPINT_GT);
        result = _mm512_mask_blend_epi16(cmp_mask, result, b);
    }
    
    return result;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    __m512 result_f;
    __m512d result_d;
    
    if (selector & 2) {
        // V16SFmode blend with immediate mask
        __mmask16 mask16 = 0xAAAA;
        result_f = _mm512_mask_blend_ps(mask16, a, b);
        
        // Blend with computed mask
        __m512 zero = _mm512_setzero_ps();
        __mmask16 cmp_mask = _mm512_cmp_ps_mask(a, zero, _CMP_GT_OQ);
        result_f = _mm512_mask_blend_ps(cmp_mask, result_f, b);
    } else {
        // V8DFmode blend with immediate mask
        __mmask8 mask8 = 0xAA;
        result_d = _mm512_mask_blend_pd(mask8, c, d);
        
        // Convert back to float for return
        result_f = _mm512_castpd_ps(result_d);
    }
    
    return result_f;
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int selector) {
    __m512h result;
    
    // V32HFmode blend with immediate mask
    __mmask32 mask32 = 0xAAAAAAAA;
    result = _mm512_mask_blend_ph(mask32, a, b);
    
    // Blend with computed mask
    __m512h zero = _mm512_setzero_ph();
    __mmask32 cmp_mask = _mm512_cmp_ph_mask(a, zero, _CMP_GT_OQ);
    result = _mm512_mask_blend_ph(cmp_mask, result, b);
    
    // Additional blend with modified mask
    __mmask32 not_mask = _knot_mask32(cmp_mask);
    __mmask32 or_mask = _kor_mask32(mask32, not_mask);
    result = _mm512_mask_blend_ph(or_mask, result, a);
    
    return result;
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int selector) {
    __m512i result;
    
    if (selector & 4) {
        // V16SImode blend with immediate mask
        __mmask16 mask16 = 0xAAAA;
        result = _mm512_mask_blend_epi32(mask16, a, b);
        
        // Blend with computed mask
        __m512i cmp = _mm512_set1_epi32(0);
        __mmask16 cmp_mask = _mm512_cmp_epi32_mask(a, cmp, _MM_CMPINT_GT);
        result = _mm512_mask_blend_epi32(cmp_mask, result, b);
        
        // Chain with another blend using modified mask
        __mmask16 not_mask = _knot_mask16(cmp_mask);
        result = _mm512_mask_blend_epi32(not_mask, result, a);
    } else {
        // V8DImode blend with immediate mask
        __mmask8 mask8 = 0xAA;
        result = _mm512_mask_blend_epi64(mask8, c, d);
        
        // Blend with computed mask
        __m512i cmp = _mm512_set1_epi64(0);
        __mmask8 cmp_mask = _mm512_cmp_epi64_mask(c, cmp, _MM_CMPINT_GT);
        result = _mm512_mask_blend_epi64(cmp_mask, result, d);
    }
    
    return result;
}

/* Reduction helpers to prevent dead code elimination */
static inline int64_t reduce_i64(__m512i v) {
    __m256i v_low = _mm512_castsi512_si256(v);
    __m256i v_high = _mm512_extracti64x4_epi64(v, 1);
    __m256i sum256 = _mm256_add_epi64(v_low, v_high);
    
    __m128i sum128_low = _mm256_castsi256_si128(sum256);
    __m128i sum128_high = _mm256_extracti128_si256(sum256, 1);
    __m128i sum128 = _mm_add_epi64(sum128_low, sum128_high);
    
    return (int64_t)_mm_extract_epi64(sum128, 0) + 
           (int64_t)_mm_extract_epi64(sum128, 1);
}

static inline float reduce_f32(__m512 v) {
    return _mm512_reduce_add_ps(v);
}

static inline double reduce_f64(__m512d v) {
    __m256d v_low = _mm512_castpd512_pd256(v);
    __m256d v_high = _mm512_extractf64x4_pd(v, 1);
    __m256d sum256 = _mm256_add_pd(v_low, v_high);
    
    __m128d sum128_low = _mm256_castpd256_pd128(sum256);
    __m128d sum128_high = _mm256_extractf128_pd(sum256, 1);
    __m128d sum128 = _mm_add_pd(sum128_low, sum128_high);
    
    return _mm_cvtsd_f64(sum128) + _mm_cvtsd_f64(_mm_unpackhi_pd(sum128, sum128));
}

int main() {
    int64_t checksum = 0;
    
    /* Initialize vectors with distinct patterns */
    // For V64QImode and V32HImode
    int8_t i8_data[64];
    for (int i = 0; i < 64; i++) i8_data[i] = (i % 3) - 1;
    __m512i v64qi_a = _mm512_loadu_si512((void*)i8_data);
    __m512i v64qi_b = _mm512_set1_epi8(42);
    
    // For V32HImode
    int16_t i16_data[32];
    for (int i = 0; i < 32; i++) i16_data[i] = (i % 5) * 100;
    __m512i v32hi_a = _mm512_loadu_si512((void*)i16_data);
    __m512i v32hi_b = _mm512_set1_epi16(-100);
    
    // For V16SImode and V8DImode
    int32_t i32_data[16];
    for (int i = 0; i < 16; i++) i32_data[i] = i * 1000;
    __m512i v16si_a = _mm512_loadu_si512((void*)i32_data);
    __m512i v16si_b = _mm512_set1_epi32(-500);
    
    int64_t i64_data[8];
    for (int i = 0; i < 8; i++) i64_data[i] = i * 10000LL;
    __m512i v8di_a = _mm512_loadu_si512((void*)i64_data);
    __m512i v8di_b = _mm512_set1_epi64(-5000LL);
    
    // For V16SFmode
    float f32_data[16];
    for (int i = 0; i < 16; i++) f32_data[i] = i * 1.5f;
    __m512 v16sf_a = _mm512_loadu_ps(f32_data);
    __m512 v16sf_b = _mm512_set1_ps(-2.5f);
    
    // For V8DFmode
    double f64_data[8];
    for (int i = 0; i < 8; i++) f64_data[i] = i * 3.14159;
    __m512d v8df_a = _mm512_loadu_pd(f64_data);
    __m512d v8df_b = _mm512_set1_pd(-1.618);
    
#ifdef __AVX512FP16__
    // For V32HFmode and V32BFmode
    _Float16 f16_data[32];
    for (int i = 0; i < 32; i++) f16_data[i] = (_Float16)(i * 0.5);
    __m512h v32hf_a = _mm512_loadu_ph(f16_data);
    __m512h v32hf_b = _mm512_set1_ph((_Float16)-0.25);
#endif
    
    /* Execute blends in a loop with data-dependent control flow */
    for (int iter = 0; iter < 10; iter++) {
        int selector = iter % 8;
        
        // Chain blends through different modes
        __m512i int_result = blend_v64qi_v32hi(v64qi_a, v64qi_b, selector);
        
        if (selector & 1) {
            // Use integer result as input to float blend (after conversion)
            __m512 temp_f = _mm512_cvtepi32_ps(_mm512_castsi512_si256(int_result));
            __m512 float_result = blend_v16sf_v8df(temp_f, v16sf_a, v8df_a, v8df_b, selector);
            checksum += (int64_t)reduce_f32(float_result);
        } else {
            __m512i int_result2 = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, selector);
            checksum += reduce_i64(int_result2);
        }
        
        // Direct intrinsic calls for each mode
        __mmask64 m64 = (selector & 1) ? 0x5555555555555555ULL : 0xAAAAAAAAAAAAAAAAULL;
        __m512i blend64qi = _mm512_mask_blend_epi8(m64, v64qi_a, v64qi_b);
        checksum += reduce_i64(blend64qi);
        
        __mmask32 m32 = (selector * 1103515245) & 0xFFFFFFFF;
        __m512i blend32hi = _mm512_mask_blend_epi16(m32, v32hi_a, v32hi_b);
        checksum += reduce_i64(blend32hi);
        
        __mmask16 m16 = _mm512_cmp_epi32_mask(v16si_a, v16si_b, _MM_CMPINT_LT);
        __m512i blend16si = _mm512_mask_blend_epi32(m16, v16si_a, v16si_b);
        checksum += reduce_i64(blend16si);
        
        __mmask8 m8 = _mm512_cmp_epi64_mask(v8di_a, v8di_b, _MM_CMPINT_GT);
        __m512i blend8di = _mm512_mask_blend_epi64(m8, v8di_a, v8di_b);
        checksum += reduce_i64(blend8di);
        
        __mmask16 m16f = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_GT_OQ);
        __m512 blend16sf = _mm512_mask_blend_ps(m16f, v16sf_a, v16sf_b);
        checksum += (int64_t)reduce_f32(blend16sf);
        
        __mmask8 m8d = _mm512_cmp_pd_mask(v8df_a, v8df_b, _CMP_LT_OQ);
        __m512d blend8df = _mm512_mask_blend_pd(m8d, v8df_a, v8df_b);
        checksum += (int64_t)reduce_f64(blend8df);
        
#ifdef __AVX512FP16__
        if (selector & 4) {
            __m512h blend32hf = blend_v32hf_v32bf(v32hf_a, v32hf_b, selector);
            // Simple reduction for half precision
            _Float16 hsum = 0;
            _Float16 temp[32];
            _mm512_storeu_ph(temp, blend32hf);
            for (int i = 0; i < 32; i++) hsum += temp[i];
            checksum += (int64_t)(hsum * 1000);
        }
#endif
    }
    
    printf("Final checksum: %ld\n", checksum);
    return 0;
}
