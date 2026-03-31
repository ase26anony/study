/* test_avx512_blend.c
 * 
 * This program is designed to trigger the specific RTL expansion patterns
 * in i386-expand.cc lines 4303-4326 for AVX-512 blend operations.
 * It uses AVX-512 intrinsics with masking to ensure the compiler generates
 * the corresponding blend instructions for all vector modes.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Feature guards to prevent compilation errors */
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

/* Alignment attribute for all vector data */
#define ALIGN_64 __attribute__((aligned(64)))

/* Volatile store to prevent optimization */
#define FORCE_USED(x) __asm__ volatile("" : : "r"(x) : "memory")

/* ==================== V64QI (64 x 8-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(void) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 uint8_t dst[64];
    volatile ALIGN_64 uint8_t volatile_dst[64];
    
    /* Initialize with pattern data */
    for (int i = 0; i < 64; i++) {
        src1[i] = (uint8_t)(i * 3);
        src2[i] = (uint8_t)(i * 5 + 1);
    }
    
    /* Load vectors */
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask: select from v1 where (i % 4) < 2 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 4) < 2) {
            mask |= (1ULL << i);
        }
    }
    
    /* Perform blend using intrinsic - should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store result to aligned array */
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Also store to volatile array to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Compute checksum to ensure live computation */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    FORCE_USED(sum);
    return sum;
}
#else
static uint64_t test_v64qi_blend(void) { return 0; }
#endif

/* ==================== V32HI (32 x 16-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(void) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)(i * 100);
        src2[i] = (uint16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison intrinsic */
    __m512i cmp_val = _mm512_set1_epi16(1500);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Use result in arithmetic operation */
    __m512i added = _mm512_add_epi16(result, _mm512_set1_epi16(1));
    _mm512_store_si512((__m512i*)dst, added);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    FORCE_USED(sum);
    return sum;
}
#else
static uint64_t test_v32hi_blend(void) { return 0; }
#endif

/* ==================== V32HF (32 x half-precision floats) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static uint64_t test_v32hf_blend(void) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f + 0.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask: select where src1 > src2 */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, v2, v1);
    
    _mm512_store_ph(dst, result);
    
    /* Use in reduction */
    _Float16 sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    FORCE_USED(sum);
    return (uint64_t)(sum * 1000);
}
#else
static uint64_t test_v32hf_blend(void) { return 0; }
#endif

/* ==================== V32BF (32 x bfloat16) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
static uint64_t test_v32bf_blend(void) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 dst[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        float f1 = i * 0.75f;
        float f2 = i * 1.25f + 0.25f;
        src1[i] = (__bfloat16)f1;
        src2[i] = (__bfloat16)f2;
    }
    
    /* Load as integers since blend_epi16 works on the representation */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask pattern */
    __mmask32 mask = 0xAAAAAAAA; /* 10101010... pattern */
    
    /* Blend using epi16 intrinsic - bfloat16 uses 16-bit storage */
    /* Should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v2, v1);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)dst[i];
    }
    
    FORCE_USED(sum);
    return sum;
}
#else
static uint64_t test_v32bf_blend(void) { return 0; }
#endif

/* ==================== V16SI (16 x 32-bit integers) ==================== */
#if HAS_AVX512F
static uint64_t test_v16si_blend(void) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Blend with broadcasted scalar */
    __m512i scalar = _mm512_set1_epi32(9999);
    result = _mm512_mask_blend_epi32(0xAAAA, scalar, result);
    
    _mm512_store_epi32(dst, result);
    
    /* Use in loop with side effect */
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    FORCE_USED(sum);
    return (uint64_t)sum;
}
#else
static uint64_t test_v16si_blend(void) { return 0; }
#endif

/* ==================== V8DI (8 x 64-bit integers) ==================== */
#if HAS_AVX512F
static uint64_t test_v8di_blend(void) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL + 5000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask: select where index is even */
    __mmask8 mask = 0x55; /* 01010101 */
    
    /* Blend with mask - should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v2, v1);
    
    _mm512_store_epi64(dst, result);
    
    /* Use in arithmetic operation */
    __m512i multiplied = _mm512_mullo_epi64(result, _mm512_set1_epi64(2));
    _mm512_store_epi64(dst, multiplied);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)dst[i];
    }
    
    FORCE_USED(sum);
    return sum;
}
#else
static uint64_t test_v8di_blend(void) { return 0; }
#endif

/* ==================== V8DF (8 x double-precision floats) ==================== */
#if HAS_AVX512F
static uint64_t test_v8df_blend(void) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.111;
        src2[i] = i * 2.222 + 0.555;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using floating comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(result, _mm512_set1_pd(1.5));
    result = _mm512_mask_blend_pd(0x0F, multiplied, result);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    FORCE_USED(sum);
    return (uint64_t)(sum * 1000);
}
#else
static uint64_t test_v8df_blend(void) { return 0; }
#endif

/* ==================== V16SF (16 x single-precision floats) ==================== */
#if HAS_AVX512F
static uint64_t test_v16sf_blend(void) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.75f;
        src2[i] = i * 1.25f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(6.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Use in loop with volatile counter to prevent optimization */
    volatile int loop_count = 2;
    for (int k = 0; k < loop_count; k++) {
        __m512 added = _mm512_add_ps(result, _mm512_set1_ps(0.1f * k));
        result = _mm512_mask_blend_ps(0xFF00, added, result);
    }
    
    _mm512_store_ps(dst, result);
    
    /* Compute reduction */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    FORCE_USED(sum);
    return (uint64_t)(sum * 1000);
}
#else
static uint64_t test_v16sf_blend(void) { return 0; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    uint64_t total_hash = 0;
    
    printf("Testing AVX-512 blend operations...\n");
    
#if !HAS_AVX512F && !HAS_AVX512BW
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 0;
#endif
    
    /* Call all test functions and accumulate results */
    total_hash ^= test_v64qi_blend();
    total_hash ^= test_v32hi_blend();
    total_hash ^= test_v32hf_blend();
    total_hash ^= test_v32bf_blend();
    total_hash ^= test_v16si_blend();
    total_hash ^= test_v8di_blend();
    total_hash ^= test_v8df_blend();
    total_hash ^= test_v16sf_blend();
    
    /* Use argc to add some variability to control flow */
    if (argc > 1) {
        total_hash += (uint64_t)argc;
    }
    
    printf("Final hash: %lu\n", (unsigned long)total_hash);
    
    /* Return non-zero only if all tests returned 0 (unlikely) */
    return total_hash == 0 ? 1 : 0;
}
