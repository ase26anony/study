#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>
#include <x86intrin.h>

#ifdef __AVX512F__
#include <avx512fintrin.h>
#endif
#ifdef __AVX512BW__
#include <avx512bwintrin.h>
#endif
#ifdef __AVX512VL__
#include <avx512vlintrin.h>
#endif

// ==================== 64-byte integer vectors (E_V64QImode) ====================
void blend_64qi(__m512i* result, const __m512i* a, const __m512i* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate variable mask: compare elements with their index parity
        __m512i idx = _mm512_set_epi8(
            63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,
            47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i idx_offset = _mm512_set1_epi8(i);
        __m512i combined_idx = _mm512_add_epi8(idx, idx_offset);
        __mmask64 mask = _mm512_test_epi8_mask(combined_idx, _mm512_set1_epi8(1));
        
        // Load data (using aligned and unaligned patterns)
        __m512i va = (i % 2 == 0) ? _mm512_load_si512(a + i) : _mm512_loadu_si512(a + i);
        __m512i vb = (i % 2 == 0) ? _mm512_load_si512(b + i) : _mm512_loadu_si512(b + i);
        
        // Blend with variable mask
        result[i] = _mm512_mask_blend_epi8(mask, va, vb);
    }
}

// ==================== 32-word integer vectors (E_V32HImode) ====================
void blend_32hi(__m512i* result, const __m512i* a, const __m512i* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate mask based on threshold comparison
        __m512i va = _mm512_load_si512(a + i);
        __m512i vb = _mm512_load_si512(b + i);
        __m512i threshold = _mm512_set1_epi16(1000 + i * 50);
        __mmask32 mask = _mm512_cmpgt_epi16_mask(va, threshold);
        
        // Blend with variable mask
        result[i] = _mm512_mask_blend_epi16(mask, va, vb);
    }
}

// ==================== 32 half-precision float vectors (E_V32HFmode) ====================
#ifdef __AVX512FP16__
void blend_32hf(__m512h* result, const __m512h* a, const __m512h* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate mask using comparison
        __m512h va = _mm512_load_ph(a + i);
        __m512h vb = _mm512_load_ph(b + i);
        __m512h threshold = _mm512_set1_ph(0.5f + i * 0.1f);
        __mmask32 mask = _mm512_cmp_ph_mask(va, threshold, _CMP_GT_OQ);
        
        // Blend with variable mask
        result[i] = _mm512_mask_blend_ph(mask, va, vb);
    }
}
#endif

// ==================== 32 brain-float vectors (E_V32BFmode) ====================
#ifdef __AVX512BF16__
void blend_32bf(__m512bh* result, const __m512bh* a, const __m512bh* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate mask based on index parity
        __m512i idx = _mm512_set_epi16(
            31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,
            15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0
        );
        __m512i idx_offset = _mm512_set1_epi16(i);
        __m512i combined_idx = _mm512_add_epi16(idx, idx_offset);
        __mmask32 mask = _mm512_test_epi16_mask(combined_idx, _mm512_set1_epi16(1));
        
        // Load and blend
        __m512bh va = _mm512_load_si512(a + i);
        __m512bh vb = _mm512_load_si512(b + i);
        
        // Use integer blend since bfloat16 blend intrinsics may not be directly available
        __m512i vai = _mm512_castsi512_si512(va);
        __m512i vbi = _mm512_castsi512_si512(vb);
        __m512i blended = _mm512_mask_blend_epi16(mask, vai, vbi);
        result[i] = _mm512_castsi512_bh(blended);
    }
}
#endif

// ==================== 16 single-precision float vectors (E_V16SFmode) ====================
void blend_16sf(__m512* result, const __m512* a, const __m512* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate mask using dynamic comparison
        __m512 va = _mm512_load_ps(a + i);
        __m512 vb = _mm512_load_ps(b + i);
        __m512 threshold = _mm512_set1_ps(0.0f + i * 0.25f);
        __mmask16 mask = _mm512_cmp_ps_mask(va, threshold, _CMP_LT_OQ);
        
        // Blend with variable mask
        result[i] = _mm512_mask_blend_ps(mask, va, vb);
    }
}

