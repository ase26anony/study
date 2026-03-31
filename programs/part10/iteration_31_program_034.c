/* test_avx512_blend.c - AVX-512 blend intrinsics test for GCC coverage */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* Helper for volatile memory barriers */
#define COMPILER_BARRIER() __asm__ volatile("" : : : "memory")

/* Aligned allocation macro */
#define ALIGN_64 __attribute__((aligned(64)))

/* Test function for V64QI mode (64 x 8-bit integers) */
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile ALIGN_64 int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask: select from v1 where i % 2 == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 2 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Use _mm512_mask_blend_epi8 intrinsic */
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Also store to regular array */
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Use result in computation */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? argc : 10;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_load_si512((__m512i*)src1);
        result = _mm512_mask_blend_epi8(mask, temp, v2);
        COMPILER_BARRIER();
    }
    
    return sum;
}

/* Test function for V32HI mode (32 x 16-bit integers) */
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Generate mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(32);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend with mask */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(100);
    result = _mm512_mask_blend_epi16(mask, result, broadcast);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Test function for V32HF mode (32 x half-precision floats) */
static int test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t src1_data[32];  /* Store as uint16_t for _Float16 */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t dst_data[32];
    
    /* Initialize with simple pattern */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = i * 0x100;  /* Simple pattern */
        src2_data[i] = i * 0x200;
    }
    
    /* Load as integers, treat as _Float16 */
    __m512i v1 = _mm512_load_si512((__m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((__m512i*)src2_data);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* Use _mm512_mask_blend_ph intrinsic */
    __m512i result = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst_data, result);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst_data[i];
    }
    
    return sum;
}

/* Test function for V32BF mode (32 x bfloat16) */
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1[32];  /* bfloat16 stored as uint16_t */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (i << 8) | (i & 0xFF);  /* Create bfloat16-like pattern */
        src2[i] = ((31 - i) << 8) | ((31 - i) & 0xFF);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison on integer representation */
    __m512i threshold = _mm512_set1_epi16(0x4000);  /* 0.5 in bfloat16 */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, threshold);
    
    /* Blend using epi16 intrinsic (same as for bfloat16) */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Test function for V16SI mode (16 x 32-bit integers) */
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = i * 20;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Generate mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(80);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend with arithmetic operation result */
    __m512i add_result = _mm512_add_epi32(v1, _mm512_set1_epi32(5));
    __m512i result = _mm512_mask_blend_epi32(mask, v2, add_result);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Test function for V8DI mode (8 x 64-bit integers) */
static long long test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = i * 200LL;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask: select where src1[i] > 300 */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (src1[i] > 300) {
            mask |= (1 << i);
        }
    }
    
    /* Blend with mask */
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Test function for V8DF mode (8 x double-precision floats) */
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Generate mask using floating-point comparison */
    __m512d cmp_val = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with arithmetic result */
    __m512d mul_result = _mm512_mul_pd(v1, _mm512_set1_pd(2.0));
    __m512d result = _mm512_mask_blend_pd(mask, v2, mul_result);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* Test function for V16SF mode (16 x single-precision floats) */
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Generate mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend with mask */
    __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Store to volatile */
    _mm512_store_ps((float*)volatile_dst, result);
    
    /* Also store to regular array */
    _mm512_store_ps(dst, result);
    
    /* Use in loop with argc dependency */
    int loop_count = (argc > 2) ? argc : 5;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512 temp = _mm512_load_ps(src1);
        result = _mm512_mask_blend_ps(mask, temp, v2);
        COMPILER_BARRIER();
    }
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main(int argc, char **argv) {
    int total_checksum = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("AVX-512 F and BW extensions detected. Running blend tests...\n");
    
    /* Run all blend tests */
    total_checksum += test_v64qi_blend(argc);
    total_checksum += test_v32hi_blend(argc);
    total_checksum += test_v32hf_blend(argc);
    total_checksum += test_v32bf_blend(argc);
    total_checksum += test_v16si_blend(argc);
    
    long long di_sum = test_v8di_blend(argc);
    total_checksum += (int)(di_sum & 0xFFFFFFFF) + (int)(di_sum >> 32);
    
    double df_sum = test_v8df_blend(argc);
    total_checksum += (int)df_sum;
    
    float sf_sum = test_v16sf_blend(argc);
    total_checksum += (int)sf_sum;
    
    printf("All AVX-512 blend tests completed. Checksum: %d\n", total_checksum);
    
#else
    printf("AVX-512 BW extension not available. Skipping byte/word blend tests.\n");
#endif /* __AVX512BW__ */
#else
    printf("AVX-512 F extension not available. Skipping all AVX-512 blend tests.\n");
#endif /* __AVX512F__ */
    
    return total_checksum != 0 ? 0 : 1;
}
