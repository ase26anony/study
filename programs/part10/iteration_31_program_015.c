/* test_avx512_blend.c - Coverage for AVX-512 blend RTL expansion patterns */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Feature guards for compilation on non-AVX-512 systems */
#ifdef __AVX512F__
#ifdef __AVX512BW__

/* Helper for bfloat16 - using uint16_t representation */
typedef uint16_t bfloat16;

/* Volatile variables to prevent optimization */
static volatile int g_loop_count = 100;

/* ==================== V64QI (64 x int8) ==================== */
int test_v64qi_blend(void) {
    __attribute__((aligned(64))) int8_t src1[64];
    __attribute__((aligned(64))) int8_t src2[64];
    __attribute__((aligned(64))) int8_t dst[64];
    volatile __attribute__((aligned(64))) int8_t volatile_dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_loadu_si512((const __m512i*)src1);
    __m512i v2 = _mm512_loadu_si512((const __m512i*)src2);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(128);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_dst, blended);
    
    /* Also use in computation */
    __m512i add_result = _mm512_add_epi8(blended, _mm512_set1_epi8(1));
    _mm512_store_si512((__m512i*)dst, add_result);
    
    /* Reduction */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum & 0xFF;
}

/* ==================== V32HI (32 x int16) ==================== */
int test_v32hi_blend(void) {
    __attribute__((aligned(64))) int16_t src1[32];
    __attribute__((aligned(64))) int16_t src2[32];
    __attribute__((aligned(64))) int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 100);
        src2[i] = (int16_t)(i * 200 - 50);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask pattern */
    __mmask32 mask = 0xAAAAAAAA; /* 1010... pattern */
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Use in loop with volatile control */
    int loop = g_loop_count % 10 + 1;
    __m512i accum = _mm512_setzero_si512();
    
    for (int i = 0; i < loop; i++) {
        accum = _mm512_add_epi16(accum, blended);
        /* Blend with broadcast scalar */
        __m512i scalar = _mm512_set1_epi16(i);
        blended = _mm512_mask_blend_epi16(mask ^ 0x55555555, blended, scalar);
    }
    
    _mm512_store_si512((__m512i*)dst, accum);
    
    /* Reduction with side effect */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum & 0xFFFF;
}

/* ==================== V32HF (32 x half precision) ==================== */
int test_v32hf_blend(void) {
    __attribute__((aligned(64))) uint16_t src1[32]; /* Half as uint16 */
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    /* Simple half-float pattern */
    for (int i = 0; i < 32; i++) {
        src1[i] = 0x3C00 | (i & 0x1F); /* ~1.0 with variations */
        src2[i] = 0x4000 | (i & 0x1F); /* ~2.0 with variations */
    }
    
    __m512h v1 = _mm512_load_ph((const void*)src1);
    __m512h v2 = _mm512_load_ph((const void*)src2);
    
    /* Create mask from comparison */
    __m512h cmp_val = _mm512_set1_ph(1.5f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend - should trigger gen_avx512bw_blendmv32hf */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Store and reload for computation */
    _mm512_store_ph((void*)dst, blended);
    
    /* Convert to float for reduction */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        /* Simple half to float conversion (approximate) */
        uint16_t h = dst[i];
        int exp = (h >> 10) & 0x1F;
        int mant = h & 0x3FF;
        if (exp == 0) sum += mant / 1024.0f;
        else if (exp == 31) sum += 65504.0f;
        else sum += (1 + mant / 1024.0f) * (1 << (exp - 15));
    }
    
    return (int)sum;
}

