/* test_avx512_blend.c - Comprehensive AVX-512 blend intrinsics test
 * Targeting uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Feature detection macros */
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
static volatile int g_volatile_counter = 0;

/* ==================== V64QI (64x int8) ==================== */
#ifdef __AVX512BW__
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile ALIGN_64 int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask: 0xAAAAAAAAAAAAAAAA for 64-bit mask */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 2 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend using intrinsic - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Use in reduction */
    int64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? 100 : 10;
    for (int i = 0; i < loop_count; i++) {
        __m512i temp = _mm512_mask_blend_epi8(mask ^ 0x5555555555555555ULL, 
                                             v1, v2);
        __asm__ volatile("" : : "r"(temp) : "memory");
        g_volatile_counter++;
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#endif

/* ==================== V32HI (32x int16) ==================== */
#ifdef __AVX512BW__
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    volatile ALIGN_64 int16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 - 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(1500);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(999);
    __m512i blended2 = _mm512_mask_blend_epi16(mask, blended, broadcast);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended2);
    
    /* Reduction */
    int32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(blended) : "memory");
    
    return (int)(sum & 0x7FFF);
}
#endif

/* ==================== V32HF (32x half precision) ==================== */
#ifdef __AVX512BW__
#include <x86intrin.h>  /* For _Float16 if available */

#ifdef __AVX512FP16__
static int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    volatile ALIGN_64 _Float16 volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f - 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using comparison */
    __m512h cmp_val = _mm512_set1_ph(20.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512h added = _mm512_add_ph(v1, v2);
    __m512h blended2 = _mm512_mask_blend_ph(mask ^ 0xAAAAAAAA, blended, added);
    
    _mm512_store_ph((void*)volatile_dst, blended2);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)volatile_dst[i];
    }
    
    return (int)(sum * 100.0f);
}
#else
/* Fallback using integer representation for half precision */
static int test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t src1[32];  /* Half precision as uint16 */
    ALIGN_64 uint16_t src2[32];
    volatile ALIGN_64 uint16_t volatile_dst[32];
    
    /* Simple pattern */
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)(i * 0x0400);  /* Approx i * 1.0 */
        src2[i] = (uint16_t)(i * 0x0800);  /* Approx i * 2.0 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask */
    __mmask32 mask = 0xAAAAAAAA;
    
    /* Blend using epi16 intrinsic on integer representation */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Reduction */
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum & 0xFFFF);
}
#endif
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#ifdef __AVX512BW__
#ifdef __AVX512BF16__
#include <x86intrin.h>
static int test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    volatile ALIGN_64 __bfloat16 volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (__bfloat16)(i * 1.25f);
        src2[i] = (__bfloat16)(i * 2.75f);
    }
    
    __m512bh v1 = _mm512_load_si512((const __m512i*)src1);
    __m512bh v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - bfloat16 uses same mask as epi16 */
    __mmask32 mask = 0xCCCCCCCC;
    
    /* Blend using epi16 intrinsic on the integer representation */
    __m512i v1_int = _mm512_load_si512((const __m512i*)src1);
    __m512i v2_int = _mm512_load_si512((const __m512i*)src2);
    __m512i blended_int = _mm512_mask_blend_epi16(mask, v1_int, v2_int);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended_int);
    
    /* Reduction */
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)volatile_dst[i];
    }
    
    return (int)(sum & 0xFFFF);
}
#else
/* Fallback using uint16_t for bfloat16 */
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    volatile ALIGN_64 uint16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)(i * 0x4200);  /* Roughly i * 2.0 in bfloat16 */
        src2[i] = (uint16_t)(i * 0x4400);  /* Roughly i * 4.0 */
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    __mmask32 mask = 0xF0F0F0F0;
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    return (int)(sum & 0xFFFF);
}
#endif
#endif

