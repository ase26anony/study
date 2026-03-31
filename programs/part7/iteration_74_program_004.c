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
        // Generate variable mask: compare elements with threshold (i % 256)
        __mmask64 mask = _mm512_cmplt_epi8_mask(a[i], _mm512_set1_epi8(i % 256));
        // Blend using AVX512BW intrinsic
        result[i] = _mm512_mask_blend_epi8(mask, a[i], b[i]);
    }
}

// ==================== 32-word integer vectors (E_V32HImode) ====================
void blend_32hi(__m512i* result, const __m512i* a, const __m512i* b, int size) {
    for (int i = 0; i < size; i++) {
        // Variable mask based on comparison with index-dependent value
        __mmask32 mask = _mm512_cmpgt_epi16_mask(a[i], _mm512_set1_epi16(i * 10));
        // Blend using AVX512BW intrinsic
        result[i] = _mm512_mask_blend_epi16(mask, a[i], b[i]);
    }
}

// ==================== 32 half-precision float vectors (E_V32HFmode) ====================
#ifdef __AVX512FP16__
void blend_32hf(__m512h* result, const __m512h* a, const __m512h* b, int size) {
    for (int i = 0; i < size; i++) {
        // Generate mask: compare with threshold that varies per iteration
        __mmask32 mask = _mm512_cmp_ph_mask(a[i], 
                                           _mm512_set1_ph((_Float16)(i * 0.1f)), 
                                           _CMP_LT_OQ);
        // Blend using AVX512-FP16 intrinsic (emulated with cast for compatibility)
        result[i] = _mm512_mask_blend_ph(mask, a[i], b[i]);
    }
}
#endif

// ==================== 32 brain-float vectors (E_V32BFmode) ====================
#ifdef __AVX512BF16__
void blend_32bf(__m512bh* result, const __m512bh* a, const __m512bh* b, int size) {
    for (int i = 0; i < size; i++) {
        // Create mask based on index parity
        __mmask32 mask = 0;
        for (int j = 0; j < 32; j++) {
            if ((i + j) % 3 == 0) mask |= (1ULL << j);
        }
        // Blend using AVX512BF16 intrinsic
        result[i] = _mm512_mask_blend_epi16(mask, 
                                           (__m512i)a[i], 
                                           (__m512i)b[i]);
    }
}
#endif

// ==================== 16 single-precision float vectors (E_V16SFmode) ====================
void blend_16sf(__m512* result, const __m512* a, const __m512* b, int size) {
    for (int i = 0; i < size; i++) {
        // Variable mask: compare with sine wave pattern
        __m512 threshold = _mm512_set1_ps(sinf(i * 0.1f) * 100.0f);
        __mmask16 mask = _mm512_cmp_ps_mask(a[i], threshold, _CMP_GT_OQ);
        // Blend using AVX512F intrinsic
        result[i] = _mm512_mask_blend_ps(mask, a[i], b[i]);
    }
}

// ==================== 8 double-precision float vectors (E_V8DFmode) ====================
void blend_8df(__m512d* result, const __m512d* a, const __m512d* b, int size) {
    for (int i = 0; i < size; i++) {
        // Variable mask: compare with cosine pattern
        __m512d threshold = _mm512_set1_pd(cos(i * 0.1) * 50.0);
        __mmask8 mask = _mm512_cmp_pd_mask(a[i], threshold, _CMP_LT_OQ);
        // Blend using AVX512F intrinsic
        result[i] = _mm512_mask_blend_pd(mask, a[i], b[i]);
    }
}

// ==================== 16 doubleword integer vectors (E_V16SImode) ====================
void blend_16si(__m512i* result, const __m512i* a, const __m512i* b, int size) {
    for (int i = 0; i < size; i++) {
        // Variable mask: compare with index-based value
        __mmask16 mask = _mm512_cmpeq_epi32_mask(
            _mm512_and_epi32(a[i], _mm512_set1_epi32(1)),
            _mm512_set1_epi32(0)
        );
        // Blend using AVX512F intrinsic
        result[i] = _mm512_mask_blend_epi32(mask, a[i], b[i]);
    }
}

// ==================== 8 quadword integer vectors (E_V8DImode) ====================
void blend_8di(__m512i* result, const __m512i* a, const __m512i* b, int size) {
    for (int i = 0; i < size; i++) {
        // Variable mask: check if elements are divisible by 3
        __mmask8 mask = _mm512_cmpeq_epi64_mask(
            _mm512_and_epi64(a[i], _mm512_set1_epi64(3)),
            _mm512_set1_epi64(0)
        );
        // Blend using AVX512F intrinsic
        result[i] = _mm512_mask_blend_epi64(mask, a[i], b[i]);
    }
}

