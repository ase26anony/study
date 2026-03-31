#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __AVX512FP16__
#include <float.h>
#endif

/* Helper functions for different blend modes */
static inline __m512i blend_v64qi_v32hi(__m512i a, __m512i b, int selector) {
    __mmask64 mask64;
    
    if (selector & 1) {
        // Pattern 1: Alternating bytes
        mask64 = 0xAAAAAAAAAAAAAAAAULL;
    } else {
        // Pattern 2: Every 4th byte
        mask64 = 0x1111111111111111ULL;
    }
    
    __m512i result1 = _mm512_mask_blend_epi8(mask64, a, b);
    
    // Now blend with 16-bit elements using comparison-generated mask
    __mmask32 mask32 = _mm512_cmpeq_epi16_mask(result1, _mm512_setzero_si512());
    mask32 = _knot_mask32(mask32); // Invert the mask
    
    return _mm512_mask_blend_epi16(mask32, result1, _mm512_slli_epi16(b, 1));
}

static inline __m512 blend_v16sf_v8df(__m512 a, __m512 b, __m512d c, __m512d d, int selector) {
    __mmask16 mask16;
    
    if (selector & 2) {
        // Generate mask from comparison
        mask16 = _mm512_cmp_ps_mask(a, b, _CMP_LT_OQ);
    } else {
        // Fixed pattern mask
        mask16 = 0xAAAA;
    }
    
    __m512 result1 = _mm512_mask_blend_ps(mask16, a, b);
    
    // Chain to double precision blend
    __mmask8 mask8 = _mm512_cmp_pd_mask(_mm512_castps_pd(result1), 
                                       _mm512_castps_pd(b), _CMP_NEQ_UQ);
    
    // Blend with different mask pattern
    mask8 = _kor_mask8(mask8, 0x55); // OR with alternating pattern
    
    __m512d result2 = _mm512_mask_blend_pd(mask8, c, d);
    
    // Convert back to single precision for return
    return _mm512_castpd_ps(result2);
}

#ifdef __AVX512FP16__
static inline __m512h blend_v32hf_v32bf(__m512h a, __m512h b, int selector) {
    __mmask32 mask32;
    
    // Generate mask using comparison for half precision
    mask32 = _mm512_cmp_ph_mask(a, b, _CMP_GT_OQ);
    
    // Modify mask based on selector
    if (selector & 4) {
        mask32 = _knot_mask32(mask32);
    } else {
        mask32 = _kor_mask32(mask32, 0x55555555);
    }
    
    return _mm512_mask_blend_ph(mask32, a, b);
}
#endif

static inline __m512i blend_v16si_v8di(__m512i a, __m512i b, __m512i c, __m512i d, int selector) {
    __mmask16 mask16;
    __mmask8 mask8;
    
    // Generate 32-bit blend mask from comparison
    mask16 = _mm512_cmp_epi32_mask(a, b, _MM_CMPINT_LT);
    
    if (selector & 8) {
        mask16 = _knot_mask16(mask16);
    }
    
    __m512i result1 = _mm512_mask_blend_epi32(mask16, a, b);
    
    // Chain to 64-bit blend with different mask generation
    mask8 = _mm512_cmp_epi64_mask(result1, _mm512_setzero_si512(), _MM_CMPINT_EQ);
    mask8 = _kor_mask8(mask8, 0xAA); // OR with pattern
    
    return _mm512_mask_blend_epi64(mask8, c, d);
}

