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
static volatile int g_volatile_counter = 0;

/* ========== V64QImode (64-byte integers) ========== */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static void test_v64qimode(uint8_t* src1, uint8_t* src2, uint8_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask based on iteration and data */
        __mmask64 mask = _mm512_cmpeq_epi8_mask(a, b);
        mask ^= (__mmask64)(i & 0xFF);  /* Make mask data-dependent */
        mask |= (__mmask64)(g_volatile_counter & 1);  /* Prevent constant mask */
        
        __m512i result = _mm512_mask_blend_epi8(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        /* Modify source data slightly for next iteration */
        src1[0] ^= (uint8_t)i;
        src2[0] ^= (uint8_t)(i + 1);
    }
}
#endif

/* ========== V32HImode (32 half-word integers) ========== */
#ifdef __AVX512BW__
__attribute__((target("avx512bw")))
static void test_v32himode(int16_t* src1, int16_t* src2, int16_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        /* Dynamic mask using comparison */
        __mmask32 mask = _mm512_cmpeq_epi16_mask(a, b);
        mask ^= (__mmask32)(i & 0xFFFF);  /* Data-dependent mask */
        mask |= (__mmask32)(g_volatile_counter & 1);
        
        __m512i result = _mm512_mask_blend_epi16(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        src1[0] ^= (int16_t)i;
        src2[0] ^= (int16_t)(i + 1);
    }
}
#endif

/* ========== V32HFmode (32 half-precision floats) ========== */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16")))
static void test_v32hfmode(_Float16* src1, _Float16* src2, _Float16* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512h a = _mm512_loadu_ph(src1);
        __m512h b = _mm512_loadu_ph(src2);
        
        /* Create dynamic mask using comparison */
        __mmask32 mask = _mm512_cmp_ph_mask(a, b, _CMP_EQ_OQ);
        mask ^= (__mmask32)(i & 0xFFFF);
        mask |= (__mmask32)(g_volatile_counter & 1);
        
        __m512h result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_ph(dst, result);
        
        /* Modify data to prevent constant folding */
        src1[0] += (_Float16)(i * 0.1f);
        src2[0] += (_Float16)(i * 0.2f);
    }
}
#endif

/* ========== V32BFmode (32 bfloat16) ========== */
#ifdef __AVX512BF16__
__attribute__((target("avx512bf16")))
static void test_v32bfmode(__bfloat16* src1, __bfloat16* src2, __bfloat16* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        /* Load as float and convert to bfloat16 */
        __m512 a_f = _mm512_loadu_ps((float*)src1);
        __m512 b_f = _mm512_loadu_ps((float*)src2);
        __m512bh a = _mm512_cvtneps_pbh(a_f);
        __m512bh b = _mm512_cvtneps_pbh(b_f);
        
        /* Create mask - blend intrinsic works on __m512bh */
        __mmask32 mask = (__mmask32)(i * 0x55555555) & 0xFFFFFFFF;
        mask ^= (__mmask32)(g_volatile_counter);
        mask |= 0xAAAAAAAA;  /* Pattern to ensure both paths taken */
        
        __m512bh result = _mm512_mask_blend_ph(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, (__m512i)result);
        
        /* Modify source data */
        ((float*)src1)[0] += i * 0.1f;
        ((float*)src2)[0] += i * 0.2f;
    }
}
#endif

/* ========== V16SImode (16 32-bit integers) ========== */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v16simode(int32_t* src1, int32_t* src2, int32_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        __mmask16 mask = _mm512_cmpeq_epi32_mask(a, b);
        mask ^= (__mmask16)(i & 0xFFFF);
        mask |= (__mmask16)(g_volatile_counter & 1);
        
        __m512i result = _mm512_mask_blend_epi32(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        src1[0] ^= i;
        src2[0] ^= (i + 1);
    }
}
#endif

/* ========== V8DImode (8 64-bit integers) ========== */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v8dimode(int64_t* src1, int64_t* src2, int64_t* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512i a = _mm512_loadu_si512((__m512i*)src1);
        __m512i b = _mm512_loadu_si512((__m512i*)src2);
        
        __mmask8 mask = _mm512_cmpeq_epi64_mask(a, b);
        mask ^= (__mmask8)(i & 0xFF);
        mask |= (__mmask8)(g_volatile_counter & 1);
        
        __m512i result = _mm512_mask_blend_epi64(mask, a, b);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        src1[0] ^= (int64_t)i;
        src2[0] ^= (int64_t)(i + 1);
    }
}
#endif

/* ========== V8DFmode (8 double-precision floats) ========== */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v8dfmode(double* src1, double* src2, double* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512d a = _mm512_loadu_pd(src1);
        __m512d b = _mm512_loadu_pd(src2);
        
        __mmask8 mask = _mm512_cmp_pd_mask(a, b, _CMP_EQ_OQ);
        mask ^= (__mmask8)(i & 0xFF);
        mask |= (__mmask8)(g_volatile_counter & 1);
        
        __m512d result = _mm512_mask_blend_pd(mask, a, b);
        _mm512_storeu_pd(dst, result);
        
        src1[0] += i * 0.1;
        src2[0] += i * 0.2;
    }
}
#endif

/* ========== V16SFmode (16 single-precision floats) ========== */
#ifdef __AVX512F__
__attribute__((target("avx512f")))
static void test_v16sfmode(float* src1, float* src2, float* dst, int iterations) {
    for (int i = 0; i < iterations; i++) {
        __m512 a = _mm512_loadu_ps(src1);
        __m512 b = _mm512_loadu_ps(src2);
        
        __mmask16 mask = _mm512_cmp_ps_mask(a, b, _CMP_EQ_OQ);
        mask ^= (__mmask16)(i & 0xFFFF);
        mask |= (__mmask16)(g_volatile_counter & 1);
        
        __m512 result = _mm512_mask_blend_ps(mask, a, b);
        _mm512_storeu_ps(dst, result);
        
        src1[0] += i * 0.1f;
        src2[0] += i * 0.2f;
    }
}
#endif