/* ==================== V32BF (32 x bfloat16) ==================== */
int test_v32bf_blend(void) {
    __attribute__((aligned(64))) bfloat16 src1[32];
    __attribute__((aligned(64))) bfloat16 src2[32];
    __attribute__((aligned(64))) bfloat16 dst[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        /* Simple pattern: 1.0, 2.0, 3.0, ... as bfloat16 */
        uint32_t val = (i + 1) << 23; /* Float representation */
        src1[i] = (bfloat16)(val >> 16); /* Convert to bfloat16 */
        src2[i] = (bfloat16)(((i + 10) << 23) >> 16);
    }
    
    /* Load as epi16 for blending */
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create alternating mask */
    __mmask32 mask = 0x55555555; /* 0101... pattern */
    
    /* 
     * Blend using epi16 intrinsic - bfloat16 uses same width as epi16
     * Should trigger gen_avx512bw_blendmv32bf 
     */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Store result */
    _mm512_store_si512((__m512i*)dst, blended);
    
    /* Simple checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */

/* ==================== V16SI (16 x int32) ==================== */
int test_v16si_blend(void) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 1000;
        src2[i] = i * 2000 + 500;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask from comparison */
    __m512i cmp_val = _mm512_set1_epi32(8000);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512i added = _mm512_add_epi32(v1, _mm512_set1_epi32(100));
    blended = _mm512_mask_blend_epi32(mask ^ 0xFFFF, blended, added);
    
    _mm512_store_epi32(dst, blended);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return (int)sum;
}

/* ==================== V8DI (8 x int64) ==================== */
int test_v8di_blend(void) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = 1000000LL * i;
        src2[i] = 2000000LL * i + 500000;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi64(3000000);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    
    /* Use in loop */
    for (int i = 0; i < 3; i++) {
        __m512i temp = _mm512_add_epi64(blended, _mm512_set1_epi64(i * 100));
        blended = _mm512_mask_blend_epi64(mask, blended, temp);
    }
    
    _mm512_store_epi64(dst, blended);
    
    /* Compute hash */
    int64_t hash = 0;
    for (int i = 0; i < 8; i++) {
        hash ^= dst[i];
    }
    
    return (int)(hash >> 32) ^ (int)hash;
}

/* ==================== V8DF (8 x double) ==================== */
int test_v8df_blend(void) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask from comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512d multiplied = _mm512_mul_pd(blended, _mm512_set1_pd(1.1));
    blended = _mm512_mask_blend_pd(mask ^ 0xFF, blended, multiplied);
    
    _mm512_store_pd(dst, blended);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return (int)sum;
}

/* ==================== V16SF (16 x float) ==================== */
int test_v16sf_blend(void) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.5f;
        src2[i] = i * 0.75f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create complex mask pattern */
    __m512 cmp_val1 = _mm512_set1_ps(4.0f);
    __m512 cmp_val2 = _mm512_set1_ps(8.0f);
    __mmask16 mask1 = _mm512_cmp_ps_mask(v1, cmp_val1, _CMP_GT_OQ);
    __mmask16 mask2 = _mm512_cmp_ps_mask(v1, cmp_val2, _CMP_LT_OQ);
    __mmask16 mask = mask1 & mask2;
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Multiple blend operations */
    __m512 added = _mm512_add_ps(blended, _mm512_set1_ps(1.0f));
    blended = _mm512_mask_blend_ps(mask ^ 0xFFFF, blended, added);
    
    _mm512_store_ps(dst, blended);
    
    /* Final reduction with volatile store */
    volatile float vol_sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        vol_sum += dst[i];
    }
    
    return (int)vol_sum;
}

#endif /* __AVX512F__ */

/* ==================== Main Driver ==================== */
int main(int argc, char **argv) {
    int total_result = 0;
    
#ifdef __AVX512F__
    printf("AVX-512F detected, running blend tests...\n");
    
    /* Run AVX-512F tests */
    total_result ^= test_v16si_blend();
    total_result ^= test_v8di_blend();
    total_result ^= test_v8df_blend();
    total_result ^= test_v16sf_blend();
    
#ifdef __AVX512BW__
    printf("AVX-512BW detected, running byte/word tests...\n");
    
    /* Run AVX-512BW tests */
    total_result ^= test_v64qi_blend();
    total_result ^= test_v32hi_blend();
    total_result ^= test_v32hf_blend();
    total_result ^= test_v32bf_blend();
#else
    printf("AVX-512BW not available, skipping byte/word tests.\n");
#endif /* __AVX512BW__ */
    
    printf("All AVX-512 blend tests completed. Result hash: %d\n", total_result);
#else
    printf("AVX-512 not supported on this platform.\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
#endif /* __AVX512F__ */
    
    /* Use argc to affect loop counts in tests */
    if (argc > 1) {
        total_result += atoi(argv[1]);
    }
    
    return total_result & 0xFF;
}
