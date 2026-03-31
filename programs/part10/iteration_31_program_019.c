/* test_avx512_blend.c - Comprehensive test for AVX-512 blend intrinsics
 * Targeting i386-expand.cc lines 4303-4326
 * Compile with: gcc -O3 -march=skylake-avx512 -mavx512f -mavx512bw test_avx512_blend.c -o test_avx512_blend
 */

#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Alignment and volatile storage to prevent optimization */
#define ALIGN_64 __attribute__((aligned(64)))
#define FORCE_USE(x) __asm__ volatile("" : : "r"(x) : "memory")

/* Global volatile to prevent constant propagation */
volatile int g_loop_count = 100;

#ifdef __AVX512F__

/* V16SF - 16 single-precision floats */
static float test_v16sf_blend(int iterations) {
    ALIGN_64 float src1[16] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f,
                               9.0f, 10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f};
    ALIGN_64 float src2[16] = {16.0f, 15.0f, 14.0f, 13.0f, 12.0f, 11.0f, 10.0f, 9.0f,
                               8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f};
    ALIGN_64 volatile float result[16];
    
    __m512 v1 = _mm512_load_ps(src1);
    __m512 v2 = _mm512_load_ps(src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmp_ps_mask(v1, v2, _CMP_GT_OQ);
    
    /* Blend based on mask */
    __m512 blended = _mm512_mask_blend_ps(mask, v1, v2);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_ps((float*)result, blended);
    
    /* Use result in computation */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Additional blend: vector with broadcast scalar */
    __m512 scalar = _mm512_set1_ps(42.0f);
    __m512 blended2 = _mm512_mask_blend_ps(0xAAAA, v1, scalar);
    _mm512_store_ps((float*)result, blended2);
    
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    FORCE_USE(sum);
    return sum;
}