/* ========== Main Test Driver ========== */
int main() {
    const int ITERATIONS = 10;
    const int ALIGN = 64;
    
    /* Allocate aligned memory for all test cases */
    uint8_t src1_bytes[64] __attribute__((aligned(ALIGN)));
    uint8_t src2_bytes[64] __attribute__((aligned(ALIGN)));
    uint8_t dst_bytes[64] __attribute__((aligned(ALIGN)));
    
    int16_t src1_words[32] __attribute__((aligned(ALIGN)));
    int16_t src2_words[32] __attribute__((aligned(ALIGN)));
    int16_t dst_words[32] __attribute__((aligned(ALIGN)));
    
    _Float16 src1_half[32] __attribute__((aligned(ALIGN)));
    _Float16 src2_half[32] __attribute__((aligned(ALIGN)));
    _Float16 dst_half[32] __attribute__((aligned(ALIGN)));
    
    __bfloat16 src1_bf16[32] __attribute__((aligned(ALIGN)));
    __bfloat16 src2_bf16[32] __attribute__((aligned(ALIGN)));
    __bfloat16 dst_bf16[32] __attribute__((aligned(ALIGN)));
    
    int32_t src1_dwords[16] __attribute__((aligned(ALIGN)));
    int32_t src2_dwords[16] __attribute__((aligned(ALIGN)));
    int32_t dst_dwords[16] __attribute__((aligned(ALIGN)));
    
    int64_t src1_qwords[8] __attribute__((aligned(ALIGN)));
    int64_t src2_qwords[8] __attribute__((aligned(ALIGN)));
    int64_t dst_qwords[8] __attribute__((aligned(ALIGN)));
    
    double src1_double[8] __attribute__((aligned(ALIGN)));
    double src2_double[8] __attribute__((aligned(ALIGN)));
    double dst_double[8] __attribute__((aligned(ALIGN)));
    
    float src1_float[16] __attribute__((aligned(ALIGN)));
    float src2_float[16] __attribute__((aligned(ALIGN)));
    float dst_float[16] __attribute__((aligned(ALIGN)));
    
    /* Initialize with distinct patterns */
    for (int i = 0; i < 64; i++) {
        src1_bytes[i] = i;
        src2_bytes[i] = i + 64;
        dst_bytes[i] = 0;
    }
    
    for (int i = 0; i < 32; i++) {
        src1_words[i] = i;
        src2_words[i] = i + 32;
        dst_words[i] = 0;
        
        src1_half[i] = (_Float16)(i * 0.5f);
        src2_half[i] = (_Float16)(i * 0.7f);
        dst_half[i] = (_Float16)0.0f;
        
        src1_bf16[i] = (__bfloat16)(i * 0.3f);
        src2_bf16[i] = (__bfloat16)(i * 0.6f);
        dst_bf16[i] = (__bfloat16)0.0f;
    }
    
    for (int i = 0; i < 16; i++) {
        src1_dwords[i] = i * 2;
        src2_dwords[i] = i * 3;
        dst_dwords[i] = 0;
        
        src1_float[i] = i * 0.25f;
        src2_float[i] = i * 0.35f;
        dst_float[i] = 0.0f;
    }
    
    for (int i = 0; i < 8; i++) {
        src1_qwords[i] = i * 4LL;
        src2_qwords[i] = i * 5LL;
        dst_qwords[i] = 0;
        
        src1_double[i] = i * 0.125;
        src2_double[i] = i * 0.175;
        dst_double[i] = 0.0;
    }
    
    /* Execute all test functions */
    #ifdef __AVX512BW__
    test_v64qimode(src1_bytes, src2_bytes, dst_bytes, ITERATIONS);
    test_v32himode(src1_words, src2_words, dst_words, ITERATIONS);
    #endif
    
    #ifdef __AVX512FP16__
    test_v32hfmode(src1_half, src2_half, dst_half, ITERATIONS);
    #endif
    
    #ifdef __AVX512BF16__
    test_v32bfmode(src1_bf16, src2_bf16, dst_bf16, ITERATIONS);
    #endif
    
    #ifdef __AVX512F__
    test_v16simode(src1_dwords, src2_dwords, dst_dwords, ITERATIONS);
    test_v8dimode(src1_qwords, src2_qwords, dst_qwords, ITERATIONS);
    test_v8dfmode(src1_double, src2_double, dst_double, ITERATIONS);
    test_v16sfmode(src1_float, src2_float, dst_float, ITERATIONS);
    #endif
    
    /* Compute checksum to prevent dead code elimination */
    uint64_t checksum = 0;
    for (int i = 0; i < 64; i++) checksum += dst_bytes[i];
    for (int i = 0; i < 32; i++) checksum += dst_words[i];
    for (int i = 0; i < 32; i++) checksum += (uint16_t)dst_half[i];
    for (int i = 0; i < 32; i++) checksum += (uint16_t)dst_bf16[i];
    for (int i = 0; i < 16; i++) checksum += dst_dwords[i];
    for (int i = 0; i < 8; i++) checksum += dst_qwords[i];
    for (int i = 0; i < 8; i++) checksum += (uint64_t)dst_double[i];
    for (int i = 0; i < 16; i++) checksum += (uint32_t)dst_float[i];
    
    printf("Blend test completed. Checksum: %lu\n", checksum);
    return 0;
}

#ifdef __cplusplus
}
#endif
