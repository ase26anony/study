/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targets uncovered lines 4303-4326 in i386-expand.cc
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
    
    /* Blend based on mask - should trigger gen_avx512f_blendmv16sf */
    __m512 blended = _mm512_mask_blend_ps(mask, va, vb);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_ps((void*)result, blended);
    
    /* Use result in computation */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Additional blend with arithmetic operation */
    __m512 vadd = _mm512_add_ps(va, vb);
    __m512 blended2 = _mm512_mask_blend_ps(mask, va, vadd);
    _mm512_store_ps((void*)result, blended2);
    
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* V8DF - 8 double-precision floats */
static double test_v8df_blend(int iterations) {
    __attribute__((aligned(64))) double a[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    __attribute__((aligned(64))) double b[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    __attribute__((aligned(64))) volatile double result[8];
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Create mask - should trigger gen_avx512f_blendmv8df */
    __mmask8 mask = _mm512_cmp_pd_mask(va, vb, _CMP_GT_OQ);
    
    /* Blend with broadcasted scalar */
    __m512d vscalar = _mm512_set1_pd(10.0);
    __m512d blended = _mm512_mask_blend_pd(mask, va, vscalar);
    
    _mm512_store_pd((void*)result, blended);
    
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Blend inside loop with varying mask */
    for (int i = 0; i < iterations; i++) {
        __mmask8 loop_mask = mask ^ (i & 0xFF);
        __m512d blended2 = _mm512_mask_blend_pd(loop_mask, va, vb);
        _mm512_store_pd((void*)result, blended2);
        sum += result[i % 8];
    }
    
    return sum;
}

/* V16SI - 16 32-bit integers */
static int32_t test_v16si_blend(int iterations) {
    __attribute__((aligned(64))) int32_t a[16] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
    };
    __attribute__((aligned(64))) int32_t b[16] = {
        16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
    };
    __attribute__((aligned(64))) volatile int32_t result[16];
    
    __m512i va = _mm512_load_epi32(a);
    __m512i vb = _mm512_load_epi32(b);
    
    /* Create mask - should trigger gen_avx512f_blendmv16si */
    __mmask16 mask = _mm512_cmp_epi32_mask(va, vb, _MM_CMPINT_GT);
    
    /* Blend vectors */
    __m512i blended = _mm512_mask_blend_epi32(mask, va, vb);
    
    _mm512_store_epi32((void*)result, blended);
    
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Artificial dependency to prevent optimization */
    __asm__ volatile("" : : "r"(sum) : "memory");
    
    return sum;
}

/* V8DI - 8 64-bit integers */
static int64_t test_v8di_blend(int iterations) {
    __attribute__((aligned(64))) int64_t a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    __attribute__((aligned(64))) int64_t b[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    __attribute__((aligned(64))) volatile int64_t result[8];
    
    __m512i va = _mm512_load_epi64(a);
    __m512i vb = _mm512_load_epi64(b);
    
    /* Create mask - should trigger gen_avx512f_blendmv8di */
    __mmask8 mask = _mm512_cmp_epi64_mask(va, vb, _MM_CMPINT_GT);
    
    /* Blend with arithmetic result */
    __m512i vadd = _mm512_add_epi64(va, vb);
    __m512i blended = _mm512_mask_blend_epi64(mask, va, vadd);
    
    _mm512_store_epi64((void*)result, blended);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
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
    
    /* Create mask using comparison - should trigger gen_avx512bw_blendmv64qi */
    __mmask64 mask = _mm512_cmp_epi8_mask(va, vb, _MM_CMPINT_GT);
    
    /* Blend vectors */
    __m512i blended = _mm512_mask_blend_epi8(mask, va, vb);
    
    _mm512_store_si512((void*)result, blended);
    
    int8_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Additional blend in loop */
    for (int i = 0; i < iterations; i++) {
        __mmask64 loop_mask = mask ^ (i & 0xFF);
        __m512i blended2 = _mm512_mask_blend_epi8(loop_mask, va, vb);
        _mm512_store_si512((void*)result, blended2);
        sum += result[i % 64];
    }
    
    return sum;
}

/* V32HI - 32 16-bit integers */
static int16_t test_v32hi_blend(int iterations) {
    __attribute__((aligned(64))) int16_t a[32];
    __attribute__((aligned(64))) int16_t b[32];
    __attribute__((aligned(64))) volatile int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = i;
        b[i] = 31 - i;
    }
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask - should trigger gen_avx512bw_blendmv32hi */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_GT);
    
    /* Blend with broadcasted scalar */
    __m512i vscalar = _mm512_set1_epi16(100);
    __m512i blended = _mm512_mask_blend_epi16(mask, va, vscalar);
    
    _mm512_store_si512((void*)result, blended);
    
    int16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    return sum;
}

