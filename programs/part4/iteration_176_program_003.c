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
__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512h blend_v32hf(__m512h a, __m512h b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}

__attribute__((noinline, target("avx512fp16,avx512bw")))
__m512bh blend_v32bf(__m512bh a, __m512bh b, __mmask32 k) {
    return _mm512_mask_blend_ph(k, a, b);
}
#endif

// Helper function to create complex control flow
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration) {
    // Control flow that prevents constant folding
    __mmask64 mask;
    
    if (iteration % 3 == 0) {
        // Immediate mask
        mask = 0xAAAAAAAAAAAAAAAAULL;
    } else if (iteration % 3 == 1) {
        // Patterned immediate mask
        mask = 0x5555555555555555ULL;
    } else {
        // Alternating pattern
        mask = 0x3333333333333333ULL;
    }
    
    // Logical operations on masks
    if (iteration > 10) {
        mask = _kor_mask64(mask, 0x00000000FFFFFFFFULL);
    }
    
    return mask;
}

// Data dependency chain: use result from one blend to generate mask for another
__attribute__((noinline, target("avx512f,avx512bw")))
float data_dependency_chain(__m512i int_vec, __m512 float_vec, int mode) {
    float checksum = 0.0f;
    
    // Start with integer blend
    __mmask64 mask64 = generate_complex_mask64(mode);
    __m512i blended_int = blend_v64qi(int_vec, _mm512_set1_epi8(0xFF), mask64);
    
    // Use integer result to generate float mask
    __m512i cmp_result = _mm512_cmpeq_epi32_mask(blended_int, _mm512_set1_epi32(0xFF));
    __mmask16 float_mask = cmp_result;
    
    // Apply logical operations to mask
    switch (mode % 4) {
        case 0:
            float_mask = _kor_mask16(float_mask, 0xAAAA);
            break;
        case 1:
            float_mask = _kxor_mask16(float_mask, 0x5555);
            break;
        case 2:
            float_mask = _kand_mask16(float_mask, 0x0F0F);
            break;
        default:
            float_mask = _knot_mask16(float_mask);
    }
    
    // Float blend with generated mask
    __m512 blended_float = blend_v16sf(float_vec, _mm512_set1_ps(1.0f), float_mask);
    
    // Horizontal sum
    __m256 low = _mm512_castps512_ps256(blended_float);
    __m256 high = _mm512_extractf32x8_ps(blended_float, 1);
    __m256 sum8 = _mm256_add_ps(low, high);
    
    __m128 sum4 = _mm_add_ps(_mm256_castps256_ps128(sum8),
                            _mm256_extractf128_ps(sum8, 1));
    sum4 = _mm_add_ps(sum4, _mm_movehl_ps(sum4, sum4));
    sum4 = _mm_add_ss(sum4, _mm_movehdup_ps(sum4));
    
    checksum += _mm_cvtss_f32(sum4);
    
    return checksum;
}

