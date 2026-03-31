#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// ==================== NOINLINE HELPER FUNCTIONS ====================

__attribute__((noinline))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
}

__attribute__((noinline))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

#ifdef __AVX512FP16__
__attribute__((noinline))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// ==================== MASK GENERATION FUNCTIONS ====================

__mmask64 generate_mask64_comparison(__m512i a, __m512i b) {
    // Generate mask from comparison
    return _mm512_cmpeq_epi8_mask(a, b);
}

__mmask32 generate_mask32_comparison(__m512i a, __m512i b) {
    return _mm512_cmpeq_epi16_mask(a, b);
}

__mmask16 generate_mask16_comparison_ps(__m512 a, __m512 b) {
    return _mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ);
}

__mmask8 generate_mask8_comparison_pd(__m512d a, __m512d b) {
    return _mm512_cmp_pd_mask(a, b, _CMP_EQ_OQ);
}

__mmask16 generate_mask16_comparison_epi32(__m512i a, __m512i b) {
    return _mm512_cmpeq_epi32_mask(a, b);
}

__mmask8 generate_mask8_comparison_epi64(__m512i a, __m512i b) {
    return _mm512_cmpeq_epi64_mask(a, b);
}

#ifdef __AVX512FP16__
__mmask32 generate_mask32_comparison_ph(__m512h a, __m512h b) {
    return _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
}
#endif

// ==================== CONTROL FLOW & DATA DEPENDENCY ====================

__attribute__((noinline))
uint64_t process_with_control_flow(int mode_selector) {
    uint64_t checksum = 0;
    
    // Initialize test vectors with patterns
    __m512i v64qi_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v64qi_b = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    // Complex control flow with switch statement
    switch(mode_selector % 4) {
        case 0: {
            // Method 1: Comparison mask
            __mmask64 k64 = generate_mask64_comparison(v64qi_a, v64qi_b);
            __m512i result = blend_v64qi(v64qi_a, v64qi_b, k64);
            
            // Use result to generate mask for next operation
            __mmask32 k32 = _mm512_cmpeq_epi16_mask(result, _mm512_set1_epi16(0));
            
            // Combine with immediate mask using logical operation
            __mmask32 k32_imm = (__mmask32)0xAAAAAAAA;
            k32 = _kor_mask32(k32, k32_imm);
            
            __m512i v32hi_a = _mm512_set1_epi16(1);
            __m512i v32hi_b = _mm512_set1_epi16(2);
            __m512i result2 = blend_v32hi(v32hi_a, v32hi_b, k32);
            
            // Horizontal sum for checksum
            checksum += _mm512_reduce_add_epi16(result2);
            break;
        }
        
        case 1: {
            // Method 2: Immediate mask with logical operations
            __mmask16 k16_imm = (__mmask16)0xAAAA;
            __mmask16 k16_comp = generate_mask16_comparison_epi32(
                _mm512_set1_epi32(0), 
                _mm512_set1_epi32(1)
            );
            __mmask16 k16 = _kxor_mask16(k16_imm, k16_comp);
            
            __m512i v16si_a = _mm512_set1_epi32(100);
            __m512i v16si_b = _mm512_set1_epi32(200);
            __m512i result = blend_v16si(v16si_a, v16si_b, k16);
            
            checksum += _mm512_reduce_add_epi32(result);
            break;
        }
        
        case 2: {
            // Method 3: Loop-dependent mask generation
            __m512 v16sf_a = _mm512_set1_ps(1.0f);
            __m512 v16sf_b = _mm512_set1_ps(2.0f);
            __m512d v8df_a = _mm512_set1_pd(1.0);
            __m512d v8df_b = _mm512_set1_pd(2.0);
            
            __m512 accum_sf = _mm512_setzero_ps();
            __m512d accum_df = _mm512_setzero_pd();
            
            for (int i = 0; i < 8; i++) {
                // Mask depends on loop index
                __mmask16 k16 = (__mmask16)(0xFFFF >> i);
                __mmask8 k8 = (__mmask8)(0xFF >> (i % 8));
                
                if (i % 2 == 0) {
                    accum_sf = blend_v16sf(accum_sf, v16sf_a, k16);
                    accum_df = blend_v8df(accum_df, v8df_a, k8);
                } else {
                    accum_sf = blend_v16sf(accum_sf, v16sf_b, k16);
                    accum_df = blend_v8df(accum_df, v8df_b, k8);
                }
            }
            
            checksum += (uint64_t)_mm512_reduce_add_ps(accum_sf);
            checksum += (uint64_t)_mm512_reduce_add_pd(accum_df);
            break;
        }
        
        case 3: {
            // Method 4: Chain of operations with mode transitions
            __m512i v8di_a = _mm512_set1_epi64(10);
            __m512i v8di_b = _mm512_set1_epi64(20);
            
            // Start with immediate mask
            __mmask8 k8 = (__mmask8)0xAA;
            __m512i result_di = blend_v8di(v8di_a, v8di_b, k8);
            
            // Use result to generate mask for float operation
            __m512i cmp_result = _mm512_cmpeq_epi64_mask(result_di, v8di_a);
            __mmask16 k16_from_di = (__mmask16)(cmp_result * 0x1111);
            
            __m512 v16sf_a = _mm512_set1_ps(3.14f);
            __m512 v16sf_b = _mm512_set1_ps(6.28f);
            __m512 result_sf = blend_v16sf(v16sf_a, v16sf_b, k16_from_di);
            
            checksum += _mm512_reduce_add_epi64(result_di);
            checksum += (uint64_t)_mm512_reduce_add_ps(result_sf);
            break;
        }
    }
    
    return checksum;
}

