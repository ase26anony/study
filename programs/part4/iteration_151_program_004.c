#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper function for V64QImode blends
static inline __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 mask) {
    return _mm512_mask_blend_epi8(mask, a, b);
}

// Helper function for V32HImode blends
static inline __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 mask) {
    return _mm512_mask_blend_epi16(mask, a, b);
}

// Helper function for V16SImode blends
static inline __m512i blend_v16si(__m512i a, __m512i b, __mmask16 mask) {
    return _mm512_mask_blend_epi32(mask, a, b);
}

// Helper function for V8DImode blends
static inline __m512i blend_v8di(__m512i a, __m512i b, __mmask8 mask) {
    return _mm512_mask_blend_epi64(mask, a, b);
}

// Helper function for V16SFmode blends
static inline __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 mask) {
    return _mm512_mask_blend_ps(mask, a, b);
}

// Helper function for V8DFmode blends
static inline __m512d blend_v8df(__m512d a, __m512d b, __mmask8 mask) {
    return _mm512_mask_blend_pd(mask, a, b);
}

#ifdef __AVX512FP16__
// Helper function for V32HFmode blends
static inline __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}

// Helper function for V32BFmode blends (using same intrinsic as HF)
static inline __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}
#endif

// Chained blend operations: epi8 -> epi16 -> ps
static inline float chain_blend_operations(int mode_selector) {
    // Initialize vectors with distinct patterns
    __m512i a_i8 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i b_i8 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Generate mask using immediate constant
    __mmask64 mask64 = 0xAAAAAAAAAAAAAAAAULL;
    
    // First blend: V64QImode
    __m512i result_i8 = blend_v64qi(a_i8, b_i8, mask64);
    
    // Convert to 16-bit vectors for next blend
    __m512i a_i16 = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i b_i16 = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Generate mask using comparison
    __mmask32 mask32 = _mm512_cmp_epi16_mask(a_i16, b_i16, _MM_CMPINT_GT);
    
    // Second blend: V32HImode
    __m512i result_i16 = blend_v32hi(a_i16, b_i16, mask32);
    
    // Convert to float vectors for final blend
    __m512 a_f32 = _mm512_set_ps(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 b_f32 = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    
    // Generate mask using bitwise operations
    __mmask16 mask16_comp = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_GT_OQ);
    __mmask16 mask16 = _knot_mask16(mask16_comp);
    
    // Third blend: V16SFmode
    __m512 result_f32 = blend_v16sf(a_f32, b_f32, mask16);
    
    // Reduce to scalar
    return _mm512_reduce_add_ps(result_f32);
}

