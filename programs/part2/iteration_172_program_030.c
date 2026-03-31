/* AVX-512 Blend Instruction Coverage Test
 * Targets specific uncovered lines in i386-expand.cc
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw -mavx512fp16 -mavx512bf16 -fprofile-arcs -ftest-coverage avx512_blend_test.c -o avx512_blend_test
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Prevent aggressive optimization */
static volatile int g_volatile_mask = 0x55555555;

/* Function to generate dynamic masks based on input data */
__attribute__((target("avx512f,avx512bw")))
static __mmask64 generate_mask64(int seed) {
    /* Create a non-constant mask pattern */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        mask |= ((seed + i) & 1) ? (1ULL << i) : 0;
    }
    return mask ^ g_volatile_mask;
}

__attribute__((target("avx512f,avx512bw")))
static __mmask32 generate_mask32(int seed) {
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        mask |= ((seed + i) & 1) ? (1U << i) : 0;
    }
    return mask ^ (g_volatile_mask & 0xFFFFFFFF);
}

__attribute__((target("avx512f")))
static __mmask16 generate_mask16(int seed) {
    __mmask16 mask = 0;
    for (int i = 0; i < 16; i++) {
        mask |= ((seed + i) & 1) ? (1 << i) : 0;
    }
    return mask ^ (g_volatile_mask & 0xFFFF);
}

__attribute__((target("avx512f")))
static __mmask8 generate_mask8(int seed) {
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        mask |= ((seed + i) & 1) ? (1 << i) : 0;
    }
    return mask ^ (g_volatile_mask & 0xFF);
}

/* Test functions for each vector mode */

/* V64QImode - 64-byte integers */
__attribute__((target("avx512bw")))
void test_v64qimode(uint8_t* a, uint8_t* b, uint8_t* out, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    __mmask64 mask = generate_mask64(seed);
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V32HImode - 32 half-word integers */
__attribute__((target("avx512bw")))
void test_v32himode(int16_t* a, int16_t* b, int16_t* out, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V32HFmode - 32 half-precision floats (requires AVX512-FP16) */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
void test_v32hfmode(_Float16* a, _Float16* b, _Float16* out, int seed) {
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_ph(out, result);
}
#endif

/* V32BFmode - 32 bfloat16 (requires AVX512-BF16) */
#ifdef __AVX512BF16__
#include <bfloat16.h>
__attribute__((target("avx512bf16")))
void test_v32bfmode(__bfloat16* a, __bfloat16* b, __bfloat16* out, int seed) {
    __m512bh va = _mm512_loadu_bf16(a);
    __m512bh vb = _mm512_loadu_bf16(b);
    __mmask32 mask = generate_mask32(seed);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_bf16(out, result);
}
#endif

/* V16SImode - 16 single-word integers */
__attribute__((target("avx512f")))
void test_v16simode(int32_t* a, int32_t* b, int32_t* out, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    __mmask16 mask = generate_mask16(seed);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V8DImode - 8 double-word integers */
__attribute__((target("avx512f")))
void test_v8dimode(int64_t* a, int64_t* b, int64_t* out, int seed) {
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    __mmask8 mask = generate_mask8(seed);
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
}

/* V8DFmode - 8 double-precision floats */
__attribute__((target("avx512f")))
void test_v8dfmode(double* a, double* b, double* out, int seed) {
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    __mmask8 mask = generate_mask8(seed);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_storeu_pd(out, result);
}

/* V16SFmode - 16 single-precision floats */
__attribute__((target("avx512f")))
void test_v16sfmode(float* a, float* b, float* out, int seed) {
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    __mmask16 mask = generate_mask16(seed);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_storeu_ps(out, result);
}

/* Main test driver */
int main() {
    int i;
    uint64_t checksum = 0;
    
    /* Initialize test data */
    uint8_t a8[64], b8[64], out8[64];
    int16_t a16[32], b16[32], out16[32];
    int32_t a32[16], b32[16], out32[16];
    int64_t a64[8], b64[8], out64[8];
    float a32f[16], b32f[16], out32f[16];
    double a64f[8], b64f[8], out64f[8];
    
    for (i = 0; i < 64; i++) {
        a8[i] = i;
        b8[i] = 64 - i;
    }
    
    for (i = 0; i < 32; i++) {
        a16[i] = i * 2;
        b16[i] = 64 - i * 2;
    }
    
    for (i = 0; i < 16; i++) {
        a32[i] = i * 4;
        b32[i] = 64 - i * 4;
        a32f[i] = i * 0.25f;
        b32f[i] = 16.0f - i * 0.25f;
    }
    
    for (i = 0; i < 8; i++) {
        a64[i] = i * 8;
        b64[i] = 64 - i * 8;
        a64f[i] = i * 0.125;
        b64f[i] = 8.0 - i * 0.125;
    }
    
    /* Run tests in a loop to ensure execution */
    for (int iter = 0; iter < 100; iter++) {
        int seed = iter * 7;  /* Non-trivial seed pattern */
        
        /* Test each vector mode */
        test_v64qimode(a8, b8, out8, seed);
        test_v32himode(a16, b16, out16, seed);
        test_v16simode(a32, b32, out32, seed);
        test_v8dimode(a64, b64, out64, seed);
        test_v8dfmode(a64f, b64f, out64f, seed);
        test_v16sfmode(a32f, b32f, out32f, seed);
        
#ifdef __AVX512FP16__
        _Float16 a16f[32], b16f[32], out16f[32];
        for (i = 0; i < 32; i++) {
            a16f[i] = i * 0.5f;
            b16f[i] = 16.0f - i * 0.5f;
        }
        test_v32hfmode(a16f, b16f, out16f, seed);
#endif
        
#ifdef __AVX512BF16__
        __bfloat16 a16bf[32], b16bf[32], out16bf[32];
        for (i = 0; i < 32; i++) {
            a16bf[i] = bfloat16_from_float(i * 0.5f);
            b16bf[i] = bfloat16_from_float(16.0f - i * 0.5f);
        }
        test_v32bfmode(a16bf, b16bf, out16bf, seed);
#endif
        
        /* Update checksum to prevent dead code elimination */
        for (i = 0; i < 64; i++) checksum += out8[i];
        for (i = 0; i < 32; i++) checksum += out16[i];
        for (i = 0; i < 16; i++) checksum += out32[i] + (uint64_t)(out32f[i] * 1000);
        for (i = 0; i < 8; i++) checksum += out64[i] + (uint64_t)(out64f[i] * 1000);
    }
    
    printf("Checksum: %lu\n", checksum);
    return 0;
}

#ifdef __cplusplus
}
#endif
