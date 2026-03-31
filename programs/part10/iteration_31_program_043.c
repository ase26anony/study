/* test_avx512_blend.c - AVX-512 blend intrinsics coverage test */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <immintrin.h>

/* Alignment for AVX-512 vectors */
#define ALIGN_64 __attribute__((aligned(64)))

/* Feature detection */
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

#ifdef __AVX512VL__
#define HAS_AVX512VL 1
#else
#define HAS_AVX512VL 0
#endif

/* ==================== V64QI (64x int8) ==================== */
#ifdef __AVX512BW__
static int test_v64qi_blend(int argc) {
    ALIGN_64 int8_t a[64];
    ALIGN_64 int8_t b[64];
    ALIGN_64 int8_t result[64];
    volatile ALIGN_64 int8_t volatile_result[64];
    
    /* Initialize with pattern */
    for (int i = 0; i < 64; i++) {
        a[i] = (int8_t)(i * 3);
        b[i] = (int8_t)(i * 5 + 1);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi8(100);
    __mmask64 mask = _mm512_cmp_epi8_mask(va, cmp_val, _MM_CMPINT_LT);
    
    /* Blend based on mask - should trigger gen_avx512bw_blendmv64qi */
    __m512i vresult = _mm512_mask_blend_epi8(mask, va, vb);
    
    /* Store to volatile to prevent optimization */
    _mm512_store_si512((__m512i*)volatile_result, vresult);
    
    /* Also store to regular array */
    _mm512_store_si512((__m512i*)result, vresult);
    
    /* Use result in computation */
    int sum = 0;
    for (int i = 0; i < 64; i++) {
        sum += result[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 1) ? (argc % 8) + 1 : 4;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i temp = _mm512_load_si512((const __m512i*)a);
        __m512i blended = _mm512_mask_blend_epi8(mask ^ 0xAAAAAAAAAAAAAAAA, 
                                                temp, vresult);
        _mm512_store_si512((__m512i*)volatile_result, blended);
        sum += volatile_result[iter % 64];
    }
    
    return sum & 0xFF;
}
#else
static int test_v64qi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HI (32x int16) ==================== */
#ifdef __AVX512BW__
static int test_v32hi_blend(int argc) {
    ALIGN_64 int16_t a[32];
    ALIGN_64 int16_t b[32];
    ALIGN_64 int16_t result[32];
    
    for (int i = 0; i < 32; i++) {
        a[i] = (int16_t)(i * 10);
        b[i] = (int16_t)(i * 20 + 5);
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a);
    __m512i vb = _mm512_load_si512((const __m512i*)b);
    
    /* Create mask - alternating pattern */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if (i % 3 == 0) mask |= (1ULL << i);
    }
    
    /* Blend - should trigger gen_avx512bw_blendmv32hi */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    _mm512_store_si512((__m512i*)result, vresult);
    
    /* Use in reduction */
    int32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    /* Blend with broadcast scalar */
    __m512i broadcast = _mm512_set1_epi16(42);
    __m512i blended2 = _mm512_mask_blend_epi16(mask ^ 0x55555555, vresult, broadcast);
    
    /* Force side effect */
    ALIGN_64 int16_t temp[32];
    _mm512_store_si512((__m512i*)temp, blended2);
    
    for (int i = 0; i < 32; i++) {
        sum += temp[i];
    }
    
    /* argc-dependent computation */
    if (argc > 2) {
        __m512i blended3 = _mm512_mask_blend_epi16(mask, blended2, va);
        __asm__ volatile("" : : "r"(blended3) : "memory");
    }
    
    return (int)(sum & 0xFFFF);
}
#else
static int test_v32hi_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32HF (32x half precision) ==================== */
#ifdef __AVX512BW__
#include <x86intrin.h>
static int test_v32hf_blend(int argc) {
    ALIGN_64 uint16_t a_data[32];  /* Store as uint16_t for half float */
    ALIGN_64 uint16_t b_data[32];
    ALIGN_64 uint16_t result[32];
    
    /* Initialize with simple pattern (half float representation) */
    for (int i = 0; i < 32; i++) {
        /* Simple pattern: 1.0, 2.0, 3.0, ... as half floats */
        a_data[i] = (uint16_t)(0x3C00 + (i % 8));  /* 1.0 + small increment */
        b_data[i] = (uint16_t)(0x4000 + (i % 8));  /* 2.0 + small increment */
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a_data);
    __m512i vb = _mm512_load_si512((const __m512i*)b_data);
    
    /* Create mask using float comparison */
    __m512h va_h = _mm512_castsi512_ph(va);
    __m512h vb_h = _mm512_castsi512_ph(vb);
    
    /* Compare - using integer mask for simplicity */
    __mmask32 mask = 0xAAAAAAAA;  /* Alternating pattern */
    
    /* Blend half precision - should trigger gen_avx512bw_blendmv32hf */
    __m512h vresult_h = _mm512_mask_blend_ph(mask, va_h, vb_h);
    
    /* Convert back to integer for storage */
    __m512i vresult = _mm512_castph_si512(vresult_h);
    _mm512_store_si512((__m512i*)result, vresult);
    
    /* Compute checksum */
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    /* Additional blend with arithmetic result */
    __m512h add_result = _mm512_add_ph(va_h, vb_h);
    __m512h blended2 = _mm512_mask_blend_ph(mask ^ 0x55555555, vresult_h, add_result);
    
    /* Force compiler to keep computation */
    __m512i temp = _mm512_castph_si512(blended2);
    __asm__ volatile("" : : "r"(temp) : "memory");
    
    return (int)(sum & 0xFFFF);
}
#else
static int test_v32hf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V32BF (32x bfloat16) ==================== */
#ifdef __AVX512BW__
static int test_v32bf_blend(int argc) {
    ALIGN_64 uint16_t a_data[32];  /* bfloat16 stored as uint16_t */
    ALIGN_64 uint16_t b_data[32];
    ALIGN_64 uint16_t result[32];
    
    /* Initialize bfloat16 data */
    for (int i = 0; i < 32; i++) {
        /* Simple bfloat16 pattern */
        a_data[i] = (uint16_t)(0x3F80 + (i & 7));  /* ~1.0f in bfloat16 */
        b_data[i] = (uint16_t)(0x4000 + (i & 7));  /* ~2.0f in bfloat16 */
    }
    
    __m512i va = _mm512_load_si512((const __m512i*)a_data);
    __m512i vb = _mm512_load_si512((const __m512i*)b_data);
    
    /* Create mask - checkerboard pattern */
    __mmask32 mask = 0;
    for (int i = 0; i < 32; i++) {
        if ((i / 4) % 2 == 0) mask |= (1ULL << i);
    }
    
    /* 
     * For bfloat16, we use epi16 blend since there's no dedicated BF16 blend intrinsic.
     * This should still trigger gen_avx512bw_blendmv32bf when the compiler
     * recognizes the bfloat16 mode.
     */
    __m512i vresult = _mm512_mask_blend_epi16(mask, va, vb);
    _mm512_store_si512((__m512i*)result, vresult);
    
    /* Compute sum */
    uint32_t sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += result[i];
    }
    
    /* Blend with scalar broadcast */
    __m512i scalar = _mm512_set1_epi16(0x3F80);  /* 1.0 in bfloat16 */
    __m512i blended2 = _mm512_mask_blend_epi16(mask ^ 0x33333333, vresult, scalar);
    
    /* Store to volatile to prevent optimization */
    volatile ALIGN_64 uint16_t volatile_store[32];
    _mm512_store_si512((__m512i*)volatile_store, blended2);
    
    /* Use argc in computation */
    if (argc > 1) {
        for (int i = 0; i < (argc % 16); i++) {
            sum += volatile_store[i];
        }
    }
    
    return (int)(sum & 0xFFFF);
}
#else
static int test_v32bf_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V16SI (16x int32) ==================== */
#ifdef __AVX512F__
static int test_v16si_blend(int argc) {
    ALIGN_64 int32_t a[16];
    ALIGN_64 int32_t b[16];
    ALIGN_64 int32_t result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = i * 100;
        b[i] = i * 200 + 50;
    }
    
    __m512i va = _mm512_load_epi32(a);
    __m512i vb = _mm512_load_epi32(b);
    
    /* Create mask using comparison */
    __m512i cmp_val = _mm512_set1_epi32(800);
    __mmask16 mask = _mm512_cmp_epi32_mask(va, cmp_val, _MM_CMPINT_LT);
    
    /* Blend - should trigger gen_avx512f_blendmv16si */
    __m512i vresult = _mm512_mask_blend_epi32(mask, va, vb);
    _mm512_store_epi32(result, vresult);
    
    /* Reduction */
    int64_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Blend with arithmetic result */
    __m512i add_result = _mm512_add_epi32(va, vb);
    __m512i blended2 = _mm512_mask_blend_epi32(mask ^ 0xAAAA, vresult, add_result);
    
    /* Force side effect */
    ALIGN_64 int32_t temp[16];
    _mm512_store_epi32(temp, blended2);
    
    for (int i = 0; i < 16; i++) {
        sum += temp[i];
    }
    
    /* Loop with argc dependency */
    int loop_count = (argc > 0) ? (argc % 5) + 2 : 3;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512i blended3 = _mm512_mask_blend_epi32(mask | (1 << (iter % 16)), 
                                                  blended2, va);
        __asm__ volatile("" : : "r"(blended3) : "memory");
    }
    
    return (int)(sum & 0x7FFFFFFF);
}
#else
static int test_v16si_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DI (8x int64) ==================== */
#ifdef __AVX512F__
static int64_t test_v8di_blend(int argc) {
    ALIGN_64 int64_t a[8];
    ALIGN_64 int64_t b[8];
    ALIGN_64 int64_t result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = (int64_t)i * 1000LL;
        b[i] = (int64_t)i * 2000LL + 500LL;
    }
    
    __m512i va = _mm512_load_epi64(a);
    __m512i vb = _mm512_load_epi64(b);
    
    /* Create mask */
    __mmask8 mask = 0;
    for (int i = 0; i < 8; i++) {
        if (i % 2 == 0) mask |= (1 << i);
    }
    
    /* Blend - should trigger gen_avx512f_blendmv8di */
    __m512i vresult = _mm512_mask_blend_epi64(mask, va, vb);
    _mm512_store_epi64(result, vresult);
    
    /* Compute sum */
    int64_t sum = 0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Blend with broadcast scalar */
    __m512i scalar = _mm512_set1_epi64(9999);
    __m512i blended2 = _mm512_mask_blend_epi64(mask ^ 0x55, vresult, scalar);
    
    /* Store to volatile */
    volatile ALIGN_64 int64_t volatile_store[8];
    _mm512_store_epi64((void*)volatile_store, blended2);
    
    /* Use argc */
    if (argc > 1) {
        for (int i = 0; i < (argc % 8); i++) {
            sum += volatile_store[i];
        }
    }
    
    return sum;
}
#else
static int64_t test_v8di_blend(int argc) { (void)argc; return 0; }
#endif

