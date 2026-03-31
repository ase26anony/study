/* test_avx512_blend.c - AVX-512 blend intrinsics coverage test */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Alignment and volatile for preventing optimization */
#define ALIGN_64 __attribute__((aligned(64)))
#define FORCE_USE(x) __asm__ volatile("" : : "r"(x) : "memory")

/* Global volatile to prevent constant folding */
static volatile int g_volatile_counter = 1;

#ifdef __AVX512F__

/* V16SF - 16 single-precision floats */
static float test_v16sf_blend(int iterations) {
    ALIGN_64 float src1[16] = {
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    };
    ALIGN_64 float src2[16] = {
        16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f
    };
    ALIGN_64 volatile float result[16];
    
    __m512 vec1 = _mm512_load_ps(src1);
    __m512 vec2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(vec1, vec2, _CMP_GT_OQ);
    
    /* Blend based on mask - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, vec2, vec1);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_ps((void*)result, blended);
    
    /* Use result in computation */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Additional blend with broadcast scalar */
    __m512 scalar_vec = _mm512_set1_ps(100.0f);
    __m512 blended2 = _mm512_mask_blend_ps(0xAAAA, scalar_vec, blended);
    
    /* Blend with arithmetic result */
    __m512 add_result = _mm512_add_ps(blended, blended2);
    __m512 final_blend = _mm512_mask_blend_ps(0x5555, add_result, blended);
    
    /* Force use of result */
    ALIGN_64 float final_store[16];
    _mm512_store_ps(final_store, final_blend);
    FORCE_USE(final_store);
    
    return sum + final_store[0];
}