// ==================== Main function ====================
int main() {
    // Runtime CPU feature check
    if (!__builtin_cpu_supports("avx512f")) {
        printf("AVX-512F not supported on this CPU\n");
        return 1;
    }
    
    printf("AVX-512 supported. Running blend operations...\n");
    
    const int ARRAY_SIZE = 1024;
    const int VEC_SIZE = ARRAY_SIZE / 64; // 512-bit vectors
    
    // Initialize arrays with distinct patterns
    alignas(64) uint8_t a8[ARRAY_SIZE], b8[ARRAY_SIZE], r8[ARRAY_SIZE];
    alignas(64) uint16_t a16[ARRAY_SIZE], b16[ARRAY_SIZE], r16[ARRAY_SIZE];
    alignas(64) float a32f[ARRAY_SIZE], b32f[ARRAY_SIZE], r32f[ARRAY_SIZE];
    alignas(64) double a64f[ARRAY_SIZE/2], b64f[ARRAY_SIZE/2], r64f[ARRAY_SIZE/2];
    alignas(64) int32_t a32i[ARRAY_SIZE], b32i[ARRAY_SIZE], r32i[ARRAY_SIZE];
    alignas(64) int64_t a64i[ARRAY_SIZE/2], b64i[ARRAY_SIZE/2], r64i[ARRAY_SIZE/2];
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a8[i] = i % 256;
        b8[i] = 255 - (i % 256);
        a16[i] = i * 3;
        b16[i] = 65535 - i * 3;
        a32f[i] = i * 0.5f;
        b32f[i] = 1000.0f - i * 0.5f;
        a32i[i] = i * 7;
        b32i[i] = -i * 7;
        
        if (i < ARRAY_SIZE/2) {
            a64f[i] = i * 0.25;
            b64f[i] = 500.0 - i * 0.25;
            a64i[i] = i * 11LL;
            b64i[i] = -i * 11LL;
        }
    }
    
    unsigned long long checksum = 0;
    
    // Execute all blend operations
    blend_64qi((__m512i*)r8, (const __m512i*)a8, (const __m512i*)b8, VEC_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) checksum += r8[i];
    
    blend_32hi((__m512i*)r16, (const __m512i*)a16, (const __m512i*)b16, VEC_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) checksum += r16[i];
    
    blend_16sf((__m512*)r32f, (const __m512*)a32f, (const __m512*)b32f, VEC_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) checksum += (unsigned long long)r32f[i];
    
    blend_8df((__m512d*)r64f, (const __m512d*)a64f, (const __m512d*)b64f, VEC_SIZE/2);
    for (int i = 0; i < ARRAY_SIZE/2; i++) checksum += (unsigned long long)r64f[i];
    
    blend_16si((__m512i*)r32i, (const __m512i*)a32i, (const __m512i*)b32i, VEC_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) checksum += r32i[i];
    
    blend_8di((__m512i*)r64i, (const __m512i*)a64i, (const __m512i*)b64i, VEC_SIZE/2);
    for (int i = 0; i < ARRAY_SIZE/2; i++) checksum += r64i[i];
    
    #ifdef __AVX512FP16__
    alignas(64) _Float16 a16f[ARRAY_SIZE], b16f[ARRAY_SIZE], r16f[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a16f[i] = (_Float16)(i * 0.1f);
        b16f[i] = (_Float16)(100.0f - i * 0.1f);
    }
    blend_32hf((__m512h*)r16f, (const __m512h*)a16f, (const __m512h*)b16f, VEC_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) checksum += (unsigned long long)r16f[i];
    #endif
    
    #ifdef __AVX512BF16__
    alignas(64) __bfloat16 a16bf[ARRAY_SIZE], b16bf[ARRAY_SIZE], r16bf[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a16bf[i] = (__bfloat16)(i * 0.05f);
        b16bf[i] = (__bfloat16)(50.0f - i * 0.05f);
    }
    blend_32bf((__m512bh*)r16bf, (const __m512bh*)a16bf, (const __m512bh*)b16bf, VEC_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) checksum += (unsigned long long)r16bf[i];
    #endif
    
    printf("Final checksum: %llu\n", checksum);
    printf("All AVX-512 blend operations completed successfully.\n");
    
    return 0;
}