// ==================== 8 double-precision float vectors (E_V8DFmode) ====================
void blend_8df(__m512d* result, const __m512d* a, const __m512d* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate mask using sine pattern
        __m512d va = _mm512_load_pd(a + i);
        __m512d vb = _mm512_load_pd(b + i);
        __m512d indices = _mm512_set_pd(7.0 + i, 6.0 + i, 5.0 + i, 4.0 + i, 
                                        3.0 + i, 2.0 + i, 1.0 + i, 0.0 + i);
        __m512d sine_vals = _mm512_sin_pd(_mm512_mul_pd(indices, _mm512_set1_pd(0.5)));
        __mmask8 mask = _mm512_cmp_pd_mask(sine_vals, _mm512_set1_pd(0.0), _CMP_GT_OQ);
        
        // Blend with variable mask
        result[i] = _mm512_mask_blend_pd(mask, va, vb);
    }
}

// ==================== 16 doubleword integer vectors (E_V16SImode) ====================
void blend_16si(__m512i* result, const __m512i* a, const __m512i* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate mask using equality comparison
        __m512i va = _mm512_load_si512(a + i);
        __m512i vb = _mm512_load_si512(b + i);
        __m512i pattern = _mm512_set1_epi32(i * 7);
        __mmask16 mask = _mm512_cmpeq_epi32_mask(_mm512_and_epi32(va, _mm512_set1_epi32(1)), pattern);
        
        // Blend with variable mask
        result[i] = _mm512_mask_blend_epi32(mask, va, vb);
    }
}

// ==================== 8 quadword integer vectors (E_V8DImode) ====================
void blend_8di(__m512i* result, const __m512i* a, const __m512i* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate mask using greater-than comparison
        __m512i va = _mm512_load_si512(a + i);
        __m512i vb = _mm512_load_si512(b + i);
        __m512i threshold = _mm512_set1_epi64(10000 + i * 1000);
        __mmask8 mask = _mm512_cmpgt_epi64_mask(va, threshold);
        
        // Blend with variable mask
        result[i] = _mm512_mask_blend_epi64(mask, va, vb);
    }
}

