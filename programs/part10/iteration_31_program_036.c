/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines in i386-expand.cc (lines 4303-4326)
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw test_avx512_blend.c -o test_avx512_blend
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __AVX512F__
#ifdef __AVX512BW__

/* V64QI mode - requires AVX512BW */
static int test_v64qi_blend(volatile int argc) {
    __attribute__((aligned(64))) int8_t src1[64];
    __attribute__((aligned(64))) int8_t src2[64];
    __attribute__((aligned(64))) int8_t dst[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = (int8_t)(i * 3);
        src2[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison - alternating pattern */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 3 == 0) mask |= (1ULL << i);
    }
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile array to prevent optimization */
    _mm512_store_si512((__m512i*)dst, result);
    
    /* Use result in computation */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += dst[i];
    }
    
    /* Loop with argc dependency */
    for (int i = 0; i < (argc & 3); i++) {
        result = _mm512_mask_blend_epi8(mask, result, v1);
        _mm512_store_si512((__m512i*)dst, result);
    }
    
    return sum;
}

/* V32HI mode - requires AVX512BW */
static int test_v32hi_blend(volatile int argc) {
    __attribute__((aligned(64))) int16_t src1[32];
    __attribute__((aligned(64))) int16_t src2[32];
    __attribute__((aligned(64))) int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 7);
        src2[i] = (int16_t)(i * 11 + 2);
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison intrinsic */
    __m512i cmp_val = _mm512_set1_epi16(100);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger gen_avx512bw_blendmv32hi */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Blend with broadcast scalar */
    __m512i scalar = _mm512_set1_epi16(42);
    result = _mm512_mask_blend_epi16(mask ^ 0xAAAAAAAA, result, scalar);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency to prevent optimization */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V32HF mode - half precision float - requires AVX512BW */
static int test_v32hf_blend(volatile int argc) {
    __attribute__((aligned(64))) uint16_t src1[32];  /* _Float16 stored as uint16_t */
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    /* Initialize with simple pattern */
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 0x0400;  /* Approx i * 1.0 in half precision */
        src2[i] = i * 0x0C00;  /* Approx i * 3.0 in half precision */
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask - select elements where i is even */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 2) == 0) mask |= (1U << i);
    }
    
    /* Blend using integer blend intrinsic on half-float representation
     * Should trigger gen_avx512bw_blendmv32hf */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Alternative using _Float16 if available */
    #ifdef __STDC_IEC_60559_BFP__
    __attribute__((aligned(64))) _Float16 fsrc1[32];
    __attribute__((aligned(64))) _Float16 fsrc2[32];
    __attribute__((aligned(64))) _Float16 fdst[32];
    
    for (int i = 0; i < 32; i++) {
        fsrc1[i] = (_Float16)i;
        fsrc2[i] = (_Float16)(i * 2.0f);
    }
    
    __m512h fv1 = _mm512_load_ph(fsrc1);
    __m512h fv2 = _mm512_load_ph(fsrc2);
    
    /* Blend half precision floats directly */
    __m512h fresult = _mm512_mask_blend_ph(mask, fv1, fv2);
    _mm512_store_ph(fdst, fresult);
    #endif
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V32BF mode - bfloat16 - requires AVX512BW */
static int test_v32bf_blend(volatile int argc) {
    __attribute__((aligned(64))) uint16_t src1[32];  /* bfloat16 stored as uint16_t */
    __attribute__((aligned(64))) uint16_t src2[32];
    __attribute__((aligned(64))) uint16_t dst[32];
    
    /* Initialize bfloat16 pattern */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 pattern: 1.0, 2.0, 3.0, ... */
        src1[i] = (0x3F80 + i);  /* 1.0 + i in bfloat16 */
        src2[i] = (0x4000 + i);  /* 2.0 + i in bfloat16 */
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison */
    __mmask32 mask = 0x55555555;  /* Alternating pattern */
    
    /* Blend bfloat16 using integer blend intrinsic
     * Should trigger gen_avx512bw_blendmv32bf */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512i add_result = _mm512_add_epi16(v1, _mm512_set1_epi16(0x0400));
    result = _mm512_mask_blend_epi16(mask ^ 0xAAAAAAAA, result, add_result);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */

/* V16SI mode - requires AVX512F */
static int test_v16si_blend(volatile int argc) {
    __attribute__((aligned(64))) int32_t src1[16];
    __attribute__((aligned(64))) int32_t src2[16];
    __attribute__((aligned(64))) int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 13;
        src2[i] = i * 17 + 3;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask using comparison intrinsic */
    __m512i cmp_val = _mm512_set1_epi32(50);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv16si */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Use in loop with argc dependency */
    for (int i = 0; i < (argc & 7); i++) {
        __m512i temp = _mm512_add_epi32(result, _mm512_set1_epi32(i));
        result = _mm512_mask_blend_epi32(mask, result, temp);
    }
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return sum;
}

/* V8DI mode - requires AVX512F */
static int test_v8di_blend(volatile int argc) {
    __attribute__((aligned(64))) int64_t src1[8];
    __attribute__((aligned(64))) int64_t src2[8];
    __attribute__((aligned(64))) int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 23LL;
        src2[i] = i * 29LL + 5;
    }
    
    __m512i v1 = _mm512_load_si512((__m512i*)src1);
    __m512i v2 = _mm512_load_si512((__m512i*)src2);
    
    /* Create mask - select first 4 elements */
    __mmask8 mask = 0x0F;  /* 00001111 */
    
    /* Blend with mask - should trigger gen_avx512f_blendmv8di */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    /* Blend with broadcast scalar */
    __m512i scalar = _mm512_set1_epi64(999);
    result = _mm512_mask_blend_epi64(mask ^ 0xAA, result, scalar);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return (int)sum;
}

