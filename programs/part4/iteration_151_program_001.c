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

// Helper function for V32BFmode blends (same intrinsic as HF)
static inline __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 mask) {
    return _mm512_mask_blend_ph(mask, a, b);
}
#endif

// Chained blend operations: epi8 -> epi16 -> ps
static inline float chain_blend_operations(int mode_selector) {
    // Initialize source vectors with distinct patterns
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
    __mmask16 mask16 = _knot_mask16(mask16_comp); // Invert the mask
    
    // Third blend: V16SFmode
    __m512 result_f32 = blend_v16sf(a_f32, b_f32, mask16);
    
    // Reduce to scalar
    return _mm512_reduce_add_ps(result_f32);
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize vectors for each mode
    __m512i a_i8 = _mm512_set1_epi8(1);
    __m512i b_i8 = _mm512_set1_epi8(2);
    
    __m512i a_i16 = _mm512_set1_epi16(10);
    __m512i b_i16 = _mm512_set1_epi16(20);
    
    __m512i a_i32 = _mm512_set1_epi32(100);
    __m512i b_i32 = _mm512_set1_epi32(200);
    
    __m512i a_i64 = _mm512_set1_epi64(1000);
    __m512i b_i64 = _mm512_set1_epi64(2000);
    
    __m512 a_f32 = _mm512_set1_ps(1.5f);
    __m512 b_f32 = _mm512_set1_ps(2.5f);
    
    __m512d a_f64 = _mm512_set1_pd(10.5);
    __m512d b_f64 = _mm512_set1_pd(20.5);
    
    // Data-dependent control flow
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            // Use immediate mask
            __mmask64 mask64 = 0x5555555555555555ULL;
            __m512i result = blend_v64qi(a_i8, b_i8, mask64);
            
            // Horizontal sum to prevent elimination
            uint64_t sum = 0;
            for (int j = 0; j < 64; j++) {
                sum += ((int8_t*)&result)[j];
            }
            checksum += sum;
        } else if (i % 3 == 1) {
            // Use comparison mask
            __mmask32 mask32 = _mm512_cmp_epi16_mask(a_i16, b_i16, _MM_CMPINT_LT);
            __m512i result = blend_v32hi(a_i16, b_i16, mask32);
            
            // Horizontal sum
            uint64_t sum = 0;
            for (int j = 0; j < 32; j++) {
                sum += ((int16_t*)&result)[j];
            }
            checksum += sum;
        } else {
            // Use combined masks
            __mmask16 mask1 = _mm512_cmp_epi32_mask(a_i32, b_i32, _MM_CMPINT_EQ);
            __mmask16 mask2 = 0xAAAA;
            __mmask16 mask32_combined = _kor_mask16(mask1, mask2);
            
            __m512i result = blend_v16si(a_i32, b_i32, mask32_combined);
            
            // Horizontal sum
            uint64_t sum = 0;
            for (int j = 0; j < 16; j++) {
                sum += ((int32_t*)&result)[j];
            }
            checksum += sum;
        }
    }
    
    // Test V8DImode with various masks
    for (int i = 0; i < 50; i++) {
        __mmask8 mask = (i % 2 == 0) ? 0xAA : 0x55;
        __m512i result = blend_v8di(a_i64, b_i64, mask);
        
        // Reduce using intrinsic
        __m256i low = _mm512_castsi512_si256(result);
        __m256i high = _mm512_extracti64x4_epi64(result, 1);
        __m256i sum256 = _mm256_add_epi64(low, high);
        __m128i sum128 = _mm_add_epi64(_mm256_castsi256_si128(sum256),
                                      _mm256_extracti128_si256(sum256, 1));
        checksum += _mm_extract_epi64(sum128, 0) + _mm_extract_epi64(sum128, 1);
    }
    
    // Test V16SFmode and V8DFmode
    for (int i = 0; i < 25; i++) {
        // Generate mask using comparison
        __mmask16 mask_ps = _mm512_cmp_ps_mask(a_f32, b_f32, 
            (i % 2 == 0) ? _CMP_LT_OQ : _CMP_GT_OQ);
        
        __m512 result_ps = blend_v16sf(a_f32, b_f32, mask_ps);
        float sum_ps = _mm512_reduce_add_ps(result_ps);
        checksum += (uint64_t)sum_ps;
        
        // Double precision
        __mmask8 mask_pd = _mm512_cmp_pd_mask(a_f64, b_f64,
            (i % 2 == 0) ? _CMP_LT_OQ : _CMP_GT_OQ);
        
        __m512d result_pd = blend_v8df(a_f64, b_f64, mask_pd);
        double sum_pd = _mm512_reduce_add_pd(result_pd);
        checksum += (uint64_t)sum_pd;
    }
    
#ifdef __AVX512FP16__
    // Test half-precision modes if supported
    __m512h a_half = _mm512_set1_ph(1.0f);
    __m512h b_half = _mm512_set1_ph(2.0f);
    
    __m512bh a_bf16 = _mm512_set1_ph(1.0f);
    __m512bh b_bf16 = _mm512_set1_ph(2.0f);
    
    for (int i = 0; i < 20; i++) {
        __mmask32 mask_half = (i % 2 == 0) ? 0xAAAAAAAA : 0x55555555;
        
        // V32HFmode
        __m512h result_half = blend_v32hf(a_half, b_half, mask_half);
        
        // V32BFmode
        __m512bh result_bf16 = blend_v32bf(a_bf16, b_bf16, mask_half);
        
        // Simple reduction for half-precision
        uint64_t sum = 0;
        for (int j = 0; j < 32; j++) {
            sum += ((_Float16*)&result_half)[j];
            sum += ((_Float16*)&result_bf16)[j];
        }
        checksum += sum;
    }
#endif
    
    // Execute chained blend operations
    float chain_result = 0.0f;
    for (int i = 0; i < 10; i++) {
        chain_result += chain_blend_operations(i);
    }
    checksum += (uint64_t)chain_result;
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
