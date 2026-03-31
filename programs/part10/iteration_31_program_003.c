/* test_avx512_blend.c - Coverage for i386-expand.cc blend patterns */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Feature guards for compilation */
#ifdef __AVX512F__
#define HAS_AVX512F 1
#else
#define HAS_AVX512F 0
#endif

#ifdef __AVX512BW__
#define HAS_AVX512BW 1
#else
#define HAS_AVX512BW 0
#endif

/* Alignment helper */
#define ALIGN_64 __attribute__((aligned(64)))

/* Prevent optimization */
static volatile int force_volatile = 0;

/* ==================== V64QI (64x int8) ==================== */
#ifdef __AVX512BW__
static int test_v64qi_blend(int seed) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile ALIGN_64 int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i + seed);
        src2[i] = (int8_t)(i - seed * 2);
    }
    
    /* Load vectors */
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask: blend where src1[i] > 0 */
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, _mm512_setzero_si512());
    
    /* Blend using intrinsic - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store with volatile to prevent optimization */
    _mm512_store_epi32((void*)volatile_dst, blended);
    
    /* Also store to regular array for computation */
    _mm512_store_epi32(dst, blended);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Force dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}
#else
static int test_v64qi_blend(int seed) { return seed; }
#endif

/* ==================== V32HI (32x int16) ==================== */
#ifdef __AVX512BW__
static int test_v32hi_blend(int seed) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 3 + seed);
        src2[i] = (int16_t)(i * 5 - seed);
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, _mm512_set1_epi16(10));
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(0x7FFF);
    __m512i blended2 = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    _mm512_store_epi32(dst, blended2);
    
    /* Reduction */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}
#else
static int test_v32hi_blend(int seed) { return seed * 2; }
#endif

/* ==================== V32HF (32x half precision) ==================== */
#ifdef __AVX512BW__
#include <x86intrin.h>
static int test_v32hf_blend(int seed) {
    ALIGN_64 uint16_t src1_data[32];  /* Store as uint16 for half float */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t dst_data[32];
    
    /* Simple pattern for half floats */
    for (int i = 0; i < 32; i++) {
        src1_data[i] = (uint16_t)(0x3C00 | (i & 0x1F));  /* ~1.0 with variation */
        src2_data[i] = (uint16_t)(0x4000 | (i & 0x1F));  /* ~2.0 with variation */
    }
    
    __m512i v1 = _mm512_load_epi32(src1_data);
    __m512i v2 = _mm512_load_epi32(src2_data);
    
    /* Create mask: blend where (i % 3) == 0 */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 3) == 0) mask |= (1ULL << i);
    }
    
    /* Blend using epi16 intrinsic for half floats */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_epi32(dst_data, blended);
    
    /* Checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)dst_data[i];
    }
    
    return sum;
}
#else
static int test_v32hf_blend(int seed) { return seed * 3; }
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#ifdef __AVX512BW__
static int test_v32bf_blend(int seed) {
    ALIGN_64 uint16_t src1_data[32];  /* bfloat16 as uint16 */
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 uint16_t dst_data[32];
    
    for (int i = 0; i < 32; i++) {
        src1_data[i] = (uint16_t)(0x3F80 + i);  /* ~1.0 in bfloat16 */
        src2_data[i] = (uint16_t)(0x4000 + i);  /* ~2.0 in bfloat16 */
    }
    
    __m512i v1 = _mm512_load_epi32(src1_data);
    __m512i v2 = _mm512_load_epi32(src2_data);
    
    /* Load mask from array to ensure it's used */
    ALIGN_64 uint16_t mask_data[32];
    for (int i = 0; i < 32; i++) {
        mask_data[i] = (uint16_t)((i % 4) == 0 ? 0xFFFF : 0x0000);
    }
    __m512i mask_vec = _mm512_load_epi32(mask_data);
    __mmask32 mask = _mm512_cmpeq_epi16_mask(mask_vec, _mm512_set1_epi16(0xFFFF));
    
    /* Blend bfloat16 using epi16 intrinsic */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_epi32(dst_data, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)dst_data[i];
    }
    
    return sum;
}
#else
static int test_v32bf_blend(int seed) { return seed * 4; }
#endif

