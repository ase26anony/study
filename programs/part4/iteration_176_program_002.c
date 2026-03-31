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

// Helper function to create complex control flow
__attribute__((noinline))
__mmask64 generate_complex_mask64(int iteration, __m512i data) {
    __mmask64 mask;
    
    // Switch statement for control flow diversity
    switch (iteration % 4) {
        case 0:
            // Comparison mask
            mask = _mm512_cmpeq_epi8_mask(data, _mm512_set1_epi8(iteration));
            break;
        case 1:
            // Immediate mask
            mask = (__mmask64)0xAAAAAAAAAAAAAAAAULL;
            break;
        case 2:
            // Logical combination of masks
            __mmask64 m1 = _mm512_cmplt_epi8_mask(data, _mm512_set1_epi8(64));
            __mmask64 m2 = (__mmask64)0x5555555555555555ULL;
            mask = _kor_mask64(m1, m2);
            break;
        case 3:
            // Complex mask generation with dependency chain
            __mmask64 temp = _mm512_cmpgt_epi8_mask(data, _mm512_setzero_si512());
            mask = _kxor_mask64(temp, (__mmask64)0xFFFFFFFFFFFFFFFFULL);
            break;
    }
    
    // Additional control flow to prevent optimization
    if (iteration > 10) {
        mask = _kand_mask64(mask, (__mmask64)0x0F0F0F0F0F0F0F0FULL);
    }
    
    return mask;
}

// Similar functions for other mask types
__attribute__((noinline))
__mmask32 generate_complex_mask32(int iteration, __m512i data) {
    __mmask32 mask;
    
    if (iteration % 2 == 0) {
        mask = _mm512_cmpeq_epi16_mask(data, _mm512_set1_epi16(iteration));
    } else {
        mask = (__mmask32)0xAAAAAAAA;
        if (iteration > 5) {
            mask = _kor_mask32(mask, 0x55555555);
        }
    }
    
    return mask;
}

__attribute__((noinline))
__mmask16 generate_complex_mask16_float(int iteration, __m512 data) {
    __mmask16 mask;
    
    // Loop-based mask generation
    for (int i = 0; i < 2; i++) {
        if (i == 0) {
            mask = _mm512_cmp_ps_mask(data, _mm512_set1_ps(iteration), _CMP_LT_OQ);
        } else {
            __mmask16 temp = _mm512_cmp_ps_mask(data, _mm512_set1_ps(iteration * 2), _CMP_GT_OQ);
            mask = _kand_mask16(mask, temp);
        }
    }
    
    return mask;
}

__attribute__((noinline))
__mmask8 generate_complex_mask8_double(int iteration, __m512d data) {
    __mmask8 mask;
    
    // Nested if-else for control flow
    if (iteration < 3) {
        mask = _mm512_cmp_pd_mask(data, _mm512_set1_pd(1.0), _CMP_EQ_OQ);
    } else if (iteration < 6) {
        mask = (__mmask8)0xAA;
    } else {
        __mmask8 m1 = _mm512_cmp_pd_mask(data, _mm512_set1_pd(0.5), _CMP_LT_OQ);
        __mmask8 m2 = (__mmask8)0x55;
        mask = _kxor_mask8(m1, m2);
    }
    
    return mask;
}

// Horizontal sum reduction to prevent optimization
float horizontal_sum_ps(__m512 v) {
    __m256 vlow = _mm512_castps512_ps256(v);
    __m256 vhigh = _mm512_extractf32x8_ps(v, 1);
    vlow = _mm256_add_ps(vlow, vhigh);
    
    __m128 v128 = _mm256_castps256_ps128(vlow);
    __m128 vhigh128 = _mm256_extractf128_ps(vlow, 1);
    v128 = _mm_add_ps(v128, vhigh128);
    
    v128 = _mm_hadd_ps(v128, v128);
    v128 = _mm_hadd_ps(v128, v128);
    
    return _mm_cvtss_f32(v128);
}

double horizontal_sum_pd(__m512d v) {
    __m256d vlow = _mm512_castpd512_pd256(v);
    __m256d vhigh = _mm512_extractf64x4_pd(v, 1);
    vlow = _mm256_add_pd(vlow, vhigh);
    
    __m128d v128 = _mm256_castpd256_pd128(vlow);
    __m128d vhigh128 = _mm256_extractf128_pd(vlow, 1);
    v128 = _mm_add_pd(v128, vhigh128);
    
    return _mm_cvtsd_f64(_mm_hadd_pd(v128, v128));
}