/* Main test function with data-dependent control flow */
int main() {
    volatile int runtime_selector = 0; // Volatile to prevent compile-time optimization
    
    // Initialize vectors with distinct patterns
    __m512i vi64_1 = _mm512_set_epi8(
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,
        32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,
        48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63
    );
    
    __m512i vi64_2 = _mm512_set_epi8(
        63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
        47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
        31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
        15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
    );
    
    __m512 vf32_1 = _mm512_set_ps(
        63.0f, 62.0f, 61.0f, 60.0f, 59.0f, 58.0f, 57.0f, 56.0f,
        55.0f, 54.0f, 53.0f, 52.0f, 51.0f, 50.0f, 49.0f, 48.0f
    );
    
    __m512 vf32_2 = _mm512_set_ps(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f
    );
    
    __m512d vf64_1 = _mm512_set_pd(7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0);
    __m512d vf64_2 = _mm512_set_pd(0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0);
    
    __m512i vi32_1 = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    __m512i vi32_2 = _mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
    
    __m512i vi64_3 = _mm512_set_epi64(7,6,5,4,3,2,1,0);
    __m512i vi64_4 = _mm512_set_epi64(0,1,2,3,4,5,6,7);
    
#ifdef __AVX512FP16__
    __m512h vhf32_1 = _mm512_set_ph(
        0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f,
        8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f,
        16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 21.0f, 22.0f, 23.0f,
        24.0f, 25.0f, 26.0f, 27.0f, 28.0f, 29.0f, 30.0f, 31.0f
    );
    
    __m512h vhf32_2 = _mm512_set_ph(
        31.0f, 30.0f, 29.0f, 28.0f, 27.0f, 26.0f, 25.0f, 24.0f,
        23.0f, 22.0f, 21.0f, 20.0f, 19.0f, 18.0f, 17.0f, 16.0f,
        15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f, 8.0f,
        7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.0f
    );
#endif
    
    // Accumulator for checksum
    double checksum = 0.0;
    
    // Loop with data-dependent control flow
    for (int i = 0; i < 16; i++) {
        runtime_selector = i; // Changes each iteration
        
        // Data-dependent selection of blend operations
        if (i % 3 == 0) {
            // Test V64QImode and V32HImode blends
            __m512i result = blend_v64qi_v32hi(vi64_1, vi64_2, runtime_selector);
            
            // Horizontal sum to prevent dead code elimination
            __m256i sum256 = _mm512_castsi512_si256(_mm512_add_epi64(
                _mm512_unpacklo_epi64(result, _mm512_setzero_si512()),
                _mm512_unpackhi_epi64(result, _mm512_setzero_si512())
            ));
            __m128i sum128 = _mm_add_epi64(
                _mm256_castsi256_si128(sum256),
                _mm256_extracti128_si256(sum256, 1)
            );
            checksum += (double)_mm_extract_epi64(sum128, 0);
            checksum += (double)_mm_extract_epi64(sum128, 1);
        } 
        else if (i % 3 == 1) {
            // Test V16SFmode and V8DFmode blends
            __m512 result = blend_v16sf_v8df(vf32_1, vf32_2, vf64_1, vf64_2, runtime_selector);
            
            // Reduce single precision
            checksum += (double)_mm512_reduce_add_ps(result);
        } 
        else {
            // Test V16SImode and V8DImode blends
            __m512i result = blend_v16si_v8di(vi32_1, vi32_2, vi64_3, vi64_4, runtime_selector);
            
            // Reduce 64-bit integers
            __m512i sum = _mm512_add_epi64(
                _mm512_and_si512(result, _mm512_set1_epi64(0xFFFFFFFF)),
                _mm512_srli_epi64(result, 32)
            );
            sum = _mm512_add_epi64(
                _mm512_and_si512(sum, _mm512_set1_epi64(0xFFFF)),
                _mm512_srli_epi64(sum, 16)
            );
            
            // Extract and accumulate
            long long* ptr = (long long*)&sum;
            for (int j = 0; j < 8; j++) {
                checksum += (double)ptr[j];
            }
        }
        
#ifdef __AVX512FP16__
        // Test V32HFmode and V32BFmode blends (every 4th iteration)
        if (i % 4 == 0) {
            __m512h hresult = blend_v32hf_v32bf(vhf32_1, vhf32_2, runtime_selector);
            
            // Convert to single precision for reduction
            __m512 fresult = _mm512_cvtph_ps(_mm512_castph_si512(hresult));
            checksum += (double)_mm512_reduce_add_ps(fresult);
        }
#endif
    }
    
    // Direct intrinsic calls for each mode (ensuring all code paths are generated)
    {
        // V64QImode
        __mmask64 m64 = 0xAAAAAAAAAAAAAAAAULL;
        __m512i r1 = _mm512_mask_blend_epi8(m64, vi64_1, vi64_2);
        
        // V32HImode with comparison-generated mask
        __mmask32 m32 = _mm512_cmpeq_epi16_mask(vi64_1, vi64_2);
        __m512i r2 = _mm512_mask_blend_epi16(m32, vi64_1, vi64_2);
        
        // V16SImode
        __mmask16 m16 = _mm512_cmp_epi32_mask(vi32_1, vi32_2, _MM_CMPINT_LT);
        __m512i r3 = _mm512_mask_blend_epi32(m16, vi32_1, vi32_2);
        
        // V8DImode
        __mmask8 m8 = _mm512_cmp_epi64_mask(vi64_3, vi64_4, _MM_CMPINT_EQ);
        __m512i r4 = _mm512_mask_blend_epi64(m8, vi64_3, vi64_4);
        
        // V16SFmode
        __mmask16 m16f = _mm512_cmp_ps_mask(vf32_1, vf32_2, _CMP_LT_OQ);
        __m512 r5 = _mm512_mask_blend_ps(m16f, vf32_1, vf32_2);
        
        // V8DFmode
        __mmask8 m8f = _mm512_cmp_pd_mask(vf64_1, vf64_2, _CMP_GT_OQ);
        __m512d r6 = _mm512_mask_blend_pd(m8f, vf64_1, vf64_2);
        
        // Accumulate results to prevent elimination
        long long* ptr1 = (long long*)&r1;
        long long* ptr2 = (long long*)&r2;
        long long* ptr3 = (long long*)&r3;
        long long* ptr4 = (long long*)&r4;
        
        for (int j = 0; j < 8; j++) {
            checksum += (double)ptr1[j];
            checksum += (double)ptr2[j];
            checksum += (double)ptr3[j];
            checksum += (double)ptr4[j];
        }
        
        checksum += (double)_mm512_reduce_add_ps(r5);
        checksum += (double)_mm512_reduce_add_pd(r6);
    }
    
    printf("Final checksum: %f\n", checksum);
    return 0;
}
