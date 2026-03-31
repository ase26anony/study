/* avx512_blend_coverage.c - Test program for AVX-512 blend instruction expansion coverage */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Define fallbacks for systems without AVX-512 support */
#ifndef __AVX512F__
#warning "AVX-512F not enabled - coverage may be incomplete"
#endif

#ifndef __AVX512BW__
#warning "AVX-512BW not enabled - coverage may be incomplete"
#endif

#ifndef __AVX512FP16__
#warning "AVX512-FP16 not enabled - V32HFmode coverage may be incomplete"
#endif

#ifndef __AVX512BF16__
#warning "AVX512-BF16 not enabled - V32BFmode coverage may be incomplete"
#endif

/* Function attributes to ensure specific ISA usage */
#ifdef __GNUC__
#define AVX512F_TARGET __attribute__((target("avx512f")))
#define AVX512BW_TARGET __attribute__((target("avx512f,avx512bw")))
#define AVX512FP16_TARGET __attribute__((target("avx512f,avx512bw,avx512fp16")))
#define AVX512BF16_TARGET __attribute__((target("avx512f,avx512bw,avx512bf16")))
#else
#define AVX512F_TARGET
#define AVX512BW_TARGET
#define AVX512FP16_TARGET
#define AVX512BF16_TARGET
#endif

/* Prevent aggressive optimization */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

/* ==================== V64QImode (64 x 8-bit integers) ==================== */
AVX512BW_TARGET NOINLINE
void test_v64qimode(uint8_t* out, const uint8_t* a, const uint8_t* b, int seed) {
#ifdef __AVX512BW__
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Create dynamic mask based on seed and data */
    __m512i mask_data = _mm512_xor_si512(va, _mm512_set1_epi8(seed));
    __mmask64 mask = _mm512_cmpeq_epi8_mask(mask_data, _mm512_setzero_si512());
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
#else
    (void)out; (void)a; (void)b; (void)seed;
#endif
}

/* ==================== V32HImode (32 x 16-bit integers) ==================== */
AVX512BW_TARGET NOINLINE
void test_v32himode(uint16_t* out, const uint16_t* a, const uint16_t* b, int seed) {
#ifdef __AVX512BW__
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Dynamic mask based on seed */
    __m512i mask_data = _mm512_xor_si512(va, _mm512_set1_epi16(seed));
    __mmask32 mask = _mm512_cmpeq_epi16_mask(mask_data, _mm512_setzero_si512());
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
#else
    (void)out; (void)a; (void)b; (void)seed;
#endif
}