/* ==================== V8DF (8x double) ==================== */
#ifdef __AVX512F__
static double test_v8df_blend(int argc) {
    ALIGN_64 double a[8];
    ALIGN_64 double b[8];
    ALIGN_64 double result[8];
    
    for (int i = 0; i < 8; i++) {
        a[i] = (double)i * 1.5;
        b[i] = (double)i * 2.5 + 0.5;
    }
    
    __m512d va = _mm512_load_pd(a);
    __m512d vb = _mm512_load_pd(b);
    
    /* Create mask using comparison */
    __m512d cmp_val = _mm512_set1_pd(6.0);
    __mmask8 mask = _mm512_cmp_pd_mask(va, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv8df */
    __m512d vresult = _mm512_mask_blend_pd(mask, va, vb);
    _mm512_store_pd(result, vresult);
    
    /* Reduction */
    double sum = 0.0;
    for (int i = 0; i < 8; i++) {
        sum += result[i];
    }
    
    /* Blend with arithmetic result */
    __m512d mul_result = _mm512_mul_pd(va, vb);
    __m512d blended2 = _mm512_mask_blend_pd(mask ^ 0xAA, vresult, mul_result);
    
    /* Force computation */
    ALIGN_64 double temp[8];
    _mm512_store_pd(temp, blended2);
    
    for (int i = 0; i < 8; i++) {
        sum += temp[i];
    }
    
    /* Loop with side effects */
    int loop_count = (argc > 0) ? (argc % 4) + 1 : 2;
    for (int iter = 0; iter < loop_count; iter++) {
        __m512d blended3 = _mm512_mask_blend_pd(mask | (1 << (iter % 8)), 
                                               blended2, va);
        __asm__ volatile("" : : "r"(blended3) : "memory");
    }
    
    return sum;
}
#else
static double test_v8df_blend(int argc) { (void)argc; return 0.0; }
#endif

/* ==================== V16SF (16x float) ==================== */
#ifdef __AVX512F__
static float test_v16sf_blend(int argc) {
    ALIGN_64 float a[16];
    ALIGN_64 float b[16];
    ALIGN_64 float result[16];
    
    for (int i = 0; i < 16; i++) {
        a[i] = (float)i * 0.5f;
        b[i] = (float)i * 1.5f + 0.25f;
    }
    
    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);
    
    /* Create mask using comparison */
    __m512 cmp_val = _mm512_set1_ps(4.0f);
    __mmask16 mask = _mm512_cmp_ps_mask(va, cmp_val, _CMP_LT_OQ);
    
    /* Blend - should trigger gen_avx512f_blendmv16sf */
    __m512 vresult = _mm512_mask_blend_ps(mask, va, vb);
    _mm512_store_ps(result, vresult);
    
    /* Reduction */
    float sum = 0.0f;
    for (int i = 0; i < 16; i++) {
        sum += result[i];
    }
    
    /* Blend with multiple sources */
    __m512 add_result = _mm512_add_ps(va, vb);
    __m512 mul_result = _mm512_mul_ps(va, vb);
    
    __m512 blended2 = _mm512_mask_blend_ps(mask ^ 0x5555, vresult, add_result);
    __m512 blended3 = _mm512_mask_blend_ps(mask ^ 0xAAAA, blended2, mul_result);
    
    /* Store to volatile */
    volatile ALIGN_64 float volatile_store[16];
    _mm512_store_ps((void*)volatile_store, blended3);
    
    /* Use argc */
    if (argc > 1) {
        for (int i = 0; i < (argc % 16); i++) {
            sum += volatile_store[i];
        }
    }
    
    return sum;
}
#else
static float test_v16sf_blend(int argc) { (void)argc; return 0.0f; }
#endif

