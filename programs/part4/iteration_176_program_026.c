#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// ==================== Helper Functions (noinline to ensure expansion) ====================

__attribute__((noinline))
__m512i blend_v64qi(__m512i a, __m512i b, __mmask64 k) {
    return _mm512_mask_blend_epi8(k, a, b);
}

__attribute__((noinline))
__m512i blend_v32hi(__m512i a, __m512i b, __mmask32 k) {
    return _mm512_mask_blend_epi16(k, a, b);
}

__attribute__((noinline))
__m512i blend_v16si(__m512i a, __m512i b, __mmask16 k) {
    return _mm512_mask_blend_epi32(k, a, b);
}

__attribute__((noinline))
__m512i blend_v8di(__m512i a, __m512i b, __mmask8 k) {
    return _mm512_mask_blend_epi64(k, a, b);
}

__attribute__((noinline))
__m512 blend_v16sf(__m512 a, __m512 b, __mmask16 k) {
    return _mm512_mask_blend_ps(k, a, b);
}

__attribute__((noinline))
__m512d blend_v8df(__m512d a, __m512d b, __mmask8 k) {
    return _mm512_mask_blend_pd(k, a, b);
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

// ==================== Mask Generation Functions ====================

__mmask64 generate_mask64_comparison(__m512i a, __m512i b) {
    // Generate mask from comparison
    return _mm512_cmpeq_epi8_mask(a, b);
}

__mmask32 generate_mask32_logical(__mmask32 k1, __mmask32 k2) {
    // Combine masks using logical operations
    return _kor_mask32(k1, k2);
}

__mmask16 generate_mask16_immediate() {
    // Immediate mask pattern
    return (__mmask16)0xAAAA;
}

__mmask8 generate_mask8_control_flow(int iteration) {
    // Mask depends on control flow
    if (iteration % 3 == 0) {
        return (__mmask8)0xFF;
    } else if (iteration % 3 == 1) {
        return (__mmask8)0xAA;
    } else {
        return (__mmask8)0x55;
    }
}

// ==================== Main Test Function ====================

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i v64qi_a = _mm512_set1_epi8(0x55);
    __m512i v64qi_b = _mm512_set1_epi8(0xAA);
    
    __m512i v32hi_a = _mm512_set1_epi16(0x5555);
    __m512i v32hi_b = _mm512_set1_epi16(0xAAAA);
    
    __m512i v16si_a = _mm512_set1_epi32(0x55555555);
    __m512i v16si_b = _mm512_set1_epi32(0xAAAAAAAA);
    
    __m512i v8di_a = _mm512_set1_epi64(0x5555555555555555ULL);
    __m512i v8di_b = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL);
    
    __m512 v16sf_a = _mm512_set1_ps(1.0f);
    __m512 v16sf_b = _mm512_set1_ps(2.0f);
    
    __m512d v8df_a = _mm512_set1_pd(1.0);
    __m512d v8df_b = _mm512_set1_pd(2.0);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    
    __m512bh v32bf_a = _mm512_set1_ph(1.0f);
    __m512bh v32bf_b = _mm512_set1_ph(2.0f);
