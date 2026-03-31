#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Prevent inlining to ensure each blend gets expanded independently
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
__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to generate masks based on loop index (control flow)
__mmask64 generate_dynamic_mask64(int i) {
    // Different mask patterns based on i
    switch (i % 4) {
        case 0: return 0xAAAAAAAAAAAAAAAAULL;  // Alternating pattern
        case 1: return 0x5555555555555555ULL;  // Opposite alternating
        case 2: return 0xFFFFFFFFFFFFFFFFULL;  // All ones
        default: return 0x0ULL;                // All zeros
    }
}

__mmask32 generate_dynamic_mask32(int i) {
    return (__mmask32)(0xAAAAAAAAU >> (i % 4));
}

__mmask16 generate_dynamic_mask16(int i) {
    return (__mmask16)(0xAAAAU >> (i % 4));
}

__mmask8 generate_dynamic_mask8(int i) {
    return (__mmask8)(0xAAU >> (i % 4));
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test vectors with patterned data
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
    __m512h v32hf_a = _mm512_set1_ph(5.0f);
    __m512h v32hf_b = _mm512_set1_ph(6.0f);
    
    __m512bh v32bf_a = _mm512_castsi512_ph(_mm512_set1_epi16(0x7C00)); // 1.0 in FP16
    __m512bh v32bf_b = _mm512_castsi512_ph(_mm512_set1_epi16(0x7E00)); // 1.5 in FP16
#endif
    
    // Test 1: Loop with dynamic mask generation (control flow)
    for (int i = 0; i < 8; i++) {
        // Generate masks using different methods
        __mmask64 k64 = generate_dynamic_mask64(i);
        __mmask32 k32 = generate_dynamic_mask32(i);
        __mmask16 k16 = generate_dynamic_mask16(i);
        __mmask8 k8 = generate_dynamic_mask8(i);
        
        // Immediate masks
        __mmask64 k64_imm = 0xCCCCCCCCCCCCCCCCULL;
        __mmask32 k32_imm = 0xCCCCCCCCU;
        __mmask16 k16_imm = 0xCCCCU;
        __mmask8 k8_imm = 0xCCU;
        
        // Logical mask operations
        __mmask64 k64_combined = _kor_mask64(k64, k64_imm);
        __mmask32 k32_combined = _kxor_mask32(k32, k32_imm);
        __mmask16 k16_combined = _kand_mask16(k16, k16_imm);
        __mmask8 k8_combined = _kor_mask8(k8, k8_imm);
        
        // Comparison masks (dynamic from vector data)
        __mmask16 cmp_mask16 = _mm512_cmpeq_epi32_mask(v16si_a, v16si_b);
        __mmask8 cmp_mask8 = _mm512_cmpeq_epi64_mask(v8di_a, v8di_b);
        
        // Perform blends with different mask types
        __m512i res64qi = blend_v64qi(v64qi_a, v64qi_b, k64_combined);
        __m512i res32hi = blend_v32hi(v32hi_a, v32hi_b, k32_combined);
        __m512i res16si = blend_v16si(v16si_a, v16si_b, k16_combined | cmp_mask16);
        __m512i res8di = blend_v8di(v8di_a, v8di_b, k8_combined | cmp_mask8);
        
        // Data dependency chain: use integer result to generate float mask
        __mmask16 float_mask = _mm512_cmpeq_epi32_mask(res16si, v16si_a);
        __m512 res16sf = blend_v16sf(v16sf_a, v16sf_b, float_mask);
        
        // More data dependency: use float result for double blend
        __mmask8 double_mask = _mm512_cmp_ps_mask(_mm512_castsi512_ps(res16si), 
                                                 v16sf_a, _CMP_EQ_OQ);
        __m512d res8df = blend_v8df(v8df_a, v8df_b, double_mask);
        
#ifdef __AVX512FP16__
        // Half-precision blends
        __m512h res32hf = blend_v32hf(v32hf_a, v32hf_b, k32_combined);
        __m512bh res32bf = blend_v32bf(v32bf_a, v32bf_b, k32_combined);
#endif
        
        // Accumulate results to prevent optimization
        alignas(64) uint8_t buf64[64];
        alignas(64) uint16_t buf32[32];
        alignas(64) uint32_t buf16[16];
        alignas(64) uint64_t buf8[8];
        alignas(64) float buf16f[16];
        alignas(64) double buf8d[8];
        
        _mm512_store_si512(buf64, res64qi);
        _mm512_store_si512(buf32, res32hi);
        _mm512_store_si512(buf16, res16si);
        _mm512_store_si512(buf8, res8di);
        _mm512_store_ps(buf16f, res16sf);
        _mm512_store_pd(buf8d, res8df);
        
        for (int j = 0; j < 64; j++) checksum += buf64[j];
        for (int j = 0; j < 32; j++) checksum += buf32[j];
        for (int j = 0; j < 16; j++) checksum += buf16[j];
        for (int j = 0; j < 8; j++) checksum += buf8[j];
        for (int j = 0; j < 16; j++) checksum += (uint64_t)buf16f[j];
        for (int j = 0; j < 8; j++) checksum += (uint64_t)buf8d[j];
    }
    
    // Test 2: Switch statement for different blend modes
    int mode_selector = 3;
    __m512i final_result;
    
    switch (mode_selector) {
        case 0:
            final_result = blend_v64qi(v64qi_a, v64qi_b, 0xF0F0F0F0F0F0F0F0ULL);
            break;
        case 1:
            final_result = blend_v32hi(v32hi_a, v32hi_b, 0xF0F0F0F0U);
            break;
        case 2:
            final_result = blend_v16si(v16si_a, v16si_b, 0xF0F0U);
            break;
        case 3:
            final_result = blend_v8di(v8di_a, v8di_b, 0xF0U);
            break;
        default:
            final_result = _mm512_setzero_si512();
    }
    
    // Test 3: If-else chain with dependent blends
    __mmask16 test_mask = 0xAAAA;
    __m512 test_result;
    
    if (test_mask != 0) {
        test_result = blend_v16sf(v16sf_a, v16sf_b, test_mask);
        __mmask8 derived_mask = _mm512_cmp_ps_mask(test_result, v16sf_a, _CMP_GT_OQ);
        __m512d final_double = blend_v8df(v8df_a, v8df_b, derived_mask);
        
        alignas(64) double final_buf[8];
        _mm512_store_pd(final_buf, final_double);
        for (int i = 0; i < 8; i++) checksum += (uint64_t)final_buf[i];
    } else {
        test_result = blend_v16sf(v16sf_b, v16sf_a, 0x5555);
    }
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
