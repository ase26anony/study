/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Feature guards for AVX-512 extensions */
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

/* Helper to create volatile dependency */
#define FORCE_USE(x) __asm__ volatile("" : : "r"(x) : "memory")

/* ==================== V64QI (64x int8) ==================== */
#if HAS_AVX512BW
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile int8_t volatile_dst[64] ALIGN_64;
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask: select from v1 where i % 2 == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 2) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v2, v1);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Also blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi8(0x7F);
    __m512i blended2 = _mm512_mask_blend_epi8(mask, blended, broadcast);
    
    /* Use in loop with argc dependency */
    int sum = 0;
    for (int i = 0; i < (argc % 8 + 1); i++) {
        __m512i temp = _mm512_mask_blend_epi8(mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, temp);
        sum += dst[i * 8];
    }
    
    FORCE_USE(sum);
    _mm512_store_si512((__m512i*)dst, blended2);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < 64; i++) {
        checksum += dst[i];
    }
    return checksum;
}
#endif

/* ==================== V32HI (32x int16) ==================== */
#if HAS_AVX512BW
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(1600);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi16(v1, _mm512_set1_epi16(100));
    __m512i blended2 = _mm512_mask_blend_epi16(mask, blended, added);
    
    /* Use in reduction */
    _mm512_store_si512((__m512i*)dst, blended2);
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Loop with argc dependency */
    for (int iter = 0; iter < (argc % 4 + 1); iter++) {
        __mmask32 alt_mask = _mm512_cmpeq_epi16_mask(v1, v2);
        __m512i temp = _mm512_mask_blend_epi16(alt_mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, temp);
        sum += dst[iter];
    }
    
    FORCE_USE(sum);
    return sum;
}
#endif

/* ==================== V32HF (32x half precision) ==================== */
#if HAS_AVX512BW
#include <x86intrin.h>  /* For _Float16 if available */
#ifdef __AVX512FP16__
static int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f + 0.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using float comparison */
    __m512h cmp_val = _mm512_set1_ph(20.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v2, v1);
    
    /* Store and compute checksum */
    _mm512_store_ph(dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        /* Convert to int for checksum */
        sum += (int)(dst[i] * 100);
    }
    
    /* Additional blend in loop */
    for (int i = 0; i < (argc % 3); i++) {
        __m512h temp = _mm512_mask_blend_ph(mask, v1, v2);
        _mm512_store_ph(dst, temp);
        sum += (int)(dst[i] * 50);
    }
    
    return sum;
}
#else
static int test_v32hf_blend(int argc) {
    (void)argc;
    printf("AVX512-FP16 not supported, skipping V32HF test\n");
    return 0;
}
#endif
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#if HAS_AVX512BW
#ifdef __AVX512BF16__
#include <x86intrin.h>
static int test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 dst[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        uint16_t val1 = (i * 100) & 0xFFFF;
        uint16_t val2 = (i * 200 + 50) & 0xFFFF;
        src1[i] = *(__bfloat16*)&val1;
        src2[i] = *(__bfloat16*)&val2;
    }
    
    /* Load as integers for blending */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask: select where i % 3 == 0 */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 3) == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Blend using epi16 intrinsic - bfloat16 uses 16-bit lanes
     * Should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v2, v1);
    
    /* Store and compute integer checksum */
    _mm512_store_si512((__m512i*)dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        uint16_t val;
        memcpy(&val, &dst[i], sizeof(val));
        sum += val;
    }
    
    /* Loop with argc */
    for (int iter = 0; iter < (argc % 5); iter++) {
        __m512i temp = _mm512_mask_blend_epi16(mask, v1, v2);
        _mm512_store_si512((__m512i*)dst, temp);
        uint16_t val;
        memcpy(&val, &dst[iter], sizeof(val));
        sum += val;
    }
    
    return sum;
}
#else
static int test_v32bf_blend(int argc) {
    (void)argc;
    printf("AVX512-BF16 not supported, skipping V32BF test\n");
    return 0;
}
#endif
#endif

/* ==================== V16SI (16x int32) ==================== */
#if HAS_AVX512F
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    volatile int32_t volatile_dst[16] ALIGN_64;
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v2, v1);
    
    /* Store to volatile */
    _mm512_store_epi32((void*)volatile_dst, blended);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi32(0x7FFFFFFF);
    __m512i blended2 = _mm512_mask_blend_epi32(mask, blended, broadcast);
    
    /* Use in reduction with loop */
    int sum = 0;
    for (int iter = 0; iter < (argc % 6 + 1); iter++) {
        __mmask16 alt_mask = _mm512_cmpeq_epi32_mask(v1, v2);
        __m512i temp = _mm512_mask_blend_epi32(alt_mask, v1, v2);
        _mm512_store_epi32(dst, temp);
        
        for (int i = 0; i < 16; i++) {
            sum += dst[i] >> (iter + 1);
        }
    }
    
    _mm512_store_epi32(dst, blended2);
    FORCE_USE(sum);
    
    /* Final checksum */
    int checksum = 0;
    for (int i = 0; i < 16; i++) {
        checksum += dst[i];
    }
    return checksum;
}
#endif

