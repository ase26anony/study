/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targeting uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QI - 64 x 8-bit integers */
static int test_v64qi_blend(void) {
    __attribute__((aligned(64))) int8_t src1[64];
    __attribute__((aligned(64))) int8_t src2[64];
    __attribute__((aligned(64))) int8_t dst[64];
    volatile __attribute__((aligned(64))) int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 64 - i;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select from v1 where i % 2 == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 2 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend using intrinsic - should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Also store to regular array */
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V32HI - 32 x 16-bit integers */
static int test_v32hi_blend(void) {
    __attribute__((aligned(64))) int16_t src1[32];
    __attribute__((aligned(64))) int16_t src2[32];
    __attribute__((aligned(64))) int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(32);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcasted scalar */
    __m512i broadcast = _mm512_set1_epi16(100);
    result = _mm512_mask_blend_epi16(mask, result, broadcast);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V32HF - 32 x half-precision floats */
static int test_v32hf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* Store as uint16_t for _Float16 */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    __attribute__((aligned(64))) uint16_t dst_data[32];
    
    for (int i = 0; i < 32; i++) {
        src1_data[i] = i * 0x3C00;  /* 1.0 pattern */
        src2_data[i] = i * 0x4000;  /* 2.0 pattern */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1_data);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2_data);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* Blend using half-precision intrinsic - should trigger gen_avx512bw_blendmv32hf */
    __m512i result = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst_data, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst_data[i];
    }
    
    return sum;
}

/* V32BF - 32 x bfloat16 floats */
static int test_v32bf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1[32];
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (i + 1) << 8;  /* Simple bfloat16 pattern */
        src2[i] = (i + 32) << 8;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask for blending */
    __mmask32 mask = 0x55555555;  /* 0101... pattern */
    
    /* For bfloat16, use epi16 blend on integer representation
     * Should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V16SI - 16 x 32-bit integers */
static int test_v16si_blend(void) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10;
        src2[i] = i * 20;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(80);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i add_result = _mm512_add_epi32(v1, _mm512_set1_epi32(5));
    result = _mm512_mask_blend_epi32(mask, result, add_result);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DI - 8 x 64-bit integers */
static long long test_v8di_blend(void) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 100LL;
        src2[i] = i * 200LL;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask */
    __mmask8 mask = 0xAA;  /* 10101010 */
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DF - 8 x double-precision floats */
static double test_v8df_blend(void) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Blend with arithmetic operation */
    __m512d mul_result = _mm512_mul_pd(v1, _mm512_set1_pd(2.0));
    result = _mm512_mask_blend_pd(mask, result, mul_result);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V16SF - 16 x single-precision floats */
static float test_v16sf_blend(void) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Use in loop with argc dependency */
    volatile int loop_count = 3;
    for (int i = 0; i < loop_count; i++) {
        __m512 add_result = _mm512_add_ps(v1, _mm512_set1_ps(i * 1.0f));
        result = _mm512_mask_blend_ps(mask, result, add_result);
    }
    
    _mm512_store_ps(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main(int argc, char **argv) {
    int total_result = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    printf("Testing AVX-512 blend intrinsics...\n");
    
    /* Call all test functions with varied control flow */
    if (argc > 1) {
        total_result += test_v64qi_blend();
        total_result += test_v32hi_blend();
        total_result += test_v32hf_blend();
        total_result += test_v32bf_blend();
    }
    
    total_result += test_v16si_blend();
    total_result += (int)test_v8di_blend();
    
    /* Use results in floating point computations */
    float float_sum = test_v16sf_blend();
    double double_sum = test_v8df_blend();
    
    total_result += (int)float_sum;
    total_result += (int)double_sum;
    
    /* Additional loop with blend operations */
    for (int i = 0; i < (argc > 1 ? 2 : 1); i++) {
        total_result += test_v64qi_blend() % 256;
    }
    
    printf("Final checksum: %d\n", total_result);
    
#else
    printf("AVX-512BW not supported on this platform\n");
#endif
#else
    printf("AVX-512F not supported on this platform\n");
#endif
    
    return total_result != 0 ? 0 : 1;
}
