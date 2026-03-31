/* AVX-512 Blend Coverage Test for i386-expand.cc */
/* Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_test.c -o avx512_blend_test */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prevent aggressive optimization */
static volatile int g_volatile_mask = 0x55555555;

/* Function to generate dynamic mask based on input */
static __mmask64 generate_mask64(int seed) {
    return (__mmask64)((seed * 0x5DEECE66DLL + 0xBLL) & 0xFFFFFFFFFFFFFFFFULL);
}

static __mmask32 generate_mask32(int seed) {
    return (__mmask32)((seed * 0x5DEECE66DLL + 0xBLL) & 0xFFFFFFFF);
}

static __mmask16 generate_mask16(int seed) {
    return (__mmask16)((seed * 0x5DEECE66DLL + 0xBLL) & 0xFFFF);
}

static __mmask8 generate_mask8(int seed) {
    return (__mmask8)((seed * 0x5DEECE66DLL + 0xBLL) & 0xFF);
}

#ifdef __AVX512BW__
/* V64QImode - 64-byte integer blend */
__attribute__((target("avx512bw")))
static void test_v64qimode(int iter) {
    __m512i a = _mm512_set1_epi8(iter);
    __m512i b = _mm512_set1_epi8(iter + 1);
    __mmask64 mask = generate_mask64(iter);
    
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Store to prevent optimization */
    alignas(64) uint8_t output[64];
    _mm512_storeu_si512((__m512i*)output, result);
    
    /* Use result to affect control flow */
    if (output[0] & 1) {
        g_volatile_mask ^= 1;
    }
}

/* V32HImode - 32 half-word integer blend */
__attribute__((target("avx512bw")))
static void test_v32himode(int iter) {
    __m512i a = _mm512_set1_epi16(iter);
    __m512i b = _mm512_set1_epi16(iter + 1);
    __mmask32 mask = generate_mask32(iter);
    
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    alignas(64) uint16_t output[32];
    _mm512_storeu_si512((__m512i*)output, result);
    
    if (output[0] & 1) {
        g_volatile_mask ^= 2;
    }
}
#endif /* __AVX512BW__ */

#ifdef __AVX512FP16__
/* V32HFmode - 32 half-precision float blend */
__attribute__((target("avx512fp16")))
static void test_v32hfmode(int iter) {
    _Float16 val_a = (_Float16)(iter * 0.1f);
    _Float16 val_b = (_Float16)(iter * 0.2f);
    __m512h a = _mm512_set1_ph(val_a);
    __m512h b = _mm512_set1_ph(val_b);
    __mmask32 mask = generate_mask32(iter);
    
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    alignas(64) _Float16 output[32];
    _mm512_storeu_ph(output, result);
    
    if ((int)output[0] & 1) {
        g_volatile_mask ^= 4;
    }
}
#endif /* __AVX512FP16__ */

#ifdef __AVX512BF16__
/* V32BFmode - 32 bfloat16 blend */
__attribute__((target("avx512bf16")))
static void test_v32bfmode(int iter) {
    /* Use same intrinsic as FP16 but with bfloat16 type */
    __m512bh a = _mm512_set1_epi16(iter << 8);  /* Create bfloat16 pattern */
    __m512bh b = _mm512_set1_epi16((iter + 1) << 8);
    __mmask32 mask = generate_mask32(iter);
    
    __m512bh result = _mm512_mask_blend_ph(mask, a, b);
    
    alignas(64) uint16_t output[32];
    _mm512_storeu_si512((__m512i*)output, (__m512i)result);
    
    if (output[0] & 1) {
        g_volatile_mask ^= 8;
    }
}
#endif /* __AVX512BF16__ */

#ifdef __AVX512F__
/* V16SImode - 32-bit integer blend */
__attribute__((target("avx512f")))
static void test_v16simode(int iter) {
    __m512i a = _mm512_set1_epi32(iter);
    __m512i b = _mm512_set1_epi32(iter + 1);
    __mmask16 mask = generate_mask16(iter);
    
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    alignas(64) int32_t output[16];
    _mm512_storeu_si512((__m512i*)output, result);
    
    if (output[0] & 1) {
        g_volatile_mask ^= 16;
    }
}

/* V8DImode - 64-bit integer blend */
__attribute__((target("avx512f")))
static void test_v8dimode(int iter) {
    __m512i a = _mm512_set1_epi64(iter);
    __m512i b = _mm512_set1_epi64(iter + 1);
    __mmask8 mask = generate_mask8(iter);
    
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    alignas(64) int64_t output[8];
    _mm512_storeu_si512((__m512i*)output, result);
    
    if (output[0] & 1) {
        g_volatile_mask ^= 32;
    }
}

/* V8DFmode - double precision float blend */
__attribute__((target("avx512f")))
static void test_v8dfmode(int iter) {
    __m512d a = _mm512_set1_pd(iter * 0.1);
    __m512d b = _mm512_set1_pd(iter * 0.2);
    __mmask8 mask = generate_mask8(iter);
    
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    alignas(64) double output[8];
    _mm512_storeu_pd(output, result);
    
    if ((int)output[0] & 1) {
        g_volatile_mask ^= 64;
    }
}

/* V16SFmode - single precision float blend */
__attribute__((target("avx512f")))
static void test_v16sfmode(int iter) {
    __m512 a = _mm512_set1_ps(iter * 0.1f);
    __m512 b = _mm512_set1_ps(iter * 0.2f);
    __mmask16 mask = generate_mask16(iter);
    
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    alignas(64) float output[16];
    _mm512_storeu_ps(output, result);
    
    if ((int)output[0] & 1) {
        g_volatile_mask ^= 128;
    }
}
#endif /* __AVX512F__ */

/* Main test driver */
int main(void) {
    int i;
    int checksum = 0;
    
    printf("Testing AVX-512 blend instruction coverage...\n");
    
    /* Run multiple iterations to ensure coverage */
    for (i = 0; i < 100; i++) {
#ifdef __AVX512BW__
        test_v64qimode(i);
        test_v32himode(i);
#endif
        
#ifdef __AVX512FP16__
        test_v32hfmode(i);
#endif
        
#ifdef __AVX512BF16__
        test_v32bfmode(i);
#endif
        
#ifdef __AVX512F__
        test_v16simode(i);
        test_v8dimode(i);
        test_v8dfmode(i);
        test_v16sfmode(i);
#endif
        
        /* Accumulate volatile mask to prevent dead code elimination */
        checksum += g_volatile_mask;
    }
    
    printf("Test completed. Checksum: %d\n", checksum);
    
    /* Check if all required ISAs were available */
#ifndef __AVX512F__
    printf("WARNING: AVX512F not enabled\n");
#endif
#ifndef __AVX512BW__
    printf("WARNING: AVX512BW not enabled\n");
#endif
#ifndef __AVX512FP16__
    printf("WARNING: AVX512FP16 not enabled\n");
#endif
#ifndef __AVX512BF16__
    printf("WARNING: AVX512BF16 not enabled\n");
#endif
    
    return 0;
}

#ifdef __cplusplus
}
#endif