/* V8DF - 8 double-precision floats */
static double test_v8df_blend(int iterations) {
    ALIGN_64 double src1[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    ALIGN_64 double src2[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    ALIGN_64 volatile double result[8];
    
    __m512d vec1 = _mm512_load_pd(src1);
    __m512d vec2 = _mm512_load_pd(src2);
    
    /* Create mask - should trigger gen_avx512f_blendmv8df */
    __mmask8 mask = _mm512_cmp_pd_mask(vec1, vec2, _CMP_GT_OQ);
    __m512d blended = _mm512_mask_blend_pd(mask, vec2, vec1);
    
    _mm512_store_pd((void*)result, blended);
    
    /* Use in loop with varying mask */
    double sum = 0.0;
    for (int i = 0; i < iterations; i++) {
        __mmask8 dynamic_mask = (i % 2) ? 0xFF : 0xAA;
        __m512d temp = _mm512_mask_blend_pd(dynamic_mask, 
                                           _mm512_set1_pd(i * 0.5), 
                                           blended);
        ALIGN_64 double temp_store[8];
        _mm512_store_pd(temp_store, temp);
        sum += temp_store[i % 8];
    }
    
    return sum;
}

/* V16SI - 16 32-bit integers */
static int32_t test_v16si_blend(int iterations) {
    ALIGN_64 int32_t src1[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    ALIGN_64 int32_t src2[16] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    ALIGN_64 volatile int32_t result[16];
    
    __m512i vec1 = _mm512_load_epi32(src1);
    __m512i vec2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison - should trigger gen_avx512f_blendmv16si */
    __mmask16 mask = _mm512_cmp_epi32_mask(vec1, vec2, _MM_CMPINT_GT);
    __m512i blended = _mm512_mask_blend_epi32(mask, vec2, vec1);
    
    _mm512_store_epi32((void*)result, blended);
    
    /* Blend with arithmetic operation */
    __m512i add_result = _mm512_add_epi32(blended, _mm512_set1_epi32(100));
    __m512i final_blend = _mm512_mask_blend_epi32(0xAAAA, add_result, blended);
    
    /* Reduction */
    int32_t sum = 0;
    ALIGN_64 int32_t final_store[16];
    _mm512_store_epi32(final_store, final_blend);
    for (int i = 0; i < 16; i++) {
        sum += final_store[i];
    }
    
    return sum;
}

/* V8DI - 8 64-bit integers */
static int64_t test_v8di_blend(int iterations) {
    ALIGN_64 int64_t src1[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ALIGN_64 int64_t src2[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    ALIGN_64 volatile int64_t result[8];
    
    __m512i vec1 = _mm512_load_epi64(src1);
    __m512i vec2 = _mm512_load_epi64(src2);
    
    /* Should trigger gen_avx512f_blendmv8di */
    __mmask8 mask = _mm512_cmp_epi64_mask(vec1, vec2, _MM_CMPINT_GT);
    __m512i blended = _mm512_mask_blend_epi64(mask, vec2, vec1);
    
    _mm512_store_epi64((void*)result, blended);
    
    /* Complex blend pattern */
    int64_t sum = 0;
    for (int i = 0; i < iterations; i++) {
        __mmask8 dynamic_mask = (__mmask8)(g_volatile_counter + i);
        __m512i temp = _mm512_mask_blend_epi64(dynamic_mask & 0xFF,
                                              _mm512_set1_epi64(i),
                                              blended);
        ALIGN_64 int64_t temp_store[8];
        _mm512_store_epi64(temp_store, temp);
        sum += temp_store[i % 8];
    }
    
    return sum;
}

#endif /* __AVX512F__ */

#ifdef __AVX512BW__

/* V64QI - 64 8-bit integers */
static int8_t test_v64qi_blend(int iterations) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 volatile int8_t result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i);
        src2[i] = (int8_t)(63 - i);
    }
    
    __m512i vec1 = _mm512_load_si512(src1);
    __m512i vec2 = _mm512_load_si512(src2);
    
    /* Create mask - should trigger gen_avx512bw_blendmv64qi */
    __mmask64 mask = _mm512_cmp_epi8_mask(vec1, vec2, _MM_CMPINT_GT);
    __m512i blended = _mm512_mask_blend_epi8(mask, vec2, vec1);
    
    _mm512_store_si512((void*)result, blended);
    
    /* Multiple blends with different sources */
    __m512i broadcast = _mm512_set1_epi8(42);
    __m512i blended2 = _mm512_mask_blend_epi8(0xAAAAAAAAAAAAAAAA, broadcast, blended);
    
    /* Reduction */
    int8_t sum = 0;
    ALIGN_64 int8_t final_store[64];
    _mm512_store_si512(final_store, blended2);
    for (int i = 0; i < 64; i++) {
        sum += final_store[i];
    }
    
    return sum;
}

/* V32HI - 32 16-bit integers */
static int16_t test_v32hi_blend(int iterations) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 volatile int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 2);
        src2[i] = (int16_t)(1000 - i * 3);
    }
    
    __m512i vec1 = _mm512_load_si512(src1);
    __m512i vec2 = _mm512_load_si512(src2);
    
    /* Should trigger gen_avx512bw_blendmv32hi */
    __mmask32 mask = _mm512_cmp_epi16_mask(vec1, vec2, _MM_CMPINT_GT);
    __m512i blended = _mm512_mask_blend_epi16(mask, vec2, vec1);
    
    _mm512_store_si512((void*)result, blended);
    
    /* Blend in loop with arithmetic */
    int16_t sum = 0;
    for (int i = 0; i < iterations; i++) {
        __mmask32 dynamic_mask = (__mmask32)((i * 0x55555555) & 0xFFFFFFFF);
        __m512i add_result = _mm512_add_epi16(blended, _mm512_set1_epi16(i));
        __m512i temp = _mm512_mask_blend_epi16(dynamic_mask, add_result, blended);
        
        ALIGN_64 int16_t temp_store[32];
        _mm512_store_si512(temp_store, temp);
        sum += temp_store[i % 32];
    }
    
    return sum;
}

/* V32HF - 32 half-precision floats */
static __fp16 test_v32hf_blend(int iterations) {
    ALIGN_64 __fp16 src1[32];
    ALIGN_64 __fp16 src2[32];
    ALIGN_64 volatile __fp16 result[32];
    
    /* Initialize half-precision values */
    for (int i = 0; i < 32; i++) {
        src1[i] = (__fp16)(i * 0.5f);
        src2[i] = (__fp16)(10.0f - i * 0.3f);
    }
    
    __m512h vec1 = _mm512_load_ph(src1);
    __m512h vec2 = _mm512_load_ph(src2);
    
    /* Should trigger gen_avx512bw_blendmv32hf */
    __mmask32 mask = _mm512_cmp_ph_mask(vec1, vec2, _CMP_GT_OQ);
    __m512h blended = _mm512_mask_blend_ph(mask, vec2, vec1);
    
    _mm512_store_ph((void*)result, blended);
    
    /* Additional operations */
    __m512h scalar = _mm512_set1_ph((__fp16)5.0f);
    __m512h blended2 = _mm512_mask_blend_ph(0xAAAAAAAA, scalar, blended);
    
    /* Compute sum */
    __fp16 sum = 0;
    ALIGN_64 __fp16 final_store[32];
    _mm512_store_ph(final_store, blended2);
    for (int i = 0; i < 32; i++) {
        sum += final_store[i];
    }
    
    return sum;
}

