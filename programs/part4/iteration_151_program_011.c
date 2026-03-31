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
    
    // Generate mask dynamically using comparison
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
    __mmask16 mask16_base = 0xAAAA;
    __mmask16 mask16_inv = _knot_mask16(mask16_base);
    __mmask16 mask16 = _kor_mask16(mask16_base, 0x5555);
    
    // Third blend: V16SFmode (conditionally based on mode_selector)
    __m512 result_f32;
    if (mode_selector & 1) {
        result_f32 = blend_v16sf(a_f32, b_f32, mask16);
    } else {
        result_f32 = blend_v16sf(b_f32, a_f32, mask16_inv);
    }
    
    // Reduce to scalar
    return _mm512_reduce_add_ps(result_f32);
}

int main() {
    float checksum = 0.0f;
    
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
    __m512h vec_f16_a = _mm512_set1_ph((__fp16)0.5f);
    __m512h vec_f16_b = _mm512_set1_ph((__fp16)1.5f);
    
    __m512bh vec_bf16_a = _mm512_set1_bh((__bf16)0.25f);
    __m512bh vec_bf16_b = _mm512_set1_bh((__bf16)0.75f);
#endif
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 10; i++) {
        // Vary mask patterns based on loop iteration
        __mmask64 mask64 = (i % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0xAAAAAAAAAAAAAAAAULL;
        __mmask32 mask32 = (i % 3) ? 0xFFFFFFFF : 0x55555555;
        __mmask16 mask16 = (i % 4) ? 0xFFFF : 0xAAAA;
        __mmask8 mask8 = (i % 5) ? 0xFF : 0xAA;
        
        // Data-dependent selection of blend type
        if (i % 2 == 0) {
            // Blend V64QImode
            __m512i result_i8 = blend_v64qi(vec_i8_a, vec_i8_b, mask64);
            
            // Reduce and accumulate
            uint64_t sum_i8 = 0;
            uint8_t* ptr = (uint8_t*)&result_i8;
            for (int j = 0; j < 64; j++) {
                sum_i8 += ptr[j];
            }
            checksum += (float)sum_i8;
        } else {
            // Blend V32HImode
            __m512i result_i16 = blend_v32hi(vec_i16_a, vec_i16_b, mask32);
            
            // Reduce and accumulate
            uint64_t sum_i16 = 0;
            uint16_t* ptr = (uint16_t*)&result_i16;
            for (int j = 0; j < 32; j++) {
                sum_i16 += ptr[j];
            }
            checksum += (float)sum_i16;
        }
        
        // Always blend V16SImode
        __m512i result_i32 = blend_v16si(vec_i32_a, vec_i32_b, mask16);
        uint64_t sum_i32 = 0;
        uint32_t* ptr32 = (uint32_t*)&result_i32;
        for (int j = 0; j < 16; j++) {
            sum_i32 += ptr32[j];
        }
        checksum += (float)sum_i32;
        
        // Blend V8DImode with dynamic mask generation
        __mmask8 mask8_dyn = _mm512_cmp_epi64_mask(vec_i64_a, vec_i64_b, _MM_CMPINT_LT);
        __m512i result_i64 = blend_v8di(vec_i64_a, vec_i64_b, mask8_dyn);
        uint64_t sum_i64 = 0;
        uint64_t* ptr64 = (uint64_t*)&result_i64;
        for (int j = 0; j < 8; j++) {
            sum_i64 += ptr64[j];
        }
        checksum += (float)sum_i64;
        
        // Blend V16SFmode
        __m512 result_f32 = blend_v16sf(vec_f32_a, vec_f32_b, mask16);
        checksum += _mm512_reduce_add_ps(result_f32);
        
        // Blend V8DFmode
        __m512d result_f64 = blend_v8df(vec_f64_a, vec_f64_b, mask8);
        double sum_f64 = 0.0;
        double* ptr_f64 = (double*)&result_f64;
        for (int j = 0; j < 8; j++) {
            sum_f64 += ptr_f64[j];
        }
        checksum += (float)sum_f64;
        
#ifdef __AVX512FP16__
        // Blend V32HFmode
        __m512h result_f16 = blend_v32hf(vec_f16_a, vec_f16_b, mask32);
        
        // Blend V32BFmode (using same intrinsic)
        __m512bh result_bf16 = blend_v32bf(vec_bf16_a, vec_bf16_b, mask32);
        
        // Reduce FP16 results (manual reduction)
        __fp16* ptr_f16 = (__fp16*)&result_f16;
        float sum_f16 = 0.0f;
        for (int j = 0; j < 32; j++) {
            sum_f16 += (float)ptr_f16[j];
        }
        checksum += sum_f16;
#endif
    }
    
    // Execute chained blend operations
    for (int i = 0; i < 5; i++) {
        checksum += chain_blend_operations(i);
    }
    
    // Additional test: mixed mode transitions in a complex pattern
    {
        __m512i a = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __m512i b = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        
        // Generate mask using comparison
        __mmask16 mask_cmp = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LT);
        
        // Blend with comparison mask
        __m512i blended = blend_v16si(a, b, mask_cmp);
        
        // Convert to float and blend again
        __m512 a_f = _mm512_cvtepi32_ps(a);
        __m512 b_f = _mm512_cvtepi32_ps(b);
        __m512 blended_f = blend_v16sf(a_f, b_f, mask_cmp);
        
        // Reduce both results
        uint64_t sum_int = 0;
        uint32_t* ptr_int = (uint32_t*)&blended;
        for (int j = 0; j < 16; j++) {
            sum_int += ptr_int[j];
        }
        checksum += (float)sum_int + _mm512_reduce_add_ps(blended_f);
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
