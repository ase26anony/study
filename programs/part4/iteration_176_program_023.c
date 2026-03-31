#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Function declarations with noinline to ensure separate expansion
__attribute__((noinline)) __m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline)) __m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline)) __m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline)) __m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline)) __m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline)) __m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to create complex control flow
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask = 0;
    
    // Switch statement for different mask generation methods
    switch (iteration % 4) {
        case 0:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0x55));
            break;
        case 1:
            // Immediate mask
            mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
            break;
        case 2:
            // Logical combination of masks
            __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(64));
            __mmask64 m2 = (__mmask64)0x5555555555555555ULL;
            mask = _kor_mask64(m1, m2);
            break;
        case 3:
            // Pattern based on iteration
            mask = (__mmask64)(0xFFFFFFFFFFFFFFFFULL >> (iteration % 64));
            break;
    }
    
    return mask;
}

// Data dependency chain function
float process_data_dependency_chain(int mode_selector) {
    // Initialize vectors with patterned data
    __m512i int_vec1 = _mm512_set_epi8(
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
    );
    
    __m512i int_vec2 = _mm512_set1_epi8(0xFF);
    
    // Start with V64QImode blend
    __mmask64 mask64 = generate_complex_mask64(mode_selector, int_vec1);
    __m512i result64qi = blend_v64qi(int_vec1, int_vec2, mask64);
    
    // Use result to generate mask for V32HImode
    __mmask32 mask32 = _mm512_cmpeq_epi16_mask(result64qi, _mm512_set1_epi16(0xFFFF));
    __m512i result32hi = blend_v32hi(
        _mm512_set1_epi16(0x1234),
        _mm512_set1_epi16(0x5678),
        mask32
    );
    
    // Generate mask for V16SImode from previous result
    __mmask16 mask16 = _mm512_cmpeq_epi32_mask(result32hi, _mm512_set1_epi32(0x12345678));
    __m512i result16si = blend_v16si(
        _mm512_set1_epi32(0xDEADBEEF),
        _mm512_set1_epi32(0xCAFEBABE),
        mask16
    );
    
    // Generate mask for V8DImode
    __mmask8 mask8 = _mm512_cmpeq_epi64_mask(result16si, _mm512_set1_epi64(0xDEADBEEFCAFEBABEULL));
    __m512i result8di = blend_v8di(
        _mm512_set1_epi64(0x1111111111111111ULL),
        _mm512_set1_epi64(0x2222222222222222ULL),
        mask8
    );
    
    // Perform horizontal sum on final integer result
    __m256i sum256 = _mm256_add_epi64(
        _mm512_castsi512_si256(result8di),
        _mm512_extracti64x4_epi64(result8di, 1)
    );
    __m128i sum128 = _mm_add_epi64(
        _mm256_castsi256_si128(sum256),
        _mm256_extracti128_si256(sum256, 1)
    );
    
    uint64_t sum = _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
    return (float)sum;
}