/* ==================== V32HFmode (32 x half-precision floats) ==================== */
#ifdef __AVX512FP16__
AVX512FP16_TARGET NOINLINE
void test_v32hfmode(_Float16* out, const _Float16* a, const _Float16* b, int seed) {
    __m512h va = _mm512_loadu_ph(a);
    __m512h vb = _mm512_loadu_ph(b);
    
    /* Create dynamic mask - compare with threshold based on seed */
    __m512h threshold = _mm512_set1_ph((_Float16)(seed * 0.01f));
    __mmask32 mask = _mm512_cmp_ph_mask(va, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_ph(out, result);
}
#else
void test_v32hfmode(_Float16* out, const _Float16* a, const _Float16* b, int seed) {
    (void)out; (void)a; (void)b; (void)seed;
}
#endif

/* ==================== V32BFmode (32 x bfloat16) ==================== */
#ifdef __AVX512BF16__
AVX512BF16_TARGET NOINLINE
void test_v32bfmode(__bfloat16* out, const __bfloat16* a, const __bfloat16* b, int seed) {
    /* Load bfloat16 data - need to use 32-bit loads and convert */
    __m512bh va = _mm512_loadu_bf16(a);
    __m512bh vb = _mm512_loadu_bf16(b);
    
    /* Create mask using integer operations since bfloat16 comparisons are limited */
    __m512i va_int = _mm512_loadu_si512((const __m512i*)a);
    __m512i seed_vec = _mm512_set1_epi16(seed);
    __mmask32 mask = _mm512_test_epi16_mask(va_int, seed_vec);
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_storeu_bf16(out, result);
}
#else
void test_v32bfmode(__bfloat16* out, const __bfloat16* a, const __bfloat16* b, int seed) {
    (void)out; (void)a; (void)b; (void)seed;
}
#endif

/* ==================== V16SImode (16 x 32-bit integers) ==================== */
AVX512F_TARGET NOINLINE
void test_v16simode(int32_t* out, const int32_t* a, const int32_t* b, int seed) {
#ifdef __AVX512F__
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Dynamic mask using comparison */
    __m512i seed_vec = _mm512_set1_epi32(seed);
    __mmask16 mask = _mm512_cmpeq_epi32_mask(_mm512_and_si512(va, seed_vec), _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
#else
    (void)out; (void)a; (void)b; (void)seed;
#endif
}

/* ==================== V8DImode (8 x 64-bit integers) ==================== */
AVX512F_TARGET NOINLINE
void test_v8dimode(int64_t* out, const int64_t* a, const int64_t* b, int seed) {
#ifdef __AVX512F__
    __m512i va = _mm512_loadu_si512((const __m512i*)a);
    __m512i vb = _mm512_loadu_si512((const __m512i*)b);
    
    /* Dynamic mask */
    __m512i seed_vec = _mm512_set1_epi64(seed);
    __mmask8 mask = _mm512_cmpeq_epi64_mask(_mm512_and_si512(va, seed_vec), _mm512_setzero_si512());
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_storeu_si512((__m512i*)out, result);
#else
    (void)out; (void)a; (void)b; (void)seed;
#endif
}

/* ==================== V8DFmode (8 x double precision) ==================== */
AVX512F_TARGET NOINLINE
void test_v8dfmode(double* out, const double* a, const double* b, int seed) {
#ifdef __AVX512F__
    __m512d va = _mm512_loadu_pd(a);
    __m512d vb = _mm512_loadu_pd(b);
    
    /* Create dynamic mask using comparison with threshold */
    __m512d threshold = _mm512_set1_pd(seed * 0.5);
    __mmask8 mask = _mm512_cmp_pd_mask(va, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, va, vb);
    
    _mm512_storeu_pd(out, result);
#else
    (void)out; (void)a; (void)b; (void)seed;
#endif
}

/* ==================== V16SFmode (16 x single precision) ==================== */
AVX512F_TARGET NOINLINE
void test_v16sfmode(float* out, const float* a, const float* b, int seed) {
#ifdef __AVX512F__
    __m512 va = _mm512_loadu_ps(a);
    __m512 vb = _mm512_loadu_ps(b);
    
    /* Dynamic mask */
    __m512 threshold = _mm512_set1_ps(seed * 0.25f);
    __mmask16 mask = _mm512_cmp_ps_mask(va, threshold, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, va, vb);
    
    _mm512_storeu_ps(out, result);
#else
    (void)out; (void)a; (void)b; (void)seed;
#endif
}

/* ==================== Main test driver ==================== */
int main(void) {
    /* Initialize test data */
    uint8_t a8[64], b8[64], out8[64];
    uint16_t a16[32], b16[32], out16[32];
    int32_t a32[16], b32[16], out32[16];
    int64_t a64[8], b64[8], out64[8];
    float afloat[16], bfloat[16], outfloat[16];
    double adouble[8], bdouble[8], outdouble[8];
    
#ifdef __AVX512FP16__
    _Float16 ahalf[32], bhalf[32], outhalf[32];
#endif
    
#ifdef __AVX512BF16__
    __bfloat16 abf16[32], bbf16[32], outbf16[32];
#endif
    
    /* Fill arrays with distinct patterns */
    for (int i = 0; i < 64; i++) {
        a8[i] = i * 3;
        b8[i] = i * 5 + 1;
        if (i < 32) {
            a16[i] = i * 7;
            b16[i] = i * 11 + 2;
#ifdef __AVX512FP16__
            ahalf[i] = (_Float16)(i * 0.1f);
            bhalf[i] = (_Float16)(i * 0.2f + 0.5f);
#endif
#ifdef __AVX512BF16__
            abf16[i] = (__bfloat16)(i * 0.15f);
            bbf16[i] = (__bfloat16)(i * 0.25f + 0.3f);
#endif
        }
        if (i < 16) {
            a32[i] = i * 13;
            b32[i] = i * 17 + 3;
            afloat[i] = i * 0.3f;
            bfloat[i] = i * 0.4f + 1.0f;
        }
        if (i < 8) {
            a64[i] = i * 19;
            b64[i] = i * 23 + 4;
            adouble[i] = i * 0.5;
            bdouble[i] = i * 0.6 + 2.0;
        }
    }
    
    /* Run tests multiple times with different seeds to ensure dynamic masks */
    unsigned long long checksum = 0;
    
    for (int iter = 0; iter < 3; iter++) {
        int seed = iter * 12345 + 6789;
        
        /* Test each vector mode */
        test_v64qimode(out8, a8, b8, seed);
        test_v32himode(out16, a16, b16, seed);
        
#ifdef __AVX512FP16__
        test_v32hfmode(outhalf, ahalf, bhalf, seed);
#endif
        
#ifdef __AVX512BF16__
        test_v32bfmode(outbf16, abf16, bbf16, seed);
#endif
        
        test_v16simode(out32, a32, b32, seed);
        test_v8dimode(out64, a64, b64, seed);
        test_v8dfmode(outdouble, adouble, bdouble, seed);
        test_v16sfmode(outfloat, afloat, bfloat, seed);
        
        /* Accumulate checksum to prevent dead code elimination */
        for (int i = 0; i < 64; i++) {
            checksum += out8[i];
            if (i < 32) checksum += out16[i];
            if (i < 16) checksum += out32[i] + (unsigned)outfloat[i];
            if (i < 8) checksum += (unsigned long long)out64[i] + (unsigned long long)outdouble[i];
        }
    }
    
    printf("Test completed. Checksum: %llu\n", checksum);
    return 0;
}
