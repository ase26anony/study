#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Helper functions for different blend modes */

static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 k8, __mmask32 k16) {
    // Chain epi8 -> epi16 blend
    __m512i blend8 = _mm512_mask_blend_epi8(k8, a, b);
    __m512i blend16 = _mm512_mask_blend_epi16(k16, blend8, b);
    return blend16;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d,
                                      __mmask16 k32, __mmask8 k64) {
    // Chain ps -> pd blend
    __m512 blend_ps = _mm512_mask_blend_ps(k32, a, b);
    __m512d blend_pd = _mm512_mask_blend_pd(k64, c, d);
    // Convert pd result back to ps for mixing
    __m512 pd_as_ps = _mm512_castpd_ps(blend_pd);
    return _mm512_add_ps(blend_ps, pd_as_ps);
}

static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 k16) {
    // FP16 blend
    __m512h blend_hf = _mm512_mask_blend_ph(k16, a, b);
    // BFloat16 uses same intrinsic but different type
    return blend_hf;
}

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d,
                                       __mmask16 k32, __mmask8 k64) {
    // Chain epi32 -> epi64 blend
    __m512i blend32 = _mm512_mask_blend_epi32(k32, a, b);
    __m512i blend64 = _mm512_mask_blend_epi64(k64, c, d);
    return _mm512_add_epi32(blend32, _mm512_castsi512_si512(blend64));
}

/* Mask generation helpers */
static inline __mmask64 generate_mask64_pattern(int pattern_type) {
    if (pattern_type == 0) {
        // Immediate-like pattern: alternating bytes
        return 0xAAAAAAAAAAAAAAAAULL;
    } else if (pattern_type == 1) {
        // All ones
        return 0xFFFFFFFFFFFFFFFFULL;
    } else {
        // Checkerboard pattern
        return 0x5555555555555555ULL;
    }
}

static inline __mmask32 generate_mask32_dynamic(__m512i a, __m512i b) {
    // Generate mask via comparison
    return _mm512_cmp_epi16_mask(a, b, _MM_CMPINT_GT);
}

static inline __mmask16 generate_mask16_dynamic(__m512 a, __m512 b) {
    // Generate mask via float comparison
    return _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
}

static inline __mmask8 generate_mask8_dynamic(__m512d a, __m512d b) {
    // Generate mask via double comparison
    return _mm512_cmp_pd_mask(a, b, _CMP_GT_OQ);
}

