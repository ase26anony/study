/* test_avx512_blend.c - AVX-512 blend intrinsics test for GCC RTL coverage */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Global volatile to prevent optimization */
volatile int g_volatile = 0;

#ifdef __AVX512F__

/* V16SF - 16 single-precision floats */
float test_v16sf_blend(void) {
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
    __m512 vcmp = _mm512_set1_ps(8.5f);
    __mmask16 mask = _mm512_cmp_ps_mask(va, vcmp, _CMP_GT_OQ);
    
    /* Blend based on mask */
    __m512 vblend = _mm512_mask_blend_ps(mask, va, vb);
    
    /* Store with volatile to prevent optimization */
    _mm512_store_ps((void*)result, vblend);
    
    /* Use result in computation */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V8DF - 8 double-precision floats */
double test_v8df_blend(void) {
    __attribute__((aligned(64))) double a[8] = {
        1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0
    };
    __attribute__((aligned(64))) double b[8] = {
        8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0
    };
    __attribute__((aligned(64))) volatile double result[8];
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Create mask: blend where a > 4.5 */
    __m512d vcmp = _mm512_set1_pd(4.5);
    __mmask8 mask = _mm512_cmp_pd_mask(va, vcmp, _CMP_GT_OQ);
    
    /* Blend with mask */
    __m512d vblend = _mm512_mask_blend_pd(mask, va, vb);
    
    /* Store volatile */
    _mm512_store_pd((void*)result, vblend);
    
    /* Compute sum */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Blend with broadcast scalar */
    __m512d vscalar = _mm512_set1_pd(10.0);
    __mmask8 mask2 = 0xAA; /* 10101010 pattern */
    __m512d vblend2 = _mm512_mask_blend_pd(mask2, vblend, vscalar);
    
    /* Use in computation */
    __m512d vadd = _mm512_add_pd(vblend2, _mm512_set1_pd(1.0));
    __m512d vfinal = _mm512_mask_blend_pd(mask, vadd, vblend);
    
    /* Force side effect */
    __attribute__((aligned(64))) double temp[8];
    _mm512_store_pd(temp, vfinal);
    
    return sum + temp[0];
}

/* V16SI - 16 32-bit integers */
int32_t test_v16si_blend(void) {
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
    __m512i vcmp = _mm512_set1_epi32(8);
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vcmp, _MM_CMPINT_GT);
    
    /* Blend with mask */
    __m512i vblend = _mm512_mask_blend_epi32(mask, va, vb);
    
    /* Store volatile */
    _mm512_store_epi32((void*)result, vblend);
    
    /* Use in loop with argc dependency */
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i] * (i + 1);
    }
    
    /* Blend with arithmetic result */
    __m512i vadd = _mm512_add_epi32(va, _mm512_set1_epi32(100));
    __mmask16 mask2 = 0x5555; /* 01010101... pattern */
    __m512i vblend2 = _mm512_mask_blend_epi32(mask2, vblend, vadd);
    
    /* Force computation */
    __attribute__((aligned(64))) int32_t temp[16];
    _mm512_store_epi32(temp, vblend2);
    
    return sum + temp[0];
}

/* V8DI - 8 64-bit integers */
int64_t test_v8di_blend(void) {
    __attribute__((aligned(64))) int64_t a[8] = {
        100, 200, 300, 400, 500, 600, 700, 800
    };
    __attribute__((aligned(64))) int64_t b[8] = {
        800, 700, 600, 500, 400, 300, 200, 100
    };
    __attribute__((aligned(64))) volatile int64_t result[8];
    
    __m512i va = _mm512_load_epi64(a);
    __m512i vb = _mm512_load_epi64(b);
    
    /* Create mask */
    __m512i vcmp = _mm512_set1_epi64(450);
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vcmp, _MM_CMPINT_GT);
    
    /* Blend with mask */
    __m512i vblend = _mm512_mask_blend_epi64(mask, va, vb);
    
    /* Store volatile */
    _mm512_store_epi64((void*)result, vblend);
    
    /* Compute sum */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Artificial dependency */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

#endif /* __AVX512F__ */

#ifdef __AVX512BW__

/* V64QI - 64 8-bit integers */
int32_t test_v64qi_blend(void) {
    __attribute__((aligned(64))) uint8_t a[64];
    __attribute__((aligned(64))) uint8_t b[64];
    
    /* Initialize with patterns */
    for (int i = 0; i < 64; i++) {
        a[i] = i;
        b[i] = 63 - i;
    }
    
    __attribute__((aligned(64))) volatile uint8_t result[64];
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask: blend where (i % 3) == 0 */
    __mmask64 mask = 0;
    for (int i = 0; i < 64; i++) {
        if ((i % 3) == 0) {
            mask |= (1ULL << i);
        }
    }
    
    /* Blend with mask */
    __m512i vblend = _mm512_mask_blend_epi8(mask, va, vb);
    
    /* Store volatile */
    _mm512_store_si512((void*)result, vblend);
    
    /* Compute checksum */
    int32_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i] * (i + 1);
    }
    
    return sum;
}