#endif
    
    // ==================== Test 1: V64QImode ====================
    {
        // Generate mask through comparison
        __mmask64 mask64 = generate_mask64_comparison(v64qi_a, v64qi_b);
        
        // Blend with control flow
        __m512i result64qi;
        for (int i = 0; i < 4; i++) {
            if (i % 2 == 0) {
                result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
            } else {
                // Use immediate mask
                result64qi = blend_v64qi(v64qi_b, v64qi_a, (__mmask64)0xAAAAAAAAAAAAAAAAULL);
            }
            
            // Create dependency chain: use result to generate new mask
            mask64 = _mm512_cmpeq_epi8_mask(result64qi, v64qi_a);
        }
        
        // Reduce to checksum
        uint8_t* ptr = (uint8_t*)&result64qi;
        for (int i = 0; i < 64; i++) {
            checksum += ptr[i];
        }
    }
    
    // ==================== Test 2: V32HImode ====================
    {
        // Generate mask using logical operations
        __mmask32 mask32_1 = _mm512_cmpeq_epi16_mask(v32hi_a, v32hi_b);
        __mmask32 mask32_2 = (__mmask32)0xAAAAAAAA;
        __mmask32 mask32 = generate_mask32_logical(mask32_1, mask32_2);
        
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        
        // Reduce to checksum
        uint16_t* ptr = (uint16_t*)&result32hi;
        for (int i = 0; i < 32; i++) {
            checksum += ptr[i];
        }
    }
    
    // ==================== Test 3: V16SImode ====================
    {
        // Switch statement for different blend modes
        __m512i result16si;
        for (int mode = 0; mode < 3; mode++) {
            switch (mode) {
                case 0:
                    result16si = blend_v16si(v16si_a, v16si_b, (__mmask16)0xAAAA);
                    break;
                case 1:
                    result16si = blend_v16si(v16si_b, v16si_a, (__mmask16)0x5555);
                    break;
                case 2:
                    result16si = blend_v16si(v16si_a, v16si_b, 
                        _mm512_cmpeq_epi32_mask(v16si_a, v16si_b));
                    break;
            }
        }
        
        // Reduce to checksum
        uint32_t* ptr = (uint32_t*)&result16si;
        for (int i = 0; i < 16; i++) {
            checksum += ptr[i];
        }
    }
    
    // ==================== Test 4: V8DImode ====================
    {
        // Control flow dependent mask generation
        __m512i result8di;
        for (int i = 0; i < 5; i++) {
            __mmask8 mask8 = generate_mask8_control_flow(i);
            result8di = blend_v8di(v8di_a, v8di_b, mask8);
        }
        
        // Reduce to checksum
        uint64_t* ptr = (uint64_t*)&result8di;
        for (int i = 0; i < 8; i++) {
            checksum += ptr[i];
        }
    }
    
    // ==================== Test 5: V16SFmode ====================
    {
        // Generate mask from float comparison
        __mmask16 mask16_sf = _mm512_cmp_ps_mask(v16sf_a, v16sf_b, _CMP_LT_OQ);
        
        // Combine with immediate mask
        __mmask16 mask16 = _kor_mask16(mask16_sf, generate_mask16_immediate());
        
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16);
        
        // Reduce to checksum
        float* ptr = (float*)&result16sf;
        for (int i = 0; i < 16; i++) {
            checksum += (uint64_t)ptr[i];
        }
    }
    
    // ==================== Test 6: V8DFmode ====================
    {
        // Dependency chain: use previous result to generate mask
        __m512d temp = v8df_a;
        __m512d result8df;
        
        for (int i = 0; i < 3; i++) {
            __mmask8 mask8_df = _mm512_cmp_pd_mask(temp, v8df_b, _CMP_GT_OQ);
            result8df = blend_v8df(temp, v8df_b, mask8_df);
            temp = result8df;  // Create dependency chain
        }
        
        // Reduce to checksum
        double* ptr = (double*)&result8df;
        for (int i = 0; i < 8; i++) {
            checksum += (uint64_t)ptr[i];
        }
    }
    
#ifdef __AVX512FP16__
    // ==================== Test 7: V32HFmode ====================
    {
        // Generate mask with control flow
        __mmask32 mask32_hf;
        for (int i = 0; i < 2; i++) {
            if (i == 0) {
                mask32_hf = (__mmask32)0xAAAAAAAA;
            } else {
                // Create dependency: use previous blend result
                __m512h temp_hf = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
                mask32_hf = _mm512_cmp_ph_mask(temp_hf, v32hf_a, _CMP_EQ_OQ);
            }
        }
        
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask32_hf);
        
        // Reduce to checksum
        _Float16* ptr = (_Float16*)&result32hf;
        for (int i = 0; i < 32; i++) {
            checksum += (uint64_t)ptr[i];
        }
    }
    
    // ==================== Test 8: V32BFmode ====================
    {
        // Test brain float (bfloat16) mode
        __mmask32 mask32_bf = (__mmask32)0x55555555;
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask32_bf);
        
        // Reduce to checksum
        __bf16* ptr = (__bf16*)&result32bf;
        for (int i = 0; i < 32; i++) {
            checksum += (uint64_t)ptr[i];
        }
    }
#endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