#ifdef __AVX512FP16__
__attribute__((noinline))
uint64_t process_half_precision(int iteration) {
    uint64_t checksum = 0;
    
    // Initialize half-precision vectors
    __m512h v32hf_a = _mm512_set1_ph((_Float16)1.0);
    __m512h v32hf_b = _mm512_set1_ph((_Float16)2.0);
    __m512bh v32bf_a = _mm512_castph_sbh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_sbh(v32hf_b);
    
    // Complex mask generation with control flow
    __mmask32 k32;
    if (iteration % 3 == 0) {
        // Comparison mask
        k32 = generate_mask32_comparison_ph(v32hf_a, v32hf_b);
    } else if (iteration % 3 == 1) {
        // Immediate mask
        k32 = (__mmask32)0xAAAAAAAA;
    } else {
        // Logical combination
        __mmask32 k1 = generate_mask32_comparison_ph(v32hf_a, _mm512_set1_ph((_Float16)0.0));
        __mmask32 k2 = (__mmask32)0x55555555;
        k32 = _kor_mask32(k1, k2);
    }
    
    // Perform blends in both half-precision formats
    __m512h result_hf = blend_v32hf(v32hf_a, v32hf_b, k32);
    __m512bh result_bf = blend_v32bf(v32bf_a, v32bf_b, k32);
    
    // Convert to integer for checksum
    __m512i result_int = _mm512_castph_si512(result_hf);
    checksum += _mm512_reduce_add_epi16(result_int);
    
    return checksum;
}
#endif

// ==================== MAIN FUNCTION ====================

int main() {
    uint64_t final_checksum = 0;
    
    printf("Starting AVX-512 blend coverage test...\n");
    
    // Test all modes through control flow variations
    for (int i = 0; i < 8; i++) {
        final_checksum += process_with_control_flow(i);
    }
    
    // Test V64QImode with direct intrinsic
    {
        __m512i a = _mm512_set1_epi8(1);
        __m512i b = _mm512_set1_epi8(2);
        __mmask64 k = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
        __m512i result = _mm512_mask_blend_epi8(k, a, b);
        final_checksum += _mm512_reduce_add_epi8(result);
    }
    
    // Test V32HImode with direct intrinsic
    {
        __m512i a = _mm512_set1_epi16(10);
        __m512i b = _mm512_set1_epi16(20);
        __mmask32 k = (__mmask32)0xAAAAAAAA;
        __m512i result = _mm512_mask_blend_epi16(k, a, b);
        final_checksum += _mm512_reduce_add_epi16(result);
    }
    
    // Test V16SFmode with comparison mask
    {
        __m512 a = _mm512_set1_ps(1.0f);
        __m512 b = _mm512_set1_ps(2.0f);
        __mmask16 k = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
        __m512 result = _mm512_mask_blend_ps(k, a, b);
        final_checksum += (uint64_t)_mm512_reduce_add_ps(result);
    }
    
    // Test V8DFmode with logical mask operations
    {
        __m512d a = _mm512_set1_pd(3.14);
        __m512d b = _mm512_set1_pd(6.28);
        __mmask8 k1 = _mm512_cmp_pd_mask(a, b, _CMP_LT_OQ);
        __mmask8 k2 = (__mmask8)0x0F;
        __mmask8 k = _kor_mask8(k1, k2);
        __m512d result = _mm512_mask_blend_pd(k, a, b);
        final_checksum += (uint64_t)_mm512_reduce_add_pd(result);
    }
    
    // Test V16SImode with data-dependent mask
    {
        __m512i a = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
        __m512i b = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
        __mmask16 k = _mm512_cmpeq_epi32_mask(a, b);
        __m512i result = _mm512_mask_blend_epi32(k, a, b);
        final_checksum += _mm512_reduce_add_epi32(result);
    }
    
    // Test V8DImode with immediate mask
    {
        __m512i a = _mm512_set1_epi64(100);
        __m512i b = _mm512_set1_epi64(200);
        __mmask8 k = (__mmask8)0xAA;
        __m512i result = _mm512_mask_blend_epi64(k, a, b);
        final_checksum += _mm512_reduce_add_epi64(result);
    }
    
#ifdef __AVX512FP16__
    printf("Testing half-precision modes...\n");
    
    // Test V32HFmode and V32BFmode
    for (int i = 0; i < 4; i++) {
        final_checksum += process_half_precision(i);
    }
    
    // Direct intrinsic test for half-precision
    {
        __m512h a = _mm512_set1_ph((_Float16)1.5);
        __m512h b = _mm512_set1_ph((_Float16)3.0);
        __mmask32 k = (__mmask32)0xAAAAAAAA;
        __m512h result = _mm512_mask_blend_ph(k, a, b);
        __m512i result_int = _mm512_castph_si512(result);
        final_checksum += _mm512_reduce_add_epi16(result_int);
    }
#endif
    
    printf("Final checksum: %lu\n", final_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
