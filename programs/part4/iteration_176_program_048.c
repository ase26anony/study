#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

// Prevent inlining to ensure each intrinsic gets expanded independently
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

// Helper function to generate masks based on loop index (control flow)
__mmask64 generate_dynamic_mask64(int iteration) {
    // Complex mask generation with control flow
    __mmask64 mask = 0;
    
    if (iteration % 3 == 0) {
        // Pattern 1: Alternating bits
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (iteration % 3 == 1) {
        // Pattern 2: Every 3rd bit set
        mask = 0x9249249249249249ULL;
    } else {
        // Pattern 3: Checkerboard
        mask = 0x5555555555555555ULL;
    }
    
    // Apply logical operations based on iteration
    if (iteration > 10) {
        mask = _kor_mask64(mask, 0x1111111111111111ULL);
    }
    
    return mask;
}

// Data dependency chain: result from one blend influences next blend
float process_data_dependency_chain(int mode_selector) {
    // Initialize test data
    __m512i int_data1 = _mm512_set_epi32(
        31, 30, 29, 28, 27, 26, 25, 24,
        23, 22, 21, 20, 19, 18, 17, 16
    );
    
    __m512i int_data2 = _mm512_set_epi32(
        0, 1, 2, 3, 4, 5, 6, 7,
        8, 9, 10, 11, 12, 13, 14, 15
    );
    
    __m512 float_data1 = _mm512_set_ps(
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f
    );
    
    __m512 float_data2 = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512d double_data1 = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    __m512d double_data2 = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    
    float checksum = 0.0f;
    
    // Switch statement for control flow (prevents constant folding)
    switch (mode_selector % 4) {
        case 0: {
            // Generate mask from comparison
            __mmask16 mask = _mm512_cmpeq_epi32_mask(
                _mm512_and_epi32(int_data1, _mm512_set1_epi32(1)),
                _mm512_setzero_si512()
            );
            
            // Blend 16si
            __m512i blended_si = blend_v16si(int_data1, int_data2, mask);
            
            // Use result to generate new mask for float blend
            __mmask16 float_mask = _mm512_cmp_ps_mask(
                _mm512_cvtepi32_ps(blended_si),
                _mm512_set1_ps(15.0f),
                _CMP_GT_OS
            );
            
            // Blend 16sf
            __m512 blended_sf = blend_v16sf(float_data1, float_data2, float_mask);
            
            // Horizontal sum
            checksum = _mm512_reduce_add_ps(blended_sf);
            break;
        }
        
        case 1: {
            // Immediate mask with logical operations
            __mmask32 mask32 = 0xAAAAAAAA;  // Immediate mask
            mask32 = _kxor_mask32(mask32, 0x55555555);  // Logical operation
            
            // Blend 32hi
            __m512i blended_hi = blend_v32hi(
                _mm512_set1_epi16(100),
                _mm512_set1_epi16(200),
                mask32
            );
            
            // Generate mask from blended result
            __mmask64 mask64 = _mm512_cmpeq_epi8_mask(
                blended_hi,
                _mm512_set1_epi8(100)
            );
            
            // Blend 64qi
            __m512i blended_qi = blend_v64qi(
                _mm512_set1_epi8(10),
                _mm512_set1_epi8(20),
                mask64
            );
            
            // Reduce
            checksum = (float)_mm512_reduce_add_epi64(blended_qi);
            break;
        }
        
        case 2: {
            // Complex mask generation with control flow
            __mmask8 mask8 = 0;
            for (int i = 0; i < 8; i++) {
                if ((mode_selector >> i) & 1) {
                    mask8 |= (1 << i);
                }
            }
            
            // Blend 8di
            __m512i blended_di = blend_v8di(
                _mm512_set1_epi64(1000),
                _mm512_set1_epi64(2000),
                mask8
            );
            
            // Use result to generate mask for double blend
            __mmask8 double_mask = _mm512_cmp_pd_mask(
                _mm512_cvtepi64_pd(blended_di),
                _mm512_set1_pd(1500.0),
                _CMP_LT_OS
            );
            
            // Blend 8df
            __m512d blended_df = blend_v8df(double_data1, double_data2, double_mask);
            
            // Horizontal sum
            checksum = (float)_mm512_reduce_add_pd(blended_df);
            break;
        }
        
        case 3: {
            // Mixed integer/float operations
            __mmask16 mask = _mm512_cmp_ps_mask(float_data1, float_data2, _CMP_GT_OS);
            mask = _kxor_mask16(mask, 0xFFFF);  // Logical operation
            
            __m512 blended = blend_v16sf(float_data1, float_data2, mask);
            
            // Convert to integer and blend again
            __m512i int_from_float = _mm512_cvtps_epi32(blended);
            __mmask16 int_mask = _mm512_test_epi32_mask(int_from_float, _mm512_set1_epi32(0x1));
            
            __m512i final_int = blend_v16si(int_data1, int_data2, int_mask);
            
            checksum = (float)_mm512_reduce_add_epi32(final_int);
            break;
        }
    }
    
    return checksum;
}

int main() {
    float total_checksum = 0.0f;
    
    // Loop with control flow to prevent optimization
    for (int i = 0; i < 20; i++) {
        // Generate dynamic masks
        __mmask64 mask64 = generate_dynamic_mask64(i);
        __mmask32 mask32 = (__mmask32)(mask64 & 0xFFFFFFFF);
        __mmask16 mask16 = (__mmask16)(mask64 & 0xFFFF);
        __mmask8 mask8 = (__mmask8)(mask64 & 0xFF);
        
        // Test all modes with different data patterns
        __m512i qi_data1 = _mm512_set1_epi8(i);
        __m512i qi_data2 = _mm512_set1_epi8(i * 2);
        __m512i blended_qi = blend_v64qi(qi_data1, qi_data2, mask64);
        total_checksum += (float)_mm512_reduce_add_epi64(blended_qi);
        
        __m512i hi_data1 = _mm512_set1_epi16(i * 10);
        __m512i hi_data2 = _mm512_set1_epi16(i * 20);
        __m512i blended_hi = blend_v32hi(hi_data1, hi_data2, mask32);
        total_checksum += (float)_mm512_reduce_add_epi32(blended_hi);
        
        __m512i si_data1 = _mm512_set1_epi32(i * 100);
        __m512i si_data2 = _mm512_set1_epi32(i * 200);
        __m512i blended_si = blend_v16si(si_data1, si_data2, mask16);
        total_checksum += (float)_mm512_reduce_add_epi32(blended_si);
        
        __m512i di_data1 = _mm512_set1_epi64(i * 1000LL);
        __m512i di_data2 = _mm512_set1_epi64(i * 2000LL);
        __m512i blended_di = blend_v8di(di_data1, di_data2, mask8);
        total_checksum += (float)_mm512_reduce_add_epi64(blended_di);
        
        __m512 sf_data1 = _mm512_set1_ps(i * 1.5f);
        __m512 sf_data2 = _mm512_set1_ps(i * 3.0f);
        __m512 blended_sf = blend_v16sf(sf_data1, sf_data2, mask16);
        total_checksum += _mm512_reduce_add_ps(blended_sf);
        
        __m512d df_data1 = _mm512_set1_pd(i * 2.5);
        __m512d df_data2 = _mm512_set1_pd(i * 5.0);
        __m512d blended_df = blend_v8df(df_data1, df_data2, mask8);
        total_checksum += (float)_mm512_reduce_add_pd(blended_df);
        
#ifdef __AVX512FP16__
        // Test half-precision modes if available
        __m512h hf_data1 = _mm512_set1_ph((_Float16)(i * 0.5f));
        __m512h hf_data2 = _mm512_set1_ph((_Float16)(i * 1.0f));
        __m512h blended_hf = blend_v32hf(hf_data1, hf_data2, mask32);
        
        __m512bh bf_data1 = _mm512_castph_bh(hf_data1);
        __m512bh bf_data2 = _mm512_castph_bh(hf_data2);
        __m512bh blended_bf = blend_v32bf(bf_data1, bf_data2, mask32);
        
        // Convert to float for checksum
        __m512 hf_as_float = _mm512_cvtph_ps(blended_hf);
        total_checksum += _mm512_reduce_add_ps(hf_as_float);
#endif
        
        // Data dependency chain with control flow
        total_checksum += process_data_dependency_chain(i);
    }
    
    printf("Final checksum: %f\n", total_checksum);
    return 0;
}
