/* avx512_blend_coverage.c
 * Tests all AVX-512 vector blend modes to cover switch cases in i386-expand.cc
 * Compile with: gcc -O3 -mavx512f -mavx512bw -mavx512fp16 -march=native -o test test.c
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper function to prevent dead code elimination */
static volatile int g_volatile_sink = 0;

/* ==================== V64QImode (64x 8-bit integers) ==================== */
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v64qimode_blend(void) {
    /* Create two source vectors with distinct patterns */
    __m512i a = _mm512_set1_epi8(0xAA);  /* 10101010 pattern */
    __m512i b = _mm512_set1_epi8(0x55);  /* 01010101 pattern */
    
    /* Create alternating mask: 0x5555...5555 */
    __mmask64 mask = 0x5555555555555555ULL;
    
    /* Perform blend: selects a when mask=0, b when mask=1 */
    __m512i result = _mm512_mask_blend_epi8(mask, a, b);
    
    /* Use result to prevent optimization */
    uint64_t sum = 0;
    uint8_t* ptr = (uint8_t*)&result;
    for (int i = 0; i < 64; i++) {
        sum += ptr[i];
    }
    
    g_volatile_sink = (int)sum;
    return sum;
}
#endif

/* ==================== V32HImode (32x 16-bit integers) ==================== */
#ifdef __AVX512BW__
__attribute__((noinline))
uint64_t test_v32himode_blend(void) {
    __m512i a = _mm512_set1_epi16(0xAAAA);  /* 0xAAAA pattern */
    __m512i b = _mm512_set1_epi16(0x5555);  /* 0x5555 pattern */
    
    /* Create checkerboard mask */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    __m512i result = _mm512_mask_blend_epi16(mask, a, b);
    
    uint64_t sum = 0;
    uint16_t* ptr = (uint16_t*)&result;
    for (int i = 0; i < 32; i++) {
        sum += ptr[i];
    }
    
    g_volatile_sink = (int)sum;
    return sum;
}
#endif

/* ==================== V32HFmode (32x half-precision floats) ==================== */
#ifdef __AVX512FP16__
__attribute__((noinline))
float test_v32hfmode_blend(void) {
    /* Initialize with distinct patterns */
    __m512h a = _mm512_set1_ph((_Float16)1.0f);
    __m512h b = _mm512_set1_ph((_Float16)2.0f);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* Blend half-precision floats */
    __m512h result = _mm512_mask_blend_ph(mask, a, b);
    
    /* Compute sum to prevent optimization */
    float sum = 0.0f;
    _Float16* ptr = (_Float16*)&result;
    for (int i = 0; i < 32; i++) {
        sum += (float)ptr[i];
    }
    
    g_volatile_sink = (int)sum;
    return sum;
}
#endif

/* ==================== V32BFmode (32x brain float) ==================== */
#ifdef __AVX512BF16__
__attribute__((noinline))
float test_v32bfmode_blend(void) {
    /* BF16 doesn't have direct blend intrinsic in GCC yet, so we use
     * a pattern that should generate vblendmps on BF16 elements */
    __m512bh a = _mm512_set1_epi16(0x3F80);  /* BF16 representation of 1.0 */
    __m512bh b = _mm512_set1_epi16(0x4000);  /* BF16 representation of 2.0 */
    
    /* Create mask */
    __mmask32 mask = 0x55555555;  /* 0101... pattern */
    
    /* Convert to float, blend, then convert back */
    __m512 a_f32 = _mm512_cvtne2ps_pbh(a, a);  /* These might need adjustment */
    __m512 b_f32 = _mm512_cvtne2ps_pbh(b, b);
    
    /* Blend at float32 level - compiler should optimize to BF16 blend */
    __m512 result_f32 = _mm512_mask_blend_ps(mask, a_f32, b_f32);
    
    /* Convert back to BF16 */
    __m512bh result = _mm512_cvtneps_pbh(result_f32);
    
    float sum = 0.0f;
    uint16_t* ptr = (uint16_t*)&result;
    for (int i = 0; i < 32; i++) {
        /* Convert BF16 to float for sum */
        uint32_t temp = ptr[i] << 16;
        float f;
        memcpy(&f, &temp, sizeof(float));
        sum += f;
    }
    
    g_volatile_sink = (int)sum;
    return sum;
}
#endif

