/* test_avx512_blend.c - Coverage for i386-expand.cc blend patterns */
#include <stdio.h>
#include <stdint.h>
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

/* Volatile store to prevent optimization */
static inline void force_store(void* ptr) {
    __asm__ volatile("" : : "r"(ptr) : "memory");
}

/* ==================== V64QI (64x int8) ==================== */
#if HAS_AVX512BW
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
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(32);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store with volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    force_store(volatile_dst);
    
    /* Also use in reduction */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? 4 : 2;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_load_si512((const __m512i*)src1);
        __mmask64 alt_mask = _mm512_cmplt_epi8_mask(temp, _mm512_set1_epi8(iter * 16));
        __m512i result = _mm512_mask_blend_epi8(alt_mask, temp, blended);
        _mm512_store_si512((__m512i*)dst, result);
        sum += dst[iter % 64];
    }
    
    return sum & 0xFF; /* Return checksum */
}
#else
static int test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI (32x int16) ==================== */
#if HAS_AVX512BW
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 100;
        src2[i] = -i * 50;
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask pattern */
    __mmask32 mask = 0xAAAAAAAA; /* 101010... pattern */
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Use blend result in arithmetic operation */
    __m512i multiplied = _mm512_mullo_epi16(blended, _mm512_set1_epi16(3));
    __m512i final = _mm512_mask_blend_epi16(mask ^ 0xFFFFFFFF, multiplied, blended);
    
    _mm512_store_si512((__m512i*)dst, final);
    
    /* Reduction with loop */
    int sum = 0;
    int loop_count = (argc > 2) ? 8 : 4;
    for (int iter = 0; iter < loop_count; iter++) {
        for (int i = 0; i < 32; i++) {
            sum += dst[i] >> 4;
        }
    }
    
    return sum & 0xFFFF;
}
#else
static int test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32x half precision) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f);
        src2[i] = (_Float16)(i * 1.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask from comparison */
    __m512h cmp_val = _mm512_set1_ph((_Float16)8.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Store and compute reduction */
    _mm512_store_ph(dst, blended);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        /* Convert to int for checksum */
        sum += (int)(dst[i] * 100.0f);
    }
    
    /* Additional blend with broadcast scalar */
    __m512h scalar = _mm512_set1_ph((_Float16)2.0f);
    __mmask32 alt_mask = 0x55555555;
    __m512h blended2 = _mm512_mask_blend_ph(alt_mask, blended, scalar);
    _mm512_store_ph(dst, blended2);
    
    for (int i = 0; i < 32; i++) {
        sum += (int)(dst[i] * 50.0f);
    }
    
    return sum & 0xFFFFFF;
}
#else
static int test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
#include <bfloat16.h>
static int test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 0.25f);
        src2[i] = bfloat16_from_float(i * 0.75f);
    }
    
    /* Load as integers for blending */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - blend using integer intrinsic */
    __mmask32 mask = 0x33333333;
    
    /* Blend - should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)dst, blended);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += bfloat16_to_float(dst[i]) * 1000.0f;
    }
    
    /* Loop with argc dependency */
    int iterations = (argc > 3) ? 3 : 1;
    for (int iter = 0; iter < iterations; iter++) {
        __mmask32 dynamic_mask = (iter % 2) ? 0x0F0F0F0F : 0xF0F0F0F0;
        __m512i reblended = _mm512_mask_blend_epi16(dynamic_mask, blended, v1);
        _mm512_store_si512((__m512i*)dst, reblended);
        
        for (int i = 0; i < 16; i++) { /* Sample half */
            sum += bfloat16_to_float(dst[i]) * 500.0f;
        }
    }
    
    return sum & 0xFFFFFF;
}
#else
static int test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI (16x int32) ==================== */
#if HAS_AVX512F
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask from comparison */
    __m512i cmp = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Use in arithmetic chain */
    __m512i added = _mm512_add_epi32(blended, _mm512_set1_epi32(42));
    __mmask16 alt_mask = _mm512_cmplt_epi32_mask(added, _mm512_set1_epi32(10000));
    __m512i final = _mm512_mask_blend_epi32(alt_mask, added, blended);
    
    _mm512_store_epi32(dst, final);
    
    /* Reduction with loop */
    int sum = 0;
    int loop_count = (argc > 1) ? argc % 5 + 1 : 2;
    for (int iter = 0; iter < loop_count; iter++) {
        for (int i = 0; i < 16; i++) {
            sum += dst[i] >> (iter % 4);
        }
    }
    
    return sum;
}
#else
static int test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI (8x int64) ==================== */
#if HAS_AVX512F
static long long test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1LL << (i * 2);
        src2[i] = 1LL << (i * 2 + 1);
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = 0xAA; /* 10101010 */
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    /* Blend with broadcast scalar */
    __m512i scalar = _mm512_set1_epi64(0xFFFFFFFF);
    __mmask8 scalar_mask = _mm512_cmpgt_epi64_mask(blended, _mm512_set1_epi64(100));
    __m512i final = _mm512_mask_blend_epi64(scalar_mask, blended, scalar);
    
    _mm512_store_epi64(dst, final);
    
    /* Compute checksum */
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Additional operation in loop */
    volatile int vol = argc;
    for (int i = 0; i < (vol % 3 + 1); i++) {
        __m512i temp = _mm512_load_epi64(src1);
        __mmask8 dyn_mask = (i % 2) ? 0x0F : 0xF0;
        __m512i result = _mm512_mask_blend_epi64(dyn_mask, temp, final);
        _mm512_store_epi64(dst, result);
        sum += dst[i % 8];
    }
    
    return sum;
}
#else
static long long test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF (8x double) ==================== */
#if HAS_AVX512F
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
    
    /* Create mask from comparison */
    __m512d cmp = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Use in arithmetic operation */
    __m512d multiplied = _mm512_mul_pd(blended, _mm512_set1_pd(1.5));
    __mmask8 alt_mask = _mm512_cmp_pd_mask(multiplied, _mm512_set1_pd(10.0), _CMP_GT_OQ);
    __m512d final = _mm512_mask_blend_pd(alt_mask, multiplied, blended);
    
    _mm512_store_pd(dst, final);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Loop with volatile dependency */
    volatile int vol_iter = (argc > 1) ? 2 : 1;
    for (int iter = 0; iter < vol_iter; iter++) {
        __m512d temp = _mm512_load_pd(src1);
        __mmask8 iter_mask = (iter % 2) ? 0x0F : 0xF0;
        __m512d result = _mm512_mask_blend_pd(iter_mask, temp, final);
        _mm512_store_pd(dst, result);
        sum += dst[iter % 8];
    }
    
    return sum;
}
#else
static double test_v8df_blend(int argc) { (void)argc; return 0.0; }
#endif

