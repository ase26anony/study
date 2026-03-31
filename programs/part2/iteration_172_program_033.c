/* AVX-512 blend coverage test for i386-expand.cc lines 4303-4326 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Function attributes to ensure proper ISA targeting */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
void test_v64qi_blend(void) {
    /* V64QImode: 64 bytes */
    __m512i a = _mm512_set1_epi8(0x11);
    __m512i b = _mm512_set1_epi8(0x22);
    
    /* Create dynamic mask: alternating pattern based on runtime value */
    volatile int seed = 42;
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Store to prevent optimization */
    volatile __m512i store_result = result;
    (void)store_result;
}

__attribute__((target("avx512bw")))
void test_v32hi_blend(void) {
    /* V32HImode: 32 half-words */
    __m512i a = _mm512_set1_epi16(0x1111);
    __m512i b = _mm512_set1_epi16(0x2222);
    
    volatile int seed = 123;
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i * seed) % 5 == 0) {
            mask |= (1U << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    volatile __m512i store_result = result;
    (void)store_result;
}
#endif

#ifdef __AVX512FP16__
__attribute__((target("avx512fp16,avx512bw")))
void test_v32hf_blend(void) {
    /* V32HFmode: 32 half-precision floats */
    _Float16 pattern_a[32], pattern_b[32];
    for (int i = 0; i < 32; i++) {
        pattern_a[i] = (_Float16)(1.0f + i * 0.1f);
        pattern_b[i] = (_Float16)(2.0f + i * 0.1f);
    }
    
    __m512h a = _mm512_loadu_ph(pattern_a);
    __m512h b = _mm512_loadu_ph(pattern_b);
    
    volatile int seed = 456;
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i + seed) % 4 == 0) {
            mask |= (1U << i);
        }
    }
    
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    volatile __m512h store_result = result;
    (void)store_result;
}
#endif

#ifdef __AVX512BF16__
__attribute__((target("avx512bf16,avx512bw")))
void test_v32bf_blend(void) {
    /* V32BFmode: 32 bfloat16 */
    __m512bh a, b;
    
    /* Initialize with some pattern */
    uint16_t pattern_a[32], pattern_b[32];
    for (int i = 0; i < 32; i++) {
        pattern_a[i] = 0x3C00 + i; /* ~1.0 in bfloat16 */
        pattern_b[i] = 0x4000 + i; /* ~2.0 in bfloat16 */
    }
    
    memcpy(&a, pattern_a, sizeof(a));
    memcpy(&b, pattern_b, sizeof(b));
    
    volatile int seed = 789;
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i * seed) % 3 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Use the same intrinsic as V32HFmode but with bfloat16 types */
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    volatile __m512bh store_result = result;
    (void)store_result;
}
#endif

#ifdef __AVX512F__
__attribute__((target("avx512f")))
void test_v16si_blend(void) {
    /* V16SImode: 16 signed ints */
    __m512i a = _mm512_set1_epi32(0x11111111);
    __m512i b = _mm512_set1_epi32(0x22222222);
    
    volatile int seed = 101;
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i + seed) % 2 == 0) {
            mask |= (1U << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    volatile __m512i store_result = result;
    (void)store_result;
}

__attribute__((target("avx512f")))
void test_v8di_blend(void) {
    /* V8DImode: 8 double ints */
    __m512i a = _mm512_set1_epi64(0x1111111111111111ULL);
    __m512i b = _mm512_set1_epi64(0x2222222222222222ULL);
    
    volatile int seed = 202;
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i * seed) % 3 == 0) {
            mask |= (1U << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    volatile __m512i store_result = result;
    (void)store_result;
}

__attribute__((target("avx512f")))
void test_v8df_blend(void) {
    /* V8DFmode: 8 doubles */
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    volatile int seed = 303;
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i + seed) % 2 == 0) {
            mask |= (1U << i);
        }
    }
    
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    volatile __m512d store_result = result;
    (void)store_result;
}

__attribute__((target("avx512f")))
void test_v16sf_blend(void) {
    /* V16SFmode: 16 floats */
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    volatile int seed = 404;
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        if ((i * seed) % 3 == 0) {
            mask |= (1U << i);
        }
    }
    
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    volatile __m512 store_result = result;
    (void)store_result;
}
#endif

/* Main test driver */
int main(void) {
    int checksum = 0;
    
#ifdef __AVX512BW__
    test_v64qi_blend();
    checksum += 1;
    
    test_v32hi_blend();
    checksum += 2;
#endif

#ifdef __AVX512FP16__
    test_v32hf_blend();
    checksum += 4;
#endif

#ifdef __AVX512BF16__
    test_v32bf_blend();
    checksum += 8;
#endif

#ifdef __AVX512F__
    test_v16si_blend();
    checksum += 16;
    
    test_v8di_blend();
    checksum += 32;
    
    test_v8df_blend();
    checksum += 64;
    
    test_v16sf_blend();
    checksum += 128;
#endif

    printf("AVX-512 blend test completed. Checksum: %d\n", checksum);
    
    /* Return non-zero if any required ISA was missing */
#ifdef __AVX512F__
    return 0;
#else
    return 1;
#endif
}

#ifdef __cplusplus
}
#endif