int main() {
    // Initialize data with distinct patterns
    __m512i vec_i8_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vec_i8_b = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vec_i16_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i vec_i16_b = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vec_i32_a = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i vec_i32_b = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i vec_i64_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i vec_i64_b = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    __m512 vec_f32_a = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 vec_f32_b = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512d vec_f64_a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d vec_f64_b = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    // For FP16, we need to use appropriate initialization
    // Note: _mm512_set_ph requires AVX512-FP16
    #ifdef __AVX512FP16__
    __m512h vec_f16_a, vec_f16_b;
    // Initialize with pattern
    {
        short init_data[32];
        for (int i = 0; i < 32; i++) {
            init_data[i] = i;  // Half-precision pattern
        }
        memcpy(&vec_f16_a, init_data, sizeof(vec_f16_a));
        for (int i = 0; i < 32; i++) {
            init_data[i] = 31 - i;
        }
        memcpy(&vec_f16_b, init_data, sizeof(vec_f16_b));
    }
    #endif
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Data-dependent control flow
    int mode_selector = 0;
    
    // Loop with varying blend modes
    for (int iter = 0; iter < 4; iter++) {
        mode_selector = iter % 3;
        
        if (mode_selector == 0) {
            // Use immediate mask patterns
            __mmask64 k8 = generate_mask64_pattern(iter);
            __mmask32 k16 = 0xAAAAAAAA;  // Alternating pattern
            
            // V64QImode blend
            __m512i blend_epi8 = _mm512_mask_blend_epi8(k8, vec_i8_a, vec_i8_b);
            
            // V32HImode blend
            __m512i blend_epi16 = _mm512_mask_blend_epi16(k16, vec_i16_a, vec_i16_b);
            
            // Chain operations
            __m512i chained = blend_v64qi_v32hi(blend_epi8, blend_epi16, k8, k16);
            
            // Reduction
            long long sum = _mm512_reduce_add_epi64(chained);
            checksum += (double)sum;
            
        } else if (mode_selector == 1) {
            // Use dynamically generated masks via comparisons
            
            // V16SImode blend with dynamic mask
            __mmask16 k32 = generate_mask16_dynamic(vec_f32_a, vec_f32_b);
            __m512i blend_epi32 = _mm512_mask_blend_epi32(k32, vec_i32_a, vec_i32_b);
            
            // V8DImode blend with dynamic mask
            __mmask8 k64 = generate_mask8_dynamic(vec_f64_a, vec_f64_b);
            __m512i blend_epi64 = _mm512_mask_blend_epi64(k64, vec_i64_a, vec_i64_b);
            
            // Chain integer blends
            __m512i chained_int = blend_v16si_v8di(blend_epi32, vec_i32_b, 
                                                   blend_epi64, vec_i64_b,
                                                   k32, k64);
            
            // Reduction
            long long sum = _mm512_reduce_add_epi64(chained_int);
            checksum += (double)sum;
            
        } else {
            // Float blends with mixed mask operations
            
            // V16SFmode blend
            __mmask16 k32_float = _mm512_cmp_ps_mask(vec_f32_a, vec_f32_b, _CMP_GT_OQ);
            __m512 blend_ps = _mm512_mask_blend_ps(k32_float, vec_f32_a, vec_f32_b);
            
            // V8DFmode blend
            __mmask8 k64_double = _mm512_cmp_pd_mask(vec_f64_a, vec_f64_b, _CMP_LT_OQ);
            __m512d blend_pd = _mm512_mask_blend_pd(k64_double, vec_f64_a, vec_f64_b);
            
            // Chain float blends
            __m512 chained_float = blend_v16sf_v8df(blend_ps, vec_f32_b,
                                                    blend_pd, vec_f64_b,
                                                    k32_float, k64_double);
            
            // Reduction
            float sum_float = _mm512_reduce_add_ps(chained_float);
            checksum += (double)sum_float;
            
            #ifdef __AVX512FP16__
            // V32HFmode and V32BFmode blends
            __mmask32 k16_half = 0xAAAAAAAA;  // Alternating mask
            __m512h blend_half = _mm512_mask_blend_ph(k16_half, vec_f16_a, vec_f16_b);
            
            // Simple reduction for half-precision
            {
                short* half_data = (short*)&blend_half;
                float half_sum = 0.0f;
                for (int i = 0; i < 32; i++) {
                    half_sum += (float)half_data[i];
                }
                checksum += (double)half_sum;
            }
            #endif
        }
        
        // Modify data for next iteration to create different comparison results
        vec_i8_a = _mm512_add_epi8(vec_i8_a, _mm512_set1_epi8(1));
        vec_f32_a = _mm512_add_ps(vec_f32_a, _mm512_set1_ps(1.0f));
        vec_f64_a = _mm512_add_pd(vec_f64_a, _mm512_set1_pd(1.0));
    }
    
    // Additional test: Nested conditional blends
    int alt_selector = 1;
    for (int i = 0; i < 2; i++) {
        if (alt_selector > 0) {
            // Complex mask manipulation
            __mmask16 k1 = _mm512_cmp_ps_mask(vec_f32_a, vec_f32_b, _CMP_EQ_OQ);
            __mmask16 k2 = _mm512_cmp_ps_mask(vec_f32_a, _mm512_set1_ps(8.0f), _CMP_GT_OQ);
            __mmask16 k_combined = _kor_mask16(k1, k2);
            __mmask16 k_inverted = _knot_mask16(k_combined);
            
            __m512 blend_complex = _mm512_mask_blend_ps(k_inverted, vec_f32_a, vec_f32_b);
            float sum = _mm512_reduce_add_ps(blend_complex);
            checksum += (double)sum;
            
            alt_selector = -alt_selector;
        } else {
            // Integer blend with bitwise mask operations
            __mmask32 k16_int = generate_mask32_dynamic(vec_i16_a, vec_i16_b);
            __mmask32 k16_alt = 0x55555555;  // Different pattern
            __mmask32 k_mixed = _kor_mask32(k16_int, k16_alt);
            
            __m512i blend_mixed = _mm512_mask_blend_epi16(k_mixed, vec_i16_a, vec_i16_b);
            long long sum = _mm512_reduce_add_epi64(blend_mixed);
            checksum += (double)sum;
            
            alt_selector = -alt_selector + 1;
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
