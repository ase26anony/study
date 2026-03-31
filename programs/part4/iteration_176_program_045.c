#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure each intrinsic gets expanded independently
__attribute__((noinline, target("avx512bw")))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline, target("avx512bw")))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline, target("avx512f")))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline, target("avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to generate masks based on loop index
__mmask64 generate_dynamic_mask_64(int i) {
    // Alternate between different mask patterns
    switch (i % 4) {
        case 0: return 0xAAAAAAAAAAAAAAAAULL;  // Alternating pattern
        case 1: return 0x5555555555555555ULL;  // Opposite alternating
        case 2: return 0xFFFFFFFFFFFFFFFFULL;  // All ones
        default: return 0x0ULL;                // All zeros
    }
}

__mmask32 generate_dynamic_mask_32(int i) {
    return (__mmask32)(0xAAAAAAAAU >> (i % 4));
}

__mmask16 generate_dynamic_mask_16(int i) {
    return (__mmask16)(0xAAAA >> (i % 4));
}

__mmask8 generate_dynamic_mask_8(int i) {
    return (__mmask8)(0xAA >> (i % 4));
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i v64qi_a = _mm512_set1_epi8(0x11);
    __m512i v64qi_b = _mm512_set1_epi8(0x22);
    
    __m512i v32hi_a = _mm512_set1_epi16(0x1111);
    __m512i v32hi_b = _mm512_set1_epi16(0x2222);
    
    __m512i v16si_a = _mm512_set1_epi32(0x11111111);
    __m512i v16si_b = _mm512_set1_epi32(0x22222222);
    
    __m512i v8di_a = _mm512_set1_epi64(0x1111111111111111ULL);
    __m512i v8di_b = _mm512_set1_epi64(0x2222222222222222ULL);
    
    __m512 v16sf_a = _mm512_set1_ps(1.0f);
    __m512 v16sf_b = _mm512_set1_ps(2.0f);
    
    __m512d v8df_a = _mm512_set1_pd(1.0);
    __m512d v8df_b = _mm512_set1_pd(2.0);
    
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    
    __m512bh v32bf_a = _mm512_castsi512_ph(_mm512_set1_epi16(0x3C00)); // 1.0 in FP16
    __m512bh v32bf_b = _mm512_castsi512_ph(_mm512_set1_epi16(0x4000)); // 2.0 in FP16
    #endif
    
    // Control flow: loop with mask generation based on iteration
    for (int i = 0; i < 8; i++) {
        // Method 1: Immediate masks
        __mmask64 mask64_imm = generate_dynamic_mask_64(i);
        __mmask32 mask32_imm = generate_dynamic_mask_32(i);
        __mmask16 mask16_imm = generate_dynamic_mask_16(i);
        __mmask8 mask8_imm = generate_dynamic_mask_8(i);
        
        // Method 2: Comparison masks (dynamic)
        __mmask16 cmp_mask16 = _mm512_cmpeq_epi32_mask(
            _mm512_add_epi32(v16si_a, _mm512_set1_epi32(i)),
            v16si_b
        );
        
        __mmask8 cmp_mask8 = _mm512_cmpeq_epi64_mask(
            _mm512_add_epi64(v8di_a, _mm512_set1_epi64(i)),
            v8di_b
        );
        
        __mmask16 cmp_mask_ps = _mm512_cmp_ps_mask(
            v16sf_a, 
            _mm512_add_ps(v16sf_b, _mm512_set1_ps(i)),
            _CMP_LT_OQ
        );
        
        __mmask8 cmp_mask_pd = _mm512_cmp_pd_mask(
            v8df_a,
            _mm512_add_pd(v8df_b, _mm512_set1_pd(i)),
            _CMP_LT_OQ
        );
        
        // Method 3: Logical mask operations
        __mmask64 mask64_logic = _kor_mask64(mask64_imm, 
            _mm512_cmpeq_epi8_mask(v64qi_a, v64qi_b));
        
        __mmask32 mask32_logic = _kxor_mask32(mask32_imm, 
            _mm512_cmpeq_epi16_mask(v32hi_a, v32hi_b));
        
        __mmask16 mask16_logic = _kand_mask16(mask16_imm, cmp_mask16);
        __mmask8 mask8_logic = _kand_mask8(mask8_imm, cmp_mask8);
        
        // Perform blends with different mask types
        __m512i result_v64qi = blend_v64qi(v64qi_a, v64qi_b, mask64_logic);
        __m512i result_v32hi = blend_v32hi(v32hi_a, v32hi_b, mask32_logic);
        __m512i result_v16si = blend_v16si(v16si_a, v16si_b, mask16_logic);
        __m512i result_v8di = blend_v8di(v8di_a, v8di_b, mask8_logic);
        
        __m512 result_v16sf = blend_v16sf(v16sf_a, v16sf_b, cmp_mask_ps);
        __m512d result_v8df = blend_v8df(v8df_a, v8df_b, cmp_mask_pd);
        
        #ifdef __AVX512FP16__
        __mmask32 mask32_hf = _kxor_mask32(mask32_imm, 
            (__mmask32)(i * 0x55555555U));
        __m512h result_v32hf = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
        __m512bh result_v32bf = blend_v32bf(v32bf_a, v32bf_b, mask32_hf);
        #endif
        
        // Data dependency chain: Use integer blend results to generate new masks
        if (i > 0) {
            // Create dependency between different mode blends
            __mmask16 new_mask = _mm512_cmpeq_epi32_mask(
                result_v16si,
                _mm512_set1_epi32(0x11111111 + i)
            );
            
            result_v16sf = blend_v16sf(result_v16sf, v16sf_b, new_mask);
            
            // Further dependency
            __mmask8 new_mask_pd = _mm512_cmpeq_epi64_mask(
                result_v8di,
                _mm512_set1_epi64(0x1111111111111111ULL + i)
            );
            
            result_v8df = blend_v8df(result_v8df, v8df_b, new_mask_pd);
        }
        
        // Complex control flow: switch based on iteration
        switch (i % 3) {
            case 0:
                // Use immediate mask
                result_v64qi = blend_v64qi(result_v64qi, v64qi_a, mask64_imm);
                break;
            case 1:
                // Use comparison mask
                result_v32hi = blend_v32hi(result_v32hi, v32hi_a, 
                    _mm512_cmpeq_epi16_mask(result_v32hi, v32hi_b));
                break;
            case 2:
                // Use logical combination
                result_v16si = blend_v16si(result_v16si, v16si_a, mask16_logic);
                break;
        }
        
        // Accumulate checksum to prevent optimization
        uint8_t* ptr64 = (uint8_t*)&result_v64qi;
        for (int j = 0; j < 64; j++) checksum += ptr64[j];
        
        uint16_t* ptr32 = (uint16_t*)&result_v32hi;
        for (int j = 0; j < 32; j++) checksum += ptr32[j];
        
        uint32_t* ptr16 = (uint32_t*)&result_v16si;
        for (int j = 0; j < 16; j++) checksum += ptr16[j];
        
        uint64_t* ptr8 = (uint64_t*)&result_v8di;
        for (int j = 0; j < 8; j++) checksum += ptr8[j];
        
        float* ptr_sf = (float*)&result_v16sf;
        for (int j = 0; j < 16; j++) checksum += (uint64_t)ptr_sf[j];
        
        double* ptr_df = (double*)&result_v8df;
        for (int j = 0; j < 8; j++) checksum += (uint64_t)ptr_df[j];
        
        #ifdef __AVX512FP16__
        _Float16* ptr_hf = (_Float16*)&result_v32hf;
        for (int j = 0; j < 32; j++) checksum += (uint64_t)ptr_hf[j];
        
        _Float16* ptr_bf = (_Float16*)&result_v32bf;
        for (int j = 0; j < 32; j++) checksum += (uint64_t)ptr_bf[j];
        #endif
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