// Float processing with control flow
float process_float_blends(int iteration) {
    float checksum = 0.0f;
    
    // Initialize float vectors
    __m512 float_vec1 = _mm512_set_ps(
        63.0f, 62.0f, 61.0f, 60.0f, 59.0f, 58.0f, 57.0f, 56.0f,
        55.0f, 54.0f, 53.0f, 52.0f, 51.0f, 50.0f, 49.0f, 48.0f,
        47.0f, 46.0f, 45.0f, 44.0f, 43.0f, 42.0f, 41.0f, 40.0f,
        39.0f, 38.0f, 37.0f, 36.0f, 35.0f, 34.0f, 33.0f, 32.0f,
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
    
    __m512 float_vec2 = _mm512_set1_ps(100.0f);
    
    // Complex if-else chain for mask generation
    __mmask16 float_mask;
    if (iteration % 3 == 0) {
        // Comparison mask
        float_mask = _mm512_cmp_ps_mask(float_vec1, _mm512_set1_ps(32.0f), _CMP_GT_OQ);
    } else if (iteration % 3 == 1) {
        // Immediate mask
        float_mask = (__mmask16)0xAAAA;
    } else {
        // Logical operation on masks
        __mmask16 m1 = _mm512_cmp_ps_mask(float_vec1, _mm512_set1_ps(16.0f), _CMP_LT_OQ);
        __mmask16 m2 = (__mmask16)0x5555;
        float_mask = _kor_mask16(m1, m2);
    }
    
    // Perform V16SFmode blend
    __m512 float_result = blend_v16sf(float_vec1, float_vec2, float_mask);
    
    // Initialize double vectors
    __m512d double_vec1 = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    __m512d double_vec2 = _mm512_set1_pd(50.0);
    
    // Generate mask for V8DFmode based on float result
    __mmask8 double_mask = _mm512_cmp_pd_mask(
        _mm512_cvtps_pd(_mm512_castps512_ps256(float_result)),
        _mm512_set1_pd(75.0),
        _CMP_GT_OQ
    );
    
    // Perform V8DFmode blend
    __m512d double_result = blend_v8df(double_vec1, double_vec2, double_mask);
    
    // Horizontal reduction for checksum
    checksum += _mm512_reduce_add_ps(float_result);
    checksum += (float)_mm512_reduce_add_pd(double_result);
    
    return checksum;
}

#ifdef __AVX512FP16__
// Half-precision processing
float process_half_precision(int iteration) {
    float checksum = 0.0f;
    
    // Initialize half-precision vectors
    __m512h half_vec1 = _mm512_set1_ph((_Float16)1.5f);
    __m512h half_vec2 = _mm512_set1_ph((_Float16)3.0f);
    
    __m512bh bfloat_vec1 = _mm512_set1_bh((__bf16)2.0f);
    __m512bh bfloat_vec2 = _mm512_set1_bh((__bf16)4.0f);
    
    // Generate masks with loop-dependent conditions
    __mmask32 half_mask;
    for (int i = 0; i < 4; i++) {
        if (i == iteration % 4) {
            half_mask = (__mmask32)(0xFFFFFFFFUL >> (i * 8));
            break;
        }
    }
    
    // Perform V32HFmode blend
    __m512h half_result = blend_v32hf(half_vec1, half_vec2, half_mask);
    
    // Perform V32BFmode blend with modified mask
    __mmask32 bfloat_mask = _kxor_mask32(half_mask, (__mmask32)0xFFFFFFFF);
    __m512bh bfloat_result = blend_v32bf(bfloat_vec1, bfloat_vec2, bfloat_mask);
    
    // Convert to float for checksum
    __m512 half_as_float = _mm512_cvtph_ps(half_result);
    checksum += _mm512_reduce_add_ps(half_as_float);
    
    return checksum;
}
#endif

int main() {
    float total_checksum = 0.0f;
    
    printf("Starting AVX-512 blend coverage test...\n");
    
    // Loop with non-trivial control flow
    for (int i = 0; i < 8; i++) {
        // Process integer blends with data dependency chain
        total_checksum += process_data_dependency_chain(i);
        
        // Process float blends
        total_checksum += process_float_blends(i);
        
        #ifdef __AVX512FP16__
        // Process half-precision blends
        total_checksum += process_half_precision(i);
        #endif
        
        // Additional direct calls to ensure all modes are covered
        if (i % 2 == 0) {
            // Direct V64QImode call
            __m512i v1 = _mm512_set1_epi8(i);
            __m512i v2 = _mm512_set1_epi8(i * 2);
            __mmask64 k = (__mmask64)(0x5555555555555555ULL ^ (i * 0x1111111111111111ULL));
            __m512i res = blend_v64qi(v1, v2, k);
            
            // Add to checksum
            __m256i sum256 = _mm256_add_epi8(
                _mm512_castsi512_si256(res),
                _mm512_extracti64x4_epi64(res, 1)
            );
            // Simple reduction
            for (int j = 0; j < 32; j++) {
                total_checksum += ((int8_t*)&sum256)[j];
            }
        }
    }
    
    printf("Final checksum: %f\n", total_checksum);
    printf("Test completed.\n");
    
    return 0;
}