/* ==================== V16SI (16x int32) ==================== */
#ifdef __AVX512F__
static int test_v16si_blend(int seed) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 10 + seed;
        src2[i] = i * 20 - seed;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, _mm512_set1_epi32(50));
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(100));
    __m512i blended2 = _mm512_mask_blend_epi32(mask, blended, added);
    
    _mm512_store_epi32(dst, blended2);
    
    /* Reduction in loop with volatile dependency */
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
        if (force_volatile) sum += 1;  /* Prevent dead code elimination */
    }
    
    return sum;
}
#else
static int test_v16si_blend(int seed) { return seed * 5; }
#endif

/* ==================== V8DI (8x int64) ==================== */
#ifdef __AVX512F__
static long long test_v8di_blend(int seed) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 100 + seed;
        src2[i] = (int64_t)i * 200 - seed;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, _mm512_set1_epi64(300));
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_epi64(dst, blended);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}
#else
static long long test_v8di_blend(int seed) { return seed * 6LL; }
#endif

/* ==================== V8DF (8x double) ==================== */
#ifdef __AVX512F__
static double test_v8df_blend(int seed) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)(i + 1) * 0.5 + seed * 0.1;
        src2[i] = (double)(i + 1) * 1.5 - seed * 0.1;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, _mm512_set1_pd(2.0), _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Additional blend in loop to ensure usage */
    volatile int loop_count = 8;
    for (int i = 0; i < loop_count; i++) {
        __m512d temp = _mm512_set1_pd(i * 0.25);
        blended = _mm512_mask_blend_pd(mask, blended, temp);
    }
    
    _mm512_store_pd(dst, blended);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return sum;
}
#else
static double test_v8df_blend(int seed) { return seed * 7.0; }
#endif

/* ==================== V16SF (16x float) ==================== */
#ifdef __AVX512F__
static float test_v16sf_blend(int seed) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)(i + 1) * 0.25f + seed * 0.01f;
        src2[i] = (float)(i + 1) * 0.75f - seed * 0.01f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(2.0f), _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Blend with multiplied result */
    __m512 multiplied = _mm512_mul_ps(v1, _mm512_set1_ps(1.5f));
    __m512 blended2 = _mm512_mask_blend_ps(mask, blended, multiplied);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_ps((void*)volatile_dst, blended2);
    _mm512_store_ps(dst, blended2);
    
    /* Reduction with memory barrier */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
        __asm__ volatile("" : : "r"(sum) : "memory");
    }
    
    return sum;
}
#else
static float test_v16sf_blend(int seed) { return seed * 8.0f; }
#endif

/* ==================== MAIN DRIVER ==================== */
int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int result = 0;
    
    printf("Testing AVX-512 blend patterns...\n");
    
#if HAS_AVX512BW
    printf("AVX512BW enabled\n");
#else
    printf("AVX512BW not available\n");
#endif

#if HAS_AVX512F
    printf("AVX512F enabled\n");
#else
    printf("AVX512F not available\n");
#endif
    
    /* Call all test functions with control flow */
    result += test_v64qi_blend(seed);
    result += test_v32hi_blend(seed);
    result += test_v32hf_blend(seed);
    result += test_v32bf_blend(seed);
    result += test_v16si_blend(seed);
    
    long long di_result = test_v8di_blend(seed);
    result += (int)(di_result & 0xFFFFFFFF);
    
    double df_result = test_v8df_blend(seed);
    result += (int)(df_result * 100.0);
    
    float sf_result = test_v16sf_blend(seed);
    result += (int)(sf_result * 100.0f);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Return non-zero if no AVX-512 support (for testing fallback) */
    if (!HAS_AVX512F && !HAS_AVX512BW) {
        printf("No AVX-512 support detected - running in fallback mode\n");
        return 1;
    }
    
    return 0;
}
