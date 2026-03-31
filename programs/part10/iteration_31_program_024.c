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

/* V64QImode - 64-byte integers */
static int test_v64qi_blend(void) {
    __attribute__((aligned(64))) uint8_t src1[64];
    __attribute__((aligned(64))) uint8_t src2[64];
    __attribute__((aligned(64))) uint8_t dst[64];
    volatile __attribute__((aligned(64))) uint8_t volatile_dst[64];
    
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
    
    /* This should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Also use in computation */
    __m512i sum = _mm512_add_epi8(result, _mm512_set1_epi8(1));
    _mm512_store_si512((__m512i*)dst, sum);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += dst[i];
    }
    
    __asm__ volatile("" : : "r"(checksum) : "memory");
    return checksum;
}

/* V32HImode - 32 half-word integers */
static int test_v32hi_blend(void) {
    __attribute__((aligned(64))) uint16_t src1[32];
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 2;
        src2[i] = i * 3;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(32);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* This should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcasted scalar */
    __m512i broadcast = _mm512_set1_epi16(100);
    result = _mm512_mask_blend_epi16(0xAAAAAAAA, result, broadcast);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V32HFmode - 32 half-precision floats */
static int test_v32hf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1_data[32];  /* _Float16 storage */
    __attribute__((aligned(64))) uint16_t src2_data[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    /* Initialize as half floats */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = 0x3C00 | (i & 0x3FF);  /* ~1.0 + small variation */
        src2_data[i] = 0x4000 | (i & 0x3FF);  /* ~2.0 + small variation */
    }
    
    __m512h v1 = *(__m512h*)src1_data;
    __m512h v2 = *(__m512h*)src2_data;
    
    /* Create mask: select where i < 16 */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i < 16) mask |= (1U << i);
    }
    
    /* This should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, v2, v1);
    
    /* Store and compute checksum */
    *(__m512h*)dst = result;
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V32BFmode - 32 bfloat16 floats */
static int test_v32bf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1[32];
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        src1[i] = (0x3F80 + i) & 0xFFFF;  /* ~1.0 in bfloat16 */
        src2[i] = (0x4000 + i) & 0xFFFF;  /* ~2.0 in bfloat16 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;  /* 1010... pattern */
    
    /* For bfloat16, we use epi16 blend on the integer representation
     * This should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */

/* V16SImode - 16 single-word integers */
static int test_v16si_blend(void) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 100;
        src2[i] = i * 200;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp = _mm512_set1_epi32(800);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp);
    
    /* This should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Use in arithmetic operation */
    __m512i added = _mm512_add_epi32(result, _mm512_set1_epi32(1));
    result = _mm512_mask_blend_epi32(0x00FF, added, result);
    
    _mm512_store_epi32(dst, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DImode - 8 double-word integers */
static int test_v8di_blend(void) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000000LL * i;
        src2[i] = 2000000000LL * i;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = 0xAA;  /* 10101010 */
    
    /* This should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_epi64(dst, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return (int)(sum & 0x7FFFFFFF);
}

/* V8DFmode - 8 double-precision floats */
static int test_v8df_blend(void) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1.0 * (i + 1);
        src2[i] = 2.0 * (i + 1);
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp = _mm512_set1_pd(4.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp, _CMP_GT_OQ);
    
    /* This should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
    result = _mm512_mask_blend_pd(0x0F, multiplied, result);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return (int)(sum * 100.0);
}

/* V16SFmode - 16 single-precision floats */
static int test_v16sf_blend(void) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float dst[16];
    volatile __attribute__((aligned(64))) float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = 0.5f * i;
        src2[i] = 1.5f * i;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp, _CMP_LT_OQ);
    
    /* This should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Store to volatile array in loop to prevent optimization */
    for (int iter = 0; iter < 3; iter++) {
        __m512 temp = _mm512_add_ps(result, _mm512_set1_ps(iter * 0.1f));
        result = _mm512_mask_blend_ps(0xAAAA, temp, result);
        _mm512_store_ps(volatile_dst, result);
    }
    
    _mm512_store_ps(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return (int)(sum * 10.0f);
}

#endif /* __AVX512F__ */

/* Main driver function */
int main(int argc, char **argv) {
    int total_checksum = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F supported, testing blends...\n");
    
    /* Use argc to create variable loop count */
    int iterations = (argc > 1) ? atoi(argv[1]) : 1;
    if (iterations < 1) iterations = 1;
    if (iterations > 10) iterations = 10;
    
    for (int iter = 0; iter < iterations; iter++) {
        total_checksum += test_v16sf_blend();
        total_checksum += test_v8df_blend();
        total_checksum += test_v16si_blend();
        total_checksum += test_v8di_blend();
        
#ifdef __AVX512BW__
        total_checksum += test_v64qi_blend();
        total_checksum += test_v32hi_blend();
        total_checksum += test_v32hf_blend();
        total_checksum += test_v32bf_blend();
#endif
    }
    
    printf("Total checksum: %d\n", total_checksum);
#else
    printf("AVX-512 not supported on this platform\n");
    total_checksum = -1;
#endif
    
    return total_checksum & 0xFF;
}