/* ==================== V16SI (16x int32) ==================== */
#ifdef __AVX512F__
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    volatile ALIGN_64 int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 - 500;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Blend with arithmetic operation */
    __m512i multiplied = _mm512_mullo_epi32(v1, _mm512_set1_epi32(2));
    __m512i blended2 = _mm512_mask_blend_epi32(mask ^ 0xAAAA, blended, multiplied);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended2);
    
    /* Reduction with loop dependency */
    int64_t sum = 0;
    for (int i = 0; i < argc * 2; i++) {
        if (i < 16) {
            sum += volatile_dst[i];
        }
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#endif

/* ==================== V8DI (8x int64) ==================== */
#ifdef __AVX512F__
static int test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    volatile ALIGN_64 int64_t volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 1000000LL;
        src2[i] = (int64_t)i * 2000000LL - 500000LL;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi64(4000000LL);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi64(9999999LL);
    __m512i blended2 = _mm512_mask_blend_epi64(mask ^ 0xAA, blended, broadcast);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended2);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += volatile_dst[i];
    }
    
    /* Force side effect */
    __asm__ volatile("" : : "r"(blended), "r"(blended2) : "memory");
    
    return (int)((sum >> 32) & 0x7FFFFFFF);
}
#endif

/* ==================== V8DF (8x double) ==================== */
#ifdef __AVX512F__
static int test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    volatile ALIGN_64 double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)i * 1.5;
        src2[i] = (double)i * 2.5 - 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(3.0));
    __m512d blended2 = _mm512_mask_blend_pd(mask ^ 0x55, blended, multiplied);
    
    _mm512_store_pd(volatile_dst, blended2);
    
    /* Reduction with volatile loop counter */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += volatile_dst[i] * (g_volatile_counter + 1);
    }
    
    return (int)(sum * 1000.0);
}
#endif

/* ==================== V16SF (16x float) ==================== */
#ifdef __AVX512F__
static int test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)i * 1.25f;
        src2[i] = (float)i * 2.75f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(10.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Blend with arithmetic operation */
    __m512 added = _mm512_add_ps(v1, v2);
    __m512 blended2 = _mm512_mask_blend_ps(mask ^ 0xAAAA, blended, added);
    
    _mm512_store_ps(volatile_dst, blended2);
    
    /* Reduction in loop with argc dependency */
    float sum = 0.0f;
    int iterations = (argc > 0) ? argc : 1;
    for (int iter = 0; iter < iterations; iter++) {
        for (int i = 0; i < 16; i++) {
            sum += volatile_dst[i] * (iter + 1);
        }
    }
    
    return (int)(sum * 100.0f);
}
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    int total_hash = 0;
    
    printf("AVX-512 Blend Intrinsics Test\n");
    printf("Targeting i386-expand.cc lines 4303-4326\n\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected:\n");
#ifdef __AVX512F__
    printf("  AVX-512F: YES\n");
#endif
#ifdef __AVX512BW__
    printf("  AVX-512BW: YES\n");
#endif
#ifdef __AVX512FP16__
    printf("  AVX-512-FP16: YES\n");
#endif
#ifdef __AVX512BF16__
    printf("  AVX-512-BF16: YES\n");
#endif
    printf("\n");
    
    /* Run all available tests */
#ifdef __AVX512BW__
    printf("Testing V64QI (64x int8)...\n");
    total_hash ^= test_v64qi_blend(argc);
    
    printf("Testing V32HI (32x int16)...\n");
    total_hash ^= test_v32hi_blend(argc);
    
    printf("Testing V32HF (32x half precision)...\n");
    total_hash ^= test_v32hf_blend(argc);
    
    printf("Testing V32BF (32x bfloat16)...\n");
    total_hash ^= test_v32bf_blend(argc);
#endif
    
#ifdef __AVX512F__
    printf("Testing V16SI (16x int32)...\n");
    total_hash ^= test_v16si_blend(argc);
    
    printf("Testing V8DI (8x int64)...\n");
    total_hash ^= test_v8di_blend(argc);
    
    printf("Testing V8DF (8x double)...\n");
    total_hash ^= test_v8df_blend(argc);
    
    printf("Testing V16SF (16x float)...\n");
    total_hash ^= test_v16sf_blend(argc);
#endif
    
    printf("\nFinal hash: %d (0x%08X)\n", total_hash, total_hash);
    
    /* Use result to affect return code */
    return (total_hash == 0) ? 0 : (total_hash & 255);
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
}