/* V8DF mode - double precision float - requires AVX512F */
static int test_v8df_blend(volatile int argc) {
    __attribute__((aligned(64))) double src1[8];
    __attribute__((aligned(64))) double src2[8];
    __attribute__((aligned(64))) double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.5;
        src2[i] = i * 2.5 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask using comparison intrinsic */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_LT_OQ);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv8df */
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512d mul_result = _mm512_mul_pd(v1, _mm512_set1_pd(2.0));
    result = _mm512_mask_blend_pd(mask ^ 0xAA, result, mul_result);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return (int)sum;
}

/* V16SF mode - single precision float - requires AVX512F */
static int test_v16sf_blend(volatile int argc) {
    __attribute__((aligned(64))) float src1[16];
    __attribute__((aligned(64))) float src2[16];
    __attribute__((aligned(64))) float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.75f;
        src2[i] = i * 1.25f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison intrinsic */
    __m512 cmp_val = _mm512_set1_ps(6.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger gen_avx512f_blendmv16sf */
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Complex pattern with loop */
    for (int i = 0; i < (argc & 3); i++) {
        __m512 add_result = _mm512_add_ps(result, _mm512_set1_ps(1.0f));
        result = _mm512_mask_blend_ps(mask, result, add_result);
        
        /* Rotate mask */
        mask = (mask << 1) | (mask >> 15);
    }
    
    _mm512_store_ps(dst, result);
    
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    return (int)sum;
}

#endif /* __AVX512F__ */

int main(int argc, char **argv) {
    int total_result = 0;
    
    #ifdef __AVX512F__
    printf("AVX512F detected, testing blends...\n");
    
    /* Seed RNG for reproducible results */
    srand(42);
    
    /* Make argc volatile to prevent constant propagation */
    volatile int vol_argc = argc;
    
    /* Test AVX512F blends */
    total_result += test_v16si_blend(vol_argc);
    total_result += test_v8di_blend(vol_argc);
    total_result += test_v8df_blend(vol_argc);
    total_result += test_v16sf_blend(vol_argc);
    
    #ifdef __AVX512BW__
    printf("AVX512BW detected, testing byte/word blends...\n");
    
    /* Test AVX512BW blends */
    total_result += test_v64qi_blend(vol_argc);
    total_result += test_v32hi_blend(vol_argc);
    total_result += test_v32hf_blend(vol_argc);
    total_result += test_v32bf_blend(vol_argc);
    #else
    printf("AVX512BW not available, skipping byte/word blend tests\n");
    #endif /* __AVX512BW__ */
    
    #else
    printf("AVX512 not available on this platform\n");
    printf("Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw\n");
    return 1;
    #endif /* __AVX512F__ */
    
    printf("Total checksum: %d\n", total_result);
    
    /* Use result to affect return code */
    return (total_result != 0) ? 0 : 1;
}