/* V32HF - 32 half-precision floats */
static __m512h test_v32hf_blend(int iterations) {
    __attribute__((aligned(64))) _Float16 a[32];
    __attribute__((aligned(64))) _Float16 b[32];
    __attribute__((aligned(64))) volatile _Float16 result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (_Float16)(i * 0.5f);
        b[i] = (_Float16)((31 - i) * 0.5f);
    }
    
    __m512h va = _mm512_load_ph(a);
    __m512h vb = _mm512_load_ph(b);
    
    /* Create mask - should trigger gen_avx512bw_blendmv32hf */
    __mmask32 mask = _mm512_cmp_ph_mask(va, vb, _CMP_GT_OQ);
    
    /* Blend vectors */
    __m512h blended = _mm512_mask_blend_ph(mask, va, vb);
    
    _mm512_store_ph((void*)result, blended);
    
    /* Use result to prevent optimization */
    __asm__ volatile("" : : "r"(result[0]) : "memory");
    
    return blended;
}

/* V32BF - 32 bfloat16 floats */
static __m512bh test_v32bf_blend(int iterations) {
    /* For bfloat16, we use the integer blend since there's no direct bfloat16 blend intrinsic */
    __attribute__((aligned(64))) uint16_t a[32];  /* bfloat16 as uint16_t */
    __attribute__((aligned(64))) uint16_t b[32];
    __attribute__((aligned(64))) volatile uint16_t result[32];
    
    /* Simple bfloat16 pattern */
    for (int i = 0; i < 32; i++) {
        a[i] = i << 8;  /* Simple pattern for bfloat16 */
        b[i] = (31 - i) << 8;
    }
    
    __m512i va = _mm512_load_si512(a);
    __m512i vb = _mm512_load_si512(b);
    
    /* Create mask - should trigger gen_avx512bw_blendmv32bf */
    __mmask32 mask = _mm512_cmp_epi16_mask(va, vb, _MM_CMPINT_GT);
    
    /* Blend using epi16 intrinsic on bfloat16 representation */
    __m512i blended = _mm512_mask_blend_epi16(mask, va, vb);
    
    _mm512_store_si512((void*)result, blended);
    
    /* Use result */
    __asm__ volatile("" : : "r"(result[0]) : "memory");
    
    return _mm512_castsi512_bh(blended);
}
#endif /* __AVX512BW__ */

int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    if (iterations < 1) iterations = 1;
    if (iterations > 100) iterations = 100;
    
    volatile int dummy = iterations;  /* Prevent constant propagation */
    iterations = dummy;
    
    uint64_t checksum = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend patterns...\n");
    
    /* Test V16SF */
    float v16sf_result = test_v16sf_blend(iterations);
    checksum += (uint64_t)(v16sf_result * 1000);
    printf("  V16SF blend: %f\n", v16sf_result);
    
    /* Test V8DF */
    double v8df_result = test_v8df_blend(iterations);
    checksum += (uint64_t)(v8df_result * 1000);
    printf("  V8DF blend: %f\n", v8df_result);
    
    /* Test V16SI */
    int32_t v16si_result = test_v16si_blend(iterations);
    checksum += (uint64_t)v16si_result;
    printf("  V16SI blend: %d\n", v16si_result);
    
    /* Test V8DI */
    int64_t v8di_result = test_v8di_blend(iterations);
    checksum += (uint64_t)v8di_result;
    printf("  V8DI blend: %ld\n", v8di_result);
#else
    printf("AVX-512F not supported on this platform\n");
#endif

#ifdef __AVX512BW__
    printf("\nTesting AVX-512BW blend patterns...\n");
    
    /* Test V64QI */
    int8_t v64qi_result = test_v64qi_blend(iterations);
    checksum += (uint64_t)v64qi_result;
    printf("  V64QI blend: %d\n", v64qi_result);
    
    /* Test V32HI */
    int16_t v32hi_result = test_v32hi_blend(iterations);
    checksum += (uint64_t)v32hi_result;
    printf("  V32HI blend: %d\n", v32hi_result);
    
    /* Test V32HF */
    __m512h v32hf_result = test_v32hf_blend(iterations);
    /* Extract first element for checksum */
    _Float16 v32hf_first;
    _mm512_store_ph(&v32hf_first, v32hf_result);
    checksum += (uint64_t)(*((uint16_t*)&v32hf_first));
    printf("  V32HF blend executed\n");
    
    /* Test V32BF */
    __m512bh v32bf_result = test_v32bf_blend(iterations);
    /* Extract first element for checksum */
    uint16_t v32bf_first;
    _mm512_store_si512(&v32bf_first, _mm512_castbh_si512(v32bf_result));
    checksum += (uint64_t)v32bf_first;
    printf("  V32BF blend executed\n");
#else
    printf("AVX-512BW not supported on this platform\n");
#endif

    printf("\nFinal checksum: %lu\n", checksum);
    
    /* Use checksum to affect return value */
    return (int)(checksum % 256);
}
