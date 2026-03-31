/* test_avx512_blend.c - Coverage for AVX-512 blend RTL expansion patterns */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Alignment for AVX-512 vectors */
#define ALIGN_64 __attribute__((aligned(64)))

/* Feature guards for modular compilation */
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

/* ==================== V64QI Mode (AVX512BW) ==================== */
#if HAS_AVX512BW
static int test_v64qi_blend(int argc) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 uint8_t dst[64];
    volatile ALIGN_64 uint8_t volatile_dst[64];
    
    /* Initialize with pattern data */
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
    
    /* Blend using intrinsic - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Use in computation with loop dependent on argc */
    int sum = 0;
    int loop_count = (argc > 1) ? 100 : 50;
    for (int iter = 0; iter < loop_count; iter++) {
        /* Re-blend with different mask each iteration */
        __mmask64 iter_mask = mask ^ (iter & 0xFF);
        __m512i temp = _mm512_mask_blend_epi8(iter_mask, v1, v2);
        
        /* Horizontal sum of bytes */
        __m512i sum64 = _mm512_sad_epu8(temp, _mm512_setzero_si512());
        sum += _mm512_extract_epi64(sum64, 0) +
               _mm512_extract_epi64(sum64, 1) +
               _mm512_extract_epi64(sum64, 2) +
               _mm512_extract_epi64(sum64, 3) +
               _mm512_extract_epi64(sum64, 4) +
               _mm512_extract_epi64(sum64, 5) +
               _mm512_extract_epi64(sum64, 6) +
               _mm512_extract_epi64(sum64, 7);
    }
    
    /* Artificial dependency to prevent dead code elimination */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum & 0xFF;
}
#else
static int test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI Mode (AVX512BW) ==================== */
#if HAS_AVX512BW
static int test_v32hi_blend(int argc) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    volatile ALIGN_64 uint16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)(i * 100);
        src2[i] = (uint16_t)(i * 200 + 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison intrinsic */
    __m512i cmp_val = _mm512_set1_epi16(1000);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(0xABCD);
    __mmask32 alt_mask = 0xAAAAAAAA; /* Alternating pattern */
    __m512i blended2 = _mm512_mask_blend_epi16(alt_mask, blended, broadcast);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += ((uint16_t*)&blended2)[i];
    }
    
    /* Loop with argc-dependent iteration */
    for (int i = 0; i < (argc % 10 + 1); i++) {
        __m512i temp = _mm512_mask_blend_epi16(mask ^ i, v1, v2);
        __asm__ volatile("" : : "x"(temp) : "memory");
    }
    
    return (int)(sum & 0xFFFF);
}
#else
static int test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF Mode (AVX512BW) ==================== */
#if HAS_AVX512BW && defined(__AVX512FP16__)
static int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    volatile ALIGN_64 _Float16 volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f + 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask: select elements where i is even */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i & 1) == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Blend - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    _mm512_store_ph((_Float16*)volatile_dst, blended);
    
    /* Use in arithmetic operation */
    __m512h added = _mm512_add_ph(v1, v2);
    __m512h blended_arith = _mm512_mask_blend_ph(mask, blended, added);
    
    /* Reduction */
    _Float16 sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += ((_Float16*)&blended_arith)[i];
    }
    
    __asm__ volatile("" : : "x"(blended_arith), "r"(sum) : "memory");
    
    return (int)(sum * 100);
}
#else
static int test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF Mode (AVX512BW) ==================== */
#if HAS_AVX512BW && defined(__AVX512BF16__)
#include <x86intrin.h>
static int test_v32bf_blend(int argc) {
    ALIGN_64 __bfloat16 src1[32];
    ALIGN_64 __bfloat16 src2[32];
    volatile ALIGN_64 __bfloat16 volatile_dst[32];
    
    /* Initialize bfloat16 data */
    for (int i = 0; i < 32; i++) {
        uint16_t val1 = (i * 37) & 0x7FFF;
        uint16_t val2 = (i * 73 + 0x4000) & 0x7FFF;
        src1[i] = (__bfloat16)val1;
        src2[i] = (__bfloat16)val2;
    }
    
    /* Load as integers for blending */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask */
    __mmask32 mask = 0x55555555;
    
    /* Blend using epi16 intrinsic (bfloat16 uses 16-bit lanes)
       Should trigger gen_avx512bw_blendmv32bf */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi16(v1, _mm512_set1_epi16(0x1000));
    __m512i blended2 = _mm512_mask_blend_epi16(mask ^ 0xAAAAAAAA, blended, added);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += ((uint16_t*)&blended2)[i];
    }
    
    /* argc-dependent loop */
    for (int i = 0; i < (argc % 5 + 2); i++) {
        __m512i temp = _mm512_mask_blend_epi16(mask >> i, v1, v2);
        __asm__ volatile("" : : "x"(temp) : "memory");
    }
    
    return sum & 0x7FFF;
}
#else
static int test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI Mode (AVX512F) ==================== */
#if HAS_AVX512F
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    volatile ALIGN_64 int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 - 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    _mm512_store_epi32((void*)volatile_dst, blended);
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi32(0xDEADBEEF);
    __m512i blended2 = _mm512_mask_blend_epi32(0xF0F0, blended, broadcast);
    
    /* Horizontal addition */
    __m512i sum512 = _mm512_add_epi32(blended2, _mm512_srli_epi32(blended2, 16));
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += ((int32_t*)&sum512)[i];
    }
    
    /* Loop with side effects */
    for (int i = 0; i < (argc % 8 + 4); i++) {
        __m512i temp = _mm512_mask_blend_epi32(mask ^ i, v1, v2);
        volatile_dst[i % 16] = ((int32_t*)&temp)[0];
    }
    
    return sum;
}
#else
static int test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI Mode (AVX512F) ==================== */
#if HAS_AVX512F
static int test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    volatile ALIGN_64 int64_t volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 1000000LL;
        src2[i] = (int64_t)i * 2000000LL - 500000LL;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask: select elements where (i % 2) == 0 */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((i % 2) == 0) {
            mask |= (1 << i);
        }
    }
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_epi64((void*)volatile_dst, blended);
    
    /* Blend with arithmetic result */
    __m512i multiplied = _mm512_mullo_epi64(v1, _mm512_set1_epi64(2));
    __m512i blended_arith = _mm512_mask_blend_epi64(0xAA, blended, multiplied);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += ((int64_t*)&blended_arith)[i];
    }
    
    /* argc-dependent computation */
    int iterations = (argc > 2) ? 20 : 10;
    for (int i = 0; i < iterations; i++) {
        __m512i temp = _mm512_mask_blend_epi64((mask + i) & 0xFF, v1, v2);
        __asm__ volatile("" : : "x"(temp) : "memory");
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#else
static int test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF Mode (AVX512F) ==================== */
#if HAS_AVX512F
static int test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    volatile ALIGN_64 double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.25;
        src2[i] = i * 2.75 - 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison */
    __m512d cmp = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp, _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    _mm512_store_pd(volatile_dst, blended);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(v1, _mm512_set1_pd(1.5));
    __m512d blended_arith = _mm512_mask_blend_pd(0xCC, blended, multiplied);
    
    /* Horizontal addition */
    __m512d sum512 = _mm512_add_pd(blended_arith, _mm512_permute_pd(blended_arith, 0x55));
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += ((double*)&sum512)[i];
    }
    
    /* Loop with volatile store */
    for (int i = 0; i < (argc % 6 + 3); i++) {
        __m512d temp = _mm512_mask_blend_pd(mask ^ i, v1, v2);
        volatile_dst[i % 8] = ((double*)&temp)[0];
    }
    
    return (int)(sum * 1000.0);
}
#else
static int test_v8df_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SF Mode (AVX512F) ==================== */
#if HAS_AVX512F
static int test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 1.5f - 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __m512 cmp = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    _mm512_store_ps(volatile_dst, blended);
    
    /* Blend with multiple sources */
    __m512 added = _mm512_add_ps(v1, v2);
    __m512 multiplied = _mm512_mul_ps(v1, _mm512_set1_ps(2.0f));
    __m512 blended_complex = _mm512_mask_blend_ps(0xAAAA, blended, added);
    blended_complex = _mm512_mask_blend_ps(0x5555, blended_complex, multiplied);
    
    /* Reduction */
    __m512 sum512 = _mm512_add_ps(blended_complex, _mm512_movehdup_ps(blended_complex));
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += ((float*)&sum512)[i];
    }
    
    /* argc-dependent loop with side effects */
    int loop_count = (argc > 1) ? argc * 2 : 8;
    for (int i = 0; i < loop_count; i++) {
        __m512 temp = _mm512_mask_blend_ps((mask + i) & 0xFFFF, v1, v2);
        __asm__ volatile("" : : "x"(temp) : "memory");
    }
    
    return (int)(sum * 100.0f);
}
#else
static int test_v16sf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    int total_hash = 0;
    
    printf("Testing AVX-512 blend coverage...\n");
    
#if HAS_AVX512F || HAS_AVX512BW
    printf("AVX-512 extensions detected.\n");
    
    /* Call all test functions and accumulate results */
    total_hash ^= test_v64qi_blend(argc);
    total_hash ^= test_v32hi_blend(argc);
    total_hash ^= test_v32hf_blend(argc);
    total_hash ^= test_v32bf_blend(argc);
    total_hash ^= test_v16si_blend(argc);
    total_hash ^= test_v8di_blend(argc);
    total_hash ^= test_v8df_blend(argc);
    total_hash ^= test_v16sf_blend(argc);
    
    printf("Total hash: %d\n", total_hash);
    
    /* Use result to affect return code */
    return (total_hash == 0) ? 0 : (total_hash & 255);
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
#endif
}
