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

__attribute__((noinline, target("avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    // Note: __m512bh is for bfloat16, use appropriate intrinsic
    return _mm512_mask_blend_ph(k, a, b);
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

// Helper function to generate masks with control flow
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    __mmask64 mask;
    
    // Control flow to prevent constant folding
    if (iteration % 3 == 0) {
        // Method 1: Comparison mask
        __m512i a = _mm512_set1_epi8(iteration);
        __m512i b = _mm512_set1_epi8(iteration / 2);
        mask = _mm512_cmpeq_epi8_mask(a, b);
    } else if (iteration % 3 == 1) {
        // Method 2: Immediate mask
        mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Method 3: Logical combination
        __mmask64 m1 = (__mmask64)0x5555555555555555ULL;
        __mmask64 m2 = (__mmask64)0xFFFFFFFF00000000ULL;
        mask = _kor_mask64(m1, m2);
    }
    
    return mask;
}

// Similar mask generators for other types
__attribute__((noinline))
__mmask32 generate_mask32_hi(int iteration) {
    if (iteration % 2 == 0) {
        __m512i a = _mm512_set1_epi16(iteration);
        __m512i b = _mm512_set1_epi16(iteration * 2);
        return _mm512_cmpgt_epi16_mask(a, b);
    } else {
        return (__mmask32)0xAAAAAAAA;
    }
}

__attribute__((noinline))
__mmask16 generate_mask16_sf(int iteration, __m512 prev_result) {
    // Create data dependency chain
    __m512 threshold = _mm512_set1_ps(100.0f);
    __m512 cmp_result = _mm512_sub_ps(prev_result, threshold);
    return _mm512_cmp_ps_mask(cmp_result, _mm512_setzero_ps(), _CMP_GT_OQ);
}

__attribute__((noinline))
__mmask8 generate_mask8_df(int iteration) {
    switch (iteration % 4) {
        case 0: return (__mmask8)0xAA;
        case 1: return (__mmask8)0x55;
        case 2: return (__mmask8)0xF0;
        default: return (__mmask8)0x0F;
    }
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    int8_t pattern8[64];
    int16_t pattern16[32];
    float pattern_float[16];
    double pattern_double[8];
    _Float16 pattern_half[32];
    __bf16 pattern_bf16[32];
    
    for (int i = 0; i < 64; i++) pattern8[i] = i % 16;
    for (int i = 0; i < 32; i++) pattern16[i] = i * 3;
    for (int i = 0; i < 16; i++) pattern_float[i] = i * 1.5f;
    for (int i = 0; i < 8; i++) pattern_double[i] = i * 2.5;
    for (int i = 0; i < 32; i++) pattern_half[i] = (_Float16)(i * 0.5f);
    for (int i = 0; i < 32; i++) pattern_bf16[i] = (__bf16)(i * 0.25f);
    
    __m512i v64qi_a = _mm512_loadu_si512(pattern8);
    __m512i v64qi_b = _mm512_set1_epi8(0xFF);
    
    __m512i v32hi_a = _mm512_loadu_si512(pattern16);
    __m512i v32hi_b = _mm512_set1_epi16(0x7FFF);
    
    __m512 v16sf_a = _mm512_loadu_ps(pattern_float);
    __m512 v16sf_b = _mm512_set1_ps(999.0f);
    
    __m512d v8df_a = _mm512_loadu_pd(pattern_double);
    __m512d v8df_b = _mm512_set1_pd(888.0);
    
    __m512h v32hf_a = _mm512_loadu_ph(pattern_half);
    __m512h v32hf_b = _mm512_set1_ph((_Float16)777.0f);
    
    __m512bh v32bf_a = _mm512_loadu_ph(pattern_bf16);
    __m512bh v32bf_b = _mm512_set1_ph((__bf16)666.0f);
    
    __m512i v16si_a = _mm512_set_epi32(31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16);
    __m512i v16si_b = _mm512_set1_epi32(0xFFFFFFFF);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(0x7FFFFFFFFFFFFFFFULL);
    
    // Main test loop with control flow
    for (int iter = 0; iter < 10; iter++) {
        // Generate masks with different methods
        __mmask64 mask64 = generate_complex_mask64(iter);
        __mmask32 mask32_hi = generate_mask32_hi(iter);
        __mmask32 mask32_hf = (__mmask32)(0x55555555 ^ iter);
        __mmask32 mask32_bf = (__mmask32)(0xAAAAAAAA ^ iter);
        __mmask16 mask16_si = (__mmask16)(0xAAAA ^ iter);
        __mmask8 mask8_di = generate_mask8_df(iter);
        
        // Perform blends with data dependency chain
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        
        // Use previous result to influence next mask (dependency chain)
        __mmask16 mask16_sf = generate_mask16_sf(iter, v16sf_a);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_sf);
        
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32_hi);
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16_si);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8_di);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8_di);
        
        // Horizontal reduction to prevent optimization removal
        __m512i sum64qi = _mm512_sad_epu8(result64qi, _mm512_setzero_si512());
        __m512i sum32hi = _mm512_madd_epi16(result32hi, _mm512_set1_epi16(1));
        __m512 sum16sf = _mm512_add_ps(result16sf, _mm512_setzero_ps());
        __m512d sum8df = _mm512_add_pd(result8df, _mm512_setzero_pd());
        
        // Accumulate to checksum
        uint64_t temp[8];
        _mm512_storeu_si512(temp, sum64qi);
        checksum += temp[0] + temp[4];
        
        _mm512_storeu_si512(temp, sum32hi);
        checksum += temp[0] + temp[2] + temp[4] + temp[6];
        
        float fsum[16];
        _mm512_storeu_ps(fsum, sum16sf);
        for (int i = 0; i < 16; i++) checksum += (uint64_t)fsum[i];
        
        double dsum[8];
        _mm512_storeu_pd(dsum, sum8df);
        for (int i = 0; i < 8; i++) checksum += (uint64_t)dsum[i];
        
        // Update inputs for next iteration (prevent constant folding)
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.1f));
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
