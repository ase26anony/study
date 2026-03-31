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
    
    // Generate mask using comparison intrinsic
    __m512i cmp_a = _mm512_set1_epi16(15);
    __m512i cmp_b = _mm512_set1_epi16(16);
    __mmask32 mask32 = _mm512_cmpgt_epi16_mask(cmp_a, cmp_b);
    
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
    
    // Third blend: V16SFmode
    __m512 result_f32 = blend_v16sf(a_f32, b_f32, mask16);
    
    // Reduce to scalar
    return _mm512_reduce_add_ps(result_f32);
}

int main() {
    double checksum = 0.0;
    
    // Initialize vectors for each mode
    __m512i a_epi32 = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    __m512i b_epi32 = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i a_epi64 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i b_epi64 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    
    __m512d a_pd = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d b_pd = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    
    // Data-dependent control flow
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            // Use immediate mask for V16SImode
            __mmask16 mask16 = 0xAAAA;
            __m512i result = blend_v16si(a_epi32, b_epi32, mask16);
            
            // Reduce and accumulate
            int64_t sum = 0;
            int32_t* ptr = (int32_t*)&result;
            for (int j = 0; j < 16; j++) {
                sum += ptr[j];
            }
            checksum += sum;
        } else {
            // Use comparison mask for V8DImode
            __m512i cmp_a = _mm512_set1_epi64(3);
            __m512i cmp_b = _mm512_set1_epi64(4);
            __mmask8 mask8 = _mm512_cmpgt_epi64_mask(cmp_a, cmp_b);
            
            __m512i result = blend_v8di(a_epi64, b_epi64, mask8);
            
            // Reduce and accumulate
            int64_t sum = 0;
            int64_t* ptr = (int64_t*)&result;
            for (int j = 0; j < 8; j++) {
                sum += ptr[j];
            }
            checksum += sum;
        }
        
        // V8DFmode blend with varying masks
        __mmask8 mask_pd;
        if (i % 3 == 0) {
            mask_pd = 0xAA;  // Immediate
        } else if (i % 3 == 1) {
            // Generate mask via comparison
            __m512d cmp_a = _mm512_set1_pd(3.0);
            __m512d cmp_b = _mm512_set1_pd(2.0);
            mask_pd = _mm512_cmp_pd_mask(cmp_a, cmp_b, _CMP_GT_OQ);
        } else {
            // Generate mask via bitwise operations
            __mmask8 m1 = 0xAA;
            __mmask8 m2 = 0x55;
            mask_pd = _kor_mask8(m1, m2);
        }
        
        __m512d result_pd = blend_v8df(a_pd, b_pd, mask_pd);
        
        // Reduce and accumulate
        double sum_pd = _mm512_reduce_add_pd(result_pd);
        checksum += sum_pd;
    }
    
    // Call chained blend operations
    float chain_result = chain_blend_operations(1);
    checksum += chain_result;
    
#ifdef __AVX512FP16__
    // Initialize FP16 vectors
    __m512h a_f16 = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    
    __m512h b_f16 = _mm512_set_ph(
        31.0f,30.0f,29.0f,28.0f,27.0f,26.0f,25.0f,24.0f,
        23.0f,22.0f,21.0f,20.0f,19.0f,18.0f,17.0f,16.0f,
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    // V32HFmode blend
    __mmask32 mask_f16 = 0xAAAAAAAA;
    __m512h result_f16 = blend_v32hf(a_f16, b_f16, mask_f16);
    
    // V32BFmode blend (using same data cast to bfloat16)
    __m512bh a_bf16 = _mm512_castph_bf16(a_f16);
    __m512bh b_bf16 = _mm512_castph_bf16(b_f16);
    __m512bh result_bf16 = blend_v32bf(a_bf16, b_bf16, mask_f16);
    
    // Reduce FP16 results
    float sum_f16 = 0.0f;
    _Float16* ptr_f16 = (_Float16*)&result_f16;
    for (int i = 0; i < 32; i++) {
        sum_f16 += ptr_f16[i];
    }
    checksum += sum_f16;
#endif
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
