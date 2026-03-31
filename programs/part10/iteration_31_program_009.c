/* test_avx512_blend.c - AVX-512 blend intrinsics coverage test */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef __AVX512F__
#include <immintrin.h>
#endif

/* Global volatile to prevent optimization */
volatile int g_volatile = 0;

/* Aligned data arrays */
#define ALIGN_64 __attribute__((aligned(64)))

/* Helper to generate pseudo-random mask values */
static inline __mmask64 generate_mask64(int seed) {
    return (__mmask64)(0xAAAAAAAAAAAAAAAAULL ^ (seed * 0x5555555555555555ULL));
}

static inline __mmask32 generate_mask32(int seed) {
    return (__mmask32)(0xAAAAAAAAUL ^ (seed * 0x55555555UL));
}

static inline __mmask16 generate_mask16(int seed) {
    return (__mmask16)(0xAAAA ^ (seed * 0x5555));
}

static inline __mmask8 generate_mask8(int seed) {
    return (__mmask8)(0xAA ^ (seed * 0x55));
}

#ifdef __AVX512BW__
/* V64QI mode - triggers gen_avx512bw_blendmv64qi */
int test_v64qi_blend(int argc) {
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
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(100);
    __mmask64 mask = _mm512_cmpgt_epi8_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger V64QImode case */
    __m512i result = _mm512_mask_blend_epi8(mask, v1, v2);
    
    /* Store to volatile array to prevent optimization */
    _mm512_storeu_si512((__m512i*)volatile_dst, result);
    
    /* Also use in reduction */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += volatile_dst[i];
    }
    
    /* Artificial dependency on argc */
    if (argc > 1) {
        __m512i broadcast = _mm512_set1_epi8(argc);
        result = _mm512_mask_blend_epi8(mask, result, broadcast);
        _mm512_storeu_si512((__m512i*)dst, result);
        
        for (int i = 0; i < 64; i++) {
            sum += dst[i];
        }
    }
    
    /* Memory barrier */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum & 0xFF;
}

/* V32HI mode - triggers gen_avx512bw_blendmv32hi */
int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 int16_t dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (int16_t)(i * 7);
        src2[i] = (int16_t)(i * 11 + 3);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Generate mask from comparison */
    __m512i cmp_val = _mm512_set1_epi16(150);
    __mmask32 mask = _mm512_cmpgt_epi16_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger V32HImode case */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Use result in arithmetic operation then blend */
    __m512i added = _mm512_add_epi16(v1, _mm512_set1_epi16(1));
    result = _mm512_mask_blend_epi16(mask ^ 0xAAAAAAAA, result, added);
    
    /* Store and compute sum */
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    /* Loop with blend inside */
    for (int iter = 0; iter < (argc & 3); iter++) {
        __m512i temp = _mm512_load_si512((const __m512i*)src1);
        __m512i blend_with_scalar = _mm512_set1_epi16(iter * 50);
        result = _mm512_mask_blend_epi16(generate_mask32(iter), temp, blend_with_scalar);
        _mm512_store_si512((const __m512i*)dst, result);
        
        for (int i = 0; i < 16; i += 4) {
            sum += dst[i];
        }
    }
    
    return sum & 0xFFFF;
}

/* V32HF mode - triggers gen_avx512bw_blendmv32hf */
#ifdef __AVX512FP16__
int test_v32hf_blend(int argc) {
    ALIGN_64 _Float16 src1[32];
    ALIGN_64 _Float16 src2[32];
    ALIGN_64 _Float16 dst[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = (_Float16)(i * 1.5f);
        src2[i] = (_Float16)(i * 2.5f + 1.0f);
    }
    
    __m512h v1 = _mm512_load_ph(src1);
    __m512h v2 = _mm512_load_ph(src2);
    
    /* Create mask */
    __m512h cmp_val = _mm512_set1_ph(20.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger V32HFmode case */
    __m512h result = _mm512_mask_blend_ph(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512h multiplied = _mm512_mul_ph(v1, _mm512_set1_ph(2.0f));
    result = _mm512_mask_blend_ph(mask ^ 0x55555555, result, multiplied);
    
    _mm512_store_ph(dst, result);
    
    /* Compute checksum */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)dst[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return (int)sum;
}
#endif /* __AVX512FP16__ */

/* V32BF mode - triggers gen_avx512bw_blendmv32bf */
/* Note: bfloat16 uses same integer representation as epi16 for blending */
int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t src1[32];  /* bfloat16 as uint16_t */
    ALIGN_64 uint16_t src2[32];
    ALIGN_64 uint16_t dst[32];
    
    /* Simple bfloat16-like pattern */
    for (int i = 0; i < 32; i++) {
        src1[i] = (uint16_t)((i + 1) << 8);
        src2[i] = (uint16_t)((i + 2) << 8);
    }
    
    __m512i v1 = _mm512_load_si512((const __m512i*)src1);
    __m512i v2 = _mm512_load_si512((const __m512i*)src2);
    
    /* Create mask - treat as 16-bit integers */
    __mmask32 mask = generate_mask32(argc);
    
    /* Blend using epi16 intrinsic - should trigger V32BFmode case */
    __m512i result = _mm512_mask_blend_epi16(mask, v1, v2);
    
    /* Additional blend with scalar */
    __m512i scalar = _mm512_set1_epi16(0x3F80);  /* bfloat16 1.0 */
    result = _mm512_mask_blend_epi16(mask ^ 0x33333333, result, scalar);
    
    _mm512_store_si512((__m512i*)dst, result);
    
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += dst[i];
    }
    
    return sum & 0xFFFF;
}
#endif /* __AVX512BW__ */