int main() {
    uint64_t checksum = 0;
    
    // Initialize test data with patterns
    __m512i v64qi_a = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v64qi_b = _mm512_set1_epi8(0xFF);
    
    __m512i v32hi_a = _mm512_set_epi16(
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v32hi_b = _mm512_set1_epi16(0x7FFF);
    
    __m512i v16si_a = _mm512_set_epi32(
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512i v16si_b = _mm512_set1_epi32(0xFFFFFFFF);
    
    __m512i v8di_a = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i v8di_b = _mm512_set1_epi64(0x7FFFFFFFFFFFFFFFULL);
    
    __m512 v16sf_a = _mm512_set_ps(
        15.0f,14.0f,13.0f,12.0f,11.0f,10.0f,9.0f,8.0f,
        7.0f,6.0f,5.0f,4.0f,3.0f,2.0f,1.0f,0.0f
    );
    
    __m512 v16sf_b = _mm512_set1_ps(1.0f);
    
    __m512d v8df_a = _mm512_set_pd(7.0,6.0,5.0,4.0,3.0,2.0,1.0,0.0);
    __m512d v8df_b = _mm512_set1_pd(0.5);
    
    // Main test loop with control flow
    for (int i = 0; i < 8; i++) {
        // Generate masks with different methods
        __mmask64 mask64 = generate_complex_mask64(i, v64qi_a);
        __mmask32 mask32 = generate_complex_mask32(i, v32hi_a);
        __mmask16 mask16_int = (__mmask16)(0xAAAA >> (i % 4));
        __mmask8 mask8_int = (__mmask8)(0xAA >> (i % 2));
        __mmask16 mask16_float = generate_complex_mask16_float(i, v16sf_a);
        __mmask8 mask8_double = generate_complex_mask8_double(i, v8df_a);
        
        // Perform blends for all modes
        __m512i result64qi = blend_v64qi(v64qi_a, v64qi_b, mask64);
        __m512i result32hi = blend_v32hi(v32hi_a, v32hi_b, mask32);
        __m512i result16si = blend_v16si(v16si_a, v16si_b, mask16_int);
        __m512i result8di = blend_v8di(v8di_a, v8di_b, mask8_int);
        __m512 result16sf = blend_v16sf(v16sf_a, v16sf_b, mask16_float);
        __m512d result8df = blend_v8df(v8df_a, v8df_b, mask8_double);
        
        // Create dependency chain: use integer result to modify float data
        if (i > 0) {
            // Use previous integer blend result to generate new mask for float blend
            __mmask16 new_mask = _mm512_cmpeq_epi32_mask(
                result16si, 
                _mm512_set1_epi32(i)
            );
            result16sf = blend_v16sf(result16sf, _mm512_set1_ps(i * 2.0f), new_mask);
        }
        
        // Accumulate results to prevent optimization
        // Extract and sum elements from each result
        uint8_t* ptr64qi = (uint8_t*)&result64qi;
        for (int j = 0; j < 64; j++) {
            checksum += ptr64qi[j];
        }
        
        uint16_t* ptr32hi = (uint16_t*)&result32hi;
        for (int j = 0; j < 32; j++) {
            checksum += ptr32hi[j];
        }
        
        uint32_t* ptr16si = (uint32_t*)&result16si;
        for (int j = 0; j < 16; j++) {
            checksum += ptr16si[j];
        }
        
        uint64_t* ptr8di = (uint64_t*)&result8di;
        for (int j = 0; j < 8; j++) {
            checksum += ptr8di[j];
        }
        
        // Add floating point results (convert to integer for checksum)
        checksum += (uint64_t)horizontal_sum_ps(result16sf);
        checksum += (uint64_t)horizontal_sum_pd(result8df);
        
        // Modify input data for next iteration
        v64qi_a = _mm512_add_epi8(v64qi_a, _mm512_set1_epi8(1));
        v16sf_a = _mm512_add_ps(v16sf_a, _mm512_set1_ps(0.1f));
    }
    
    // Half-precision tests (conditional compilation)
    #ifdef __AVX512FP16__
    __m512h v32hf_a = _mm512_set1_ph(1.0f);
    __m512h v32hf_b = _mm512_set1_ph(2.0f);
    __m512bh v32bf_a = _mm512_castsi512_ph(_mm512_set1_epi16(0x3C00)); // 1.0 in FP16
    __m512bh v32bf_b = _mm512_castsi512_ph(_mm512_set1_epi16(0x4000)); // 2.0 in FP16
    
    // Test half-precision blends with different control flow
    for (int i = 0; i < 4; i++) {
        __mmask32 mask_half;
        
        // Switch statement for half-precision masks
        switch (i) {
            case 0:
                mask_half = (__mmask32)0xAAAAAAAA;
                break;
            case 1:
                mask_half = (__mmask32)0x55555555;
                break;
            case 2:
                mask_half = (__mmask32)0x0F0F0F0F;
                break;
            case 3:
                mask_half = (__mmask32)0xF0F0F0F0;
                break;
        }
        
        __m512h result32hf = blend_v32hf(v32hf_a, v32hf_b, mask_half);
        __m512bh result32bf = blend_v32bf(v32bf_a, v32bf_b, mask_half);
        
        // Add to checksum
        uint16_t* ptr_half = (uint16_t*)&result32hf;
        for (int j = 0; j < 32; j++) {
            checksum += ptr_half[j];
        }
    }
    #endif
    
    printf("Final checksum: %lu\n", checksum);
    return 0;
}
