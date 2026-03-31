/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines in i386-expand.cc (4303-4326)
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw test_avx512_blend.c -o test_avx512_blend
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* Force alignment for all arrays */
#define ALIGN_64 __attribute__((aligned(64)))

/* Volatile variables to prevent optimization */
static volatile int g_volatile_counter = 0;

/* Test function for V64QImode - uses _mm512_mask_blend_epi8 */
static int test_v64qi_blend(int seed) {
    ALIGN_64 uint8_t src1[64];
    ALIGN_64 uint8_t src2[64];
    ALIGN_64 uint8_t dst[64];
    volatile ALIGN_64 uint8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (uint8_t)(i + seed);
        src2[i] = (uint8_t)(255 - i - seed);
    }
    
    /* Load vectors */
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask: alternate pattern */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i + seed) % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Perform blend - should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store with volatile to prevent optimization */
    _mm512_storeu_si512((__m512i*)volatile_dst, result);
    
    /* Use result in computation */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* Test function for V32HImode - uses _mm512_mask_blend_epi16 */
static int test_v32hi_blend(int seed) {
    ALIGN_64 uint16_t src1[32];
    ALIGN_64 uint16_t src2[32];
    volatile ALIGN_64 uint16_t volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)(i * 10 + seed);
        src2[i] = (uint16_t)(65535 - i * 10 - seed);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi16(100);
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, cmp_val, _MM_CMPINT_GT);
    
    /* Blend with broadcasted scalar */
    __m512i broadcast = _mm512_set1_epi16(0x7FFF);
    __m512i result = _mm512_mask_blend_epi16(mask, v1, broadcast);
    
    /* Store volatile */
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    /* Reduction */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    return sum;
}

/* Test function for V32HFmode - uses _mm512_mask_blend_ph */
static int test_v32hf_blend(int seed) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    volatile ALIGN_64 _Float16 volatile_dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 0.5f + seed);
        src2[i] = (_Float16)(100.0f - i * 0.5f - seed);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask using FP comparison */
    __m512h cmp_val = _mm512_set1_ph(25.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with arithmetic result */
    __m512h add_result = _mm512_add_ph(v1, _mm512_set1_ph(10.0f));
    __m512h result = _mm512_mask_blend_ph(mask, v1, add_result);
    
    _mm512_store_ph((_Float16*)volatile_dst, result);
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += (int)volatile_dst[i];
    }
    
    return sum;
}

/* Test function for V32BFmode - uses _mm512_mask_blend_epi16 on bfloat16 */
static int test_v32bf_blend(int seed) {
    ALIGN_64 uint16_t src1[32];  /* bfloat16 as uint16_t */
    ALIGN_64 uint16_t src2[32];
    volatile ALIGN_64 uint16_t volatile_dst[32];
    
    /* Simulate bfloat16 values */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16-like pattern */
        src1[i] = (uint16_t)((i + seed) << 8);
        src2[i] = (uint16_t)(((31 - i + seed) << 8) | 0x7F);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((src1[i] & 0x8000) == 0) {  /* Positive numbers */
            mask |= (1U << i);
        }
    }
    
    /* Blend using epi16 intrinsic */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    _mm512_store_si512((__m512i*)volatile_dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += volatile_dst[i];
    }
    
    return sum;
}

/* Test function for V16SImode - uses _mm512_mask_blend_epi32 */
static int test_v16si_blend(int seed) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    volatile ALIGN_64 int32_t volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 100 + seed;
        src2[i] = 10000 - i * 100 - seed;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(500);
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, cmp_val, _MM_CMPINT_LT);
    
    /* Blend in loop to prevent optimization */
    __m512i result = v1;
    for (int i = 0; i < 3; i++) {
        result = _mm512_mask_blend_epi32(mask, result, v2);
        mask = ~mask;  /* Flip mask */
    }
    
    _mm512_store_epi32((int*)volatile_dst, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += volatile_dst[i];
    }
    
    return sum;
}

/* Test function for V8DImode - uses _mm512_mask_blend_epi64 */
static long long test_v8di_blend(int seed) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    volatile ALIGN_64 int64_t volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = (int64_t)i * 1000LL + seed;
        src2[i] = 100000LL - (int64_t)i * 1000LL - seed;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if ((seed + i) % 2 == 0) {
            mask |= (1 << i);
        }
    }
    
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    _mm512_store_epi64((long long*)volatile_dst, result);
    
    long long sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += volatile_dst[i];
    }
    
    return sum;
}

/* Test function for V8DFmode - uses _mm512_mask_blend_pd */
static double test_v8df_blend(int seed) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    volatile ALIGN_64 double volatile_dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5 + seed * 0.1;
        src2[i] = 100.0 - i * 1.5 - seed * 0.1;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using FP comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with arithmetic operation result */
    __m512d mul_result = _mm512_mul_pd(v1, _mm512_set1_pd(2.0));
    __m512d result = _mm512_mask_blend_pd(mask, v1, mul_result);
    
    _mm512_store_pd((double*)volatile_dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += volatile_dst[i];
    }
    
    return sum;
}

/* Test function for V16SFmode - uses _mm512_mask_blend_ps */
static float test_v16sf_blend(int seed) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    volatile ALIGN_64 float volatile_dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.25f + seed * 0.01f;
        src2[i] = 50.0f - i * 0.25f - seed * 0.01f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask */
    __m512 cmp_val = _mm512_set1_ps(2.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Multiple blends in loop */
    __m512 result = v1;
    int loop_count = g_volatile_counter % 4 + 1;
    for (int i = 0; i < loop_count; i++) {
        result = _mm512_mask_blend_ps(mask, result, v2);
        mask = _mm512_cmp_ps_mask(result, cmp_val, _CMP_GT_OQ);
    }
    
    _mm512_store_ps((float*)volatile_dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += volatile_dst[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */
#endif /* __AVX512F__ */

int main(int argc, char *argv[]) {
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    long long total_hash = 0;
    
#ifdef __AVX512F__
#ifdef __AVX512BW__
    
    printf("Testing AVX-512 blend intrinsics to cover i386-expand.cc lines 4303-4326\n");
    
    /* Call all test functions multiple times with different seeds */
    for (int iter = 0; iter < 3; iter++) {
        int current_seed = seed + iter * 100;
        
        total_hash += test_v64qi_blend(current_seed);
        total_hash += test_v32hi_blend(current_seed);
        total_hash += test_v32hf_blend(current_seed);
        total_hash += test_v32bf_blend(current_seed);
        total_hash += test_v16si_blend(current_seed);
        total_hash += test_v8di_blend(current_seed);
        total_hash += (long long)test_v8df_blend(current_seed);
        total_hash += (long long)test_v16sf_blend(current_seed);
        
        /* Modify volatile to affect control flow */
        g_volatile_counter++;
    }
    
    printf("Final hash: %lld\n", total_hash);
    
#else
    printf("AVX-512BW not supported on this platform\n");
    return 1;
#endif
#else
    printf("AVX-512F not supported on this platform\n");
    return 1;
#endif
    
    return 0;
}