int main() {
    uint64_t final_checksum = 0;
    
    // Initialize test vectors with patterned data
    __m512i v64qi_a = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(0xFF);
    
    __m512i v32hi_a = _mm512_set_epi16(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(0xFFFF);
    
    __m512i v16si_a = _mm512_set_epi32(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
    );
    
    __m512i v16si_b = _mm512_set1_epi32(0xFFFFFFFF);
    
    __m512i v8di_a = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    __m512i v8di_b = _mm512_set1_epi64(0xFFFFFFFFFFFFFFFFULL);
    
    __m512 v16sf_a = _mm512_set_ps(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(100.0f);
    
    __m512d v8df_a = _mm512_set_pd(0.0,1.0,2.0,3.0,4.0,5.0,6.0,7.0);
    __m512d v8df_b = _mm512_set1_pd(100.0);
    
#ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set_ph(
        0.0f,1.0f,2.0f,3.0f,4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,12.0f,13.0f,14.0f,15.0f,
        16.0f,17.0f,18.0f,19.0f,20.0f,21.0f,22.0f,23.0f,
        24.0f,25.0f,26.0f,27.0f,28.0f,29.0f,30.0f,31.0f
    );
    
    __m512h v32hf_b = _mm512_set1_ph(100.0f);
    
    __m512bh v32bf_a = _mm512_castph_bh(v32hf_a);
    __m512bh v32bf_b = _mm512_castph_bh(v32hf_b);
#endif
    
    // Loop with control flow to prevent optimization
    for (int i = 0; i < 5; i++) {
        // Generate masks using different methods
        __mmask64 mask64;
        __mmask32 mask32;
        __mmask16 mask16;
        __mmask8 mask8;
        
        // Comparison-based masks
        if (i % 2 == 0) {
            mask64 = _mm512_cmpeq_epi8_mask(v64qi_a, _mm512_set1_epi8(i));
            mask32 = _mm512_cmpeq_epi16_mask(v32hi_a, _mm512_set1_epi16(i));
            mask16 = _mm512_cmpeq_epi32_mask(v16si_a, _mm512_set1_epi32(i));
            mask8 = _mm512_cmpeq_epi64_mask(v8di_a, _mm512_set1_epi64(i));
        } else {
            // Immediate masks
            mask64 = (__mmask64)(0xAAAAAAAAAAAAAAAAULL >> (i * 4));
            mask32 = (__mmask32)(0xAAAAAAAAUL >> (i * 2));
            mask16 = (__mmask16)(0xAAAA >> i);
            mask8 = (__mmask8)(0xAA >> i);
        }
        
        // Logical mask operations
        if (i > 2) {
            mask64 = _kor_mask64(mask64, 0x00000000FFFFFFFFULL);
            mask32 = _kxor_mask32(mask32, 0xFFFFFFFFUL);
            mask16 = _kand_mask16(mask16, 0x0F0F);
            mask8 = _knot_mask8(mask8);
        }
        
        // Call blend functions for all modes
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8);
        
        // Generate float masks from comparisons
        __mmask16 float_mask = _mm512_cmp_ps_mask(v16sf_a, _mm512_set1_ps(i), _CMP_EQ_OQ);
        __mmask8 double_mask = _mm512_cmp_pd_mask(v8df_a, _mm512_set1_pd(i), _CMP_EQ_OQ);
        
        // Apply logical operations
        float_mask = _kor_mask16(float_mask, (__mmask16)(0xAAAA >> i));
        double_mask = _kxor_mask8(double_mask, (__mmask8)(0xAA >> i));
        
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, float_mask);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, double_mask);
        
#ifdef __AVX512FP16__
        // Half-precision masks
        __mmask32 half_mask;
        if (i % 2 == 0) {
            half_mask = _mm512_cmp_ph_mask(v32hf_a, _mm512_set1_ph(i), _CMP_EQ_OQ);
        } else {
            half_mask = (__mmask32)(0xAAAAAAAAUL >> (i * 2));
        }
        
        half_mask = _kor_mask32(half_mask, 0x55555555UL);
        
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, half_mask);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, half_mask);
#endif
        
        // Data dependency chain test
        float chain_result = data_dependency_chain(v64qi_a, v16sf_a, i);
        
        // Accumulate results to prevent optimization
        // Simple reduction: sum first few elements
        uint8_t* p64qi = (uint8_t*)&result64qi;
        uint16_t* p32hi = (uint16_t*)&result32hi;
        int32_t* p16si = (int32_t*)&result16si;
        int64_t* p8di = (int64_t*)&result8di;
        float* p16sf = (float*)&result16sf;
        double* p8df = (double*)&result8df;
        
        for (int j = 0; j < 8; j++) {
            final_checksum += p64qi[j];
            final_checksum += p32hi[j];
            final_checksum += p16si[j];
            final_checksum += p8di[j];
            final_checksum += (uint64_t)(p16sf[j] * 1000);
            final_checksum += (uint64_t)(p8df[j] * 1000);
        }
        
        final_checksum += (uint64_t)(chain_result * 1000);
    }
    
    printf("Final checksum: %lu\n", final_checksum);
    return 0;
}