/* V32BF - 32 bfloat16 values */
static uint16_t test_v32bf_blend(int iterations) {
    ALIGN_64 uint16_t src1[32];  /* bfloat16 as uint16_t */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 volatile uint16_t result[32];
    
    /* Initialize bfloat16 patterns (simple representation) */
    for (int i = 0; i < 32; i++) {
        /* Create simple bfloat16 pattern: sign=0, exponent=127, mantissa=i */
        src1[i] = (uint16_t)((0x7F << 7) | (i & 0x7F));
        src2[i] = (uint16_t)((0x7F << 7) | ((31 - i) & 0x7F));
    }
    
    /* Load as integers for blending */
    __m512i vec1 = _mm512_load_si512(src1);
    __m512i vec2 = _mm512_load_si512(src2);
    
    /* For bfloat16, we use epi16 blend on the integer representation */
    /* Should trigger gen_avx512bw_blendmv32bf */
    __mmask32 mask = 0xAAAAAAAA;  /* Alternating pattern */
    __m512i blended = _mm512_mask_blend_epi16(mask, vec2, vec1);
    
    _mm512_store_si512((void*)result, blended);
    
    /* Compute checksum */
    uint16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */

/* Main driver function */
int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations <= 0) iterations = 10;
    
    printf("Running AVX-512 blend tests with %d iterations\n", iterations);
    
    /* Accumulate results to ensure computation isn't optimized away */
    uint64_t total_hash = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported\n");
    
    /* V16SF test */
    float v16sf_result = test_v16sf_blend(iterations);
    total_hash += (uint64_t)(v16sf_result * 1000);
    printf("  V16SF blend result: %f\n", v16sf_result);
    
    /* V8DF test */
    double v8df_result = test_v8df_blend(iterations);
    total_hash += (uint64_t)(v8df_result * 1000);
    printf("  V8DF blend result: %f\n", v8df_result);
    
    /* V16SI test */
    int32_t v16si_result = test_v16si_blend(iterations);
    total_hash += (uint64_t)v16si_result;
    printf("  V16SI blend result: %d\n", v16si_result);
    
    /* V8DI test */
    int64_t v8di_result = test_v8di_blend(iterations);
    total_hash += (uint64_t)v8di_result;
    printf("  V8DI blend result: %ld\n", (long)v8di_result);
#else
    printf("AVX-512F not supported\n");
#endif
    
#ifdef __AVX512BW__
    printf("AVX-512BW supported\n");
    
    /* V64QI test */
    int8_t v64qi_result = test_v64qi_blend(iterations);
    total_hash += (uint64_t)v64qi_result;
    printf("  V64QI blend result: %d\n", v64qi_result);
    
    /* V32HI test */
    int16_t v32hi_result = test_v32hi_blend(iterations);
    total_hash += (uint64_t)v32hi_result;
    printf("  V32HI blend result: %d\n", v32hi_result);
    
    /* V32HF test */
    __fp16 v32hf_result = test_v32hf_blend(iterations);
    total_hash += (uint64_t)(v32hf_result * 1000);
    printf("  V32HF blend result: %f\n", (float)v32hf_result);
    
    /* V32BF test */
    uint16_t v32bf_result = test_v32bf_blend(iterations);
    total_hash += (uint64_t)v32bf_result;
    printf("  V32BF blend result: %u\n", v32bf_result);
#else
    printf("AVX-512BW not supported\n");
#endif
    
    printf("Total hash: %lu\n", (unsigned long)total_hash);
    
    /* Use result to affect return value */
    return (total_hash > 0) ? 0 : 1;
}
