#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper functions for different blend combinations
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int mode_selector) {
    // V64QImode blend
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    __m512i result64qi = _mm512_mask_blend_epi8(mask64, a, b);
    
    // V32HImode blend with dynamic mask based on comparison
    __m512i cmp_val = _mm512_set1_epi16(100);
    __mmask32 mask32 = _mm512_cmp_epi16_mask(result64qi, cmp_val, _MM_CMPINT_LT);
    
    // Chain: use result from epi8 blend as input to epi16 blend
    __m512i result32hi = _mm512_mask_blend_epi16(mask32, result64qi, b);
    
    return result32hi;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int iter) {
    // V16SFmode blend with immediate mask
    __mmask16 mask16_sf = 0xAAAA;
    __m512 result16sf = _mm512_mask_blend_ps(mask16_sf, a, b);
    
    // V8DFmode blend with comparison-generated mask
    __m512d cmp_double = _mm512_set1_pd(0.5);
    __mmask8 mask8_df = _mm512_cmp_pd_mask(c, cmp_double, _CMP_LT_OQ);
    
    // Modify mask based on iteration
    if (iter % 2) {
        mask8_df = _knot_mask8(mask8_df);
    }
    
    __m512d result8df = _mm512_mask_blend_pd(mask8_df, c, d);
    
    // Convert double result back to float for chaining
    __m512 df_as_ps = _mm512_castpd_ps(result8df);
    
    // Another blend with combined masks
    __mmask16 final_mask = _kor_mask16(mask16_sf, _mm512_cmp_ps_mask(df_as_ps, b, _CMP_NEQ_UQ));
    return _mm512_mask_blend_ps(final_mask, result16sf, df_as_ps);
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int condition) {
    // V16SImode blend
    __mmask16 mask16_si;
    if (condition > 0) {
        mask16_si = 0x5555;  // Alternating pattern
    } else {
        // Generate mask via comparison
        __m512i threshold = _mm512_set1_epi32(1000);
        mask16_si = _mm512_cmp_epi32_mask(a, threshold, _MM_CMPINT_GT);
    }
    
    __m512i result16si = _mm512_mask_blend_epi32(mask16_si, a, b);
    
    // V8DImode blend with chained input
    __mmask8 mask8_di = _mm512_cmp_epi64_mask(result16si, _mm512_set1_epi64(500), _MM_CMPINT_EQ);
    __m512i result8di = _mm512_mask_blend_epi64(mask8_di, c, d);
    
    // Combine results
    return _mm512_add_epi32(result16si, _mm512_castsi512_si256(result8di));
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __m512h c, __m512bh d, int selector) {
    // V32HFmode blend
    __mmask32 mask32_hf;
    if (selector % 3 == 0) {
        mask32_hf = 0xAAAAAAAA;  // Immediate constant
    } else if (selector % 3 == 1) {
        // Generate mask via comparison
        __m512h zero = _mm512_set1_ph(0.0f);
        mask32_hf = _mm512_cmp_ph_mask(a, zero, _CMP_GT_OQ);
    } else {
        // Complex mask generation
        __m512h one = _mm512_set1_ph(1.0f);
        __mmask32 mask1 = _mm512_cmp_ph_mask(a, one, _CMP_LT_OQ);
        __mmask32 mask2 = _mm512_cmp_ph_mask(b, zero, _CMP_NEQ_UQ);
        mask32_hf = _knot_mask32(_kor_mask32(mask1, mask2));
    }
    
    __m512h result32hf = _mm512_mask_blend_ph(mask32_hf, a, b);
    
    // For V32BFmode, we use the same intrinsic but with __m512bh type
    // Note: BFloat16 uses same intrinsic but different type
    __m512bh result32bf = _mm512_castph_bh(_mm512_mask_blend_ph(mask32_hf, 
        _mm512_castbh_ph(d), _mm512_castbh_ph(_mm512_castph_bh(c))));
    
    // Chain operations
    return _mm512_add_ph(result32hf, _mm512_castbh_ph(result32bf));
}
#endif

// Reduction helpers
static inline int64_t reduce_add_epi64(__m512i v) {
    __m256i v256_lo = _mm512_castsi512_si256(v);
    __m256i v256_hi = _mm512_extracti64x4_epi64(v, 1);
    
    __m256i sum256 = _mm256_add_epi64(v256_lo, v256_hi);
    __m128i sum128_lo = _mm256_castsi256_si128(sum256);
    __m128i sum128_hi = _mm256_extracti128_si256(sum256, 1);
    
    __m128i sum128 = _mm_add_epi64(sum128_lo, sum128_hi);
    
    return (int64_t)_mm_extract_epi64(sum128, 0) + 
           (int64_t)_mm_extract_epi64(sum128, 1);
}

static inline float reduce_add_ps(__m512 v) {
    __m256 v256_lo = _mm512_castps512_ps256(v);
    __m256 v256_hi = _mm512_extractf32x8_ps(v, 1);
    
    __m256 sum256 = _mm256_add_ps(v256_lo, v256_hi);
    __m128 sum128_lo = _mm256_castps256_ps128(sum256);
    __m128 sum128_hi = _mm256_extractf128_ps(sum256, 1);
    
    __m128 sum128 = _mm_add_ps(sum128_lo, sum128_hi);
    
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    
    return _mm_cvtss_f32(sum128);
}