/* ==================== V16SF (16x float) ==================== */
#if HAS_AVX512F
static float test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.25f;
        src2[i] = i * 0.75f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask from comparison */
    __m512 cmp = _mm512_set1_ps(3.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp, _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512 added = _mm512_add_ps(blended, _mm512_set1_ps(1.0f));
    __mmask16 alt_mask = _mm512_cmp_ps_mask(added, _mm512_set1_ps(5.0f), _CMP_LT_OQ);
    __m512 final = _mm512_mask_blend_ps(alt_mask, added, blended);
    
    _mm512_store_ps(dst, final);
    
    /* Reduction with loop */
    float sum = 0.0f;
    int loop_count = (argc > 2) ? 3 : 1;
    for (int iter = 0; iter < loop_count; iter++) {
        for (int i = 0; i < 16; i++) {
            sum += dst[i];
        }
    }
    
    /* Additional blend inside loop */
    for (int i = 0; i < (argc % 2 + 1); i++) {
        __m512 temp = _mm512_load_ps(src1);
        __mmask16 dyn_mask = (i % 2) ? 0x00FF : 0xFF00;
        __m512 result = _mm512_mask_blend_ps(dyn_mask, temp, final);
        _mm512_store_ps(dst, result);
        sum += dst[i % 16];
    }
    
    return sum;
}
#else
static float test_v16sf_blend(int argc) { (void)argc; return 0.0f; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char** argv) {
    int total_checksum = 0;
    
    printf("Testing AVX-512 blend patterns...\n");
    
#if HAS_AVX512BW || HAS_AVX512F
    /* Call all test functions and accumulate results */
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
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Use result to affect return code (prevents dead code elimination) */
    return (total_checksum != 0) ? 0 : 1;
#else
    printf("AVX-512 not supported on this platform\n");
    return 0;
#endif
}