/* V32HI - 32 16-bit integers */
int32_t test_v32hi_blend(void) {
    __attribute__((aligned(64))) int16_t a[32];
    __attribute__((aligned(64))) int16_t b[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = i * 10;
        b[i] = (31 - i) * 10;
    }
    
    __attribute__((aligned(64))) volatile int16_t result[32];
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask using comparison */
    __m512i vcmp = _mm512_set1_epi16(150);
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vcmp, _MM_CMPINT_GT);
    
    /* Blend with mask */
    __m512i vblend = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Store volatile */
    _mm512_store_si512((void*)result, vblend);
    
    /* Use in computation */
    int32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    /* Blend with arithmetic operation */
    __m512i vmul = _mm512_mullo_epi16(va, _mm512_set1_epi16(2));
    __mmask32 mask2 = 0xAAAAAAAA; /* 1010... pattern */
    __m512i vblend2 = _mm512_mask_blend_epi16(mask2, vblend, vmul);
    
    /* Force side effect */
    __attribute__((aligned(64))) int16_t temp[32];
    _mm512_store_si512(temp, vblend2);
    
    return sum + temp[0];
}

/* V32HF - 32 half-precision floats */
#ifdef __AVX512FP16__
float test_v32hf_blend(void) {
    __attribute__((aligned(64))) _Float16 a[32];
    __attribute__((aligned(64))) _Float16 b[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)((31 - i) * 0.5f);
    }
    
    __attribute__((aligned(64))) volatile _Float16 result[32];
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Create mask */
    __m512h vcmp = _mm512_set1_ph((_Float16)8.0f);
    __mmask32 mask = _mm512_cmp_ph_mask(va, vcmp, _CMP_GT_OQ);
    
    /* Blend with mask */
    __m512h vblend = _mm512_mask_blend_ph(mask, va, vb);
    
    /* Store volatile */
    _mm512_store_ph((void*)result, vblend);
    
    /* Compute sum */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        sum += (float)result[i];
    }
    
    return sum;
}
#endif /* __AVX512FP16__ */

/* V32BF - 32 bfloat16 floats */
#ifdef __AVX512BF16__
float test_v32bf_blend(void) {
    __attribute__((aligned(64))) __bfloat16 a[32];
    __attribute__((aligned(64))) __bfloat16 b[32];
    
    /* Initialize bfloat16 values */
    for (int i = 0; i < 32; i++) {
        /* Simple pattern: 1.0, 2.0, 3.0, ... */
        uint16_t val = (i + 1) << 8; /* Approximate representation */
        a[i] = (__bfloat16)val;
        b[i] = (__bfloat16)((32 - i) << 8);
    }
    
    __attribute__((aligned(64))) volatile __bfloat16 result[32];
    
    /* Load as integers for blending */
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask: blend every other element */
    __mmask32 mask = 0xAAAAAAAA; /* 1010... pattern */
    
    /* Blend using epi16 intrinsic (bfloat16 uses 16-bit storage) */
    __m512i vblend = _mm512_mask_blend_epi16(mask, va, vb);
    
    /* Store volatile */
    _mm512_store_si512((void*)result, vblend);
    
    /* Compute approximate sum */
    float sum = 0.0f;
    for (int i = 0; i < 32; i++) {
        /* Convert to float for sum */
        uint16_t val = (uint16_t)result[i];
        sum += (float)(val >> 8); /* Rough approximation */
    }
    
    return sum;
}
#endif /* __AVX512BF16__ */

#endif /* __AVX512BW__ */

/* Main driver function */
int main(int argc, char *argv[]) {
    int loop_count = argc > 1 ? atoi(argv[1]) : 1;
    if (loop_count < 1) loop_count = 1;
    
    uint64_t total_hash = 0;
    
#ifdef __AVX512F__
    printf("AVX512F supported, testing V16SF, V8DF, V16SI, V8DI blends...\n");
    
    /* Call each test function multiple times based on loop_count */
    for (int i = 0; i < loop_count; i++) {
        float f1 = test_v16sf_blend();
        total_hash += *(uint32_t*)&f1;
        
        double d1 = test_v8df_blend();
        total_hash += *(uint64_t*)&d1;
        
        int32_t i1 = test_v16si_blend();
        total_hash += i1;
        
        int64_t l1 = test_v8di_blend();
        total_hash += l1;
    }
#else
    printf("AVX512F not supported, skipping tests...\n");
#endif

#ifdef __AVX512BW__
    printf("AVX512BW supported, testing V64QI, V32HI blends...\n");
    
    for (int i = 0; i < loop_count; i++) {
        int32_t qi = test_v64qi_blend();
        total_hash += qi;
        
        int32_t hi = test_v32hi_blend();
        total_hash += hi;
        
#ifdef __AVX512FP16__
        printf("AVX512FP16 supported, testing V32HF blend...\n");
        float hf = test_v32hf_blend();
        total_hash += *(uint32_t*)&hf;
#endif
        
#ifdef __AVX512BF16__
        printf("AVX512BF16 supported, testing V32BF blend...\n");
        float bf = test_v32bf_blend();
        total_hash += *(uint32_t*)&bf;
#endif
    }
#else
    printf("AVX512BW not supported, skipping byte/word tests...\n");
#endif
    
    printf("Total hash: %lu\n", (unsigned long)total_hash);
    
    /* Use result to affect return code */
    return (total_hash & 0xFF) == 0 ? 0 : 1;
}
