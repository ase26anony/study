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
    // Initialize data
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
    
    // Second blend: V32HImode (with data-dependent control flow)
    __m512i result_i16;
    if (mode_selector & 1) {
        result_i16 = blend_v32hi(a_i16, b_i16, mask32);
    } else {
        // Use complemented mask
        result_i16 = blend_v32hi(a_i16, b_i16, _knot_mask32(mask32));
    }
    
    // Convert to float for final blend
    __m512 a_f32 = _mm512_set_ps(
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f
    );
    
    __m512 b_f32 = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    // Generate mask using bitwise operations
    __mmask16 mask16_1 = _mm512_cmp_ps_mask(a_f32, b_f32, _CMP_GT_OQ);
    __mmask16 mask16_2 = _mm512_cmp_ps_mask(a_f32, _mm512_set1_ps(15.0f), _CMP_LT_OQ);
    __mmask16 mask16 = _kor_mask16(mask16_1, mask16_2);
    
    // Third blend: V16SFmode
    __m512 result_f32 = blend_v16sf(a_f32, b_f32, mask16);
    
    // Reduce to scalar
    return _mm512_reduce_add_ps(result_f32);
}

// Function to test V16SImode and V8DImode blends
static inline double test_integer_blends(int iteration) {
    // V16SImode blend
    __m512i a_si = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i b_si = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Vary mask generation based on iteration
    __mmask16 mask16_si;
    if (iteration % 3 == 0) {
        mask16_si = 0xAAAA;  // Immediate constant
    } else if (iteration % 3 == 1) {
        mask16_si = _mm512_cmp_epi32_mask(a_si, b_si, _MM_CMPINT_EQ);
    } else {
        mask16_si = _mm512_cmp_epi32_mask(a_si, _mm512_set1_epi32(7), _MM_CMPINT_GT);
    }
    
    __m512i result_si = blend_v16si(a_si, b_si, mask16_si);
    
    // V8DImode blend
    __m512i a_di = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i b_di = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    __mmask8 mask8_di = _mm512_cmp_epi64_mask(a_di, b_di, _MM_CMPINT_LT);
    __m512i result_di = blend_v8di(a_di, b_di, mask8_di);
    
    // Reduce both results
    int64_t sum_si = 0;
    int64_t sum_di = 0;
    
    // Manual reduction to prevent optimization
    for (int i = 0; i < 16; i++) {
        sum_si += ((int32_t*)&result_si)[i];
    }
    
    for (int i = 0; i < 8; i++) {
        sum_di += ((int64_t*)&result_di)[i];
    }
    
    return (double)sum_si + (double)sum_di;
}

// Function to test V8DFmode blends
static inline double test_double_blend(void) {
    __m512d a_df = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    __m512d b_df = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    
    // Generate mask using comparison with different predicates
    __mmask8 mask8_df = _mm512_cmp_pd_mask(a_df, b_df, _CMP_LE_OQ);
    
    __m512d result_df = blend_v8df(a_df, b_df, mask8_df);
    
    // Reduce
    return _mm512_reduce_add_pd(result_df);
}

#ifdef __AVX512FP16__
// Function to test half-precision blends
static inline float test_half_precision_blends(int selector) {
    // Initialize FP16 data
    __m512h a_hf = _mm512_set_ph(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
        16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
        24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
    );
    
    __m512h b_hf = _mm512_set_ph(
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    // Vary mask based on selector
    __mmask32 mask32_hf;
    if (selector == 0) {
        mask32_hf = 0xAAAAAAAA;  // Immediate constant
    } else {
        mask32_hf = _mm512_cmp_ph_mask(a_hf, b_hf, _CMP_GT_OQ);
    }
    
    __m512h result_hf = blend_v32hf(a_hf, b_hf, mask32_hf);
    
    // Convert to float for reduction (simplified reduction)
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += ((_Float16*)&result_hf)[i];
    }
    
    return sum;
}
#endif

int main(void) {
    double checksum = 0.0;
    
    // Test chained blend operations with different mode selectors
    for (int i = 0; i < 4; i++) {
        checksum += chain_blend_operations(i);
    }
    
    // Test integer blends in a loop
    for (int i = 0; i < 6; i++) {
        checksum += test_integer_blends(i);
    }
    
    // Test double precision blend
    checksum += test_double_blend();
    
#ifdef __AVX512FP16__
    // Test half-precision blends
    for (int i = 0; i < 3; i++) {
        checksum += test_half_precision_blends(i);
    }
#endif
    
    // Print final checksum
    printf("Final checksum: %f\n", checksum);
    
    return 0;
}