static inline double reduce_add_pd(__m512d v) {
    __m256d v256_lo = _mm512_castpd512_pd256(v);
    __m256d v256_hi = _mm512_extractf64x4_pd(v, 1);
    
    __m256d sum256 = _mm256_add_pd(v256_lo, v256_hi);
    __m128d sum128_lo = _mm256_castpd256_pd128(sum256);
    __m128d sum128_hi = _mm256_extractf128_pd(sum256, 1);
    
    __m128d sum128 = _mm_add_pd(sum128_lo, sum128_hi);
    
    return _mm_cvtsd_f64(_mm_hadd_pd(sum128, sum128));
}

int main() {
    double checksum = 0.0;
    
    // Initialize data with distinct patterns
    int8_t int8_data[64];
    int16_t int16_data[32];
    int32_t int32_data[16];
    int64_t int64_data[8];
    float float_data[16];
    double double_data[8];
    
    for (int i = 0; i < 64; i++) int8_data[i] = (i % 3) - 1;
    for (int i = 0; i < 32; i++) int16_data[i] = i * 100;
    for (int i = 0; i < 16; i++) int32_data[i] = i * 1000;
    for (int i = 0; i < 8; i++) int64_data[i] = i * 10000LL;
    for (int i = 0; i < 16; i++) float_data[i] = i * 0.125f;
    for (int i = 0; i < 8; i++) double_data[i] = i * 0.25;
    
    __m512i v64qi_a = _mm512_loadu_si512((void*)int8_data);
    __m512i v64qi_b = _mm512_set1_epi8(42);
    
    __m512i v32hi_a = _mm512_loadu_si512((void*)int16_data);
    __m512i v32hi_b = _mm512_set1_epi16(-100);
    
    __m512i v16si_a = _mm512_loadu_si512((void*)int32_data);
    __m512i v16si_b = _mm512_set1_epi32(999);
    
    __m512i v8di_a = _mm512_loadu_si512((void*)int64_data);
    __m512i v8di_b = _mm512_set1_epi64(-5000);
    
    __m512 v16sf_a = _mm512_loadu_ps(float_data);
    __m512 v16sf_b = _mm512_set1_ps(3.14159f);
    
    __m512d v8df_a = _mm512_loadu_pd(double_data);
    __m512d v8df_b = _mm512_set1_pd(2.71828);
    
    // Data-dependent control flow with loops
    for (int iter = 0; iter < 10; iter++) {
        int mode_selector = iter % 4;
        
        // Chain blends through different modes
        __m512i int_result = blend_v64qi_v32hi(v64qi_a, v64qi_b, mode_selector);
        
        // Conditional blend selection
        if (mode_selector == 0) {
            // Direct V16SImode blend with immediate mask
            __mmask16 mask = 0xFFFF;
            __m512i temp = _mm512_mask_blend_epi32(mask, v16si_a, v16si_b);
            int_result = _mm512_add_epi32(int_result, temp);
        } else if (mode_selector == 1) {
            // Direct V8DImode blend with comparison mask
            __mmask8 mask = _mm512_cmp_epi64_mask(v8di_a, v8di_b, _MM_CMPINT_GT);
            __m512i temp = _mm512_mask_blend_epi64(mask, v8di_a, v8di_b);
            int_result = _mm512_add_epi32(int_result, _mm512_castsi512_si256(temp));
        }
        
        // Blend integer and float results
        __m512 float_result = blend_v16sf_v8df(v16sf_a, v16sf_b, v8df_a, v8df_b, iter);
        
        // More integer blends with chaining
        __m512i int_result2 = blend_v16si_v8di(v16si_a, v16si_b, v8di_a, v8di_b, iter);
        
        // Accumulate reductions
        checksum += reduce_add_epi64(int_result);
        checksum += reduce_add_ps(float_result);
        checksum += reduce_add_epi64(int_result2);
        checksum += reduce_add_pd(v8df_a);  // Direct use of double vector
        
        // Additional direct blends in loop to ensure coverage
        if (iter % 3 == 0) {
            // V64QImode with dynamic mask
            __mmask64 mask = _mm512_cmp_epi8_mask(v64qi_a, v64qi_b, _MM_CMPINT_EQ);
            __m512i blend_result = _mm512_mask_blend_epi8(mask, v64qi_a, v64qi_b);
            checksum += reduce_add_epi64(blend_result);
        }
        
        if (iter % 3 == 1) {
            // V32HImode with immediate mask
            __m512i blend_result = _mm512_mask_blend_epi16(0x5555, v32hi_a, v32hi_b);
            checksum += reduce_add_epi64(blend_result);
        }
    }
    
#ifdef __AVX512FP16__
    // Half-precision blends
    __m512h v32hf_a = _mm512_set1_ph(1.5f);
    __m512h v32hf_b = _mm512_set1_ph(2.5f);
    __m512h v32hf_c = _mm512_set1_ph(0.5f);
    __m512bh v32bf_d = _mm512_set1_bh(0x3C00);  // 1.0 in bfloat16
    
    for (int i = 0; i < 5; i++) {
        __m512h hf_result = blend_v32hf_v32bf(v32hf_a, v32hf_b, v32hf_c, v32bf_d, i);
        
        // Manual reduction for half precision
        __m256h hf256_lo = _mm512_castph512_ph256(hf_result);
        __m256h hf256_hi = _mm512_extractf32x8_ph(hf_result, 1);
        
        // Convert to float for reduction
        __m512 hf_as_float = _mm512_cvtph_ps(hf_result);
        checksum += reduce_add_ps(hf_as_float);
    }
#endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
