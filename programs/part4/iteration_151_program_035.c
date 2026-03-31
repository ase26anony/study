#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, __mmask64 mask64, __mmask32 mask32) {
    // Chain epi8 -> epi16 blend
    __m512i blend8 = _mm512_mask_blend_epi8(mask64, a, b);
    __m512i blend16 = _mm512_mask_blend_epi16(mask32, blend8, _mm512_add_epi16(b, _mm512_set1_epi16(1)));
    return blend16;
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, 
                                      __mmask16 mask16, __mmask8 mask8) {
    // Chain ps -> pd blend
    __m512 blend_ps = _mm512_mask_blend_ps(mask16, a, b);
    __m512d blend_pd = _mm512_mask_blend_pd(mask8, _mm512_castps_pd(blend_ps), c);
    return _mm512_castpd_ps(blend_pd);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, __mmask32 mask32_a, __mmask32 mask32_b) {
    // Chain two half-precision blends
    __m512h blend1 = _mm512_mask_blend_ph(mask32_a, a, b);
    __m512h blend2 = _mm512_mask_blend_ph(mask32_b, blend1, 
                                         _mm512_add_ph(b, _mm512_set1_ph((__fp16)1.0f)));
    return blend2;
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d,
                                       __mmask16 mask16, __mmask8 mask8) {
    // Chain epi32 -> epi64 blend
    __m512i blend32 = _mm512_mask_blend_epi32(mask16, a, b);
    __m512i blend64 = _mm512_mask_blend_epi64(mask8, blend32, c);
    return blend64;
}

/* Function to generate varied masks */
static inline __mmask64 generate_mask64(int method) {
    if (method == 0) {
        // Immediate-like pattern
        return 0xAAAAAAAAAAAAAAAAULL;
    } else if (method == 1) {
        // Alternating pattern
        return 0x5555555555555555ULL;
    } else {
        // Checkerboard pattern
        return 0x3333333333333333ULL;
    }
}

static inline __mmask32 generate_mask32(int method) {
    if (method == 0) {
        return 0xAAAAAAAA;
    } else if (method == 1) {
        return 0x55555555;
    } else {
        return 0x33333333;
    }
}

static inline __mmask16 generate_mask16(int method) {
    if (method == 0) {
        return 0xAAAA;
    } else if (method == 1) {
        return 0x5555;
    } else {
        return 0x3333;
    }
}

static inline __mmask8 generate_mask8(int method) {
    if (method == 0) {
        return 0xAA;
    } else if (method == 1) {
        return 0x55;
    } else {
        return 0x33;
    }
}