/* ==================== V8DI (8x int64) ==================== */
#if HAS_AVX512F
static long long test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 10000LL;
        src2[i] = (int64_t)i * 20000LL + 5000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask: select where i % 2 == 0 */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i % 2) == 0) {
            mask |= (1 << i);
        }
    }
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi64(v1, _mm512_set1_epi64(3));
    __m512i blended2 = _mm512_mask_blend_epi64(mask, blended, multiplied);
    
    /* Store and compute sum */
    _mm512_store_epi64(dst, blended2);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Additional blends in argc-dependent loop */
    for (int iter = 0; iter < (argc % 4); iter++) {
        __mmask8 alt_mask = _mm512_cmpgt_epi64_mask(v1, _mm512_set1_epi64(30000));
        __m512i temp = _mm512_mask_blend_epi64(alt_mask, v1, v2);
        _mm512_store_epi64(dst, temp);
        sum += dst[iter % 8];
    }
    
    return sum;
}
#endif

/* ==================== V8DF (8x double) ==================== */
#if HAS_AVX512F
static double test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    volatile double volatile_dst[8] ALIGN_64;
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.25;
        src2[i] = i * 2.75 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using double comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v2, v1);
    
    /* Store to volatile */
    _mm512_store_pd((double*)volatile_dst, blended);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d blended2 = _mm512_mask_blend_pd(mask, blended, multiplied);
    
    /* Use in reduction */
    _mm512_store_pd(dst, blended2);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Loop with argc dependency */
    for (int iter = 0; iter < (argc % 3 + 1); iter++) {
        __mmask8 alt_mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
        __m512d temp = _mm512_mask_blend_pd(alt_mask, v1, v2);
        _mm512_store_pd(dst, temp);
        sum += dst[iter % 8];
    }
    
    FORCE_USE(sum);
    return sum;
}
#endif

/* ==================== V16SF (16x float) ==================== */
#if HAS_AVX512F
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.1f;
        src2[i] = i * 2.2f + 0.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using float comparison */
    __m512 cmp_val = _mm512_set1_ps(8.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v2, v1);
    
    /* Blend with arithmetic result */
    __m512 added = _mm512_add_ps(v1, _mm512_set1_ps(10.0f));
    __m512 blended2 = _mm512_mask_blend_ps(mask, blended, added);
    
    /* Store and compute sum */
    _mm512_store_ps(dst, blended2);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    /* Additional blends in loop */
    for (int iter = 0; iter < (argc % 5); iter++) {
        __mmask16 alt_mask = _mm512_cmp_ps_mask(v1, v2, _CMP_EQ_OQ);
        __m512 temp = _mm512_mask_blend_ps(alt_mask, v1, v2);
        _mm512_store_ps(dst, temp);
        sum += dst[iter % 16];
    }
    
    return sum;
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    int total_checksum = 0;
    
    printf("Testing AVX-512 blend intrinsics to cover i386-expand.cc lines 4303-4326\n");
    
#if HAS_AVX512BW && HAS_AVX512F
    printf("AVX-512F and AVX-512BW supported, running all tests...\n");
    
    /* Run all tests with argc dependency */
#if HAS_AVX512BW
    total_checksum += test_v64qi_blend(argc);
    total_checksum += test_v32hi_blend(argc);
    
    #ifdef __AVX512FP16__
    total_checksum += test_v32hf_blend(argc);
    #endif
    
    #ifdef __AVX512BF16__
    total_checksum += test_v32bf_blend(argc);
    #endif
#endif
    
#if HAS_AVX512F
    total_checksum += test_v16si_blend(argc);
    
    long long di_sum = test_v8di_blend(argc);
    total_checksum += (int)(di_sum & 0xFFFFFFFF) + (int)(di_sum >> 32);
    
    double df_sum = test_v8df_blend(argc);
    total_checksum += (int)df_sum;
    
    float sf_sum = test_v16sf_blend(argc);
    total_checksum += (int)sf_sum;
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Use result to affect return code */
    return (total_checksum != 0) ? 0 : 1;
    
#else
    printf("AVX-512 not fully supported on this compiler/platform.\n");
    printf("Required: AVX-512F and AVX-512BW extensions\n");
    printf("Compile with: -march=skylake-avx512 or -mavx512f -mavx512bw\n");
    return 2;
#endif
}
