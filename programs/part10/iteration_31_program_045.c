/* test_avx512_blend.c - Coverage for i386-expand.cc blend patterns */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#ifdef __AVX512F__
/* V16SF - 16 single-precision floats */
static float test_v16sf_blend(int iterations) {
    __attribute__((aligned(64))) float a[16] = {
        1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f
    };
    __attribute__((aligned(64))) float b[16] = {
        16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
        8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f
    };
    __attribute__((aligned(64))) volatile float result[16];
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _CMP_GT_OQ);
    
    /* Blend based on mask */
    __m512 vblend = _mm512_mask_blend_ps(mask, va, vb);
    
    /* Store with volatile to prevent optimization */
    _mm512_store_ps((void*)result, vblend);
    
    /* Use result in computation */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Artificial dependency to prevent dead code elimination */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V8DF - 8 double-precision floats */
static double test_v8df_blend(int iterations) {
    __attribute__((aligned(64))) double a[8] = {
        1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0
    };
    __attribute__((aligned(64))) double b[8] = {
        8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0
    };
    __attribute__((aligned(64))) volatile double result[8];
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Create mask: blend where a > b */
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
    
    /* Blend with broadcasted scalar */
    __m512d vscalar = _mm512_set1_pd(3.14159);
    __m512d vblend = _mm512_mask_blend_pd(mask, va, vscalar);
    
    _mm512_store_pd((void*)result, vblend);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* V16SI - 16 32-bit integers */
static int32_t test_v16si_blend(int iterations) {
    __attribute__((aligned(64))) int32_t a[16] = {
        1, 2, 3, 4, 5, 6, 7, 8,
        9, 10, 11, 12, 13, 14, 15, 16
    };
    __attribute__((aligned(64))) int32_t b[16] = {
        16, 15, 14, 13, 12, 11, 10, 9,
        8, 7, 6, 5, 4, 3, 2, 1
    };
    __attribute__((aligned(64))) volatile int32_t result[16];
    
    __m512i va = _mm512_load_epi32(a);
    __m512i vb = _mm512_load_epi32(b);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_GT);
    
    /* Blend with arithmetic result */
    __m512i vadd = _mm512_add_epi32(va, vb);
    __m512i vblend = _mm512_mask_blend_epi32(mask, va, vadd);
    
    _mm512_store_epi32((void*)result, vblend);
    
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* V8DI - 8 64-bit integers */
static int64_t test_v8di_blend(int iterations) {
    __attribute__((aligned(64))) int64_t a[8] = {
        100, 200, 300, 400, 500, 600, 700, 800
    };
    __attribute__((aligned(64))) int64_t b[8] = {
        800, 700, 600, 500, 400, 300, 200, 100
    };
    __attribute__((aligned(64))) volatile int64_t result[8];
    
    __m512i va = _mm512_load_epi64(a);
    __m512i vb = _mm512_load_epi64(b);
    
    /* Create mask: blend where a < b */
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_LT);
    
    /* Blend two loaded vectors */
    __m512i vblend = _mm512_mask_blend_epi64(mask, va, vb);
    
    _mm512_store_epi64((void*)result, vblend);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif /* __AVX512F__ */

#ifdef __AVX512BW__
/* V64QI - 64 8-bit integers */
static int8_t test_v64qi_blend(int iterations) {
    __attribute__((aligned(64))) int8_t a[64];
    __attribute__((aligned(64))) int8_t b[64];
    __attribute__((aligned(64))) volatile int8_t result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 63 - i;
    }
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask: blend where (i % 2) == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 2) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend using mask */
    __m512i vblend = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((void*)result, vblend);
    
    int8_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* V32HI - 32 16-bit integers */
static int16_t test_v32hi_blend(int iterations) {
    __attribute__((aligned(64))) int16_t a[32];
    __attribute__((aligned(64))) int16_t b[32];
    __attribute__((aligned(64))) volatile int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 10;
        b[i] = (31 - i) * 10;
    }
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_GT);
    
    /* Blend with broadcasted scalar */
    __m512i vscalar = _mm512_set1_epi16(999);
    __m512i vblend = _mm512_mask_blend_epi16(mask, va, vscalar);
    
    _mm512_store_si512((void*)result, vblend);
    
    int16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* V32HF - 32 half-precision floats */
static __fp16 test_v32hf_blend(int iterations) {
    __attribute__((aligned(64))) __fp16 a[32];
    __attribute__((aligned(64))) __fp16 b[32];
    __attribute__((aligned(64))) volatile __fp16 result[32];
    
    /* Initialize with pattern */
    for (int i = 0; i < 32; i++) {
        a[i] = (__fp16)(i * 0.5f);
        b[i] = (__fp16)((31 - i) * 0.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Create mask: blend where a > b */
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    /* Blend using mask */
    __m512h vblend = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph((void*)result, vblend);
    
    __fp16 sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}

/* V32BF - 32 bfloat16 floats (use integer blend) */
static uint16_t test_v32bf_blend(int iterations) {
    __attribute__((aligned(64))) uint16_t a[32];  /* bfloat16 as uint16_t */
    __attribute__((aligned(64))) uint16_t b[32];
    __attribute__((aligned(64))) volatile uint16_t result[32];
    
    /* Initialize with bfloat16 pattern */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 pattern: sign=0, exponent=127, fraction=i */
        a[i] = (uint16_t)((0x7F << 7) | (i & 0x7F));
        b[i] = (uint16_t)((0x7F << 7) | ((31 - i) & 0x7F));
    }
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask: blend every other element */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i % 3) == 0) {
            mask |= (1U << i);
        }
    }
    
    /* Blend bfloat16 using 16-bit integer blend */
    __m512i vblend = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((void*)result, vblend);
    
    uint16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    __asm__ volatile("" : : "r"(sum) : "memory");
    return sum;
}
#endif /* __AVX512BW__ */

/* Main driver with checksum computation */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? argc : 3;
    uint64_t checksum = 0;
    
#ifdef __AVX512F__
    printf("AVX512F supported, testing V16SF/V8DF/V16SI/V8DI blends...\n");
    
    /* Test in loop to prevent optimization */
    for (int i = 0; i < iterations; i++) {
        float fsum = test_v16sf_blend(iterations);
        checksum += (uint64_t)(fsum * 1000);
        
        double dsum = test_v8df_blend(iterations);
        checksum += (uint64_t)(dsum * 1000);
        
        int32_t isum = test_v16si_blend(iterations);
        checksum += (uint64_t)isum;
        
        int64_t lsum = test_v8di_blend(iterations);
        checksum += (uint64_t)lsum;
    }
#else
    printf("AVX512F not supported, skipping tests...\n");
#endif

#ifdef __AVX512BW__
    printf("AVX512BW supported, testing V64QI/V32HI/V32HF/V32BF blends...\n");
    
    for (int i = 0; i < iterations; i++) {
        int8_t bsum = test_v64qi_blend(iterations);
        checksum += (uint64_t)bsum;
        
        int16_t ssum = test_v32hi_blend(iterations);
        checksum += (uint64_t)ssum;
        
        __fp16 hsum = test_v32hf_blend(iterations);
        checksum += (uint64_t)(hsum * 1000);
        
        uint16_t bfsum = test_v32bf_blend(iterations);
        checksum += (uint64_t)bfsum;
    }
#else
    printf("AVX512BW not supported, skipping tests...\n");
#endif

    printf("Final checksum: %lu\n", (unsigned long)checksum);
    
    /* Use checksum to affect return value */
    return (checksum & 0xFF);
}