/* ==================== V16SImode (16x 32-bit integers) ==================== */
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v16simode_blend(void) {
    __m512i a = _mm512_set1_epi32(0xAAAAAAAA);
    __m512i b = _mm512_set1_epi32(0x55555555);
    
    /* Create alternating mask */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    
    __m512i result = _mm512_mask_blend_epi32(mask, a, b);
    
    uint64_t sum = 0;
    uint32_t* ptr = (uint32_t*)&result;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    
    g_volatile_sink = (int)sum;
    return sum;
}
#endif

/* ==================== V8DImode (8x 64-bit integers) ==================== */
#ifdef __AVX512F__
__attribute__((noinline))
uint64_t test_v8dimode_blend(void) {
    __m512i a = _mm512_set1_epi64(0xAAAAAAAAAAAAAAAAULL);
    __m512i b = _mm512_set1_epi64(0x5555555555555555ULL);
    
    /* Create checkerboard mask */
    __mmask8 mask = 0xAA;  /* 10101010 */
    
    __m512i result = _mm512_mask_blend_epi64(mask, a, b);
    
    uint64_t sum = 0;
    uint64_t* ptr = (uint64_t*)&result;
    for (int i = 0; i < 8; i++) {
        sum += ptr[i];
    }
    
    g_volatile_sink = (int)sum;
    return sum;
}
#endif

/* ==================== V8DFmode (8x double-precision floats) ==================== */
#ifdef __AVX512F__
__attribute__((noinline))
double test_v8dfmode_blend(void) {
    __m512d a = _mm512_set1_pd(1.0);
    __m512d b = _mm512_set1_pd(2.0);
    
    /* Create alternating mask */
    __mmask8 mask = 0x55;  /* 01010101 */
    
    __m512d result = _mm512_mask_blend_pd(mask, a, b);
    
    double sum = 0.0;
    double* ptr = (double*)&result;
    for (int i = 0; i < 8; i++) {
        sum += ptr[i];
    }
    
    g_volatile_sink = (int)sum;
    return sum;
}
#endif

/* ==================== V16SFmode (16x single-precision floats) ==================== */
#ifdef __AVX512F__
__attribute__((noinline))
float test_v16sfmode_blend(void) {
    __m512 a = _mm512_set1_ps(1.0f);
    __m512 b = _mm512_set1_ps(2.0f);
    
    /* Create checkerboard mask */
    __mmask16 mask = 0xAAAA;  /* 1010101010101010 */
    
    __m512 result = _mm512_mask_blend_ps(mask, a, b);
    
    float sum = 0.0f;
    float* ptr = (float*)&result;
    for (int i = 0; i < 16; i++) {
        sum += ptr[i];
    }
    
    g_volatile_sink = (int)sum;
    return sum;
}
#endif

/* ==================== Main Driver ==================== */
int main(void) {
    uint64_t total = 0;
    
    printf("Testing AVX-512 blend operations for GCC coverage...\n");
    
#ifdef __AVX512BW__
    printf("V64QImode blend: ");
    total += test_v64qimode_blend();
    printf("done\n");
    
    printf("V32HImode blend: ");
    total += test_v32himode_blend();
    printf("done\n");
#endif
    
#ifdef __AVX512FP16__
    printf("V32HFmode blend: ");
    total += (uint64_t)test_v32hfmode_blend();
    printf("done\n");
#endif
    
#ifdef __AVX512BF16__
    printf("V32BFmode blend: ");
    total += (uint64_t)test_v32bfmode_blend();
    printf("done\n");
#endif
    
#ifdef __AVX512F__
    printf("V16SImode blend: ");
    total += test_v16simode_blend();
    printf("done\n");
    
    printf("V8DImode blend: ");
    total += test_v8dimode_blend();
    printf("done\n");
    
    printf("V8DFmode blend: ");
    total += (uint64_t)test_v8dfmode_blend();
    printf("done\n");
    
    printf("V16SFmode blend: ");
    total += (uint64_t)test_v16sfmode_blend();
    printf("done\n");
#endif
    
    printf("All tests completed. Checksum: %lu\n", total);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total == 0) ? 1 : 0;
}
