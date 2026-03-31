/* test_avx512_blend.c - Coverage for i386-expand.cc lines 4303-4326 */
#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define ALIGN_64 __attribute__((aligned(64)))

/* Feature guards */
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

/* Volatile variables to prevent optimization */
static volatile int g_loop_count = 100;
static volatile float g_scale = 2.5f;

/* ========== V64QI (64 x int8) ========== */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(int argc) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 int8_t dst[64];
    volatile ALIGN_64 int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask: blend where src1[i] < 100 */
    __mmask64 mask = _mm512_cmplt_epi8_mask(v1, _mm512_set1_epi8(100));
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Use in loop with argc dependency */
    int loop = (argc > 1) ? g_loop_count : 4;
    __m512i accum = _mm512_setzero_si512();
    
    for (int i = 0; i < loop; i++) {
        /* Blend with broadcast scalar */
        __m512i scalar = _mm512_set1_epi8((int8_t)i);
        blended = _mm512_mask_blend_epi8(mask, blended, scalar);
        
        /* Add to prevent optimization */
        accum = _mm512_add_epi8(accum, blended);
    }
    
    _mm512_store_si512((__m512i*)dst, accum);
    
    /* Store to volatile to ensure side effect */
    memcpy((void*)volatile_dst, dst, 64);
    
    /* Compute checksum */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += (uint8_t)dst[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#else
static uint64_t test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ========== V32HI (32 x int16) ========== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 - 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, _mm512_set1_epi16(500));
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi16(v1, _mm512_set1_epi16(3));
    blended = _mm512_mask_blend_epi16(mask, blended, multiplied);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return (uint64_t)sum;
}
#else
static uint64_t test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ========== V32HF (32 x half precision) ========== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
#include <float.h>
static uint64_t test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f - 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Store and compute checksum */
    _mm512_store_ph(dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        /* Convert to integer for checksum */
        sum += (uint16_t)(dst[i] * 1000);
    }
    
    (void)argc;
    return sum;
}
#else
static uint64_t test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ========== V32BF (32 x bfloat16) ========== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
#include <immintrin.h>
static uint64_t test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 dst[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        float f1 = i * 1.25f;
        float f2 = i * 2.75f;
        src1[i] = (__bfloat16)f1;
        src2[i] = (__bfloat16)f2;
    }
    
    /* Load as epi16 for blending */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - blend where index is even */
    __mmask32 mask = 0xAAAAAAAA; /* 1010... pattern */
    
    /* Blend using epi16 intrinsic - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)dst[i];
    }
    
    (void)argc;
    return sum;
}
#else
static uint64_t test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ========== V16SI (16 x int32) ========== */
#if HAS_AVX512F
static uint64_t test_v16si_blend(int argc) {
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
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, _mm512_set1_epi32(8000));
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Use in arithmetic operation */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(argc));
    blended = _mm512_mask_blend_epi32(mask, blended, added);
    
    _mm512_store_epi32(dst, blended);
    
    /* Compute sum */
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return (uint64_t)sum;
}
#else
static uint64_t test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ========== V8DI (8 x int64) ========== */
#if HAS_AVX512F
static uint64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 10000LL;
        src2[i] = i * 20000LL + 2500;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, _mm512_set1_epi64(30000));
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_epi64(dst, blended);
    
    /* Compute sum */
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)dst[i];
    }
    
    (void)argc;
    return sum;
}
#else
static uint64_t test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ========== V8DF (8 x double) ========== */
#if HAS_AVX512F
static uint64_t test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    volatile ALIGN_64 double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.25;
        src2[i] = i * 2.75 - 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512d scaled = _mm512_mul_pd(v1, _mm512_set1_pd(g_scale));
    blended = _mm512_mask_blend_pd(mask, blended, scaled);
    
    _mm512_store_pd(dst, blended);
    
    /* Store to volatile */
    for (int i = 0; i < 8; i++) {
        volatile_dst[i] = dst[i];
    }
    
    /* Compute integer checksum */
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)(dst[i] * 1000.0);
    }
    
    (void)argc;
    return sum;
}
#else
static uint64_t test_v8df_blend(int argc) { (void)argc; return 0; }
#endif

/* ========== V16SF (16 x float) ========== */
#if HAS_AVX512F
static uint64_t test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1.5f;
        src2[i] = i * 3.0f - 1.0f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask - blend where src1 > 10.0 */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, _mm512_set1_ps(10.0f), _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Use in loop with reduction */
    float sum_f = 0.0f;
    int loop = (argc > 2) ? 3 : 1;
    
    for (int i = 0; i < loop; i++) {
        __m512 temp = _mm512_add_ps(blended, _mm512_set1_ps((float)i));
        blended = _mm512_mask_blend_ps(mask, blended, temp);
        
        /* Horizontal sum */
        sum_f += _mm512_reduce_add_ps(blended);
    }
    
    _mm512_store_ps(dst, blended);
    
    /* Create artificial dependency */
    __asm__ volatile("" : : "r"(sum_f) : "memory");
    
    return (uint64_t)(sum_f * 1000.0f);
}
#else
static uint64_t test_v16sf_blend(int argc) { (void)argc; return 0; }
#endif

/* ========== Main Driver ========== */
int main(int argc, char** argv) {
    uint64_t total_hash = 0;
    
    printf("Testing AVX-512 blend patterns...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected.\n");
    
    /* Call all test functions */
    total_hash ^= test_v64qi_blend(argc);
    total_hash ^= test_v32hi_blend(argc);
    total_hash ^= test_v32hf_blend(argc);
    total_hash ^= test_v32bf_blend(argc);
    total_hash ^= test_v16si_blend(argc);
    total_hash ^= test_v8di_blend(argc);
    total_hash ^= test_v8df_blend(argc);
    total_hash ^= test_v16sf_blend(argc);
    
    printf("Total hash: 0x%016lx\n", total_hash);
    
    /* Use result to affect return code */
    return (int)(total_hash % 256);
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
}
