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
    __mmask64 mask;
    
    // Switch statement for control flow diversity
    switch (iteration % 4) {
        case 0:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(0));
            break;
        case 1:
            // Immediate mask (alternating pattern)
            mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
            break;
        case 2:
            // Logical combination of masks
            __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(64));
            __mmask64 m2 = (__mmask64)0x5555555555555555ULL;
            mask = _kor_mask64(m1, m2);
            break;
        case 3:
            // Generated from loop index
            mask = (__mmask64)((uint64_t)1 << (iteration % 64));
            break;
    }
    
    return mask;
}

// Data dependency chain function
float process_dependency_chain(int mode_selector) {
    // Initialize vectors with patterned data
    __m512i int_vec1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i int_vec2 = _mm512_set1_epi8(100);
    __m512 float_vec1 = _mm512_set_ps(
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    );
    __m512 float_vec2 = _mm512_set1_ps(50.0f);
    __m512d double_vec1 = _mm512_set_pd(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    __m512d double_vec2 = _mm512_set1_pd(100.0);
    
    float checksum = 0.0f;
    
    // Loop with index-dependent mask generation
    for (int i = 0; i < 8; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_complex_mask64(i, int_vec1);
        __mmask32 mask32 = (__mmask32)(0xAAAAAAAAU >> (i % 4));
        __mmask16 mask16 = _mm512_cmp_ps_mask(float_vec1, float_vec2, _CMP_LT_OQ);
        __mmask8 mask8 = _mm512_cmp_pd_mask(double_vec1, double_vec2, _CMP_GT_OQ);
        
        // If-else chain for control flow
        if (i % 2 == 0) {
            // V64QImode blend
            __m512i result64qi = blend_v64qi(int_vec1, int_vec2, mask64);
            
            // Use result to generate mask for next blend (data dependency)
            __mmask32 new_mask32 = _mm512_cmpeq_epi16_mask(
                result64qi, 
                _mm512_set1_epi16(100)
            );
            
            // V32HImode blend with derived mask
            __m512i result32hi = blend_v32hi(
                _mm512_set1_epi16(i),
                _mm512_set1_epi16(i * 2),
                new_mask32
            );
            
            // Accumulate results
            checksum += (float)_mm512_reduce_add_epi16(result32hi);
        } else {
            // V16SFmode blend
            __m512 result16sf = blend_v16sf(float_vec1, float_vec2, mask16);
            
            // V8DFmode blend
            __m512d result8df = blend_v8df(double_vec1, double_vec2, mask8);
            
            // Accumulate results
            checksum += _mm512_reduce_add_ps(result16sf);
            checksum += (float)_mm512_reduce_add_pd(result8df);
        }
        
        // Switch statement for mode selection
        switch (mode_selector) {
            case 0:
                // V16SImode blend
                __m512i result16si = blend_v16si(
                    _mm512_set1_epi32(i),
                    _mm512_set1_epi32(i * 3),
                    mask16
                );
                checksum += (float)_mm512_reduce_add_epi32(result16si);
                break;
                
            case 1:
                // V8DImode blend
                __m512i result8di = blend_v8di(
                    _mm512_set1_epi64(i),
                    _mm512_set1_epi64(i * 5),
                    mask8
                );
                checksum += (float)_mm512_reduce_add_epi64(result8di);
                break;
        }
        
        // Modify vectors for next iteration
        int_vec1 = _mm512_add_epi8(int_vec1, _mm512_set1_epi8(1));
        float_vec1 = _mm512_add_ps(float_vec1, _mm512_set1_ps(1.0f));
        double_vec1 = _mm512_add_pd(double_vec1, _mm512_set1_pd(1.0));
    }
    
    return checksum;
}

#ifdef __AVX512FP16__
float process_half_precision() {
    __m512h half_vec1 = _mm512_set1_ph(1.0f);
    __m512h half_vec2 = _mm512_set1_ph(2.0f);
    __m512bh bfloat_vec1 = _mm512_set1_bh(1.0f);
    __m512bh bfloat_vec2 = _mm512_set1_bh(3.0f);
    
    float checksum = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        // Generate masks with logical operations
        __mmask32 mask_half = (__mmask32)(0x55555555U ^ (i * 0x11111111U));
        __mmask32 mask_bfloat = _kor_mask32(
            (__mmask32)(0xAAAAAAAAU),
            (__mmask32)(i * 0x01010101U)
        );
        
        // V32HFmode blend
        __m512h result32hf = blend_v32hf(half_vec1, half_vec2, mask_half);
        
        // V32BFmode blend
        __m512bh result32bf = blend_v32bf(bfloat_vec1, bfloat_vec2, mask_bfloat);
        
        // Convert and accumulate (simplified - actual reduction would need more code)
        __m512 float_result = _mm512_cvtph_ps(result32hf);
        checksum += _mm512_reduce_add_ps(float_result);
        
        // Modify for next iteration
        half_vec1 = _mm512_add_ph(half_vec1, _mm512_set1_ph(0.5f));
        bfloat_vec1 = _mm512_add_bh(bfloat_vec1, _mm512_set1_bh(0.5f));
    }
    
    return checksum;
}
#endif

int main() {
    float total_checksum = 0.0f;
    
    printf("Starting AVX-512 blend coverage test...\n");
    
    // Test with different mode selectors
    for (int selector = 0; selector < 2; selector++) {
        total_checksum += process_dependency_chain(selector);
    }
    
#ifdef __AVX512FP16__
    printf("Testing half-precision blends (requires -mavx512fp16)...\n");
    total_checksum += process_half_precision();
#endif
    
    printf("Final checksum: %f\n", total_checksum);
    printf("Test completed.\n");
    
    return 0;
}
