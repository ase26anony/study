/* AVX-512 blend coverage test for i386-expand.cc */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

/* Compile-time feature checks */
#if !defined(__AVX512F__)
#error "AVX-512F required for this test"
#endif

#if !defined(__AVX512BW__)
#warning "AVX-512BW required for full coverage"
#endif

#if !defined(__AVX512FP16__)
#warning "AVX512-FP16 required for V32HFmode coverage"
#endif

#if !defined(__AVX512BF16__)
#warning "AVX512-BF16 required for V32BFmode coverage"
#endif

/* Function attributes for specific ISA requirements */
#ifdef __cplusplus
extern "C" {
#endif

/* V64QImode and V32HImode require AVX512BW */
__attribute__((target("avx512bw,avx512f")))
static void test_v64qi_v32hi(void) {
    /* V64QImode: 64 bytes */
    uint8_t src1_bytes[64];
    uint8_t src2_bytes[64];
    uint8_t dst_bytes[64];
    
    for (int i = 0; i < 64; i++) {
        src1_bytes[i] = i;
        src2_bytes[i] = i + 64;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1_bytes);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2_bytes);
    
    /* Dynamic mask based on element index parity */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst_bytes, result);
    
    /* V32HImode: 32 half-words */
    uint16_t src1_words[32];
    uint16_t src2_words[32];
    uint16_t dst_words[32];
    
    for (int i = 0; i < 32; i++) {
        src1_words[i] = i * 2;
        src2_words[i] = i * 2 + 1;
    }
    
    v1 = _mm512_loadu_si512((const __m512i*)src1_words);
    v2 = _mm512_loadu_si512((const __m512i*)src2_words);
    
    /* Dynamic mask */
    __mmask32 mask32 = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 4 == 0) {
            mask32 |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    result = _mm512_mask_blend_epi16(mask32, v1, v2);
    _mm512_storeu_si512((__m512i*)dst_words, result);
}

/* V32HFmode requires AVX512-FP16 */
#ifdef __AVX512FP16__
__attribute__((target("avx512fp16,avx512f")))
static void test_v32hf(void) {
    _Float16 src1_half[32];
    _Float16 src2_half[32];
    _Float16 dst_half[32];
    
    for (int i = 0; i < 32; i++) {
        src1_half[i] = (_Float16)(i * 0.5f);
        src2_half[i] = (_Float16)(i * 1.5f);
    }
    
    __m512h v1 = _mm512_loadu_ph(src1_half);
    __m512h v2 = _mm512_loadu_ph(src2_half);
    
    /* Dynamic mask */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 5 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_storeu_ph(dst_half, result);
}
#endif

/* V32BFmode requires AVX512-BF16 */
#ifdef __AVX512BF16__
__attribute__((target("avx512bf16,avx512f")))
static void test_v32bf(void) {
    /* Use __bfloat16 type if available, otherwise use uint16_t */
    #ifdef __bf16
    __bf16 src1_bf[32];
    __bf16 src2_bf[32];
    __bf16 dst_bf[32];
    #else
    uint16_t src1_bf[32];
    uint16_t src2_bf[32];
    uint16_t dst_bf[32];
    #endif
    
    for (int i = 0; i < 32; i++) {
        #ifdef __bf16
        src1_bf[i] = __bfloat162bfloat16((float)(i * 0.25f));
        src2_bf[i] = __bfloat162bfloat16((float)(i * 0.75f));
        #else
        src1_bf[i] = i * 100;
        src2_bf[i] = i * 200;
        #endif
    }
    
    __m512bh v1 = _mm512_loadu_si512((const __m512i*)src1_bf);
    __m512bh v2 = _mm512_loadu_si512((const __m512i*)src2_bf);
    
    /* Dynamic mask */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 6 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512bh result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_storeu_si512((__m512i*)dst_bf, result);
}
#endif

