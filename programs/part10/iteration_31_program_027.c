/* test_avx512_blend.c - Coverage test for AVX-512 blend RTL patterns */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Alignment attribute for all vector data */
#define ALIGN_64 __attribute__((aligned(64)))

/* Feature detection and fallback */
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

/* Volatile variable to prevent optimization */
static volatile int g_volatile_counter = 0;

/* ==================== V64QI (64x8-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v64qi_blend(int argc) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 uint8_t dst[64];
    volatile ALIGN_64 uint8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (uint8_t)(i * 3);
        src2[i] = (uint8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask: select elements where (i % 3) == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 3) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend using intrinsic that should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile array to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Use result in computation */
    uint64_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Blend with broadcasted scalar */
    __m512i broadcast = _mm512_set1_epi8(0xAA);
    __m512i result2 = _mm512_mask_blend_epi8(mask, v1, broadcast);
    _mm512_store_si512((__m512i*)dst, result2);
    
    /* Artificial dependency through asm */
    uint64_t asm_sum = sum;
    __asm__ volatile("" : "+r"(asm_sum) : : "memory");
    
    /* Loop with argc-dependent iterations */
    int loop_count = (argc > 1) ? (argc % 10) + 1 : 5;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_load_si512((const __m512i*)src1);
        __m512i blended = _mm512_mask_blend_epi8(mask, temp, 
            _mm512_add_epi8(temp, _mm512_set1_epi8(iter)));
        _mm512_store_si512((__m512i*)dst, blended);
        
        for (int i = 0; i < 8; i++) {
            asm_sum += dst[i * 8];
        }
    }
    
    return asm_sum + g_volatile_counter;
}
#else
static uint64_t test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI (32x16-bit integers) ==================== */
#if HAS_AVX512BW
static uint64_t test_v32hi_blend(int argc) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    volatile ALIGN_64 uint16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)(i * 100);
        src2[i] = (uint16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison intrinsic */
    __m512i cmp_val = _mm512_set1_epi16(500);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Reduction */
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi16(v1, _mm512_set1_epi16(3));
    __m512i blended_arith = _mm512_mask_blend_epi16(mask, v1, multiplied);
    _mm512_store_si512((__m512i*)dst, blended_arith);
    
    for (int i = 0; i < 32; i += 2) {
        sum += dst[i];
    }
    
    /* Loop with volatile dependency */
    for (int i = 0; i < g_volatile_counter % 4; i++) {
        __m512i temp = _mm512_load_si512((const __m512i*)src1);
        __m512i blend_result = _mm512_mask_blend_epi16(
            mask, 
            temp, 
            _mm512_slli_epi16(temp, 1)
        );
        _mm512_store_si512((__m512i*)dst, blend_result);
        sum += dst[0];
    }
    
    return sum;
}
#else
static uint64_t test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32x half-precision floats) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static uint64_t test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    volatile ALIGN_64 _Float16 volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f + 0.5f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using float comparison */
    __m512h cmp_val = _mm512_set1_ph(20.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512bw_blendmv32hf */
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_store_ph((void*)volatile_dst, result);
    
    /* Compute checksum */
    uint64_t sum = 0;
    union { _Float16 f; uint16_t u; } converter;
    for (int i = 0; i < 32; i++) {
        converter.f = volatile_dst[i];
        sum += converter.u;
    }
    
    /* Blend with scalar broadcast */
    __m512h scalar = _mm512_set1_ph(10.0f);
    __m512h blended_scalar = _mm512_mask_blend_ph(mask, v1, scalar);
    _mm512_store_ph(dst, blended_scalar);
    
    for (int i = 0; i < 32; i += 4) {
        converter.f = dst[i];
        sum += converter.u;
    }
    
    return sum + argc;
}
#else
static uint64_t test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
#include <bfloat16.h>
static uint64_t test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    ALIGN_64 __bfloat16 dst[32];
    volatile ALIGN_64 __bfloat16 volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = bfloat16_from_float(i * 1.25f);
        src2[i] = bfloat16_from_float(i * 2.75f + 0.25f);
    }
    
    /* Load as integers since direct bfloat16 blend intrinsic may not exist */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create pattern mask */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 4 == 0) mask |= (1U << i);
    }
    
    /* Use epi16 blend on bfloat16 data - should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (uint16_t)volatile_dst[i];
    }
    
    /* Additional blend with arithmetic */
    __m512i added = _mm512_add_epi16(v1, _mm512_set1_epi16(0x1000));
    __m512i blended_arith = _mm512_mask_blend_epi16(mask, v1, added);
    _mm512_store_si512((__m512i*)dst, blended_arith);
    
    for (int i = 0; i < 32; i += 8) {
        sum += (uint16_t)dst[i];
    }
    
    return sum + argc;
}
#else
static uint64_t test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI (16x32-bit integers) ==================== */
#if HAS_AVX512F
static uint64_t test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    volatile ALIGN_64 int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    _mm512_store_epi32((void*)volatile_dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += (uint64_t)volatile_dst[i];
    }
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi32(0xDEADBEEF);
    __m512i blended_scalar = _mm512_mask_blend_epi32(mask, v1, broadcast);
    _mm512_store_epi32(dst, blended_scalar);
    
    for (int i = 0; i < 16; i++) {
        sum ^= (uint64_t)dst[i];
    }
    
    /* Loop with argc-dependent iterations */
    int iterations = (argc > 0) ? (argc % 8) + 2 : 3;
    for (int iter = 0; iter < iterations; iter++) {
        __m512i temp = _mm512_load_si512((const __m512i*)src1);
        __m512i shifted = _mm512_slli_epi32(temp, iter);
        __m512i blended = _mm512_mask_blend_epi32(mask, temp, shifted);
        _mm512_store_si512((__m512i*)dst, blended);
        
        sum += dst[iter % 16];
    }
    
    return sum;
}
#else
static uint64_t test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI (8x64-bit integers) ==================== */
#if HAS_AVX512F
static uint64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    volatile ALIGN_64 int64_t volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 10000LL;
        src2[i] = (int64_t)i * 20000LL + 5000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) mask |= (1 << i);
    }
    
    /* Should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    _mm512_store_epi64((void*)volatile_dst, result);
    
    uint64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += (uint64_t)volatile_dst[i];
    }
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi64(v1, _mm512_set1_epi64(3));
    __m512i blended_arith = _mm512_mask_blend_epi64(mask, v1, multiplied);
    _mm512_store_epi64(dst, blended_arith);
    
    for (int i = 0; i < 8; i++) {
        sum ^= (uint64_t)dst[i];
    }
    
    return sum + argc;
}
#else
static uint64_t test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF (8x double-precision floats) ==================== */
#if HAS_AVX512F
static uint64_t test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    volatile ALIGN_64 double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (double)i * 1.25;
        src2[i] = (double)i * 2.75 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using float comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    _mm512_store_pd((void*)volatile_dst, result);
    
    /* Compute integer checksum from double bits */
    uint64_t sum = 0;
    union { double d; uint64_t u; } converter;
    for (int i = 0; i < 8; i++) {
        converter.d = volatile_dst[i];
        sum += converter.u;
    }
    
    /* Blend with scalar */
    __m512d scalar = _mm512_set1_pd(3.14159);
    __m512d blended_scalar = _mm512_mask_blend_pd(mask, v1, scalar);
    _mm512_store_pd(dst, blended_scalar);
    
    for (int i = 0; i < 8; i++) {
        converter.d = dst[i];
        sum ^= converter.u;
    }
    
    /* Loop with volatile dependency */
    for (int i = 0; i < (g_volatile_counter % 3); i++) {
        __m512d temp = _mm512_load_pd(src1);
        __m512d multiplied = _mm512_mul_pd(temp, _mm512_set1_pd(1.1));
        __m512d blended = _mm512_mask_blend_pd(mask, temp, multiplied);
        _mm512_store_pd(dst, blended);
        converter.d = dst[0];
        sum += converter.u;
    }
    
    return sum;
}
#else
static uint64_t test_v8df_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SF (16x single-precision floats) ==================== */
#if HAS_AVX512F
static uint64_t test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = (float)i * 1.5f;
        src2[i] = (float)i * 3.0f + 0.5f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(10.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    _mm512_store_ps((void*)volatile_dst, result);
    
    /* Compute checksum */
    uint64_t sum = 0;
    union { float f; uint32_t u; } converter;
    for (int i = 0; i < 16; i++) {
        converter.f = volatile_dst[i];
        sum += converter.u;
    }
    
    /* Blend with arithmetic result */
    __m512 multiplied = _mm512_mul_ps(v1, _mm512_set1_ps(2.0f));
    __m512 blended_arith = _mm512_mask_blend_ps(mask, v1, multiplied);
    _mm512_store_ps(dst, blended_arith);
    
    for (int i = 0; i < 16; i += 4) {
        converter.f = dst[i];
        sum ^= (uint64_t)converter.u;
    }
    
    /* argc-dependent loop */
    int loop_count = (argc > 0) ? (argc % 5) + 1 : 2;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512 temp = _mm512_load_ps(src1);
        __m512 added = _mm512_add_ps(temp, _mm512_set1_ps((float)iter));
        __m512 blended = _mm512_mask_blend_ps(mask, temp, added);
        _mm512_store_ps(dst, blended);
        
        converter.f = dst[iter % 16];
        sum += converter.u;
    }
    
    return sum;
}
#else
static uint64_t test_v16sf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    uint64_t total_hash = 0;
    
    printf("AVX-512 Blend Coverage Test\n");
    printf("===========================\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected:\n");
    #if HAS_AVX512F
    printf("  AVX-512F: YES\n");
    #endif
    #if HAS_AVX512BW
    printf("  AVX-512BW: YES\n");
    #endif
    
    /* Update volatile counter */
    g_volatile_counter = argc;
    
    /* Run all blend tests */
    printf("\nRunning blend tests...\n");
    
    #if HAS_AVX512BW
    total_hash ^= test_v64qi_blend(argc);
    printf("  V64QI test completed\n");
    
    total_hash ^= test_v32hi_blend(argc);
    printf("  V32HI test completed\n");
    
    #if defined(__AVX512FP16__)
    total_hash ^= test_v32hf_blend(argc);
    printf("  V32HF test completed\n");
    #else
    printf("  V32HF test skipped (AVX512-FP16 not enabled)\n");
    #endif
    
    #if defined(__AVX512BF16__)
    total_hash ^= test_v32bf_blend(argc);
    printf("  V32BF test completed\n");
    #else
    printf("  V32BF test skipped (AVX512-BF16 not enabled)\n");
    #endif
    #endif
    
    #if HAS_AVX512F
    total_hash ^= test_v16si_blend(argc);
    printf("  V16SI test completed\n");
    
    total_hash ^= test_v8di_blend(argc);
    printf("  V8DI test completed\n");
    
    total_hash ^= test_v8df_blend(argc);
    printf("  V8DF test completed\n");
    
    total_hash ^= test_v16sf_blend(argc);
    printf("  V16SF test completed\n");
    #endif
    
    printf("\nFinal hash: 0x%016llx\n", (unsigned long long)total_hash);
    
    /* Use result to affect return code (prevents dead code elimination) */
    return (int)(total_hash % 256);
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
}