int main() {
    // Initialize patterned data
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
    
    #ifdef __AVX512FP16__
    __m512h vec_f16_a = _mm512_set_ph(
        (__fp16)0.0f,(__fp16)1.0f,(__fp16)2.0f,(__fp16)3.0f,
        (__fp16)4.0f,(__fp16)5.0f,(__fp16)6.0f,(__fp16)7.0f,
        (__fp16)8.0f,(__fp16)9.0f,(__fp16)10.0f,(__fp16)11.0f,
        (__fp16)12.0f,(__fp16)13.0f,(__fp16)14.0f,(__fp16)15.0f,
        (__fp16)16.0f,(__fp16)17.0f,(__fp16)18.0f,(__fp16)19.0f,
        (__fp16)20.0f,(__fp16)21.0f,(__fp16)22.0f,(__fp16)23.0f,
        (__fp16)24.0f,(__fp16)25.0f,(__fp16)26.0f,(__fp16)27.0f,
        (__fp16)28.0f,(__fp16)29.0f,(__fp16)30.0f,(__fp16)31.0f
    );
    
    __m512h vec_f16_b = _mm512_set_ph(
        (__fp16)31.0f,(__fp16)30.0f,(__fp16)29.0f,(__fp16)28.0f,
        (__fp16)27.0f,(__fp16)26.0f,(__fp16)25.0f,(__fp16)24.0f,
        (__fp16)23.0f,(__fp16)22.0f,(__fp16)21.0f,(__fp16)20.0f,
        (__fp16)19.0f,(__fp16)18.0f,(__fp16)17.0f,(__fp16)16.0f,
        (__fp16)15.0f,(__fp16)14.0f,(__fp16)13.0f,(__fp16)12.0f,
        (__fp16)11.0f,(__fp16)10.0f,(__fp16)9.0f,(__fp16)8.0f,
        (__fp16)7.0f,(__fp16)6.0f,(__fp16)5.0f,(__fp16)4.0f,
        (__fp16)3.0f,(__fp16)2.0f,(__fp16)1.0f,(__fp16)0.0f
    );
    #endif
    
    // Data-dependent control flow
    int loop_count = 100;
    int use_alternate = 0;
    double checksum = 0.0;
    
    for (int i = 0; i < loop_count; i++) {
        // Vary mask generation based on loop iteration
        int mask_method = i % 3;
        
        // Generate masks using different patterns
        __mmask64 mask64 = generate_mask64(mask_method);
        __mmask32 mask32 = generate_mask32(mask_method);
        __mmask16 mask16 = generate_mask16(mask_method);
        __mmask8 mask8 = generate_mask8(mask_method);
        
        // Data-dependent blend selection
        if (i % 2 == 0) {
            // Use epi8 and epi16 blends
            __m512i result_i8 = _mm512_mask_blend_epi8(mask64, vec_i8_a, vec_i8_b);
            __m512i result_i16 = _mm512_mask_blend_epi16(mask32, vec_i16_a, vec_i16_b);
            
            // Chain operations
            __m512i chained = blend_v64qi_v32hi(result_i8, result_i16, mask64, mask32);
            
            // Reduction to prevent dead code elimination
            __m256i sum256 = _mm512_castsi512_si256(_mm512_add_epi64(
                _mm512_cvtepi32_epi64(_mm512_castsi512_si256(chained)),
                _mm512_setzero_si512()
            ));
            
            // Accumulate to checksum
            int64_t sum_arr[8];
            _mm512_storeu_si512((__m512i*)sum_arr, _mm512_castsi256_si512(sum256));
            for (int j = 0; j < 8; j++) {
                checksum += sum_arr[j];
            }
        } else {
            // Use epi32 and epi64 blends
            __m512i result_i32 = _mm512_mask_blend_epi32(mask16, vec_i32_a, vec_i32_b);
            __m512i result_i64 = _mm512_mask_blend_epi64(mask8, vec_i64_a, vec_i64_b);
            
            // Chain operations
            __m512i chained = blend_v16si_v8di(result_i32, result_i64, vec_i32_a, vec_i64_a, mask16, mask8);
            
            // Reduction
            int32_t sum_arr[16];
            _mm512_storeu_si512((__m512i*)sum_arr, chained);
            for (int j = 0; j < 16; j++) {
                checksum += sum_arr[j];
            }
        }
        
        // Always execute float blends
        __m512 result_f32 = _mm512_mask_blend_ps(mask16, vec_f32_a, vec_f32_b);
        __m512d result_f64 = _mm512_mask_blend_pd(mask8, vec_f64_a, vec_f64_b);
        
        // Chain float operations
        __m512 chained_float = blend_v16sf_v8df(result_f32, vec_f32_b, result_f64, vec_f64_b, mask16, mask8);
        
        // Float reduction
        float fsum = _mm512_reduce_add_ps(chained_float);
        checksum += fsum;
        
        #ifdef __AVX512FP16__
        // Half-precision blends (V32HFmode and V32BFmode)
        __m512h result_f16 = _mm512_mask_blend_ph(mask32, vec_f16_a, vec_f16_b);
        
        // Chain half-precision operations
        __m512h chained_half = blend_v32hf_v32bf(result_f16, vec_f16_b, mask32, 
                                                 generate_mask32((mask_method + 1) % 3));
        
        // Half-precision reduction
        __fp16 hsum_arr[32];
        _mm512_storeu_ph(hsum_arr, chained_half);
        for (int j = 0; j < 32; j++) {
            checksum += hsum_arr[j];
        }
        #endif
        
        // Generate masks using comparison intrinsics (alternate method)
        if (i % 5 == 0) {
            __mmask16 cmp_mask16 = _mm512_cmp_ps_mask(vec_f32_a, vec_f32_b, _CMP_LT_OS);
            __m512 result_cmp = _mm512_mask_blend_ps(cmp_mask16, vec_f32_a, vec_f32_b);
            
            float cmp_sum = _mm512_reduce_add_ps(result_cmp);
            checksum += cmp_sum;
        }
        
        // Generate masks using bitwise operations
        if (i % 7 == 0) {
            __mmask32 mask_a = generate_mask32(0);
            __mmask32 mask_b = generate_mask32(1);
            __mmask32 combined_mask = _kor_mask32(mask_a, mask_b);
            __mmask32 inverted_mask = _knot_mask32(combined_mask);
            
            __m512i result_combined = _mm512_mask_blend_epi16(inverted_mask, vec_i16_a, vec_i16_b);
            
            int16_t sum_arr[32];
            _mm512_storeu_si512((__m512i*)sum_arr, result_combined);
            for (int j = 0; j < 32; j++) {
                checksum += sum_arr[j];
            }
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