/* V16SImode, V8DImode, V8DFmode, V16SFmode require AVX512F */
__attribute__((target("avx512f")))
static void test_avx512f_blends(void) {
    /* V16SImode: 16 signed ints */
    int32_t src1_int[16];
    int32_t src2_int[16];
    int32_t dst_int[16];
    
    for (int i = 0; i < 16; i++) {
        src1_int[i] = i * 10;
        src2_int[i] = i * 20;
    }
    
    __m512i v1_i = _mm512_loadu_si512((const __m512i*)src1_int);
    __m512i v2_i = _mm512_loadu_si512((const __m512i*)src2_int);
    
    /* Dynamic mask from comparison */
    __mmask16 mask16 = _mm512_cmpeq_epi32_mask(v1_i, v2_i);
    mask16 = ~mask16; /* Ensure non-constant mask */
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result_i = _mm512_mask_blend_epi32(mask16, v1_i, v2_i);
    _mm512_storeu_si512((__m512i*)dst_int, result_i);
    
    /* V8DImode: 8 signed long longs */
    int64_t src1_long[8];
    int64_t src2_long[8];
    int64_t dst_long[8];
    
    for (int i = 0; i < 8; i++) {
        src1_long[i] = i * 100LL;
        src2_long[i] = i * 200LL;
    }
    
    v1_i = _mm512_loadu_si512((const __m512i*)src1_long);
    v2_i = _mm512_loadu_si512((const __m512i*)src2_long);
    
    /* Dynamic mask */
    __mmask8 mask8 = 0;
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) {
            mask8 |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512f_blendmv8di */
    result_i = _mm512_mask_blend_epi64(mask8, v1_i, v2_i);
    _mm512_storeu_si512((__m512i*)dst_long, result_i);
    
    /* V8DFmode: 8 doubles */
    double src1_double[8];
    double src2_double[8];
    double dst_double[8];
    
    for (int i = 0; i < 8; i++) {
        src1_double[i] = i * 1.1;
        src2_double[i] = i * 2.2;
    }
    
    __m512d v1_d = _mm512_loadu_pd(src1_double);
    __m512d v2_d = _mm512_loadu_pd(src2_double);
    
    /* Dynamic mask from comparison */
    __mmask8 mask_double = _mm512_cmp_pd_mask(v1_d, v2_d, _CMP_EQ_OQ);
    mask_double = mask_double ^ 0xFF; /* XOR to make non-constant */
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result_d = _mm512_mask_blend_pd(mask_double, v1_d, v2_d);
    _mm512_storeu_pd(dst_double, result_d);
    
    /* V16SFmode: 16 floats */
    float src1_float[16];
    float src2_float[16];
    float dst_float[16];
    
    for (int i = 0; i < 16; i++) {
        src1_float[i] = i * 0.5f;
        src2_float[i] = i * 1.5f;
    }
    
    __m512 v1_f = _mm512_loadu_ps(src1_float);
    __m512 v2_f = _mm512_loadu_ps(src2_float);
    
    /* Dynamic mask from comparison */
    __mmask16 mask_float = _mm512_cmp_ps_mask(v1_f, v2_f, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result_f = _mm512_mask_blend_ps(mask_float, v1_f, v2_f);
    _mm512_storeu_ps(dst_float, result_f);
}

#ifdef __cplusplus
}
#endif

int main(void) {
    printf("Testing AVX-512 blend instruction expansion...\n");
    
    /* Test all blend modes in a loop to prevent optimization */
    for (int iteration = 0; iteration < 3; iteration++) {
        test_v64qi_v32hi();
        
        #ifdef __AVX512FP16__
        test_v32hf();
        #endif
        
        #ifdef __AVX512BF16__
        test_v32bf();
        #endif
        
        test_avx512f_blends();
    }
    
    printf("All AVX-512 blend tests completed.\n");
    printf("If compiled with coverage flags, this should exercise:\n");
    printf("  gen_avx512bw_blendmv64qi\n");
    printf("  gen_avx512bw_blendmv32hi\n");
    printf("  gen_avx512bw_blendmv32hf\n");
    printf("  gen_avx512bw_blendmv32bf\n");
    printf("  gen_avx512f_blendmv16si\n");
    printf("  gen_avx512f_blendmv8di\n");
    printf("  gen_avx512f_blendmv8df\n");
    printf("  gen_avx512f_blendmv16sf\n");
    
    return 0;
}