// ==================== Main function ====================
int main() {
    // Runtime CPU feature check
    if (!__builtin_cpu_supports("avx512f")) {
        printf("AVX-512F not supported on this processor\n");
        return 1;
    }
    
#ifdef __AVX512BW__
    if (!__builtin_cpu_supports("avx512bw")) {
        printf("AVX-512BW not supported on this processor\n");
        return 1;
    }
#endif
    
    printf("AVX-512 features supported. Starting blend operations...\n");
    
    const int ARRAY_SIZE = 16;
    unsigned long long checksum = 0;
    
    // Initialize arrays with distinct patterns
    alignas(64) __m512i a_64qi[ARRAY_SIZE];
    alignas(64) __m512i b_64qi[ARRAY_SIZE];
    alignas(64) __m512i r_64qi[ARRAY_SIZE];
    
    alignas(64) __m512i a_32hi[ARRAY_SIZE];
    alignas(64) __m512i b_32hi[ARRAY_SIZE];
    alignas(64) __m512i r_32hi[ARRAY_SIZE];
    
#ifdef __AVX512FP16__
    alignas(64) __m512h a_32hf[ARRAY_SIZE];
    alignas(64) __m512h b_32hf[ARRAY_SIZE];
    alignas(64) __m512h r_32hf[ARRAY_SIZE];
#endif
    
#ifdef __AVX512BF16__
    alignas(64) __m512bh a_32bf[ARRAY_SIZE];
    alignas(64) __m512bh b_32bf[ARRAY_SIZE];
    alignas(64) __m512bh r_32bf[ARRAY_SIZE];
#endif
    
    alignas(64) __m512 a_16sf[ARRAY_SIZE];
    alignas(64) __m512 b_16sf[ARRAY_SIZE];
    alignas(64) __m512 r_16sf[ARRAY_SIZE];
    
    alignas(64) __m512d a_8df[ARRAY_SIZE];
    alignas(64) __m512d b_8df[ARRAY_SIZE];
    alignas(64) __m512d r_8df[ARRAY_SIZE];
    
    alignas(64) __m512i a_16si[ARRAY_SIZE];
    alignas(64) __m512i b_16si[ARRAY_SIZE];
    alignas(64) __m512i r_16si[ARRAY_SIZE];
    
    alignas(64) __m512i a_8di[ARRAY_SIZE];
    alignas(64) __m512i b_8di[ARRAY_SIZE];
    alignas(64) __m512i r_8di[ARRAY_SIZE];
    
    // Fill arrays with pattern data
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // 64QI pattern
        for (int j = 0; j < 64; j++) {
            ((char*)&a_64qi[i])[j] = (i * 64 + j) % 256;
            ((char*)&b_64qi[i])[j] = (i * 64 + j + 128) % 256;
        }
        
        // 32HI pattern
        for (int j = 0; j < 32; j++) {
            ((short*)&a_32hi[i])[j] = (i * 32 + j) * 3;
            ((short*)&b_32hi[i])[j] = (i * 32 + j) * 5 + 100;
        }
        
        // 16SF pattern
        for (int j = 0; j < 16; j++) {
            ((float*)&a_16sf[i])[j] = (i * 16 + j) * 0.5f;
            ((float*)&b_16sf[i])[j] = (i * 16 + j) * 0.7f + 1.0f;
        }
        
        // 8DF pattern
        for (int j = 0; j < 8; j++) {
            ((double*)&a_8df[i])[j] = (i * 8 + j) * 0.25;
            ((double*)&b_8df[i])[j] = (i * 8 + j) * 0.35 + 0.5;
        }
        
        // 16SI pattern
        for (int j = 0; j < 16; j++) {
            ((int*)&a_16si[i])[j] = (i * 16 + j) * 11;
            ((int*)&b_16si[i])[j] = (i * 16 + j) * 13 + 7;
        }
        
        // 8DI pattern
        for (int j = 0; j < 8; j++) {
            ((long long*)&a_8di[i])[j] = (i * 8 + j) * 100LL;
            ((long long*)&b_8di[i])[j] = (i * 8 + j) * 150LL + 50LL;
        }
    }
    
    // Execute blend operations
    blend_64qi(r_64qi, a_64qi, b_64qi, ARRAY_SIZE);
    blend_32hi(r_32hi, a_32hi, b_32hi, ARRAY_SIZE);
    
#ifdef __AVX512FP16__
    blend_32hf(r_32hf, a_32hf, b_32hf, ARRAY_SIZE);
#endif
    
#ifdef __AVX512BF16__
    blend_32bf(r_32bf, a_32bf, b_32bf, ARRAY_SIZE);
#endif
    
    blend_16sf(r_16sf, a_16sf, b_16sf, ARRAY_SIZE);
    blend_8df(r_8df, a_8df, b_8df, ARRAY_SIZE);
    blend_16si(r_16si, a_16si, b_16si, ARRAY_SIZE);
    blend_8di(r_8di, a_8di, b_8di, ARRAY_SIZE);
    
    // Calculate checksum to prevent dead code elimination
    for (int i = 0; i < ARRAY_SIZE; i++) {
        // XOR all result elements into checksum
        for (int j = 0; j < 64; j++) {
            checksum ^= ((unsigned char*)&r_64qi[i])[j];
        }
        for (int j = 0; j < 32; j++) {
            checksum ^= ((unsigned short*)&r_32hi[i])[j];
        }
        for (int j = 0; j < 16; j++) {
            checksum ^= ((unsigned int*)&r_16sf[i])[j];
        }
        for (int j = 0; j < 8; j++) {
            checksum ^= ((unsigned long long*)&r_8df[i])[j];
        }
        for (int j = 0; j < 16; j++) {
            checksum ^= ((unsigned int*)&r_16si[i])[j];
        }
        for (int j = 0; j < 8; j++) {
            checksum ^= ((unsigned long long*)&r_8di[i])[j];
        }
    }
    
    printf("Blend operations completed. Checksum: 0x%016llx\n", checksum);
    printf("If checksum is non-zero, blend operations were executed.\n");
    
    return 0;
}