#ifdef __AVX512F__
/* V16SI mode - triggers gen_avx512f_blendmv16si */
int test_v16si_blend(int argc) {
    ALIGN_64 int32_t src1[16];
    ALIGN_64 int32_t src2[16];
    ALIGN_64 int32_t dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 13;
        src2[i] = i * 17 + 5;
    }
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Generate mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(50);
    __mmask16 mask = _mm512_cmpgt_epi32_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger V16SImode case */
    __m512i result = _mm512_mask_blend_epi32(mask, v1, v2);
    
    /* Use in loop with varying masks */
    for (int iter = 0; iter < (argc & 7); iter++) {
        __m512i temp = _mm512_load_epi32(src1);
        __m512i blend_val = _mm512_set1_epi32(iter * 100);
        result = _mm512_mask_blend_epi32(generate_mask16(iter), temp, blend_val);
    }
    
    _mm512_store_epi32(dst, result);
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += dst[i];
    }
    
    /* Memory barrier with volatile */
    g_volatile = sum;
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V8DI mode - triggers gen_avx512f_blendmv8di */
int test_v8di_blend(int argc) {
    ALIGN_64 int64_t src1[8];
    ALIGN_64 int64_t src2[8];
    ALIGN_64 int64_t dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 23LL;
        src2[i] = i * 29LL + 7;
    }
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __m512i cmp_val = _mm512_set1_epi64(100);
    __mmask8 mask = _mm512_cmpgt_epi64_mask(v1, cmp_val);
    
    /* Blend with mask - should trigger V8DImode case */
    __m512i result = _mm512_mask_blend_epi64(mask, v1, v2);
    
    /* Blend result with arithmetic operation */
    __m512i multiplied = _mm512_mullo_epi64(v1, _mm512_set1_epi64(3));
    result = _mm512_mask_blend_epi64(mask ^ 0xAA, result, multiplied);
    
    _mm512_store_epi64(dst, result);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    return (int)(sum & 0xFFFFFFFF);
}

/* V8DF mode - triggers gen_avx512f_blendmv8df */
int test_v8df_blend(int argc) {
    ALIGN_64 double src1[8];
    ALIGN_64 double src2[8];
    ALIGN_64 double dst[8];
    
    for (int i = 0; i < 8; i++) {
        src1[i] = i * 1.25;
        src2[i] = i * 2.75 + 0.5;
    }
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Generate mask using floating comparison */
    __m512d cmp_val = _mm512_set1_pd(5.0);
    __mmask8 mask = _mm512_cmp_pd_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger V8DFmode case */
    __m512d result = _mm512_mask_blend_pd(mask, v1, v2);
    
    /* Blend with arithmetic result */
    __m512d sqrt_val = _mm512_sqrt_pd(v1);
    result = _mm512_mask_blend_pd(mask ^ 0x55, result, sqrt_val);
    
    _mm512_store_pd(dst, result);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += dst[i];
    }
    
    /* Volatile store */
    volatile double vol_sum = sum;
    __asm__ volatile("" : : "r"(vol_sum) : "memory");
    
    return (int)sum;
}

/* V16SF mode - triggers gen_avx512f_blendmv16sf */
int test_v16sf_blend(int argc) {
    ALIGN_64 float src1[16];
    ALIGN_64 float src2[16];
    ALIGN_64 float dst[16];
    
    for (int i = 0; i < 16; i++) {
        src1[i] = i * 0.75f;
        src2[i] = i * 1.5f + 0.25f;
    }
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask */
    __m512 cmp_val = _mm512_set1_ps(8.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(v1, cmp_val, _CMP_GT_OQ);
    
    /* Blend with mask - should trigger V16SFmode case */
    __m512 result = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Complex sequence with multiple blends */
    __m512 added = _mm512_add_ps(v1, _mm512_set1_ps(1.0f));
    result = _mm512_mask_blend_ps(mask ^ 0xAAAA, result, added);
    
    __m512 multiplied = _mm512_mul_ps(v1, _mm512_set1_ps(2.0f));
    result = _mm512_mask_blend_ps(generate_mask16(argc), result, multiplied);
    
    _mm512_store_ps(dst, result);
    
    /* Reduction with loop dependency on argc */
    float sum = 0.0f;
    for (int i = 0; i < (argc % 16); i++) {
        sum += dst[i];
    }
    for (int i = (argc % 16); i < 16; i++) {
        sum += dst[i] * 0.5f;
    }
    
    return (int)sum;
}
#endif /* __AVX512F__ */

/* Main driver function */
int main(int argc, char *argv[]) {
    int total_result = 0;
    
    printf("AVX-512 Blend Coverage Test\n");
    
#ifdef __AVX512F__
    printf("AVX512F supported\n");
    
    /* Call all AVX512F test functions */
    total_result ^= test_v16si_blend(argc);
    total_result ^= test_v8di_blend(argc);
    total_result ^= test_v8df_blend(argc);
    total_result ^= test_v16sf_blend(argc);
    
#ifdef __AVX512BW__
    printf("AVX512BW supported\n");
    
    /* Call all AVX512BW test functions */
    total_result ^= test_v64qi_blend(argc);
    total_result ^= test_v32hi_blend(argc);
    total_result ^= test_v32bf_blend(argc);
    
#ifdef __AVX512FP16__
    printf("AVX512FP16 supported\n");
    total_result ^= test_v32hf_blend(argc);
#else
    printf("AVX512FP16 not supported, skipping V32HF test\n");
#endif /* __AVX512FP16__ */
    
#else
    printf("AVX512BW not supported, skipping byte/word tests\n");
#endif /* __AVX512BW__ */
    
#else
    printf("AVX512 not supported, all tests skipped\n");
    return 0;
#endif /* __AVX512F__ */
    
    printf("Final checksum: %d\n", total_result);
    
    /* Use result to affect return code */
    return (total_result == 0) ? 0 : 1;
}
