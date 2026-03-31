#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

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
__attribute__((noinline, target("avx512fp16")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to create complex mask patterns
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    // Method 1: Immediate mask with pattern
    __mmask64 mask1 = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
    
    // Method 2: Logical operation on masks
    __mmask64 mask2 = (__mmask64)(0x5555555555555555ULL << (iteration % 4));
    
    // Combine masks using logical operations
    return _kor_mask64(mask1, mask2);
}

__attribute__((noinline))
__mmask32 generate_complex_mask32(int iteration) {
    // Method 1: Immediate mask
    __mmask32 mask1 = 0xAAAAAAAA;
    
    // Method 2: Logical operation
    __mmask32 mask2 = 0x55555555 << (iteration % 3);
    
    // Combine with XOR
    return _kxor_mask32(mask1, mask2);
}

__attribute__((noinline))
__mmask16 generate_complex_mask16(int iteration) {
    // Method 1: Immediate mask
    __mmask16 mask1 = 0xAAAA;
    
    // Method 2: Logical operation
    __mmask16 mask2 = 0x5555 << (iteration % 2);
    
    // Combine with AND
    return _kand_mask16(mask1, mask2);
}

__attribute__((noinline))
__mmask8 generate_complex_mask8(int iteration) {
    // Method 1: Immediate mask
    __mmask8 mask1 = 0xAA;
    
    // Method 2: Logical operation
    __mmask8 mask2 = 0x55 << (iteration % 2);
    
    // Combine
    return mask1 ^ mask2;
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
    
    __m512bh v32bf_a = _mm512_set1_ph(7.0f);
    __m512bh v32bf_b = _mm512_set1_ph(8.0f);
#endif
    
    // Control flow: loop with mask dependency on iteration
    for (int i = 0; i < 4; i++) {
        // Generate masks using different methods based on iteration
        __mmask64 mask64;
        __mmask32 mask32;
        __mmask16 mask16;
        __mmask8 mask8;
        
        // Switch statement for different mask generation strategies
        switch (i % 3) {
            case 0:
                // Method 1: Immediate masks
                mask64 = generate_complex_mask64(i);
                mask32 = generate_complex_mask32(i);
                mask16 = generate_complex_mask16(i);
                mask8 = generate_complex_mask8(i);
                break;
                
            case 1:
                // Method 2: Comparison masks
                {
                    __m512i cmp_a = _mm512_set1_epi32(i);
                    __m512i cmp_b = _mm512_set1_epi32(i + 1);
                    mask16 = _mm512_cmpeq_epi32_mask(cmp_a, cmp_b);
                    
                    __m512 cmp_ps_a = _mm512_set1_ps((float)i);
                    __m512 cmp_ps_b = _mm512_set1_ps((float)(i + 1));
                    mask16 = _mm512_cmp_ps_mask(cmp_ps_a, cmp_ps_b, _CMP_EQ_OQ);
                    
                    __m512d cmp_pd_a = _mm512_set1_pd((double)i);
                    __m512d cmp_pd_b = _mm512_set1_pd((double)(i + 1));
                    mask8 = _mm512_cmp_pd_mask(cmp_pd_a, cmp_pd_b, _CMP_EQ_OQ);
                    
                    // Derive other masks from these
                    mask64 = (__mmask64)mask16;
                    mask32 = (__mmask32)mask16;
                }
                break;
                
            case 2:
                // Method 3: Mixed approach
                mask64 = (__mmask64)(0xF0F0F0F0F0F0F0F0ULL >> (i * 4));
                mask32 = (__mmask32)(0xF0F0F0F0 >> (i * 4));
                mask16 = (__mmask16)(0xF0F0 >> (i * 4));
                mask8 = (__mmask8)(0xF0 >> (i * 4));
                break;
        }
        
        // If-else chain where result of one blend affects next mask
        if (i % 2 == 0) {
            // Perform V64QImode blend
            __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
            
            // Use result to generate mask for next blend
            __m512i cmp = _mm512_cmpeq_epi8_mask(result64qi, v64qi_a);
            __mmask32 derived_mask32 = (__mmask32)(cmp & 0xFFFFFFFF);
            
            // Perform V32HImode blend with derived mask
            __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, derived_mask32);
            
            // Data dependency chain: use integer result for float comparison
            __m512 float_from_int = _mm512_cvtepi32_ps(_mm512_cvtepi16_epi32(_mm512_cvtepi8_epi16(_mm512_extracti64x4_epi64(result32hi, 0))));
            __mmask16 float_mask = _mm512_cmp_ps_mask(float_from_int, v16sf_a, _CMP_GT_OQ);
            
            // Perform V16SFmode blend
            __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, float_mask);
            
            // Accumulate checksum
            checksum += _mm512_reduce_add_epi64(_mm512_castps_si512(result16sf));
        } else {
            // Alternative path with different mode sequence
            // Perform V16SImode blend
            __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
            
            // Use result to generate mask for V8DImode
            __m512i cmp_di = _mm512_cmpeq_epi64_mask(result16si, v8di_a);
            __mmask8 derived_mask8 = (__mmask8)cmp_di;
            
            // Perform V8DImode blend
            __m512i result8di = blend_v8di(v8di_a, v8di_b, derived_mask8);
            
            // Convert to double for V8DFmode blend
            __m512d double_from_int = _mm512_cvtepi64_pd(result8di);
            __mmask8 double_mask = _mm512_cmp_pd_mask(double_from_int, v8df_a, _CMP_LT_OQ);
            
            // Perform V8DFmode blend
            __m512d result8df = blend_v8df(v8df_a, v8df_b, double_mask);
            
            // Accumulate checksum
            checksum += _mm512_reduce_add_epi64(_mm512_castpd_si512(result8df));
        }
        
#ifdef __AVX512FP16__
        // Half-precision blends with control flow
        if (i > 0) {
            // Generate mask for half-precision
            __mmask32 half_mask = generate_complex_mask32(i);
            
            // Perform V32HFmode blend
            __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, half_mask);
            
            // Perform V32BFmode blend with modified mask
            __mmask32 bf_mask = _kxor_mask32(half_mask, 0xFFFFFFFF);
            __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, bf_mask);
            
            // Convert to integer for checksum
            __m512i hf_int = _mm512_castph_si512(result32hf);
            __m512i bf_int = _mm512_castbh_si512(result32bf);
            checksum += _mm512_reduce_add_epi64(hf_int);
            checksum += _mm512_reduce_add_epi64(bf_int);
        }
#endif
    }
    
    // Final reduction and output
    printf("Final checksum: %lu\n", checksum);
    
    // Additional test: nested switch for mode selection
    int mode_selector = 3;
    __m512i final_result;
    
    switch (mode_selector) {
        case 1:
            final_result = blend_v64qi(v64qi_a, v64qi_b, 0xFFFFFFFFFFFFFFFFULL);
            break;
        case 2:
            final_result = blend_v32hi(v32hi_a, v32hi_b, 0xFFFFFFFF);
            break;
        case 3:
            final_result = blend_v16si(v16si_a, v16si_b, 0xFFFF);
            break;
        case 4:
            final_result = blend_v8di(v8di_a, v8di_b, 0xFF);
            break;
        default:
            final_result = _mm512_setzero_si512();
            break;
    }
    
    checksum += _mm512_reduce_add_epi64(final_result);
    printf("Final checksum with switch: %lu\n", checksum);
    
    return 0;
}