/* ==================== Main Driver ==================== */
int main(int argc, char *argv[]) {
    int total_checksum = 0;
    
    printf("Testing AVX-512 blend intrinsics coverage...\n");
    
#if HAS_AVX512BW
    printf("AVX512BW enabled - testing V64QI, V32HI, V32HF, V32BF\n");
    
    /* Test V64QI */
    int r1 = test_v64qi_blend(argc);
    total_checksum = (total_checksum * 31 + r1) & 0x7FFFFFFF;
    printf("  V64QI blend result: %d\n", r1);
    
    /* Test V32HI */
    int r2 = test_v32hi_blend(argc);
    total_checksum = (total_checksum * 31 + r2) & 0x7FFFFFFF;
    printf("  V32HI blend result: %d\n", r2);
    
    /* Test V32HF */
    int r3 = test_v32hf_blend(argc);
    total_checksum = (total_checksum * 31 + r3) & 0x7FFFFFFF;
    printf("  V32HF blend result: %d\n", r3);
    
    /* Test V32BF */
    int r4 = test_v32bf_blend(argc);
    total_checksum = (total_checksum * 31 + r4) & 0x7FFFFFFF;
    printf("  V32BF blend result: %d\n", r4);
    
#else
    printf("AVX512BW not available - skipping V64QI, V32HI, V32HF, V32BF tests\n");
#endif
    
#if HAS_AVX512F
    printf("AVX512F enabled - testing V16SI, V8DI, V8DF, V16SF\n");
    
    /* Test V16SI */
    int r5 = test_v16si_blend(argc);
    total_checksum = (total_checksum * 31 + r5) & 0x7FFFFFFF;
    printf("  V16SI blend result: %d\n", r5);
    
    /* Test V8DI */
    int64_t r6 = test_v8di_blend(argc);
    total_checksum = (total_checksum * 31 + (int)(r6 & 0x7FFFFFFF)) & 0x7FFFFFFF;
    printf("  V8DI blend result: %ld\n", (long)r6);
    
    /* Test V8DF */
    double r7 = test_v8df_blend(argc);
    total_checksum = (total_checksum * 31 + (int)r7) & 0x7FFFFFFF;
    printf("  V8DF blend result: %f\n", r7);
    
    /* Test V16SF */
    float r8 = test_v16sf_blend(argc);
    total_checksum = (total_checksum * 31 + (int)r8) & 0x7FFFFFFF;
    printf("  V16SF blend result: %f\n", r8);
    
#else
    printf("AVX512F not available - skipping V16SI, V8DI, V8DF, V16SF tests\n");
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Use argv to affect control flow */
    if (argc > 1) {
        printf("Command line arguments provided: %d\n", argc - 1);
    }
    
    return total_checksum != 0 ? 0 : 1;
}
