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

#ifdef __AVX512FP16__
__attribute__((noinline)) __m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline)) __m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

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

// Helper function to create complex mask patterns
__mmask64 generate_complex_mask64(int iteration) {
    __mmask64 mask;
    
    // Control flow that prevents constant folding
    if (iteration % 3 == 0) {
        // Method 1: Immediate mask with pattern
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (iteration % 3 == 1) {
        // Method 2: Logical combination of patterns
        __mmask64 m1 = 0x5555555555555555ULL;
        __mmask64 m2 = 0xF0F0F0F0F0F0F0F0ULL;
        mask = _kor_mask64(m1, m2);
    } else {
        // Method 3: Alternating pattern based on iteration
        mask = (iteration % 2) ? 0xFFFFFFFFFFFFFFFFULL : 0x0ULL;
    }
    
    return mask;
}

// Function with non-trivial control flow for mask generation
__mmask16 generate_dynamic_mask16(__m512 a, __m512 b, int mode) {
    __mmask16 mask;
    
    switch (mode) {
        case 0:
            // Comparison mask
            mask = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
            break;
        case 1:
            // Pattern mask
            mask = 0xAAAA;
            break;
        case 2:
            // Logical operation on comparison masks
            __mmask16 m1 = _mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ);
            __mmask16 m2 = _mm512_cmp_ps_mask(a, b, _CMP_GT_OQ);
            mask = _kxor_mask16(m1, m2);
            break;
        default:
            mask = 0xFFFF;
    }
    
    return mask;
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i v64qi_a = _mm512_set1_epi8(0x11);
    __m512i v64qi_b = _mm512_set1_epi8(0x22);
    
    __m512i v32hi_a = _mm512_set1_epi16(0x3333);
    __m512i v32hi_b = _mm512_set1_epi16(0x4444);
    
    __m512i v16si_a = _mm512_set1_epi32(0x55555555);
    __m512i v16si_b = _mm512_set1_epi32(0x66666666);
    
    __m512i v8di_a = _mm512_set1_epi64(0x7777777777777777ULL);
    __m512i v8di_b = _mm512_set1_epi64(0x8888888888888888ULL);
    
    __m512 v16sf_a = _mm512_set1_ps(1.0f);
    __m512 v16sf_b = _mm512_set1_ps(2.0f);
    
    __m512d v8df_a = _mm512_set1_pd(3.0);
    __m512d v8df_b = _mm512_set1_pd(4.0);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.5f);
    __m512h v32hf_b = _mm512_set1_ph(2.5f);
    
    __m512bh v32bf_a = _mm512_castsi512_ph(_mm512_set1_epi16(0x3C00)); // 1.0 in FP16
    __m512bh v32bf_b = _mm512_castsi512_ph(_mm512_set1_epi16(0x4000)); // 2.0 in FP16
#endif
    
    // Loop with data-dependent mask generation
    for (int i = 0; i < 10; i++) {
        // Generate masks using different methods
        __mmask64 mask64 = generate_complex_mask64(i);
        __mmask32 mask32 = (i % 2) ? 0xAAAAAAAA : 0x55555555;
        __mmask16 mask16 = generate_dynamic_mask16(v16sf_a, v16sf_b, i % 3);
        __mmask8 mask8 = (__mmask8)(0xAA >> (i % 8));
        
        // Perform blends in different modes
        __m512i res64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        __m512i res32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        __m512i res16si = blend_v16si(v16si_a, v16si_b, mask16);
        __m512i res8di = blend_v8di(v8di_a, v8di_b, mask8);
        __m512 res16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
        __m512d res8df = blend_v8df(v8df_a, v8df_b, mask8);
        
#ifdef __AVX512FP16__
        __m512h res32hf = blend_v32hf(v32hf_a, v32hf_b, mask32);
        __m512bh res32bf = blend_v32bf(v32bf_a, v32bf_b, mask32);
#endif
        
        // Create data dependency chain between different modes
        // Use result from integer blend to generate mask for float blend
        if (i > 0) {
            __mmask16 new_mask = _mm512_cmpeq_epi32_mask(res16si, v16si_a);
            res16sf = blend_v16sf(res16sf, v16sf_b, new_mask);
        }
        
        // Accumulate results into checksum (prevent optimization)
        uint8_t* ptr64qi = (uint8_t*)&res64qi;
        for (int j = 0; j < 64; j++) checksum += ptr64qi[j];
        
        uint16_t* ptr32hi = (uint16_t*)&res32hi;
        for (int j = 0; j < 32; j++) checksum += ptr32hi[j];
        
        uint32_t* ptr16si = (uint32_t*)&res16si;
        for (int j = 0; j < 16; j++) checksum += ptr16si[j];
        
        uint64_t* ptr8di = (uint64_t*)&res8di;
        for (int j = 0; j < 8; j++) checksum += ptr8di[j];
        
        float* ptr16sf = (float*)&res16sf;
        for (int j = 0; j < 16; j++) checksum += (uint64_t)ptr16sf[j];
        
        double* ptr8df = (double*)&res8df;
        for (int j = 0; j < 8; j++) checksum += (uint64_t)ptr8df[j];
        
#ifdef __AVX512FP16__
        // For half precision, we need to handle differently
        _Float16* ptr32hf = (_Float16*)&res32hf;
        for (int j = 0; j < 32; j++) checksum += (uint64_t)ptr32hf[j];
        
        _Float16* ptr32bf = (_Float16*)&res32bf;
        for (int j = 0; j < 32; j++) checksum += (uint64_t)ptr32bf[j];
#endif
        
        // Modify input data for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.1f));
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
