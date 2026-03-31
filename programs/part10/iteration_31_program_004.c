/* test_avx512_blend.c - Coverage for AVX-512 blend RTL patterns */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Alignment for AVX-512 vectors */
#define ALIGN_64 __attribute__((aligned(64)))

/* Global volatile to prevent optimization */
volatile int g_loop_count = 4;

#ifdef __AVX512F__

/* V16SF - 16 single-precision floats */
static float test_v16sf_blend(int argc) {
    ALIGN_64 float a[16] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                            9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    ALIGN_64 float b[16] = {16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
                            8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    ALIGN_64 volatile float result[16];
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    /* Create mask using comparison */
    __m512 vcmp = _mm512_set1_ps(8.5f);
    __mmask16 mask = _mm512_cmp_ps_mask(va, vcmp, _CMP_GT_OQ);
    
    /* Blend based on mask */
    __m512 vblend = _mm512_mask_blend_ps(mask, va, vb);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_ps((void*)result, vblend);
    
    /* Use in reduction */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Loop with argc dependency */
    int loops = argc > 1 ? g_loop_count : 2;
    for (int i = 0; i < loops; i++) {
        __m512 vtmp = _mm512_add_ps(va, _mm512_set1_ps(i * 1.0f));
        __m512 vblend2 = _mm512_mask_blend_ps(mask ^ 0xAAAA, vtmp, vb);
        __asm__ volatile("" : : "x"(vblend2) : "memory");
    }
    
    return sum;
}

/* V8DF - 8 double-precision floats */
static double test_v8df_blend(int argc) {
    ALIGN_64 double a[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    ALIGN_64 double b[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    ALIGN_64 volatile double result[8];
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Create mask */
    __m512d vcmp = _mm512_set1_pd(4.5);
    __mmask8 mask = _mm512_cmp_pd_mask(va, vcmp, _CMP_GT_OQ);
    
    /* Blend with mask */
    __m512d vblend = _mm512_mask_blend_pd(mask, va, vb);
    
    /* Store to volatile */
    _mm512_store_pd((void*)result, vblend);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Blend with broadcast scalar */
    __m512d vscalar = _mm512_set1_pd(100.0);
    __m512d vblend2 = _mm512_mask_blend_pd(mask ^ 0xAA, vblend, vscalar);
    __asm__ volatile("" : : "x"(vblend2) : "memory");
    
    return sum;
}

/* V16SI - 16 32-bit integers */
static int32_t test_v16si_blend(int argc) {
    ALIGN_64 int32_t a[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    ALIGN_64 int32_t b[16] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    ALIGN_64 volatile int32_t result[16];
    
    __m512i va = _mm512_load_epi32(a);
    __m512i vb = _mm512_load_epi32(b);
    
    /* Create mask using comparison */
    __m512i vcmp = _mm512_set1_epi32(8);
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vcmp, _MM_CMPINT_GT);
    
    /* Blend with mask */
    __m512i vblend = _mm512_mask_blend_epi32(mask, va, vb);
    
    /* Store to volatile */
    _mm512_store_epi32((void*)result, vblend);
    
    /* Reduction */
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Blend with arithmetic result */
    __m512i vadd = _mm512_add_epi32(va, _mm512_set1_epi32(argc));
    __m512i vblend2 = _mm512_mask_blend_epi32(mask ^ 0x5555, vadd, vb);
    __asm__ volatile("" : : "x"(vblend2) : "memory");
    
    return sum;
}

/* V8DI - 8 64-bit integers */
static int64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    ALIGN_64 int64_t b[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    ALIGN_64 volatile int64_t result[8];
    
    __m512i va = _mm512_load_epi64(a);
    __m512i vb = _mm512_load_epi64(b);
    
    /* Create mask */
    __m512i vcmp = _mm512_set1_epi64(4);
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vcmp, _MM_CMPINT_GT);
    
    /* Blend with mask */
    __m512i vblend = _mm512_mask_blend_epi64(mask, va, vb);
    
    /* Store to volatile */
    _mm512_store_epi64((void*)result, vblend);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    return sum;
}

#endif /* __AVX512F__ */

#ifdef __AVX512BW__

/* V64QI - 64 8-bit integers */
static int8_t test_v64qi_blend(int argc) {
    ALIGN_64 int8_t a[64];
    ALIGN_64 int8_t b[64];
    ALIGN_64 volatile int8_t result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 63 - i;
    }
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask - alternate pattern */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if (i % 3 == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend with mask */
    __m512i vblend = _mm512_mask_blend_epi8(mask, va, vb);
    
    /* Store to volatile */
    _mm512_store_si512((void*)result, vblend);
    
    /* Reduction */
    int8_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Loop with blend inside */
    int loops = argc > 1 ? g_loop_count : 3;
    for (int i = 0; i < loops; i++) {
        __m512i vtmp = _mm512_add_epi8(va, _mm512_set1_epi8(i));
        __m512i vblend2 = _mm512_mask_blend_epi8(mask ^ 0xAAAAAAAAAAAAAAAAULL, vtmp, vb);
        __asm__ volatile("" : : "x"(vblend2) : "memory");
    }
    
    return sum;
}

/* V32HI - 32 16-bit integers */
static int16_t test_v32hi_blend(int argc) {
    ALIGN_64 int16_t a[32];
    ALIGN_64 int16_t b[32];
    ALIGN_64 volatile int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 2;
        b[i] = (31 - i) * 2;
    }
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask using comparison */
    __m512i vcmp = _mm512_set1_epi16(30);
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vcmp, _MM_CMPINT_LT);
    
    /* Blend with mask */
    __m512i vblend = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Store to volatile */
    _mm512_store_si512((void*)result, vblend);
    
    /* Reduction */
    int16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* V32HF - 32 half-precision floats */
static __fp16 test_v32hf_blend(int argc) {
    ALIGN_64 __fp16 a[32];
    ALIGN_64 __fp16 b[32];
    ALIGN_64 volatile __fp16 result[32];
    
    /* Initialize half-precision values */
    for (int i = 0; i < 32; i++) {
        a[i] = (__fp16)(i * 0.5f);
        b[i] = (__fp16)((31 - i) * 0.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Create mask */
    __m512h vcmp = _mm512_set1_ph((__fp16)8.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(va, vcmp, _CMP_LT_OQ);
    
    /* Blend with mask */
    __m512h vblend = _mm512_mask_blend_ph(mask, va, vb);
    
    /* Store to volatile */
    _mm512_store_ph((void*)result, vblend);
    
    /* Reduction */
    __fp16 sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* V32BF - 32 bfloat16 floats */
static uint16_t test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t a[32];  /* bfloat16 as uint16_t */
    ALIGN_64 uint16_t b[32];
    ALIGN_64 volatile uint16_t result[32];
    
    /* Initialize bfloat16 patterns (simple integer representation) */
    for (int i = 0; i < 32; i++) {
        a[i] = i << 8;  /* Simple pattern */
        b[i] = (31 - i) << 8;
    }
    
    /* Load as integers for blending */
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask - alternate pattern */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 2 == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Blend using epi16 intrinsic (bfloat16 uses same width) */
    __m512i vblend = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Store to volatile */
    _mm512_store_si512((void*)result, vblend);
    
    /* Reduction */
    uint16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    return sum;
}

#endif /* __AVX512BW__ */

int main(int argc, char **argv) {
    uint64_t checksum = 0;
    
    printf("Testing AVX-512 blend patterns...\n");
    
#ifdef __AVX512F__
    printf("AVX512F supported\n");
    
    /* Call all AVX512F test functions */
    float fsum = test_v16sf_blend(argc);
    checksum += *(uint32_t*)&fsum;
    
    double dsum = test_v8df_blend(argc);
    checksum += *(uint64_t*)&dsum;
    
    int32_t isum = test_v16si_blend(argc);
    checksum += (uint32_t)isum;
    
    int64_t lsum = test_v8di_blend(argc);
    checksum += (uint64_t)lsum;
    
#ifdef __AVX512BW__
    printf("AVX512BW supported\n");
    
    /* Call all AVX512BW test functions */
    int8_t csum = test_v64qi_blend(argc);
    checksum += (uint8_t)csum;
    
    int16_t ssum = test_v32hi_blend(argc);
    checksum += (uint16_t)ssum;
    
    __fp16 hsum = test_v32hf_blend(argc);
    checksum += *(uint16_t*)&hsum;
    
    uint16_t bfsum = test_v32bf_blend(argc);
    checksum += bfsum;
#endif /* __AVX512BW__ */
    
    printf("Final checksum: 0x%016lx\n", checksum);
    return (int)(checksum & 0x7FFFFFFF);
    
#else
    printf("AVX-512 not supported on this platform\n");
    return 0;
#endif
}