int main() {
    double checksum = 0.0;
    
    // Initialize vectors for all modes
    __m512i vec_i8_a = _mm512_set1_epi8(1);
    __m512i vec_i8_b = _mm512_set1_epi8(2);
    
    __m512i vec_i16_a = _mm512_set1_epi16(10);
    __m512i vec_i16_b = _mm512_set1_epi16(20);
    
    __m512i vec_i32_a = _mm512_set1_epi32(100);
    __m512i vec_i32_b = _mm512_set1_epi32(200);
    
    __m512i vec_i64_a = _mm512_set1_epi64(1000);
    __m512i vec_i64_b = _mm512_set1_epi64(2000);
    
    __m512 vec_f32_a = _mm512_set1_ps(1.5f);
    __m512 vec_f32_b = _mm512_set1_ps(2.5f);
    
    __m512d vec_f64_a = _mm512_set1_pd(10.5);
    __m512d vec_f64_b = _mm512_set1_pd(20.5);
    
#ifdef __AVX512FP16__
    __m512h vec_f16_a = _mm512_set1_ph(1.0f);
    __m512h vec_f16_b = _mm512_set1_ph(2.0f);
    
    __m512bh vec_bf16_a = _mm512_set1_ph(1.0f);
    __m512bh vec_bf16_b = _mm512_set1_ph(2.0f);
#endif
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 100; i++) {
        // Vary mask generation based on loop iteration
        __mmask64 mask64;
        __mmask32 mask32;
        __mmask16 mask16;
        __mmask8 mask8;
        
        if (i % 3 == 0) {
            // Use immediate constants
            mask64 = 0x5555555555555555ULL;
            mask32 = 0xAAAAAAAA;
            mask16 = 0xAAAA;
            mask8 = 0xAA;
        } else if (i % 3 == 1) {
            // Use comparison-generated masks
            mask32 = _mm512_cmp_epi16_mask(vec_i16_a, vec_i16_b, _MM_CMPINT_LT);
            mask16 = _mm512_cmp_epi32_mask(vec_i32_a, vec_i32_b, _MM_CMPINT_LT);
            mask8 = _mm512_cmp_epi64_mask(vec_i64_a, vec_i64_b, _MM_CMPINT_LT);
            mask64 = (__mmask64)mask32 | ((__mmask64)mask32 << 32);
        } else {
            // Use bitwise operations on masks
            __mmask16 temp16 = _mm512_cmp_ps_mask(vec_f32_a, vec_f32_b, _CMP_LT_OQ);
            mask16 = _knot_mask16(temp16);
            mask8 = _mm512_cmp_pd_mask(vec_f64_a, vec_f64_b, _CMP_LT_OQ);
            mask32 = _kor_mask32(mask16, mask16 << 16);
            mask64 = (__mmask64)mask32 | ((__mmask64)mask32 << 32);
        }
        
        // Data-dependent selection of blend type
        __m512i result;
        if (i % 2 == 0) {
            // V64QImode blend
            result = blend_v64qi(vec_i8_a, vec_i8_b, mask64);
        } else {
            // V32HImode blend
            result = blend_v32hi(vec_i16_a, vec_i16_b, mask32);
        }
        
        // Perform blends for all modes in each iteration
        __m512i r1 = blend_v16si(vec_i32_a, vec_i32_b, mask16);
        __m512i r2 = blend_v8di(vec_i64_a, vec_i64_b, mask8);
        __m512 r3 = blend_v16sf(vec_f32_a, vec_f32_b, mask16);
        __m512d r4 = blend_v8df(vec_f64_a, vec_f64_b, mask8);
        
#ifdef __AVX512FP16__
        __m512h r5 = blend_v32hf(vec_f16_a, vec_f16_b, mask32);
        __m512bh r6 = blend_v32bf(vec_bf16_a, vec_bf16_b, mask32);
#endif
        
        // Accumulate reductions to prevent dead code elimination
        int64_t sum_i8 = _mm512_reduce_add_epi64(result);
        int64_t sum_i16 = _mm512_reduce_add_epi64(r1);
        int64_t sum_i32 = _mm512_reduce_add_epi64(r2);
        float sum_f32 = _mm512_reduce_add_ps(r3);
        double sum_f64 = _mm512_reduce_add_pd(r4);
        
        checksum += sum_i8 + sum_i16 + sum_i32 + sum_f32 + sum_f64;
        
#ifdef __AVX512FP16__
        // Manual reduction for half-precision
        __m256h r5_low = _mm512_castph512_ph256(r5);
        __m256h r5_high = _mm512_extractf32x8_ps(_mm512_castph_ps(r5), 1);
        float sum_f16 = 0;
        for (int j = 0; j < 8; j++) {
            sum_f16 += (float)r5_low[j];
            sum_f16 += (float)r5_high[j];
        }
        checksum += sum_f16;
#endif
    }
    
    // Call chained blend function
    float chain_result = chain_blend_operations(1);
    checksum += chain_result;
    
    // Additional test: blend different vector types in sequence
    {
        // Start with integer blend
        __m512i int_result = blend_v16si(
            _mm512_set1_epi32(1),
            _mm512_set1_epi32(2),
            0x5555
        );
        
        // Convert to float and blend
        __m512 float_vec = _mm512_cvtepi32_ps(int_result);
        __m512 float_result = blend_v16sf(
            float_vec,
            _mm512_set1_ps(3.0f),
            0xAAAA
        );
        
        // Convert to double and blend
        __m512d double_vec = _mm512_cvtps_pd(_mm512_castps512_ps256(float_result));
        __m512d double_result = blend_v8df(
            double_vec,
            _mm512_set1_pd(4.0),
            0x55
        );
        
        checksum += _mm512_reduce_add_pd(double_result);
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
