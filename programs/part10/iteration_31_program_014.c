/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines in i386-expand.cc: 4303-4326
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw test_avx512_blend.c -o test_avx512_blend
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QI mode - 64 x 8-bit integers */
static int test_v64qi_blend(void) {
    __attribute__((aligned(64))) uint8_t src1[64];
    __attribute__((aligned(64))) uint8_t src2[64];
    __attribute__((aligned(64))) uint8_t dst[64];
    volatile __attribute__((aligned(64))) uint8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 255 - i;
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask: select from v1 where i % 2 == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 2 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Use result in computation */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return (int)(sum & 0x7FFFFFFF);
}

/* V32HI mode - 32 x 16-bit integers */
static int test_v32hi_blend(void) {
    __attribute__((aligned(64))) uint16_t src1[32];
    __attribute__((aligned(64))) uint16_t src2[32];
    volatile __attribute__((aligned(64))) uint16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = i * 200;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(1600);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend with broadcasted scalar */
    __m512i broadcast = _mm512_set1_epi16(9999);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, broadcast, v2);
    
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}

/* V32HF mode - 32 x half-precision floats */
static int test_v32hf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* Half floats as uint16_t */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    volatile __attribute__((aligned(64))) uint16_t volatile_dst[32];
    
    /* Initialize with half-float pattern */
    for (int i = 0; i < 32; i++) {
        /* Simple pattern: 1.0, 2.0, 3.0, ... as half floats */
        src1_data[i] = 0x3C00 + (i & 0x7);  /* 1.0 + small increment */
        src2_data[i] = 0x4000 + (i & 0x7);  /* 2.0 + small increment */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Cast to __m512h for blend operation */
    __m512h h1 = _mm512_castsi512_ph(v1);
    __m512h h2 = _mm512_castsi512_ph(v2);
    
    /* Create mask: select where i % 3 == 0 */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, h2, h1);
    
    /* Store back as integers */
    __m512i result_i = _mm512_castph_si512(result);
    _mm512_store_si512((__m512i*)volatile_dst, result_i);
    
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}

/* V32BF mode - 32 x bfloat16 floats */
static int test_v32bf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* Bfloat16 as uint16_t */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    volatile __attribute__((aligned(64))) uint16_t volatile_dst[32];
    
    /* Initialize with bfloat16 pattern */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 values */
        src1_data[i] = 0x3F80 + (i & 0x3);  /* ~1.0 as bfloat16 */
        src2_data[i] = 0x4000 + (i & 0x3);  /* ~2.0 as bfloat16 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* For bfloat16, we use the same integer blend as V32HI */
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* This should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}

#endif /* __AVX512BW__ */

/* V16SI mode - 16 x 32-bit integers */
static int test_v16si_blend(void) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    volatile __attribute__((aligned(64))) int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(500));
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v2, added);
    
    _mm512_store_epi32((void*)volatile_dst, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}

/* V8DI mode - 8 x 64-bit integers */
static int test_v8di_blend(void) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    volatile __attribute__((aligned(64))) int64_t volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask: select where src1 > 30000 */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (src1[i] > 30000) {
            mask |= (1 << i);
        }
    }
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_epi64((void*)volatile_dst, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}

/* V8DF mode - 8 x double-precision floats */
static int test_v8df_blend(void) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    volatile __attribute__((aligned(64))) double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1.0 * (i + 1);
        src2[i] = 2.0 * (i + 1);
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating-point comparison */
    __m512d cmp_val = _mm512_set1_pd(4.5);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with multiplied result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v2, multiplied);
    
    _mm512_store_pd(volatile_dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum * 1000.0);
}

/* V16SF mode - 16 x single-precision floats */
static int test_v16sf_blend(void) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    volatile __attribute__((aligned(64))) float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = 0.1f * (i + 1);
        src2[i] = 0.2f * (i + 1);
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(1.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend inside a loop to prevent optimization */
    __m512 result = v1;
    volatile int loop_count = 3;
    
    for (int i = 0; i < loop_count; i++) {
        /* This should trigger gen_avx512f_blendmv16sf */
        result = _mm512_mask_blend_ps(mask, result, v2);
        /* Modify mask slightly each iteration */
        mask = ~mask;
    }
    
    _mm512_store_ps(volatile_dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum * 1000.0f);
}

#endif /* __AVX512F__ */

int main(int argc, char *argv[]) {
    int total_hash = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported, testing blend patterns...\n");
    
    /* Test AVX-512F patterns */
    total_hash ^= test_v16si_blend();
    total_hash ^= test_v8di_blend();
    total_hash ^= test_v8df_blend();
    total_hash ^= test_v16sf_blend();
    
#ifdef __AVX512BW__
    printf("AVX-512BW supported, testing byte/word blend patterns...\n");
    
    /* Test AVX-512BW patterns */
    total_hash ^= test_v64qi_blend();
    total_hash ^= test_v32hi_blend();
    total_hash ^= test_v32hf_blend();
    total_hash ^= test_v32bf_blend();
    
#else
    printf("AVX-512BW not supported, skipping byte/word blend tests\n");
#endif /* __AVX512BW__ */
    
    printf("All AVX-512 blend tests completed. Hash: %d\n", total_hash);
    
#else
    printf("AVX-512 not supported on this platform\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
#endif /* __AVX512F__ */
    
    /* Use argc to create runtime dependency */
    if (argc > 1) {
        total_hash += atoi(argv[1]);
    }
    
    return total_hash & 0xFF;
}