/* V8DF - 8 double-precision floats */
static double test_v8df_blend(int iterations) {
    ALIGN_64 double src1[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    ALIGN_64 double src2[8] = {8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
    ALIGN_64 volatile double result[8];
    
    __m512d v1 = _mm512_load_pd(src1);
    __m512d v2 = _mm512_load_pd(src2);
    
    /* Create mask */
    __mmask8 mask = _mm512_cmp_pd_mask(v1, v2, _CMP_LT_OQ);
    
    /* Blend */
    __m512d blended = _mm512_mask_blend_pd(mask, v1, v2);
    _mm512_store_pd((double*)result, blended);
    
    /* Use in loop to prevent optimization */
    double sum = 0.0;
    for (int i = 0; i < iterations % 8; i++) {
        sum += result[i];
    }
    
    /* Blend with arithmetic result */
    __m512d mul = _mm512_mul_pd(v1, v2);
    __m512d blended2 = _mm512_mask_blend_pd(0xF0, v1, mul);
    _mm512_store_pd((double*)result, blended2);
    
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    FORCE_USE(sum);
    return sum;
}

/* V16SI - 16 32-bit integers */
static int32_t test_v16si_blend(int iterations) {
    ALIGN_64 int32_t src1[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    ALIGN_64 int32_t src2[16] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    ALIGN_64 volatile int32_t result[16];
    
    __m512i v1 = _mm512_load_epi32(src1);
    __m512i v2 = _mm512_load_epi32(src2);
    
    /* Create mask using comparison */
    __mmask16 mask = _mm512_cmp_epi32_mask(v1, v2, _MM_CMPINT_GT);
    
    /* Blend */
    __m512i blended = _mm512_mask_blend_epi32(mask, v1, v2);
    _mm512_store_epi32((void*)result, blended);
    
    int32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Blend with broadcast scalar */
    __m512i scalar = _mm512_set1_epi32(999);
    __m512i blended2 = _mm512_mask_blend_epi32(0xCCCC, v1, scalar);
    _mm512_store_epi32((void*)result, blended2);
    
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    FORCE_USE(sum);
    return sum;
}

/* V8DI - 8 64-bit integers */
static int64_t test_v8di_blend(int iterations) {
    ALIGN_64 int64_t src1[8] = {100, 200, 300, 400, 500, 600, 700, 800};
    ALIGN_64 int64_t src2[8] = {800, 700, 600, 500, 400, 300, 200, 100};
    ALIGN_64 volatile int64_t result[8];
    
    __m512i v1 = _mm512_load_epi64(src1);
    __m512i v2 = _mm512_load_epi64(src2);
    
    /* Create mask */
    __mmask8 mask = _mm512_cmp_epi64_mask(v1, v2, _MM_CMPINT_LT);
    
    /* Blend */
    __m512i blended = _mm512_mask_blend_epi64(mask, v1, v2);
    _mm512_store_epi64((void*)result, blended);
    
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Blend with arithmetic operation */
    __m512i add = _mm512_add_epi64(v1, v2);
    __m512i blended2 = _mm512_mask_blend_epi64(0xAA, v1, add);
    _mm512_store_epi64((void*)result, blended2);
    
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    FORCE_USE(sum);
    return sum;
}

#endif /* __AVX512F__ */

#ifdef __AVX512BW__

/* V64QI - 64 8-bit integers */
static int8_t test_v64qi_blend(int iterations) {
    ALIGN_64 int8_t src1[64];
    ALIGN_64 int8_t src2[64];
    ALIGN_64 volatile int8_t result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        src1[i] = i;
        src2[i] = 63 - i;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask from array */
    ALIGN_64 uint64_t mask_data[1] = {0xF0F0F0F0F0F0F0F0UL};
    __mmask64 mask = _mm512_loadu_si512(mask_data)[0];
    
    /* Blend */
    __m512i blended = _mm512_mask_blend_epi8(mask, v1, v2);
    _mm512_store_si512((void*)result, blended);
    
    int8_t sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Blend in loop with varying mask */
    for (int iter = 0; iter < iterations % 4; iter++) {
        __m512i blended2 = _mm512_mask_blend_epi8(mask >> iter, v1, v2);
        _mm512_store_si512((void*)result, blended2);
        
        for (int i = 0; i < 64; i += 8) {
            sum += result[i];
        }
    }
    
    FORCE_USE(sum);
    return sum;
}

/* V32HI - 32 16-bit integers */
static int16_t test_v32hi_blend(int iterations) {
    ALIGN_64 int16_t src1[32];
    ALIGN_64 int16_t src2[32];
    ALIGN_64 volatile int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        src1[i] = i * 10;
        src2[i] = (31 - i) * 10;
    }
    
    __m512i v1 = _mm512_load_si512(src1);
    __m512i v2 = _mm512_load_si512(src2);
    
    /* Create mask using comparison */
    __mmask32 mask = _mm512_cmp_epi16_mask(v1, v2, _MM_CMPINT_GT);
    
    /* Blend */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    _mm512_store_si512((void*)result, blended);
    
    int16_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    /* Blend with broadcast scalar */
    __m512i scalar = _mm512_set1_epi16(255);
    __m512i blended2 = _mm512_mask_blend_epi16(0xAAAAAAAA, v1, scalar);
    _mm512_store_si512((void*)result, blended2);
    
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    FORCE_USE(sum);
    return sum;
}

/* V32HF - 32 half-precision floats */
static __m512h test_v32hf_blend(int iterations) {
    ALIGN_64 uint16_t src1_data[32];
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 volatile uint16_t result_data[32];
    
    /* Initialize half-precision patterns */
    for (int i = 0; i < 32; i++) {
        /* Simple half-precision pattern: 1.0, 2.0, 3.0, ... */
        src1_data[i] = (i + 1) << 10;  /* Approximate representation */
        src2_data[i] = (32 - i) << 10;
    }
    
    __m512h v1 = _mm512_load_ph(src1_data);
    __m512h v2 = _mm512_load_ph(src2_data);
    
    /* Create mask */
    __mmask32 mask = _mm512_cmp_ph_mask(v1, v2, _CMP_GT_OQ);
    
    /* Blend */
    __m512h blended = _mm512_mask_blend_ph(mask, v1, v2);
    _mm512_store_ph((void*)result_data, blended);
    
    /* Use result */
    __m512h sum_vec = blended;
    FORCE_USE(sum_vec);
    
    /* Additional blend with arithmetic */
    __m512h mul = _mm512_mul_ph(v1, v2);
    __m512h blended2 = _mm512_mask_blend_ph(0x55555555, v1, mul);
    _mm512_store_ph((void*)result_data, blended2);
    
    return blended2;
}

/* V32BF - 32 bfloat16 floats */
static __m512bh test_v32bf_blend(int iterations) {
    ALIGN_64 uint16_t src1_data[32];
    ALIGN_64 uint16_t src2_data[32];
    ALIGN_64 volatile uint16_t result_data[32];
    
    /* Initialize bfloat16 patterns */
    for (int i = 0; i < 32; i++) {
        /* bfloat16 pattern */
        src1_data[i] = (i + 1) << 8;  /* Upper 8 bits for bfloat16 */
        src2_data[i] = (32 - i) << 8;
    }
    
    /* Load as epi16 for blending since there's no direct bfloat16 blend intrinsic */
    __m512i v1 = _mm512_load_si512(src1_data);
    __m512i v2 = _mm512_load_si512(src2_data);
    
    /* Create mask */
    __mmask32 mask = 0xAAAAAAAA;  /* Alternating pattern */
    
    /* Blend using epi16 intrinsic (appropriate for bfloat16) */
    __m512i blended = _mm512_mask_blend_epi16(mask, v1, v2);
    _mm512_store_si512((void*)result_data, blended);
    
    /* Cast to bfloat16 vector type */
    __m512bh result = _mm512_castsi512_ph(blended);
    FORCE_USE(result);
    
    /* Additional blend with different mask */
    __m512i scalar = _mm512_set1_epi16(0x3F80);  /* bfloat16 1.0 */
    __m512i blended2 = _mm512_mask_blend_epi16(0xCCCCCCCC, v1, scalar);
    _mm512_store_si512((void*)result_data, blended2);
    
    return _mm512_castsi512_ph(blended2);
}

#endif /* __AVX512BW__ */

/* Main driver function */
int main(int argc, char *argv[]) {
    int iterations = g_loop_count;
    if (argc > 1) {
        iterations = atoi(argv[1]) % 100 + 1;
    }
    
    uint64_t hash = 0;
    
#ifdef __AVX512F__
    printf("Testing AVX-512F blend patterns...\n");
    
    /* V16SF */
    float f_result = test_v16sf_blend(iterations);
    hash ^= *(uint32_t*)&f_result;
    
    /* V8DF */
    double d_result = test_v8df_blend(iterations);
    hash ^= *(uint64_t*)&d_result;
    
    /* V16SI */
    int32_t i32_result = test_v16si_blend(iterations);
    hash ^= (uint64_t)i32_result;
    
    /* V8DI */
    int64_t i64_result = test_v8di_blend(iterations);
    hash ^= (uint64_t)i64_result;
    
    printf("  AVX-512F tests completed\n");
#endif

#ifdef __AVX512BW__
    printf("Testing AVX-512BW blend patterns...\n");
    
    /* V64QI */
    int8_t i8_result = test_v64qi_blend(iterations);
    hash ^= (uint64_t)i8_result;
    
    /* V32HI */
    int16_t i16_result = test_v32hi_blend(iterations);
    hash ^= (uint64_t)i16_result;
    
    /* V32HF */
    __m512h hf_result = test_v32hf_blend(iterations);
    hash ^= _mm512_extract_epi64(_mm512_castph_si512(hf_result), 0);
    
    /* V32BF */
    __m512bh bf_result = test_v32bf_blend(iterations);
    hash ^= _mm512_extract_epi64(_mm512_castph_si512(bf_result), 7);
    
    printf("  AVX-512BW tests completed\n");
#endif

#if !defined(__AVX512F__) && !defined(__AVX512BW__)
    printf("AVX-512 not supported on this platform\n");
    return 1;
#endif

    printf("Final hash: 0x%016lx\n", hash);
    printf("All blend tests completed successfully\n");
    
    return 0;
}
